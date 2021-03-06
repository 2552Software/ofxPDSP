
// OscInput.h
// ofxPDSP
// Nicola Pisanti, MIT License, 2016

#ifndef OFXPDSPMIDI_PDSPOSCINPUT_H_INCLUDED
#define OFXPDSPMIDI_PDSPOSCINPUT_H_INCLUDED

#include "ofMain.h"
#include <chrono>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "../DSP/pdspCore.h"
#include "../sequencer/SequencerSection.h"
#include "ofxOsc.h"

/*!
@brief utility class manage OSC input to the DSP
*/

namespace pdsp{ namespace osc {

class Input : public pdsp::Preparable {

private:

    //enum OscChannelMode { Gate, Value };

    class OscChannel {
    public:
        OscChannel();
        
        void deallocate();
        
        string key;
        pdsp::MessageBuffer* messageBuffer;
        //OscChannelMode mode;
        pdsp::SequencerGateOutput* gate_out;
        pdsp::SequencerValueOutput* value_out;
        
    };
    
    class _PositionedOscMessage {
    public:
        _PositionedOscMessage(){ sample = -1; };
        _PositionedOscMessage(ofxOscMessage message, int sample) : message(message), sample(sample){};
        
        
        std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;
        ofxOscMessage message;
        int sample;
    };


public:
    Input();    
    ~Input();      
    

    /*!
    @brief open the port with the given index
    @param[in] port OSC port
    */    
    void openPort( int port );

    /*!
    @brief shuts down the output
    */   
    void close();


    /*!
    @brief return true if the port has been sucessfully opened
    */   
    bool isConnected() { return connected; }
    
    /*!
    @brief enable or disable diagnostic messages
    @param[in] verbose true for enabling, false for disabling
    */   
    void setVerbose( bool verbose );


    /*!
    @brief get a trigger output for the given OSC address. Only the first value of those address will be taken, as float value. When you have used an address as trig output you can't use it as value
    @param[in] oscAddress address for OSC input
    */       
    pdsp::SequencerGateOutput& out_trig( string oscAddress );

    /*!
    @brief get a value output for the given OSC address. Only the first value of those address will be taken, as float value. When you have used an address as value output you can't use it as trigger
    @param[in] oscAddress address for OSC input
    */    
    pdsp::SequencerValueOutput& out_value( string oscAddress );


    /*!
    @brief sends 0.0f as message to all the connected trigger and value outputs.
    */   
    void clearAll();

/*!
    @cond HIDDEN_SYMBOLS
*/  
    void processOsc( int bufferSize ) noexcept;
/*!
    @endcond
*/   

protected:
    void prepareToPlay( int expectedBufferSize, double sampleRate ) override;
    void releaseResources() override;    

private:
    ofxOscReceiver  receiver;
        
    vector<OscChannel*> oscChannels;    
    
    bool            connected;
    bool            verbose;    
    
    bool    sendClearMessages; 

    
    std::atomic<int> index;
    int lastread;
    std::vector<_PositionedOscMessage>    circularBuffer;
    std::vector<_PositionedOscMessage>    readVector;


    double                                              oneSlashMicrosecForSample;

    chrono::time_point<chrono::high_resolution_clock>   bufferChrono;   

    pdsp::SequencerGateOutput invalidGate;
    pdsp::SequencerValueOutput invalidValue;


    void                                                startDaemon();
    void                                                closeDaemon();
    void                                                daemonFunction() noexcept;
    static void                                         daemonFunctionWrapper(Input* parent);
    thread                                              daemonThread;
    atomic<bool>                                        runDaemon;
    int                                                 daemonRefreshRate;

    void pushToReadVector( _PositionedOscMessage & message );
    
};

}}

#endif // OFXPDSPMIDI_PDSPOSCINPUT_H_INCLUDED
