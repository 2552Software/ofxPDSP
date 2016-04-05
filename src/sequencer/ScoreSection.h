
// ScoreSection.h
// ofxPDSP
// Nicola Pisanti, MIT License, 2016

#ifndef PDSP_SCOREROW_H_INCLUDED
#define PDSP_SCOREROW_H_INCLUDED

#include "ScoreCell.h"
#include <vector>
#include "../messages/header.h"
#include "../DSP/control/Sequencer.h"
#include <mutex>
#include "../flags.h"

namespace pdsp{
    
    /*!
    @brief Plays a single ScoreCell at time, sequences ScoreCells
    
    This class plays a single ScoreCell at time, sequences ScoreCells and has multiple outputs to connect to GateSequencer or ValueSequencer. ScoreProcessor owns a vector of ScoreSection. Remember that the ScoreSections inside a ScoreProcessor are processed from the first to the last so it could be possible for the data generated from the first ScoreSections in the vector to influence the others (in a thread-safe manner).
    */

class ScoreSection { 
    friend class ScoreProcessor;   
    friend Patchable& linkSelectedOutputToSequencer (ScoreSection& scoreSection, Sequencer& input);
    friend void linkSelectedOutputToExtSequencer (ScoreSection& scoreSection, ExtSequencer& ext);
    friend void unlinkSelectedOutputToExtSequencer (ScoreSection& scoreSection, ExtSequencer& ext);
    
private:

    //---------------------------------------INNER CLASS------------------------
    /*!
        @cond HIDDEN_SYMBOLS
    */
    class PatternStruct{
    public:
        PatternStruct() : scoreCell(nullptr), nextCell(nullptr), length(1.0), quantizeLaunch(false), quantizeGrid(0.0) {};
            
        ScoreCell*      scoreCell;
        CellChange*     nextCell;
        double          length;
        bool            quantizeLaunch;
        double          quantizeGrid;
    };    
    /*!
        @endcond
    */    

    //--------------------------------------------------------------------------
    
public:

    ScoreSection();
    ScoreSection(const ScoreSection &other);

    /*!
    @brief Sets the number of patterns contained by this ScoreSection
    @param[in] size number of ScoreCell contained by this ScoreSection

    */ 
    void resizePatterns(int size);
    
    /*!
    @brief Sets the number of outputs of this ScoreSection, default is 1.
    @param[in] size number of outputs for connection

    Set the number of outputs that can be connected to GateSequencer or ValueSequencer, default is 1.
    */ 
    void setOutputsNumber(int size);
    
    /*!
    @brief Set the output with the given index as selected output and return this ScoreSection for patching
    @param[in] index index of the out to patch, 0 if not given

    Set the output with the given index as selected output and return this ScoreSection for patching. You can connect the ScoreSection to a GateSequencer, ValueSequencer or ExtSequencer using the >> operator.
    */     
    ScoreSection& out( int index = 0 );


/*!
    @cond HIDDEN_SYMBOLS
*/
    [[deprecated("Replaced by setCell() for a less ambigous nomenclature")]]
    void setPattern( int index, ScoreCell* scoreCell, CellChange* behavior = nullptr );
    
    [[deprecated("Replaced by setChange() for a less ambigous nomenclature")]]
    void setBehavior( int index, CellChange* behavior );
/*!
    @endcond
*/    
    
    /*!
    @brief Sets the Cell at the given index to the given one
    @param[in] index index of the ScoreCell (or Sequence) to set inside the ScoreSection. If the size is too little the ScoreSection is automatically resized.
    @param[in] scoreCell pointer to a ScoreCell
    @param[in] behavior pointer to a CellChange, nullptr if not given

    Sets the score pattern at a given index. If ScoreCell is set to nullptr then nothing is played, If CellChange is set to nullptr the sequencing is stopped after playing this Pattern. Sequence is a subclass of ScoreCell easiear to manage.
    */ 
    void setCell( int index, ScoreCell* scoreCell, CellChange* behavior = nullptr );


    /*!
    @brief Sets the CellChange that determine what cell will be played after this
    @param[in] index index of the patter to set inside the ScoreSection. If the size is too little the ScoreSection is automatically resized.
    @param[in] behavior pointer to a CellChange

    */ 
    void setChange( int index, CellChange* behavior );
    
    
    /*!
    @brief Sets some values for the pattern execution, Thread-safe.
    @param[in] index index of the patter to set inside the ScoreSection. This has to be a valid index.
    @param[in] length length passed to ScoreCell when the messages are generated
    @param[in] quantizeLaunch if true the next pattern launch is quantized to the bars, if false is executed when the given length expires. 
    @param[in] quantizeGrid if the launch is quantized this is the grid division for quantizing, in bars.

    Sets the score pattern timing options.
    */     
    void setCellTiming(int index, double length, bool quantizeLaunch = true, double quantizeGrid = 1.0f );
    
    
    /*!
    @brief Immediately launch the selected pattern, with the given launch timings options. Thread-safe. Also execute the launched cell's prepareScore() routine immediately.
    @param[in] index index of the patter to set inside the ScoreSection. This has to be a valid index. A negative index stops the playing of this section (you can perform quantized stopping if quantizeLaunch = true ).
    @param[in] legato if true the launched pattern playhead does not start from 0.0f but take the value from the already playing pattern. If not given as argument, it is assumed false.
    @param[in] quantizeLaunch if true the next pattern launch is quantized to the bars, if false the pattern is lanched as soon as possible.  If not given as argument, it is assumed false.
    @param[in] quantizeGrid if the launch is quantized this is the grid division for quantizing, in bars.  If not given as argument, it is assumed 1.0 (one bar).
    
    */         
    void launchCell(int index, bool legato = false, bool quantizeLaunch=false, double quantizeGrid=1.0);
    
    
    /*!
    @brief Set the behavior of connected GateSequencer on ScoreCell changes, if true a trigger off is sent between ScoreCells. true is the standard behavior.
    @param[in] active activate if true, deactivate if false
    
    */  
    void clearOnChange(bool active);
    
    /*!
    @brief returns the current cell index. If you use it just for visualization you can consider it thread-safe.
    */ 
    int meter_current() const; 

    /*!
    @brief returns the next playing cell index. If you use it just for visualization you can consider it thread-safe.
    */ 
    int meter_next() const;     
    
    /*!
    @brief returns the playhead position in bars. Thread-safe.
    */ 
    float meter_playhead() const; 
    
private:
    void processSection(const double    &startPlayHead, 
                    const double    &endPlayHead, 
                    const double    &playHeadDifference, 
                    const double    &maxBars,
                    const double    &barsPerSample, 
                    const int       &bufferSize) noexcept;
                       
    

    void playScore(double const &range, double const &offset, const double &oneSlashBarsPerSample) noexcept;
    void allNoteOff(double const &offset, const double &oneSlashBarsPerSample) noexcept;
    void onSchedule() noexcept;
    void clearBuffers() noexcept;
    void processBuffersDestinations(const int &bufferSize) noexcept;
    
    std::vector<PatternStruct>     patterns;
    
    int                         scheduledPattern;
    double                      scheduledTime;
    
    int                         patternIndex;
    int                         patternSize;
    
    double                      scorePlayHead; //position inside the actual clip
    int                         scoreIndex;
  
    bool                        run;
    bool                        clear;
    bool                        quantizedLaunch;
    bool                        clearOnChangeFlag;
    bool                        legatoLaunch;
    
    
    std::vector<MessageBuffer>  outputs;
    //int                         outputSize;
    
    MessageBuffer*              selectedMessageBuffer;
    
    std::mutex                  patternMutex;
    
    std::atomic<int>            atomic_meter_current;
    std::atomic<int>            atomic_meter_next;
    std::atomic<float>          atomic_meter_playhead;
    
};// END ScoreSection

Patchable& linkSelectedOutputToSequencer (ScoreSection& scoreSection, Sequencer& input);
Patchable& operator>> (ScoreSection& scoreSection, Sequencer& input);

void linkSelectedOutputToExtSequencer (ScoreSection& scoreSection, ExtSequencer& ext);
void operator>> (ScoreSection& scoreSection, ExtSequencer& ext);

void unlinkSelectedOutputToExtSequencer (ScoreSection& scoreSection, ExtSequencer& ext);
void operator!= (ScoreSection& scoreSection, ExtSequencer& ext);

}//END NAMESPACE

#endif //PDSP_SCOREROW_H_INCLUDED
