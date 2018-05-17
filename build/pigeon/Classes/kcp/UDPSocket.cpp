//
//  UDPSocket.cpp
//  pigeon
//
//  Created by Vega on 2017/10/21.
//  Copyright © 2017年 gamebase. All rights reserved.
//


#include "UDPSocket.h"
using namespace std;

UDPSocket::UDPSocket(std::string ip ,int port)
:_ip(ip),
_port(port),
_addrinfo(nullptr),
_interval(10),
_work(NULL),
_writer(NULL),
_reader(NULL),
_nextUpdateTime(0),
fd(-1),
_kcp(NULL),
_isNeedReSend(false)
,_protocol(NULL)
{
	// TODO Auto-generated constructor stub
}

UDPSocket::~UDPSocket() {
	// TODO Auto-generated destructor stub
    destorySelf();
}

UDPSocket* UDPSocket::create(std::string ip ,int port,unsigned int userflag)
{
    UDPSocket* socket = new UDPSocket(ip,port);
    if (socket && socket->init(userflag))
    {
        return socket;
    }
    delete socket;
    socket = nullptr;
    return nullptr;
}

bool UDPSocket::createKCP()
{
    if(_kcp)
    {
        ikcp_release(_kcp);
        _kcp = NULL;
    }
    
    _kcp = ikcp_create(_userFlag,this );
    if(_kcp)
    {
        ikcp_wndsize(_kcp, 64, 512);
        ikcp_nodelay(_kcp, 1, _interval, 2, 1);
        _kcp->rx_minrto = 10;
        _kcp->mtu  = 512;
        _kcp->fastresend = 1;
//        _kcp->logmask = IKCP_LOG_INPUT;
//        _kcp->writelog = [](const char *log, struct IKCPCB *kcp, void *user)
//        {
//            fprintf(stdout, "%s\n",log);
//        };
        _kcp->output = [](const char *buf, int len, ikcpcb *kcp, void *user) -> int{
            UDPSocket* socket =(UDPSocket*)user;
            if(!socket)
                return -1;
            if ( socket->sendData(buf,len) )
            {
                return 0;
            }
            if(socket->getUDPProtocol())
                socket->getUDPProtocol()->notifyNetStateChange(NetState::kSendErr);
            else
                return -2;
            return -3;
        };
        return true;
    }
    return false;
}
bool UDPSocket::init(unsigned int userflag)
{
    _userFlag = userflag;

    if(createKCP()&& initAddrInfo() )
    {
        if(_writer == NULL)
        {
            _writer = new Serializer(1024);
        }
        if(_reader == NULL)
        {
            
            void* data = (void*)malloc(1024);
            _reader = new Deserializer(data,1024,[](void*buff){
                free(buff);
            });
        }
        if(_work == NULL)
        {
            _work = new CommunicationWork(this);
            _work->setLoopInterval(_interval);
        }
        if(_work)
        {
            _work->start();
        }
        return true ;
    }
    return false;
}
void UDPSocket::setKCPConfig(int sndwnd, int rcvwnd,int nodelay, int interval, int resend, int nc,int minrto,int mtu)
{
    ikcp_wndsize(_kcp, sndwnd, rcvwnd);
    ikcp_nodelay(_kcp, nodelay, interval, resend, nc);
    _kcp->rx_minrto = minrto;
    _kcp->mtu  = mtu;
    _kcp->fastresend = 1;
}

void UDPSocket::setInterval(int interval)
{
    _interval = interval;
    if(_kcp)
    {
        ikcp_nodelay(_kcp, 1, _interval, 2, 1);
    }
    if(_work)
    {
        _work->setLoopInterval(_interval );
    }
}

void UDPSocket::destorySelf()
{
    if(_work)
    {
        _work->stop();
        _work->join();
        delete _work;
    }
    _work = NULL;

    if(_writer)
    {
        delete _writer;
    }
    _writer = NULL;

    if(_reader)
    {
        delete _reader;
    }
    _reader = NULL;
    
    if(_addrinfo){
        freeaddrinfo(_addrinfo);
    }
    _addrinfo = NULL;
    if(_kcp)
    {
        ikcp_release(_kcp);
    }
    _kcp = NULL;
    if(this->fd > 0)
#ifdef WIN32
		closesocket(fd);
#else
		close(fd);
#endif // WIN32
    this->fd = -1;
}
bool UDPSocket::initAddrInfo()
{
    if(_addrinfo != nullptr){
        freeaddrinfo(_addrinfo);
        _addrinfo = NULL;
    }
    struct addrinfo hints;
    struct addrinfo* result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_V4MAPPED;
#if defined(ANDROID)
    hints.ai_flags = AI_PASSIVE;
#endif
    stringstream ss;
    string port;
    ss<<_port;
    ss>>port;
    if (getaddrinfo(_ip.c_str(), port.c_str(), &hints, &result) != 0) {
        return false;
    }
    _addrinfo = result;
    if(!_addrinfo){
        if(_protocol)
            _protocol->notifyNetStateChange(NetState::kConnectErr);
        return false;
    }
    if(fd > -1)
    {
#ifdef WIN32
		closesocket(fd);
#else
		close(fd);
#endif // WIN32
    }
    fd = socket(_addrinfo->ai_family, _addrinfo->ai_socktype, _addrinfo->ai_protocol);
    do {
#if defined(WIN32)
        u_long non_blk = 0;
        ioctlsocket(fd, FIONBIO, (unsigned long*) &non_blk);
#else
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
#if defined(__APPLE__)
        int on = 1;
        setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
#endif
    }while(0);
    return true;
}

int UDPSocket::fdset(int* maxfd, fd_set* rset, fd_set* wset, fd_set* eset)
{
    if (fd>=0) {
        if (rset) { FD_SET(fd, rset); }
        if (wset) { FD_SET(fd, wset); }
        if (eset) { FD_SET(fd, eset); }
        if (maxfd) { *maxfd = MAX(fd, *maxfd); }
    }
    return 0;
}

void UDPSocket::select(long timeout)
{
    
    int maxSockfd = -1;
    fd_set readset, writeset, exceptset;
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&exceptset);

   fdset(&maxSockfd, &readset, &writeset, &exceptset);
    if (maxSockfd>=0) {
        if (timeout<0) { timeout = 0; }
        struct timeval timeo = {timeout/1000, (int)(timeout%1000)*1000};
        int result = ::select(maxSockfd+1, &readset, &writeset, &exceptset, &timeo);
        if (result==0) {
            // timed out
        }
        else if (result<0) {
            /*
             has errors. `errno` may be:
             EBADF  An invalid file descriptor was given in one of the sets.
             EINTR  Interrupted by signals.
             EINVAL `nfds` is negative or exceeds the RLIMIT_NOFILE resource limit.
             EINVAL `timeout` is too much.
             ENOMEM Unable to allocate memory for internal tables.
             */
        }
    }
    update(&readset, &writeset, &exceptset);
    if(_protocol)
        _protocol->flush();
}

void UDPSocket::update(const fd_set *rset, const fd_set *wset, const fd_set *eset)
{
#if defined(WIN32)
        if (fd != INVALID_SOCKET && _kcp ) {
#else
        if (fd>=0 && _kcp) {
#endif
            if(FD_ISSET(fd, rset)){
                while(true){
                    _reader->clean();
                    if(receiveData())
                    {
                        if(_reader->tell() > 0 && _kcp){
                            _nextUpdateTime = 0;
                            if(ikcp_input(_kcp,(char*)_reader->buffer(),_reader->tell()) != 0 )
                            {
                                if(_protocol)
                                    _protocol->notifyNetStateChange(NetState::kKCPErr);
                                break;
                            }
                            if(_reader->tell() < _reader->length())
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        if(_protocol)
                            _protocol->notifyNetStateChange(NetState::kRevErr);
                        break;
                    }
                }
                    
            }
            if (FD_ISSET(fd, wset)) {
                do{
                    if(!_isNeedReSend){
                        _writer->clean();
                        if(_protocol)
                            _protocol->getRequestData(_writer);
                    }
                    if(_writer->tell() > 0 && _kcp){
                        if(ikcp_send(_kcp,(char*)_writer->buffer(), (int)_writer->tell()) != 0)
                        {
                            if(_protocol)
                                _protocol->notifyNetStateChange(NetState::kKCPErr);
                            _isNeedReSend = true;
                            break;
                        }
                        _nextUpdateTime = 0;
                        _isNeedReSend = false;
                    }
                    else{
                        break;
                    }
                }while(true);
            }
            gettimeofday(&_now, NULL);
            IUINT32 now = (IUINT32)(_now.tv_sec*1000 + _now.tv_usec);
            if(now > _nextUpdateTime){
                ikcp_update(_kcp, now);
                _nextUpdateTime = ikcp_check(_kcp, now);
            }
            while(true){
                _reader->clean();
                int ret = ikcp_recv(_kcp,(char*)_reader->buffer() ,(int)_reader->length());
                if(ret == -1 || ret == -2 )
                {
                    break;
                }
                if(ret == -3 )
                {
                    _reader->clean();
//                    int peeksize = ikcp_peeksize(_kcp); --报错 所以按下面的写
//                    _reader->insureBuffer(peeksize);
                    if(_reader->insureBuffer(_reader->length()*2) != 0)
                    {
                        if(_protocol)
                            _protocol->notifyNetStateChange(NetState::kMemoryErr);
                        break;
                    }
                    continue;
                }
                _reader->seek(ret, SEEK_SET);
                if(_protocol)
                    _protocol->receiveData(_reader);
            }
        }
}

bool UDPSocket::sendData(const char* data,int len) {
    if(!_addrinfo)
        return false;
    size_t totalSize = 0;
    do{
        size_t size = sendto(this->fd,data,len - totalSize, 0, _addrinfo->ai_addr, _addrinfo->ai_addrlen);
        if(size <0 )
        {
            fprintf(stdout,"--->>>send error %zd\n",size);
            return false;
        }
        if(size + totalSize < len)
        {
            totalSize += size;
        }
        else
        {
            break;
        }
    }while(true);
    return true;
}

bool UDPSocket::receiveData()
{
    if(!_addrinfo)
        return false;
#ifdef WIN32
	size_t ret = recvfrom(this->fd, (char*)_reader->buffer(), _reader->length(), 0, _addrinfo->ai_addr, (int*)&(_addrinfo->ai_addrlen));
#else
	size_t ret = recvfrom(this->fd,(void*)_reader->buffer(),_reader->length(),0,_addrinfo->ai_addr,&(_addrinfo->ai_addrlen));
#endif // WIN32


	if( ret < 0 ) {
        if(errno == EAGAIN)
        {
            return true ;
        }
        fprintf(stdout,"--->>>rev error %zd\n",ret);
        return false;
	}
    else
    {
        _reader->seek(ret,SEEK_SET);
    }
    return true;
}
bool UDPSocket::reconnect()
{
    if(initAddrInfo())
    {
        if(createKCP())
            return true;
        return false;
    }
    return false;
}
bool UDPSocket::resetSocket()
{
    return initAddrInfo();
}

