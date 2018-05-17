#ifndef __pigeon_rpctimecorrector_h
#define __pigeon_rpctimecorrector_h

#include "netbasic.h"

class TimeCorrector
{
public:
    TimeCorrector(size_t sampleCapacity);

    void reset();
    void addSample(
        const timeval& localSendTime,
        const timeval& localRecvTime,
        const timeval& remoteSendTime,
        long remoteElapsed);
    void addSample(
        int64_t localSendTime,
        int64_t localRecvTime,
        int64_t remoteSendTime,
        long remoteElapsed);

    double timeScale(double* lb, double* ub) const;
    int64_t timeMillis(int64_t localtime, long* elb, long* eub) const;
    int64_t currentTimeMillis(long* elb, long* eub) const;
private:
    struct sample_t {
        int64_t localSendTime, localRecvTime;
        int64_t remoteSendTime;
        long remoteElapsed;
    };
    typedef std::list<sample_t> SampleQueue;
private:
    size_t _capacity;
    SampleQueue _samples;
    
    int64_t _remoteTimebase, _localTimebase;
    long _baseErrLB, _baseErrUB;
    
    double _timeScale;
    double _timeScaleLB, _timeScaleUB;
    double _timeRealScale;
    double _realScaleErrLB, _realScaleErrUB;
    
    mutable std::mutex _mutex;
};

#endif
