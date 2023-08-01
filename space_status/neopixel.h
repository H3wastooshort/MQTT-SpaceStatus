#pragma once

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

#include "state.h"

#define neopixelPin 2
#define neopixelCount 5

class NeopixelInterface{
private:
    enum InterfaceState{
        UNCONNECTED, OPEN, CLOSED, DIFF, ERROR
    };

    NeoPixelBus<NeoGrbFeature,NeoEsp8266Uart1800KbpsMethod> strip;
    NeoPixelAnimator animator;
    State *state;

    InterfaceState ifState;
    long ifChange;

    bool stateChanged = true;

    RgbColor animationColor;
    int animationLED;

    void startAnimationRotate(RgbColor);
    void startAnimationSolid(RgbColor);
    void startAnimationBlinkAll(RgbColor);
    void startAnimationBlink(RgbColor,int);
    
    void updateInterfaceState();

    void animationRotate(const AnimationParam &param);
    void animationSolid(const AnimationParam &param);
    void animationBlinkAll(const AnimationParam &param);
    void animationBlink(const AnimationParam &param);

public:
    NeopixelInterface(State*);

    void loop();
};