
#ifndef PDSP_OSC_WAVETABLE_H_INCLUDED
#define PDSP_OSC_WAVETABLE_H_INCLUDED


#include "../../samplers/SampleBuffer.h"

namespace pdsp {
    
class WaveTable {
    friend class WaveTableOsc;
    
public:    

    WaveTable();
    ~WaveTable();

    /*!
    @brief add an empty wavetable to the buffer, 
    */    
    void addEmpty( int numToAdd = 1);
    
    /*!
    @brief add a waveform from a sample file located at path
    @param[in] path absolute or relative path to audio file
    
    */    
    void addSample( std::string path );

    /*!
    @brief generate a waveform from an inlined list of partials. This function is precise but not fast, use it before starting the DSP engine.
    @param[in] partials inlined array of partials 
    @param[in] harmonicScale if true each value is multiplied by a saw wave corresponding partial
    
    */    
    void addAdditiveWave( std::initializer_list<double> partials, bool harmonicScale = false );


    /*!
    @brief returns an handle to the buffer of the wave at the given index
    @param[in]  index index of the wavetable
    */    
    float* table( int index );

    /*!
    @brief returns the lenght of each wave buffer
    */  
    int tableLength() const;

    /*!
    @brief sets the table length, it is called automatically if you add samples and you should call it by yourself only if you want to generate your waves.
    */  
    void initLength( int len );

    /*!
    @brief returns the size of the wavetable
    */  
    int size() const;

    /*!
    @brief enables logs for file loading
    */      
    void setVerbose( bool verbose );
    
private:    
    float ** buffer;    
    int length;
    int tableSize;
    bool verbose;
        
    SampleBuffer loader;
};    

}


#endif // PDSP_OSC_WAVETABLE_H_INCLUDED
