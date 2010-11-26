/***************************************************************************
 *                           EDLUTMatlab.cpp                               *
 *                           -------------------                           *
 * copyright            : (C) 2010 by Jesus Garrido                        *
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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <time.h>
#include <math.h> // TODO: maybe remove this

#include <iostream>

#include "mex.h"

#include "../include/simulation/ParamReader.h"
#include "../include/simulation/Simulation.h"

#include "../include/simulation/ParameterException.h"

#include "../include/communication/ConnectionException.h"

#include "../include/spike/EDLUTFileException.h"
#include "../include/spike/EDLUTException.h"
#include "../include/spike/Network.h"


using namespace std;

/*!
 * 
 * \note Input arguments in mex file:
 * 		1. Simulation time (-time kernel parameter).
 * 		2. Network File (-nf kernel parameter).
 * 		3. Weights File (-wf kernel parameter).
 * 		4. Input File (-if kernel parameter).
 * 		5. Log File (-log kernel parameter).
 *
 * 
 * \note Obligatory parameters:
 * 			-time Simulation_Time(in_seconds) It sets the total simulation time.
 * 			-nf Network_File	It sets the network description file.
 * 			-wf Weights_File	It sets the weights file.
 * 			
 * \note  parameters:
 * 			-info 	It shows the network information.
 * 			-sf File_Name	It saves the final weights in file File_Name.
 * 			-wt Save_Weight_Step	It sets the step time between weights saving.
 * 			-st Step_Time(in_seconds) It sets the step time in simulation.
 * 			-log File_Name	It saves the activity register in file File_Name.
 * 			-if Input_File	It adds the Input_File file in the input sources of the simulation.
 * 			-of Output_File	It adds the Output_File file in the output targets of the simulation.
 * 			-ic IPDirection:Port Server|Client	It adds the connection as a server or a client in the specified address in the input sources of the simulation.
 * 			-oc IPDirection:Port Server|Client	It adds the connection as a server or a client in the specified address in the output targets of the simulation.	 
 * 			-ioc IPDirection:Port Server|Client	It adds the connection as a server or a client in the specified address in the input sources and in the output targets of the simulation.	 
 * 
 */

extern void _main();

const int numInputArgs  = 5;
const int numOutputArgs = 1;

// Function declarations.
// -----------------------------------------------------------------
double  getMatlabScalar    (const mxArray* ptr);
double& createMatlabScalar (mxArray*& ptr);

// Function definitions.
// -----------------------------------------------------------------
void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	// Check to see if we have the correct number of input and output
	// arguments.
	if (nrhs != numInputArgs)
		mexErrMsgTxt("Incorrect number of input arguments");

	if (nlhs != numOutputArgs)
		mexErrMsgTxt("Incorrect number of output arguments");

	// Get the inputs.
	double x  = getMatlabScalar(prhs[0]);
	double mu = getMatlabScalar(prhs[1]);
	double v  = getMatlabScalar(prhs[2]);

	// Create the output. It is also a double-precision scalar.
	double& p = createMatlabScalar(plhs[0]);

	// Compute the value of the univariate Normal at x.
	p = exp(-(x-mu)*(x-mu)/(2*v)) / sqrt(2*pi*v);
}

double getMatlabScalar (const mxArray* ptr) {

	// Make sure the input argument is a scalar in double-precision.
	if (!mxIsDouble(ptr) || mxGetNumberOfElements(ptr) != 1)
		mexErrMsgTxt("The input argument must be a double-precision scalar");

	return *mxGetPr(ptr);
}

double& createMatlabScalar (mxArray*& ptr) {
	ptr = mxCreateDoubleMatrix(1,1,mxREAL);
	return *mxGetPr(ptr);
}

int main(int ac, char *av[]) {
   
	clock_t startt,endt;
	cout << "Loading tables..." << endl;

	srand ( time(NULL) );
   
	try {
   		ParamReader Reader(ac, av);
			
		Simulation Simul(Reader.GetNetworkFile(), Reader.GetWeightsFile(), Reader.GetSimulationTime(), Reader.GetSimulationStepTime());
		for (unsigned int i=0; i<Reader.GetInputSpikeDrivers().size(); ++i){
			Simul.AddInputSpikeDriver(Reader.GetInputSpikeDrivers()[i]);
		}
			
		for (unsigned int i=0; i<Reader.GetOutputSpikeDrivers().size(); ++i){
			Simul.AddOutputSpikeDriver(Reader.GetOutputSpikeDrivers()[i]);
		}
		
		for (unsigned int i=0; i<Reader.GetMonitorDrivers().size(); ++i){
			Simul.AddMonitorActivityDriver(Reader.GetMonitorDrivers()[i]);
		}
		
		for (unsigned int i=0; i<Reader.GetOutputWeightDrivers().size(); ++i){
			Simul.AddOutputWeightDriver(Reader.GetOutputWeightDrivers()[i]);
		}
		Simul.SetSaveStep(Reader.GetSaveWeightStepTime());
					
		if(Reader.CheckInfo()){
			//Simul.GetNetwork()->tables_info();
			//neutypes_info();
			Simul.GetNetwork()->NetInfo();
		}
			
		cout << "Simulating network..." << endl;
		
		startt=clock();
		Simul.RunSimulation();
		endt=clock();
         
		cout << "Oky doky" << endl;     

		cout << "Elapsed time: " << (endt-startt)/(float)CLOCKS_PER_SEC << " sec" << endl;
		cout << "Number of updates: " << Simul.GetSimulationUpdates() << endl;
		cout << "Mean number of spikes in heap: " << Simul.GetHeapAcumSize()/(float)Simul.GetSimulationUpdates() << endl;
		cout << "Updates per second: " << Simul.GetSimulationUpdates()/((endt-startt)/(float)CLOCKS_PER_SEC) << endl;
	} catch (ParameterException Exc){
		cerr << Exc << endl;
		cerr << av[0] << " -time Simulation_Time -nf Network_File -wf Weights_File";
		cerr << " [-info] [-sf Final_Weights_File] [-wt Save_Weight_Step] [-st Simulation_Step_Time] [-log Activity_Register_File] [-if Input_File] ";
		cerr << "[-ic IPAddress:Port Server|Client] [-of Output_File] [-oc IPAddress:Port Server|Client] [-ioc IPAddress:Port Server|Client]" << endl;	
	} catch (ConnectionException Exc){
		cerr << Exc << endl;
		return 1;
	} catch (EDLUTFileException Exc){
		cerr << Exc << endl;
		return Exc.GetErrorNum();
	} catch (EDLUTException Exc){
		cerr << Exc << endl;
		return Exc.GetErrorNum();
	}
	return 0;
}
