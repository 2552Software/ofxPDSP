#include "ofApp.h"

// channel() API example
// some modules have an channel( int size ) method 
// that let you use that module with multiple signal inputs
// it's really useful for example for using a single pdsp::ParameterAmp 
// to control multiple modulation amounts
// or for using a filter for processing stereo channels
// modules with a channel() method also let you access different signal in/out
// with the [] operator
//
// in this example a single pdsp::ParameterAmp with multiple channels
// controls the amount of individual LFOs going to individual oscillators
// and a single pdsp::VAFilter is used to filter the voices panned in stereo

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofBackground(0);
    
    //-------------- PATCHING ----------------------
    oscillators.resize(4);
    pitches.resize( oscillators.size() );
    drift_lfo.resize( oscillators.size() );
    drift_amt.channels( oscillators.size() ); // a channel for each 
    filter.channels(2); // stereo in/out
    
    for(size_t i=0; i<oscillators.size(); ++i){
        float pan = pdsp::spread( i, oscillators.size(), 0.5f );
        // [] operator let you access signal channels of the filter
        oscillators[i].out_saw() * pdsp::panL(pan) >> filter[0]; 
        oscillators[i].out_saw() * pdsp::panR(pan) >> filter[1]; 
        
        // [] operator let you access signal channels of the drift_amt module
        1.5f >> drift_lfo[i].out_random() >> drift_amt[i] >> oscillators[i].in_pitch();     
                                               pitches[i] >> oscillators[i].in_pitch();
    }
    
    // the control input is the same for al the filter channels
    cutoff_ctrl.enableSmoothing(100.0f); // 100ms smoothing
    cutoff_ctrl >> filter.in_cutoff(); 
    reso_ctrl >> filter.in_reso(); 
    
    filter[0] * dB(-12.0f) >> engine.audio_out(0);
    filter[1] * dB(-12.0f) >> engine.audio_out(1);
        
    // --------------- GUI -------------------------- 
    gui.setup("gui", "settings.xml", 20, 20);
    
    for(size_t i=0; i<pitches.size(); ++i){
        gui.add(pitches[i].set("pitch "+ofToString(i)+" coarse", 36 + 5*i, 24, 96) );
        gui.add(pitches[i].set("pitch "+ofToString(i)+" fine", 0.0f, -0.5f, 0.5f) );
    }
    
    // the control parameter for drift_amt is the same for all the channels
    gui.add( drift_amt.set("oscillator drift", 0.0f, 0.0f, 3.0f) ); 
    gui.add( cutoff_ctrl.set("filter cutoff", 70, 20, 136) );
    gui.add( reso_ctrl.set("filter reso", 0.0f, 0.0f, 1.0f ) );
    
    //gui.loadFromFile("settings.xml");
    
    //------------SETUPS AND START AUDIO-------------
    engine.listDevices();
    engine.setDeviceID(0); // REMEMBER TO SET THIS AT THE RIGHT INDEX!!!!
    engine.setup( 44100, 512, 3); 
    
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
