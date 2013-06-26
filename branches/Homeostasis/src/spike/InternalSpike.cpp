/***************************************************************************
 *                           InternalSpike.cpp                             *
 *                           -------------------                           *
 * copyright            : (C) 2009 by Jesus Garrido and Richard Carrillo   *
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

#include "../../include/spike/InternalSpike.h"
#include "../../include/spike/Interconnection.h"

#include "../../include/spike/Neuron.h"
#include "../../include/spike/PropagatedSpike.h"

#include "../../include/neuron_model/VectorNeuronState.h"
#include "../../include/neuron_model/NeuronModel.h"
#include "../../include/neuron_model/EventDrivenNeuronModel.h"

#include "../../include/simulation/Simulation.h"
#include "../../include/simulation/EventQueue.h"

#include "../../include/communication/OutputSpikeDriver.h"

#include "../../include/learning_rules/LearningRule.h"

InternalSpike::InternalSpike():Spike() {
}
   	
InternalSpike::InternalSpike(double NewTime, Neuron * NewSource): Spike(NewTime,NewSource){
}
   		
InternalSpike::~InternalSpike(){
}

void InternalSpike::ProcessEvent(Simulation * CurrentSimulation){
	
	Neuron * neuron=this->source;  // source of the spike
	
	if (neuron->GetNeuronModel()->GetModelType() == EVENT_DRIVEN_MODEL){
		EventDrivenNeuronModel * Model = (EventDrivenNeuronModel *) neuron->GetNeuronModel();
		if(!Model->DiscardSpike(this)){
			// Add the spike to simulation spike counter
			long int Spikes = CurrentSimulation->GetTotalSpikeCounter();
			CurrentSimulation->SetTotalSpikeCounter(Spikes+1);

			neuron->GetVectorNeuronState()->NewFiredSpike(neuron->GetIndex_VectorNeuronState());
			// If it is a valid spike (not discard), generate the next spike in this cell.
			InternalSpike * NextSpike = Model->GenerateNextSpike(this);

			if (NextSpike!=0){
				CurrentSimulation->GetQueue()->InsertEvent(NextSpike);
			}
			
			CurrentSimulation->WriteSpike(this);

			Neuron * Cell = this->GetSource();
			if (Cell->IsMonitored()){
				CurrentSimulation->WriteState(neuron->GetVectorNeuronState()->GetLastUpdateTime(neuron->GetIndex_VectorNeuronState()), Cell);
			}

			// Generate the output activity
			if (neuron->IsOutputConnected()){
				PropagatedSpike * spike = new PropagatedSpike(this->GetTime() + neuron->GetOutputConnectionAt(0)->GetDelay(), neuron, 0);
				CurrentSimulation->GetQueue()->InsertEvent(spike);
			}

			for (int i=0; i<neuron->GetInputNumberWithLearning(); ++i){
				Interconnection * inter = neuron->GetInputConnectionWithLearningAt(i);
				LearningRule * Plasticity = inter->GetWeightChange();
				Plasticity->ApplyPostSynapticSpike(inter,this->time);
			}
		}
	} else { // Time-driven model (no check nor update needed
		// Add the spike to simulation spike counter
		long int Spikes = CurrentSimulation->GetTotalSpikeCounter();
		CurrentSimulation->SetTotalSpikeCounter(Spikes+1);

		CurrentSimulation->WriteSpike(this);
		
		Neuron * Cell = this->GetSource();
		if (Cell->IsMonitored()){
			CurrentSimulation->WriteState(neuron->GetVectorNeuronState()->GetLastUpdateTime(neuron->GetIndex_VectorNeuronState()), Cell);
		}
		
		// Generate the output activity
		if (neuron->IsOutputConnected()){
			PropagatedSpike * spike = new PropagatedSpike(this->GetTime() + neuron->GetOutputConnectionAt(0)->GetDelay(), neuron, 0);
			CurrentSimulation->GetQueue()->InsertEvent(spike);
			//spike->ProcessEvent(CurrentSimulation);
			//delete spike;
		}

		for (int i=0; i<neuron->GetInputNumberWithLearning(); ++i){
			Interconnection * inter = neuron->GetInputConnectionWithLearningAt(i);
			LearningRule * Plasticity = inter->GetWeightChange();
			Plasticity->ApplyPostSynapticSpike(inter,this->time);
		}
		
	}
}

   	