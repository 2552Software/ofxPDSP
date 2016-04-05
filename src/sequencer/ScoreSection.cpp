
#include "ScoreSection.h"
#include <iostream>
#include <float.h>
#include <limits.h>


pdsp::ScoreSection::ScoreSection() {
    
    patterns.clear();
    scheduledPattern = -1;
    scheduledTime = std::numeric_limits<double>::infinity();
    patternIndex = -1;
    patternSize = 0;
    scorePlayHead = 0.0;
    scoreIndex = 0;
    clearOnChangeFlag = true;
    run = false; //change to false in definitive version
    clear = true;
    quantizedLaunch = false;
    selectedMessageBuffer = nullptr;

    atomic_meter_current.store(-1);
    atomic_meter_next.store(-1);
    atomic_meter_playhead.store(0.0f);    

    setOutputsNumber(1);    
    
}


pdsp::ScoreSection::ScoreSection(const pdsp::ScoreSection &other) {
    
    resizePatterns( (int) other.patterns.size() );
    
    for(int i=0; i< (int) other.patterns.size(); ++i){
        patterns[i].scoreCell          = other.patterns[i].scoreCell;
        patterns[i].nextCell           = other.patterns[i].nextCell;
        patterns[i].length             = other.patterns[i].length;
        patterns[i].quantizeLaunch     = other.patterns[i].quantizeLaunch;
        patterns[i].quantizeGrid       = other.patterns[i].quantizeGrid;
    } 
    
    this->setOutputsNumber( (int)other.outputs.size() );
}


void pdsp::ScoreSection::resizePatterns(int size){
    if(size>0){
        if(size<patternSize){
            if(patternIndex>=size){
                patternIndex = size-1;
                if(patternIndex<0) patternIndex = 0;
            }
        }
        patterns.resize(size);
        patternSize = size;       
    }
}

void pdsp::ScoreSection::setOutputsNumber(int size){
    outputs.resize(size);
    for(MessageBuffer &buffer : outputs){
        buffer.reserve(PDSP_SCORESECTIONMESSAGERESERVE);
    }
}

void pdsp::ScoreSection::launchCell( int index, bool legato, bool quantizeLaunch, double quantizeGrid ){
    if     ( index < 0        ) { index = -1; }
    else if( index >= patternSize ) { index = patternSize-1; }

    patternMutex.lock();
        if(quantizeLaunch){
            this->quantizedLaunch = true;
            scheduledTime = -quantizeGrid;
        }else{
            scheduledTime = -1.0;
        }
        if(legato){ legatoLaunch = true; }
        scheduledPattern = index;
        atomic_meter_next.store(index);
        if( index!=-1 && patterns[index].scoreCell!=nullptr){
            patterns[index].scoreCell->executePrepareScore(patterns[index].length);
        }
    patternMutex.unlock();
}

pdsp::ScoreSection& pdsp::ScoreSection::out(int index){
    if(index>=0 && index <outputs.size() ){
        selectedMessageBuffer = &outputs[index];
        return *this;
    }else{
        return *this;
    }
}

void pdsp::ScoreSection::setCell( int index, ScoreCell* scoreCell, CellChange* behavior ){
    if(index>=0){
        if(index>=patternSize){
            resizePatterns(index+1);
            //all the new patterns are initialized with nullptr, nullptr, length=1.0, quantize = false and quantizeGrid=0.0
        }
        patternMutex.lock();
            patterns[index].scoreCell = scoreCell;
            patterns[index].nextCell  = behavior;
        patternMutex.unlock();
    }
}

void pdsp::ScoreSection::setPattern( int index, ScoreCell* scoreCell, CellChange* behavior ){
    setCell(index, scoreCell, behavior);
}

void pdsp::ScoreSection::setChange( int index, CellChange* behavior ){
    if(index>=0 && index < patternSize){
        patternMutex.lock();
            patterns[index].nextCell  = behavior;
        patternMutex.unlock();
    }
}

void pdsp::ScoreSection::setBehavior( int index, CellChange* behavior ){
    setChange(index, behavior);
}

//remember to check for quantizeGrid to be no greater than pattern length
void pdsp::ScoreSection::setCellTiming( int index, double length, bool quantizeLaunch, double quantizeGrid ){
    if(index>=0){
        if(index>=patternSize){
            resizePatterns(index+1);
            //all the new patterns are initialized with nullptr, nullptr, length=1.0, quantize = false and quantizeGrid=0.0
        }
        patternMutex.lock();
            patterns[index].length             = length;
            patterns[index].quantizeLaunch     = quantizeLaunch;
            patterns[index].quantizeGrid       = quantizeGrid;   
        patternMutex.unlock();
    }   
}



void pdsp::ScoreSection::processSection(const double &startPlayHead, 
                                const double &endPlayHead, 
                                const double &playHeadDifference, 
                                const double &maxBars,
                                const double &barsPerSample, 
                                const int &bufferSize) noexcept {
    
    patternMutex.lock();
        if(scheduledTime >= maxBars+playHeadDifference){ scheduledTime -= maxBars; } //wraps scheduled time around
        
        if(scheduledTime<0.0){ //launching scoreCell
            if(quantizedLaunch && startPlayHead!=0.0){
                double quantizeTime = - scheduledTime;
                double timeToQuantize = startPlayHead + quantizeTime;
                int rounded = static_cast<int> ( timeToQuantize / quantizeTime ); 
                scheduledTime = static_cast<double>(rounded) * quantizeTime ;
            }else{
                scheduledTime = startPlayHead;
            }
            run = true;
        } 

        if( run && patternSize>0 ){
            clearBuffers();
            double oneSlashBarsPerSample = 1.0 / barsPerSample;

            if(scheduledTime >= endPlayHead){   //more likely
                if(patternIndex!=-1 && patterns[patternIndex].scoreCell!=nullptr) playScore(playHeadDifference, 0.0, oneSlashBarsPerSample);
                
            }else if(scheduledTime <= startPlayHead){ 
                //std::cout<<"scheduled now, playhead start = "<<startPlayHead<<"--------------------\n";
                onSchedule();
                if(clearOnChangeFlag) allNoteOff(0.0, oneSlashBarsPerSample); 
                if(patternIndex!=-1 && patterns[patternIndex].scoreCell!=nullptr) playScore(playHeadDifference, 0.0, oneSlashBarsPerSample);

            }else{

                double schedulePoint = scheduledTime - startPlayHead;
                //std::cout<<"scheduled between, playhead start = "<<startPlayHead<<", schedule point = "<<schedulePoint<<"----------------\n";
                if(patternIndex!=-1 && patterns[patternIndex].scoreCell!=nullptr) playScore(schedulePoint, 0.0, oneSlashBarsPerSample); //process old clip
                onSchedule();
                if(clearOnChangeFlag) allNoteOff(schedulePoint, oneSlashBarsPerSample);
                //std::cout<<"playing remaining, playhead start = "<<startPlayHead<<"----------------\n";
                if(patternIndex!=-1 && patterns[patternIndex].scoreCell!=nullptr) playScore(playHeadDifference, schedulePoint, oneSlashBarsPerSample);//process new clip
                //std::cout<<"end processing, playhead start = "<<startPlayHead<<"----------------\n";

            }
            processBuffersDestinations(bufferSize);
            
        }else if(clear==true){
            //empty message buffers and process destinations
            clearBuffers();
            processBuffersDestinations(bufferSize);
            clear = false;
        }

        atomic_meter_playhead.store(scorePlayHead);   
    patternMutex.unlock();
    
}

//-----------------------------------------------INTERNAL FUNCTIONS---------------------------------------------

//range is the quantity in bars of score to be elaborated, offset displace the start of processing
void pdsp::ScoreSection::playScore(double const &range, double const &offset, const double &oneSlashBarsPerSample) noexcept{

    ScoreCell* patternToProcess = patterns[patternIndex].scoreCell;
        
    double patternMax = scorePlayHead + range - offset;
    
    while( (scoreIndex < patternToProcess->score.size()) && (patternToProcess->score[scoreIndex].time < patternMax) ){
        
        if( (patternToProcess->score[scoreIndex].time >= scorePlayHead) && (patternToProcess->score[scoreIndex].lane < (int)outputs.size() )   ){ //check if we are inside the outputs boundaries and inside the processed time
            int sample = static_cast<int>( (patternToProcess->score[scoreIndex].time - scorePlayHead + offset) * oneSlashBarsPerSample);
            outputs[patternToProcess->score[scoreIndex].lane].addMessage(patternToProcess->score[scoreIndex].value, sample);
            //std::cout<<"added message with sample value:"<<sample<<"\n";
        }
        
        scoreIndex++;
        
    }
    
    scorePlayHead = patternMax;
}

void pdsp::ScoreSection::onSchedule() noexcept{
    
    //clear score--------------------------------------------------------------------------
    //score.clear(); //clear score buffer
    scoreIndex = 0;
    
    //clip change routines--------------------------------------------------------------------------
    patternIndex = scheduledPattern; 

    if( patternIndex >=0 && patterns[patternIndex].scoreCell!=nullptr){ //if there is a pattern, execute it's generative routine
        patterns[patternIndex].scoreCell->executeGenerateScore( patterns[patternIndex].length );
        
        atomic_meter_current.store(patternIndex);
    }else{
        atomic_meter_current.store(-1);
        run = false;
        clear = true;
    }

    if( patternIndex >=0){
            
        if(patterns[patternIndex].nextCell!=nullptr){ //we have a behavior to get next pattern
            scheduledPattern = patterns[patternIndex].nextCell->getNextPattern(patternIndex, patternSize);
            atomic_meter_next.store(scheduledPattern);

            if(scheduledPattern<0 ){                scheduledPattern=-1; }
            else if( scheduledPattern>=patternSize ){  scheduledPattern = patternSize-1; }
            
            if( scheduledPattern!=-1 ){
                if( patterns[scheduledPattern].scoreCell!=nullptr) {
                    patterns[scheduledPattern].scoreCell->executePrepareScore(patterns[scheduledPattern].length);
                }
                
                if( patterns[scheduledPattern].quantizeLaunch ){
                    double timeToQuantize = (scheduledTime + patterns[scheduledPattern].quantizeGrid);
                    int rounded = static_cast<int> ( timeToQuantize /  patterns[scheduledPattern].quantizeGrid ); 
                    scheduledTime = static_cast<double>(rounded) * patterns[scheduledPattern].quantizeGrid ;
                }else{
                    scheduledTime = scheduledTime + patterns[patternIndex].length; //+ patterns[scheduledPattern].quantizeGrid;
                }                
            }else{
                scheduledTime = scheduledTime + patterns[patternIndex].length; 
            }
            
        }else{ //we don't have a behavior to get a next pattern --------> STOPPING ROW AFTER EXECUTION
            atomic_meter_next.store(-1);        
            //scheduledPattern = patternIndex;
            //scheduledTime = std::numeric_limits<double>::infinity();
            //run = false;
            //clear = true;
            scheduledTime = scheduledTime + patterns[patternIndex].length;
            scheduledPattern = -1;
        }        
        
    }

    //reset score playhead
    if (legatoLaunch) {
        legatoLaunch = false; // automatic pattern launch is never legato
    }else{
        // SET THIS TO START_TIME <-----------------------------------------------------------------------
        scorePlayHead = 0.0; // reset pattern playhead to zero
    }

}

void pdsp::ScoreSection::allNoteOff(double const &offset, const double &oneSlashBarsPerSample) noexcept{
    
    int sample = static_cast<int>( offset * oneSlashBarsPerSample);
    //std::cout<<"clearing with sample value:"<<sample<<"\n";
    
    for(MessageBuffer &buffer : outputs){
        if(buffer.connectedToGate){
            buffer.addMessage(0.0f, sample);
        }
    }
}

void pdsp::ScoreSection::clearBuffers() noexcept {
    for(MessageBuffer &buffer : outputs){
        buffer.clearMessages();
    }
}

void pdsp::ScoreSection::processBuffersDestinations(const int &bufferSize) noexcept {
    for(MessageBuffer &buffer : outputs){
        buffer.processDestination(bufferSize);
    }
}

int pdsp::ScoreSection::meter_current() const {
    return atomic_meter_current.load();
}

int pdsp::ScoreSection::meter_next() const {
    return atomic_meter_next.load();
}

float pdsp::ScoreSection::meter_playhead() const {
    return atomic_meter_playhead.load(); 
}

void pdsp::ScoreSection::clearOnChange(bool active) {
    clearOnChangeFlag = active;
}


//---------------------------------------------------------------------------------------------

pdsp::Patchable& pdsp::linkSelectedOutputToSequencer (pdsp::ScoreSection& scoreSection, pdsp::Sequencer& input){
    *(scoreSection.selectedMessageBuffer) >> input;
    scoreSection.selectedMessageBuffer = &scoreSection.outputs[0];
    return input;
}

pdsp::Patchable& pdsp::operator>> (pdsp::ScoreSection& scoreSection, pdsp::Sequencer& input){
    return linkSelectedOutputToSequencer(scoreSection, input); 
}

void pdsp::linkSelectedOutputToExtSequencer (pdsp::ScoreSection& scoreSection, pdsp::ExtSequencer& ext){
    ext.linkToMessageBuffer( *(scoreSection.selectedMessageBuffer) );
    scoreSection.selectedMessageBuffer = &scoreSection.outputs[0]; //reset selected out to default
}

void pdsp::operator>> (pdsp::ScoreSection& scoreSection, pdsp::ExtSequencer& ext){
    linkSelectedOutputToExtSequencer(scoreSection, ext);
}

void pdsp::unlinkSelectedOutputToExtSequencer (pdsp::ScoreSection& scoreSection, pdsp::ExtSequencer& ext){
    ext.unlinkMessageBuffer( *(scoreSection.selectedMessageBuffer) );
    scoreSection.selectedMessageBuffer = &scoreSection.outputs[0]; //reset selected out to default
}

void pdsp::operator!= (pdsp::ScoreSection& scoreSection, pdsp::ExtSequencer& ext){
    unlinkSelectedOutputToExtSequencer(scoreSection, ext);
}
