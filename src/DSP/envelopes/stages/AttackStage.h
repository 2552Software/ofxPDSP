
// AttackStage.h
// ofxPDSP
// Nicola Pisanti, MIT License, 2016


#ifndef PDSP_ENVSTAGE_ATTACK_H_INCLUDED
#define PDSP_ENVSTAGE_ATTACK_H_INCLUDED

#include "EnvelopeStage.h"

namespace pdsp{
/*!
    @cond HIDDEN_SYMBOLS
*/
	class AttackStage : public virtual EnvelopeStage{
	
	public:

		AttackStage(){
			attackTimeMs = 50.0;
			attackTCO = 0.99999; //digital 
			calculateAttackTime();
		};

		void setAttackTCO(float attackTCO){
			this->attackTCO = attackTCO;
			calculateAttackTime();
		};
	

	protected:
		float attackTimeMs;
		float attackTCO;	//TCO set the curve
		float attackCoeff;
		float attackOffset;
	
		void setAttackTime(float attackTimeMs){
			this->attackTimeMs = attackTimeMs;
			calculateAttackTime();
		};

		void calculateAttackTime(){
			float samples = sampleRate * attackTimeMs*0.001f;
			attackCoeff = exp(-log((1.0f + attackTCO) / attackTCO) / samples);
			attackOffset = (1.0f + attackTCO) * (1.0f - attackCoeff);
		}

		inline_f void Attack(int& stageSwitch, int nextStageId){

			envelopeOutput = attackOffset + envelopeOutput*attackCoeff;
			if (envelopeOutput > intensity || attackTimeMs <= 0.0f){
				//envelopeOutput = intensity;  //decativated for changint intensity
				stageSwitch = nextStageId;
			}

		}

		virtual ~AttackStage(){};
	};


/*!
    @endcond
*/

} // pdsp namespace end

#endif  // PDSP_ENVSTAGE_ATTACK_H_INCLUDED
