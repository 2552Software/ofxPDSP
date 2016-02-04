
// ofxMidiKeysProcessor.h
// ofxPDSP
// Nicola Pisanti, MIT License, 2016

#ifndef OFXPDSPMIDI_MIDIKEYSPROCESSOR_H_INCLUDED
#define OFXPDSPMIDI_MIDIKEYSPROCESSOR_H_INCLUDED


#include "ofxMidi.h"
#include "../DSP/pdspCore.h"
#include "../DSP/control/GateSequencer.h"
#include "../DSP/control/ValueSequencer.h"
#include "ofxPositionedMidiMessage.h"
#include "ofxMidiKeysBuffers.h"

#include "ofxMidiInProcessor.h"

class ofxMidiKeysProcessor {

    
public:
    ofxMidiKeysProcessor();
 
    void processMidi(const ofxMidiInProcessor &midiInProcessor, const int &bufferSize ) noexcept;
    

    int getVoicesNumber() const;
    
    void setPolyMode(int maxNotes, int unisonVoices);
    void setMonoMode(int unisonVoices, bool legato, MonoPriority priority);
    void setPitchBend( float down, float up);
    void setPortamento(PortamentoMode portamentoMode, float timeMs, pdsp::SlewMode_t slewMode = pdsp::Time, float scaling=12.0f);
    

    void setSlew(float slewTimeMs);  
      
    vector<pdsp::PatchNode>         outs_trig;
    pdsp::GateSequencer             out_singletrigger;
    
    vector<pdsp::PatchNode>         outs_pitch;
    
    pdsp::ValueSequencer            out_pitchbend;
    pdsp::ValueSequencer            out_pressure;

    pdsp::ValueNode                 unisonDetuneAmt;
    
    
private: 
    int                 maxNotes;
    int                 maxVoices;

    PortamentoMode      portamentoMode;
    float               portamentoTime;
    pdsp::SlewMode_t    slewMode; 
    float               slewTime;
   
    ofxMidiKeysBuffers      midiConverter;
    
    vector<pdsp::GateSequencer>      gates;
    vector<pdsp::ValueSequencer>     values;
    vector<pdsp::Amp>                unisonPitchDetune;
    vector<pdsp::ValueNode>          voiceUnisonMod;
    
        
};




#endif //OFXPDSPMIDI_MIDIKEYSPROCESSOR_H_INCLUDED
