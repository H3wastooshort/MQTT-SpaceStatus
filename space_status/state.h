#pragma once

enum ConnectionState{
    PRE_SERIAL, // Before Serial is connected
    PRE_WIFI,   // Before WiFi connection is established
    NO_MQTT,    // Before connection to a MQTT server is established
    CONN_OK          // All connections done
};

enum SpaceState{
    SOPEN,
    SCLOSED,
    SUNKNOWN
};

class State{
private: 
    ConnectionState connectionState = PRE_SERIAL;
    SpaceState localState = SUNKNOWN;
    SpaceState remoteState = SUNKNOWN;

public:
    ConnectionState getConnectionState();
    void setConnectionState(ConnectionState st);

    SpaceState getLocalSpaceState();
    void setLocalSpaceState(SpaceState st);

    SpaceState getRemoteSpaceState();
    void setRemoteSpaceState(SpaceState st);
};