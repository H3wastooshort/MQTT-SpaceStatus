#include "state.h"
#include <Arduino.h>

ConnectionState State::getConnectionState(){
    return connectionState;
}
void State::setConnectionState(ConnectionState st){
    connectionState = st;
}

void printSpaceState(SpaceState st) {
      switch (st) {
      case SOPEN: Serial.println("Open"); break;
      case SCLOSED: Serial.println("Closed"); break;
      case SUNKNOWN: Serial.println("Unknown"); break;
    }
}

SpaceState State::getLocalSpaceState(){
    return localState;
}
void State::setLocalSpaceState(SpaceState st){
    localState = st;
    Serial.print("Local Space State is now ");
    printSpaceState(localState);
}

SpaceState State::getRemoteSpaceState(){
    return remoteState;
}
void State::setRemoteSpaceState(SpaceState st){
    remoteState = st;
    Serial.print("Remote Space State is now ");
    printSpaceState(remoteState);
}