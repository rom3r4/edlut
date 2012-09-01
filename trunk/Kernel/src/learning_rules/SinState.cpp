/***************************************************************************
 *                           SinState.cpp                                  *
 *                           -------------------                           *
 * copyright            : (C) 2011 by Jesus Garrido                        *
 * email                : jgarrido@atc.ugr.es                              *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../../include/learning_rules/SinState.h"

#include <cmath>

#define A 1./2.

#define TERMSLUT 1000 // +5000 terms in the LUT produces an error

const float SinState::terms[11][11]  = {{1,0,0,0,0,0,0,0,0,0,0},
	{A,-A,0,0,0,0,0,0,0,0,0},
	{3./2.*pow(A,2),-4./2.*pow(A,2),1./2.*pow(A,2),0,0,0,0,0,0,0,0},
	{10./4.*pow(A,3),-15./4.*pow(A,3),6./4.*pow(A,3),-1./4.*pow(A,3),0,0,0,0,0,0,0},
	{35./8.*pow(A,4),-56./8.*pow(A,4),28./8.*pow(A,4),-8./8.*pow(A,4),1./8.*pow(A,4),0,0,0,0,0,0},
	{126./16.*pow(A,5),-210./16.*pow(A,5),120./16.*pow(A,5),-45./16.*pow(A,5),10./16.*pow(A,5),-1./16.*pow(A,5),0,0,0,0,0},
	{231./16.*pow(A,6),-99./4.*pow(A,6),495./32.*pow(A,6),-55./8.*pow(A,6),66./32.*pow(A,6),-3./8.*pow(A,6),1./32.*pow(A,6),0,0,0,0},
	{429./16.*pow(A,7),-3003./64.*pow(A,7),1001./32.*pow(A,7),-1001./64.*pow(A,7),91./16.*pow(A,7),-91./64.*pow(A,7),7./32.*pow(A,7),-1./64.*pow(A,7),0,0,0},
	{6435./128.*pow(A,8),-715./8.*pow(A,8),1001./16.*pow(A,8),-273./8.*pow(A,8),455./32.*pow(A,8),-35./8.*pow(A,8),15./16.*pow(A,8),-1./8.*pow(A,8),1./128.*pow(A,8),0,0},
	{12155./128.*pow(A,9),-21879./128.*pow(A,9),1989./16.*pow(A,9),-4641./64.*pow(A,9),1071./32.*pow(A,9),-765./64.*pow(A,9),51./16.*pow(A,9),-153./256.*pow(A,9),9./128.*pow(A,9),-1./256.*pow(A,9),0},
	{46189./256.*pow(A,10),-20995./64.*pow(A,10),62985./256.*pow(A,10),-4845./32.*pow(A,10),4845./64.*pow(A,10),-969./32.*pow(A,10),4845./512.*pow(A,10),-285./128.*pow(A,10),95./256.*pow(A,10),-5./128.*pow(A,10),1./512.*pow(A,10)}};

bool SinState::InitializedLUT = false;

float SinState::LUTStep = 0;

float SinState::SinLUT[2*TERMSLUT];

SinState::SinState(int NewExponent, float NewMaxpos): ConnectionState(NewExponent+2), exponent(NewExponent), maxpos(NewMaxpos){
	for (int i=0; i<NewExponent+2; ++i){
		ConnectionState::SetStateVariableAt(i,0); // Initialize presynaptic activity
	}

	this->tau = this->maxpos/atan((float)exponent);
	this->factor = 1./(exp(-atan((float)this->exponent))*pow(sin(atan((float)this->exponent)),(int) this->exponent));

	if (this->tau==0){
		this->tau = 1e-6;
	}

	// Initialize LUT
	if (!this->InitializedLUT){
		this->InitializedLUT = true;
		
		double const Pi=4*atan(1.);

		this->LUTStep = 2*Pi/TERMSLUT;

		for (unsigned int i=0; i<TERMSLUT; ++i){
			this->SinLUT[2*i] = sinf(this->LUTStep*i);
			this->SinLUT[2*i+1] = cosf(this->LUTStep*i);
		}
	}
}

SinState::~SinState() {
}

float SinState::GetPresynapticActivity(){
	return this->GetStateVariableAt(0);
}

float SinState::GetPostsynapticActivity(){
	return 0;
}

unsigned int SinState::GetNumberOfPrintableValues(){
	return ConnectionState::GetNumberOfPrintableValues()+2;
}

double SinState::GetPrintableValuesAt(unsigned int position){
	if (position<ConnectionState::GetNumberOfPrintableValues()){
		return ConnectionState::GetStateVariableAt(position);
	} else if (position==ConnectionState::GetNumberOfPrintableValues()) {
		return this->exponent;
	} else if (position==ConnectionState::GetNumberOfPrintableValues()+1) {
		return this->maxpos;
	} else return -1;
}

void SinState::AddElapsedTime(float ElapsedTime){
	float ElapsedRelative = ElapsedTime/this->tau;
	
	float expon = exp(-ElapsedRelative);

	// Update the activity value

	float OldExpon = *(this->StateVars+1);

	unsigned int ExponenLine = this->exponent>>1;

	const float* TermPointer = this->terms[ExponenLine]; 

	float NewActivity = this->factor*OldExpon*(*(TermPointer++))*expon;

	float NewExpon = OldExpon * expon;

	*(this->StateVars+1) = NewExpon;

	for (unsigned int grade=2; grade<=this->exponent; grade+=2){
		
		float * StateVarCos = this->StateVars+grade;
		float * StateVarSin = StateVarCos+1;

		float OldVarCos = *StateVarCos;
		float OldVarSin = *StateVarSin;

		float SinPar = grade*ElapsedRelative;

		float * index = this->SinLUT+((int)(SinPar/this->LUTStep)%(TERMSLUT<<1));
		float SinVar = *index;
		float CosVar = *(index+1);
		
		float NewVarCos = (OldVarCos*CosVar-OldVarSin*SinVar)*expon;
		float NewVarSin = (OldVarSin*CosVar+OldVarCos*SinVar)*expon;

		NewActivity += this->factor*(NewVarCos*(*(TermPointer++)));

		
		/*if(spike){  // if spike, we need to increase the e1 variable
			NewVarCos += 1;
		}*/

		*StateVarCos = NewVarCos;
		*StateVarSin = NewVarSin;
	}

	this->SetStateVariableAt(0,NewActivity);

	this->SetLastUpdateTime(this->GetLastUpdateTime()+ElapsedTime);
}

void SinState::ApplyPresynapticSpike(){
	float OldExpon = this->GetStateVariableAt(1);

	this->SetStateVariableAt(1,OldExpon+1);

	for (unsigned int grade=2; grade<=this->exponent; grade+=2){
		float OldVarCos = this->GetStateVariableAt(grade);
		this->SetStateVariableAt(grade,OldVarCos+1);
	}
}

void SinState::ApplyPostsynapticSpike(){
	return;
}


