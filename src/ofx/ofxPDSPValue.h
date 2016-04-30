
// ofxPDSPValue.h
// ofxPDSP
// Nicola Pisanti, MIT License, 2016

#ifndef OFXPDSP_PDSPVALUE_H_INCLUDED
#define OFXPDSP_PDSPVALUE_H_INCLUDED

#include "../DSP/pdspCore.h"
#include "../DSP/helpers/UsesSlew.h"
#include "../DSP/control/TriggerControl.h"

#include "ofMain.h"

typedef pdsp::TriggerControl ofxPDSPTrigger;

//-------------------------------------------------------------------------------------------------

/*!
@brief Utility class control the audio dsp parameter and bind them to an ofParameter

ofxPDSPValue conteins an ofParameter<float> and an internal atomic float value, that is read and processed in a thread-safe manner. When the ofParameter is changed also the internal value is changed. The setv() methods sets only the internal value, for setting parameter faster when you don't need the ofParameter features. The output of this class can be patched to the audio DSP and can be optionally smoothed out.

*/
class ofxPDSPValue : public pdsp::Unit, public pdsp::UsesSlew {
    
public:
    ofxPDSPValue();
    ofxPDSPValue(const ofxPDSPValue & other);
    ofxPDSPValue& operator=(const ofxPDSPValue & other);
    
    ~ofxPDSPValue();
    
    
    /*!
    @brief sets the and returns the internal ofParameter, useful to set up an UI
    @param[in] name this will become the name of the ofParameter
    @param[in] value default value
    @param[in] min minimum value 
    @param[in] max maximum value
    */    
    ofParameter<float>& set(const char * name, float value, float min, float max);


    /*!
    @brief sets the value min and max boundary when operated by the ofParameter in the UI and returns the parameter ready to be added to the UI
    @param[in] name this will become the name of the ofParameter
    @param[in] min minimum value 
    @param[in] max maximum value
    */  
    ofParameter<float>& set(const char * name, float min, float max);


    /*!
    @brief sets the value and returns this unit ready to be patched, without updating the ofParameter
    @param[in] value new value
    
    This set method don't update the ofParameter, for faster computation when you're not using this class for UI
    */   
    void setv(float value);


    /*!
    @brief sets the ofParameter, triggering a callback that also update the value. It isn't fast as setf(). Returns the unit ready to be patched.
    @param[in] value new value
    
    */   
    pdsp::Patchable& set(float value);


    /*!
    @brief returns the ofParameter ready to be added to the UI
    */  
    ofParameter<float>& getParameterSettings();


    /*!
    @brief enables the smoothing of the setted values
    @param[in] timeMs how many milliseconds will take to reach the setted value
    */  
    void enableSmoothing(float timeMs);
    
    /*!
    @brief disable the smoothing of the setted values. smoothing is disabled by default
    */  
    void disableSmoothing();

    /*!
    @brief gets the value
    */       
    float get() const;
     

private:

    ofParameter<float>  parameter;
    
    pdsp::OutputNode output;
    
    void prepareUnit( int expectedBufferSize, double sampleRate ) override;
    void releaseResources () override ;
    void process (int bufferSize) noexcept override;

    atomic<float> lastValue;
    atomic<float> value;
    
    void onSet(float &newValue);
    
};






#endif //OFXPDSP_PDSPVALUE_H_INCLUDED
