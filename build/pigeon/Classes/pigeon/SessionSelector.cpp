#include "SessionSelector.h"
#include "SessionActor.h"

SessionSelector::SessionSelector()
{
}

SessionSelector::~SessionSelector()
{
}

SessionSelector::ActorList::iterator SessionSelector::findSeesionActor(SessionActor* actor)
{
    ActorList::iterator it = _actors.begin();
    while (it!=_actors.end()) {
        if (*it == actor) {
            break;
        }
        ++it;
    }
    return it;
}

void SessionSelector::addSessionActor(SessionActor* actor)
{
    if (!actor) {
        return;
    }
    ActorList::iterator it = findSeesionActor(actor);
    if (it!=_actors.end()) {
        // already added
        return;
    }
    _actors.push_back(actor);
}

void SessionSelector::removeSessionActor(SessionActor* actor)
{
    if (!actor) {
        return;
    }
    ActorList::iterator it = findSeesionActor(actor);
    if (it==_actors.end()) {
        // not found
        return;
    }
    _actors.erase(it);
}

void SessionSelector::removeAllSessions()
{
    _actors.clear();
}

size_t SessionSelector::getSessionsCount() const
{
    return _actors.size();
}

void SessionSelector::select(long timeout)
{
    int maxSockfd = -1;
    fd_set readset, writeset, exceptset;
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&exceptset);

    ActorList active;
    ActorList::iterator it = _actors.begin();
    while (it!=_actors.end()) {
        SessionActor* actor = *it;
        actor->fdset(&maxSockfd, &readset, &writeset, &exceptset);
        active.push_back(actor);
        ++it;
    }
    
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

    it = active.begin();
    while (it!=active.end()) {
        (*it)->update(&readset, &writeset, &exceptset);
        ++it;
    }
}
