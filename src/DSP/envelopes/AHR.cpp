
#include "AHR.h"

pdsp::AHR::AHR(){
        
        stageSwitch = off;
        
        addInput("trig", input_trig);
        addOutput("signal", output);
        addInput("attack", input_attack);
        addInput("hold", input_hold);
        addInput("release", input_release);
        addInput("velocity", input_velocity);        
        updateOutputNodes();
        
        attackTCO = 0.99999;             //digital 
        releaseTCO = exp(-4.95); //digital 
        
        holdLevel = 1.0f;
        
        input_attack.setDefaultValue(0.0f);
        input_hold.setDefaultValue(50.0f);
        input_release.setDefaultValue(50.0f);
        input_velocity.setDefaultValue(1.0f);
        
        if(dynamicConstruction){
                prepareToPlay(globalBufferSize, globalSampleRate);
        }
}

pdsp::Patchable& pdsp::AHR::set(float attackTimeMs, float holdTimeMs, float releaseTimeMs){
        input_attack.setDefaultValue(attackTimeMs);
        input_hold.setDefaultValue(holdTimeMs);
        input_release.setDefaultValue(releaseTimeMs);
        return *this;
}

float pdsp::AHR::meter_output() const{
    return meter.load();
}

pdsp::Patchable& pdsp::AHR::in_trig(){
    return in("trig");
}
    
pdsp::Patchable& pdsp::AHR::in_attack(){
    return in("attack");
}

pdsp::Patchable& pdsp::AHR::in_hold(){
    return in("hold");
}

pdsp::Patchable& pdsp::AHR::in_release(){
    return in("release");
}

pdsp::Patchable& pdsp::AHR::in_velocity(){
    return in("velocity");
}

pdsp::Patchable& pdsp::AHR::out_signal(){
    return out("signal");
}

void pdsp::AHR::prepareUnit( int expectedBufferSize, double sampleRate){
        setEnvelopeSampleRate( sampleRate );
        stageSwitch = off;
        envelopeOutput = 0.0;

        calculateAttackTime();
        calculateHoldTimeAndReset();
        calculateReleaseTime();
}


void pdsp::AHR::releaseResources(){}


void pdsp::AHR::process(int bufferSize) noexcept{
        
        int trigBufferState;
        const float* trigBuffer = processInput(input_trig, trigBufferState);

        switch(trigBufferState){
            case Unchanged: case Changed:
                switch(stageSwitch){
                case off:
                    setOutputToZero(output);
                    meter.store(0.0f);
                    break;
                
                default:
                    process_run(bufferSize);
                    break;
                }
                break;                
           
            case AudioRate:
                process_T(trigBuffer, bufferSize);
                break;
            
            default: break;
        }
}

void pdsp::AHR::onRetrigger(float triggerValue, int n) {
        triggerValue = (triggerValue > 1.0f) ? 1.0f : triggerValue;
        
        if( triggerValue > 0.0f ){
                float veloCtrl = processAndGetSingleValue(input_velocity, n);
                this->intensity = (triggerValue * veloCtrl)  + (1.0f-veloCtrl); 
                stageSwitch = attackStage;
        }        

        setAttackTime(  processAndGetSingleValue(input_attack,  n) );
        setHoldTime(    processAndGetSingleValue(input_hold,    n), this->intensity);
        setReleaseTime( processAndGetSingleValue(input_release, n) );
}


void pdsp::AHR::process_run(int bufferSize){

    float* outputBuffer = getOutputBufferToFill(output);
    
    for (int n = 0; n < bufferSize; ++n){
        doEnvelope();
        outputBuffer[n] = envelopeOutput;
    }
    
    meter.store(outputBuffer[0]); 
}


void pdsp::AHR::process_T( const float* &trigBuffer, const int &bufferSize){
        
    float* outputBuffer = getOutputBufferToFill(output);

    for (int n = 0; n < bufferSize; ++n){
        if ( envTrigger( trigBuffer[n] ) ){ onRetrigger( trigBuffer[n], n ); };

        doEnvelope();
        outputBuffer[n] = envelopeOutput;
    }
    
    meter.store(outputBuffer[0]); 
}
