#include "neopixel.h"

#include <functional>
#include <algorithm>


NeopixelInterface::NeopixelInterface(State* st) : strip(neopixelCount), animator(1){
    state = st;
    strip.Begin();
}

void NeopixelInterface::loop(){
    ConnectionState connectionState = state->getConnectionState();

    updateInterfaceState();

    switch (ifState){
    case InterfaceState::UNCONNECTED:
        switch (connectionState){
        case ConnectionState::PRE_SERIAL:
            startAnimationBlink(RgbColor(255,0,0),2);
            break;
        case ConnectionState::PRE_WIFI:
            startAnimationBlink(RgbColor(255,150,150),2);
            break;
        case ConnectionState::NO_MQTT:
            startAnimationBlink(RgbColor(0,0,255),2);
            break;
        }
        break;
    case InterfaceState::OPEN:
        if (millis() - ifChange > 4000){
            startAnimationSolid(RgbColor(0,255,0));
        } else {
            startAnimationRotate(RgbColor(0,255,0));
        }
        break;
    case InterfaceState::CLOSED:
        if (millis() - ifChange > 4000){
            startAnimationSolid(RgbColor(255,0,0));
        } else {
            startAnimationRotate(RgbColor(255,0,0));
        }
        break;
    case InterfaceState::ERROR:
        if (millis() - ifChange > 4000){
            startAnimationBlinkAll(RgbColor(255,150,150));
        } else {
            startAnimationRotate(RgbColor(255,150,150));
        }
        break;
    }

    animator.UpdateAnimations();
    strip.Show();
}

void NeopixelInterface::updateInterfaceState(){
    ConnectionState connectionState = state->getConnectionState();
    SpaceState localState = state->getLocalSpaceState();
    SpaceState remoteState = state->getRemoteSpaceState();

    switch (connectionState){
    case ConnectionState::PRE_SERIAL:
    case ConnectionState::PRE_WIFI:
    case ConnectionState::NO_MQTT:
        if (ifState != InterfaceState::UNCONNECTED){
            ifState = InterfaceState::UNCONNECTED;
            ifChange = millis();
        }
        return;
    }

    if (localState != remoteState){
        if (ifState != InterfaceState::ERROR){
            ifState =  InterfaceState::ERROR;
            ifChange = millis();
        }
        return;
    }

    if (localState == SpaceState::SOPEN){
        if (ifState != InterfaceState::OPEN){
            ifState =  InterfaceState::OPEN;
            ifChange = millis();
        }
        return;
    }

    if (localState == SpaceState::SCLOSED){
        if (ifState != InterfaceState::CLOSED){
            ifState =  InterfaceState::CLOSED;
            ifChange = millis();
        }
        return;
    }
}

void NeopixelInterface::startAnimationRotate(RgbColor c){
    if (animator.IsAnimating()){
        return;
    }
    animationColor = c;
    animator.StartAnimation(0,2000, [=](const AnimationParam &param){
        animationRotate(param);
    });
}

void NeopixelInterface::startAnimationSolid(RgbColor c){
    if (animator.IsAnimating()){
        return;
    }
    animationColor = c;
    animator.StartAnimation(0,2000, [=](const AnimationParam &param){
        animationSolid(param);
    });
}

void NeopixelInterface::startAnimationBlinkAll(RgbColor c){
    if (animator.IsAnimating()){
        return;
    }
    animationColor = c;
    animator.StartAnimation(0,2000, [=](const AnimationParam &param){
        animationBlinkAll(param);
    });
}

void NeopixelInterface::startAnimationBlink(RgbColor c,int i){
    if (animator.IsAnimating()){
        return;
    }
    animationColor = c;
    animationLED = i;
    animator.StartAnimation(0,1000, [=](const AnimationParam &param){
        animationBlink(param);
    });
}

void NeopixelInterface::animationRotate(const AnimationParam &param){
    float part = 1. / ((float)neopixelCount);
    for (int i = 0; i < neopixelCount; i++){
        float localProgress = param.progress - part * i;
        localProgress = localProgress < 0 ? 0 : localProgress;

        strip.SetPixelColor(i, RgbColor::LinearBlend(RgbColor(0),animationColor,localProgress));
    }

    if (param.state == AnimationState_Completed && !stateChanged){
        animator.RestartAnimation(param.index);
    }
}
void NeopixelInterface::animationSolid(const AnimationParam &param){
    for (int i = 0; i < neopixelCount; i++){
        strip.SetPixelColor(i,animationColor);
    }

    if (param.state == AnimationState_Completed && !stateChanged){
        animator.RestartAnimation(param.index);
    }
}
void NeopixelInterface::animationBlinkAll(const AnimationParam &param){
    if (param.progress < 0.5){
        for (int i = 0; i < neopixelCount; i++){
            strip.SetPixelColor(
                i,RgbColor::LinearBlend(RgbColor(0),animationColor,param.progress*2));
        }        
    } else if (param.progress < 1){
        for (int i = 0; i < neopixelCount; i++){
            strip.SetPixelColor(
                i,RgbColor::LinearBlend(animationColor,RgbColor(0),param.progress*2));
        }
    }

    if (param.state == AnimationState_Completed && !stateChanged){
        animator.RestartAnimation(param.index);
    }
}
void NeopixelInterface::animationBlink(const AnimationParam &param){    
    if (param.progress < 0.5){
        strip.SetPixelColor(
            animationLED,RgbColor::LinearBlend(RgbColor(0),animationColor,param.progress*2));
    } else if (param.progress < 1){
        strip.SetPixelColor(
            animationLED,RgbColor::LinearBlend(animationColor,RgbColor(0),param.progress*2));
    }

    if (param.state == AnimationState_Completed && !stateChanged){
        animator.RestartAnimation(param.index);
    }
}
