#include "TimeCorrector.h"

#define MIN_BASEERR LONG_MIN
#define MAX_BASEERR LONG_MAX
#define MIN_TIMESCALE DBL_MIN // DBL_MIN>0
#define MAX_TIMESCALE 1000

static inline int64_t millis(const timeval& tv) {
	return ((int64_t)tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

TimeCorrector::TimeCorrector(size_t sampleCapacity)
{
    _capacity = sampleCapacity;
    reset();
}

// XXX reset is not thread safe
void TimeCorrector::reset()
{
    _remoteTimebase = 0;
    _localTimebase = 0;
    _baseErrLB = MIN_BASEERR;
    _baseErrUB = MAX_BASEERR;
    
    _timeScale = 1.0;
    _timeScaleLB = MIN_TIMESCALE;
    _timeScaleUB = MAX_TIMESCALE;
    
    _timeRealScale = _timeScale;
    _realScaleErrLB = _timeScaleLB-_timeScale;
    _realScaleErrUB = _timeScaleUB-_timeScale;
    
    _samples.clear();
}

void TimeCorrector::addSample(
                              const timeval& localSendTime,
                              const timeval& localRecvTime,
                              const timeval& remoteSendTime,
                              long remoteElapsed)
{
    addSample(millis(localSendTime),
              millis(localRecvTime),
              millis(remoteSendTime),
              remoteElapsed);
}

void TimeCorrector::addSample(
                              int64_t localSendTime,
                              int64_t localRecvTime,
                              int64_t remoteSendTime,
                              long remoteElapsed)
{
    if (localSendTime>localRecvTime) {
        // invalid sample
        return;
    }
    if (remoteElapsed<0) { remoteElapsed = 0; }
    int64_t localElapsed = localRecvTime-localSendTime;
    int64_t remoteBase = remoteSendTime + (localElapsed*_timeScale-remoteElapsed)/2;
    // int64_t localBase = localRecvTime;
    double scaleUB = _timeRealScale + _realScaleErrUB;
    double baseErr1 = (remoteElapsed-localElapsed*scaleUB)/2;
    double baseErr2 = (localElapsed*scaleUB-remoteElapsed)/2;
    long baseErrLB = baseErr1>MIN_BASEERR ? (long)baseErr1 : MIN_BASEERR;
    long baseErrUB = baseErr2<MAX_BASEERR ? (long)baseErr2 : MAX_BASEERR;
    
    // prefix `eq` mains equivalent
    long eqBaseErrLB, eqBaseErrUB;
    int64_t eqRemoteBase = timeMillis(localRecvTime, &eqBaseErrLB, &eqBaseErrUB);
    int64_t baseDiff = eqRemoteBase-remoteBase;
    if (eqBaseErrLB<=MIN_BASEERR || eqBaseErrUB>=MAX_BASEERR || (baseDiff<baseErrLB || baseDiff>baseErrUB)) {
        _mutex.lock();
        _remoteTimebase = remoteBase;
        _baseErrLB = baseErrLB;
        _baseErrUB = baseErrUB;
        _localTimebase = localRecvTime;
        _mutex.unlock();
    } else {
        // remoteBase+baseErrLB <= eqRemoteBase <= remoteBase+baseErrUB
        // XXX ebaseLB/ebaseUB/baseLB/basseUB 可能会溢出
        int64_t baseLB = remoteBase+baseErrLB, baseUB = remoteBase+baseErrUB;
        int64_t ebaseLB = eqRemoteBase+eqBaseErrLB, ebaseUB = eqRemoteBase+eqBaseErrUB;
        if (baseLB < ebaseLB) { baseLB = ebaseLB; }
        if (baseUB > ebaseUB) { baseUB = ebaseUB; }
        _mutex.lock();
        _remoteTimebase = (baseLB+baseUB)/2;
        int64_t errlb = baseLB - _remoteTimebase;
        _baseErrLB = (long)(errlb<LONG_MIN ? LONG_MIN : (errlb>LONG_MAX ? LONG_MAX: errlb));
        int64_t errub = baseUB - _remoteTimebase;
        _baseErrUB = (long)(errub<LONG_MIN ? LONG_MIN : (errub>LONG_MAX ? LONG_MAX: errub));
        _localTimebase = localRecvTime;
        _mutex.unlock();
    }
    
    bool scaleChanged = false;
    int validCount = 0;
    for (auto it=_samples.rbegin(); it!=_samples.rend(); ++it) {
        const sample_t& sample = *it;
        int64_t remoteDelta = remoteSendTime - sample.remoteSendTime;
        int64_t localDelta1 = localSendTime - sample.localRecvTime;
        int64_t localDelta2 = localRecvTime - sample.localSendTime;
        if (remoteDelta<=0 || localDelta1<=0 || localDelta2<=0) {
            break;
        }
        double scaleUB = (double)remoteDelta/localDelta1;
        double scaleLB = (double)remoteDelta/localDelta2;
        if (scaleLB > _timeScale || scaleUB < _timeScale) {
            _timeScale = (scaleLB+scaleUB)*0.5;
            _timeScaleLB = scaleLB;
            _timeScaleUB = scaleUB;
            scaleChanged = true;
            break;
        } else {
            // scaleLB <= _timeScale <= scaleUB
            _timeScaleLB = MAX(_timeScaleLB, scaleLB);
            _timeScaleUB = MIN(_timeScaleUB, scaleUB);
#if SMOOTH_TIMESCALE
            double preferredScale = (_timeScaleLB+_timeScaleUB)*0.5;
            _timeScale = _timeScale + (preferredScale-_timeScale) * 0.6;
#endif
        }
        validCount++;
    }
    
    // `_timeScale`发生变化，可能是通信一方突然改时间所致，故暂时保持`_timeRealScale`不变；
    // 下次收到请求时，计算出的`_timeScale`不变，则确认为通信双方时间比例不同（例如前端使用变速齿轮）。
    if (!scaleChanged) {
        _mutex.lock();
        _timeRealScale = _timeScale;
        _realScaleErrLB = _timeScaleLB-_timeScale;
        _realScaleErrUB = _timeScaleUB-_timeScale;
        _mutex.unlock();
    }
    
    // remove invalid samples
    while (_samples.size() > validCount) { _samples.pop_front(); }
    _samples.push_back({
        localSendTime, localRecvTime, remoteSendTime, remoteElapsed
    });
    // remove outdated samples
    while (_samples.size() > _capacity) { _samples.pop_front(); }
}

double TimeCorrector::timeScale(double* lb, double* ub) const
{
    _mutex.lock();
    if (lb) { *lb = _timeRealScale+_realScaleErrLB; }
    if (ub) { *ub = _timeRealScale+_realScaleErrUB; }
    double result = _timeRealScale;
    _mutex.unlock();
    return result;
}

int64_t TimeCorrector::timeMillis(int64_t localtime, long* elb, long* eub) const
{
    _mutex.lock();
    int64_t localElapsed = localtime-_localTimebase;
    if (elb) {
        double err1 = _baseErrLB + _realScaleErrLB*localElapsed;
        *elb = err1>LONG_MIN ? (long)err1 : LONG_MIN;
    }
    if (eub) {
        double err2 = _baseErrUB + _realScaleErrUB*localElapsed;
        *eub = err2<LONG_MAX ? (long)err2 : LONG_MAX;
    }
    int64_t result = _remoteTimebase + localElapsed*_timeRealScale;
    _mutex.unlock();
    return result;
}

int64_t TimeCorrector::currentTimeMillis(long* elb, long* eub) const
{
    timeval now;
    gettimeofday(&now, NULL);
    return timeMillis(millis(now), elb, eub);
}
