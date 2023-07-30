#include "state.h"

ConnectionState State::getConnectionState(){
    return connectionState;
}
void State::setConnectionState(ConnectionState st){
    connectionState = st;
}

SpaceState State::getLocalSpaceState(){
    return localState;
}
void State::setLocalSpaceState(SpaceState st){
    localState = st;
}

SpaceState State::getRemoteSpaceState(){
    return remoteState;
}
void State::setRemoteSpaceState(SpaceState st){
    remoteState = st;
}