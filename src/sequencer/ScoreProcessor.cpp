
#include "ScoreProcessor.h"



pdsp::ScoreProcessor::ScoreProcessor(){
    
    //synchronizeClockable = true;
    tempo = 120.0;
    playHead = 0.0;
    playHeadEnd = 0.0;
    maxBars = 32000.0;
    
    clearToken = 0;
    
    playhead_meter.store(0.0f);
    
    sections.clear();
    sections.reserve(16);
    sections.clear();
    
    playing.store(false);
    
    if(dynamicConstruction){
        prepareToPlay(globalBufferSize, globalSampleRate);
    }
}

void pdsp::ScoreProcessor::setSections(int sectionsNumber){
    sections.resize(sectionsNumber);
}

void pdsp::ScoreProcessor::setMaxBars(double maxBars){
    this->maxBars = maxBars;
    while(playHead > maxBars){
        playHead -= maxBars;
    }
}

//void pdsp::ScoreProcessor::synchronizeInterface(bool active){
//    synchronizeClockable = active;
//}

void pdsp::ScoreProcessor::process(int const &bufferSize) noexcept{
   
    if( playing.load() ){
        playheadMutex.lock();
            playHead = playHeadEnd;
            if( playHead > maxBars ) { playHead -= maxBars; } //wrap score
            double playHeadDifference = bufferSize * barsPerSample;
            playHeadEnd = playHead + playHeadDifference;
        playheadMutex.unlock();
        
        playhead_meter.store(playHead);
        
        //now process sections-----------------
        for(ScoreSection &sect : sections){
            sect.processSection(playHead, playHeadEnd, playHeadDifference, maxBars, barsPerSample, bufferSize);
        }
        //---------------------------------

        //if(synchronizeClockable){
            Clockable::globalBarPosition = playHead;
            Clockable::playing = true;
        //}
        
    }else{
        // on pause we send trigger offs to gates and set all the sequencers to control rate
        if(clearToken>0){
            for(ScoreSection &sect : sections){
                sect.clearBuffers();
                if(clearToken==2){ sect.allNoteOff(0.0, 0.5); } //0.5 * 0.0 = 0.0 the message will be on the first sample
                sect.processBuffersDestinations(bufferSize);
            } 
            clearToken--;
        }
    }
}

void pdsp::ScoreProcessor::prepareToPlay( int expectedBufferSize, double sampleRate ){
    this->sampleRate = sampleRate;
    setTempo(tempo);
}


void pdsp::ScoreProcessor::releaseResources() {}


void pdsp::ScoreProcessor::setTempo( float tempo ){
    this->tempo = tempo;
    barsPerSample = static_cast<double>(tempo)  / ((60.0 * 4.0) * sampleRate )  ;
    
    //if(synchronizeClockable){
        Clockable::setTempo(tempo);
    //}
    
}


void pdsp::ScoreProcessor::setPlayhead(double newPlayhead){
    playheadMutex.lock();
        playHeadEnd = newPlayhead;
        playhead_meter.store(0.0);
    playheadMutex.unlock();
}

void pdsp::ScoreProcessor::pause(){
    clearToken = 2;
    playing.store(false);
    Clockable::playing = false;
}


void pdsp::ScoreProcessor::stop(){
    clearToken = 2;
    playing.store(false);
    Clockable::playing = false;
    setPlayhead(0.0);

    for(ScoreSection &sect : sections){
        sect.scoreIndex = 0;
        //double oldPlayhead = sect.scorePlayHead ;
        sect.scorePlayHead = 0.0;
        //sect.scheduledTime -= oldPlayhead;
        sect.atomic_meter_playhead.store(0.0f);
        sect.launchCell(sect.patternIndex);
    } 
}    


void pdsp::ScoreProcessor::play(){
    playing.store(true);
    Clockable::playing = true;    
}

float pdsp::ScoreProcessor::meter_playhead(){
    return playhead_meter.load();
}

bool pdsp::ScoreProcessor::isPlaying(){
    return playing.load();
}
