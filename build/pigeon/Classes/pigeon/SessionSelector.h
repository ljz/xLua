#ifndef __pigeon_sessionselector_h
#define __pigeon_sessionselector_h

#include "netbasic.h"

class SessionActor;

class SessionSelector {
public:
    SessionSelector();
    ~SessionSelector();

    void addSessionActor(SessionActor* actor);
    void removeSessionActor(SessionActor* actor);
    void removeAllSessions();
    size_t getSessionsCount() const;

    void select(long timeout = 0);

private:
    typedef std::list<SessionActor*> ActorList;

    ActorList::iterator findSeesionActor(SessionActor* actor);

    ActorList _actors;
};

#endif
