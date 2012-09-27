/***************************************************************************
 *                           SinState.h                                    *
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

#ifndef SINSTATE_H_
#define SINSTATE_H_

#include "ConnectionState.h"

/*!
 * \file SinState.h
 *
 * \author Jesus Garrido
 * \date October 2011
 *
 * This file declares a class which abstracts the current state of a synaptic connection
 * with Sinusoidal learning rule.
 */

/*!
 * \class SinState
 *
 * \brief Synaptic connection current state.
 *
 * This class abstracts the state of a synaptic connection including Sinusoidal learning rule and defines the state variables of
 * that connection. The kernel function is f(t) = exp(-t/tau)*Sin(-t/tau)^exponent, where t represents the time since
 * the last presynaptic spike reached the cell.
 *
 * \author Jesus Garrido
 * \date October 2011
 */

class SinState : public ConnectionState{

	private:
		/*!
		 * Precalculated terms.
		 */
		const static float terms [11][11];

		/*!
		 * Is the LUT initialized?
		 */
		static bool InitializedLUT;

		/*!
		 * Precalculated sin terms.
		 */
		static float SinLUT[];

		/*!
		 * Precalculated LUT Step.
		 */
		static float LUTStep;

		/*!
		 * The exponent of the sinuidal function.
		 */
		unsigned int exponent;

		/*!
		 * Time of the maximum response rate.
		 */
		float maxpos;

		/*!
		 * Tau constant of the learning rule.
		 */
		float tau;

		/*!
		 * Corrective factor to adjust the maximum to 1.
		 */
		float factor;

	public:

		/*!
		 * \brief Default constructor with parameters.
		 *
		 * It generates a new state of a connection.
		 *
		 * \param NewExponent The exponent of the sinusoidal function.
		 * \param MaxPosition Temporal position of the peak.
		 */
		SinState(int NewExponent, float NewMaxpos);

		/*!
		 * \brief Class destructor.
		 *
		 * It destroys an object of this class.
		 */
		virtual ~SinState();

		/*!
		 * \brief It gets the value of the accumulated presynaptic activity.
		 *
		 * It gets the value of the accumulated presynaptic activity.
		 *
		 * \return The accumulated presynaptic activity.
		 */
		virtual float GetPresynapticActivity();

		/*!
		 * \brief It gets the value of the accumulated postsynaptic activity.
		 *
		 * It gets the value of the accumulated postsynaptic activity.
		 *
		 * \return The accumulated postsynaptic activity.
		 */
		virtual float GetPostsynapticActivity();


		/*!
		 * \brief It gets the number of variables that you can print in this state.
		 *
		 * It gets the number of variables that you can print in this state.
		 *
		 * \return The number of variables that you can print in this state.
		 */
		virtual unsigned int GetNumberOfPrintableValues();

		/*!
		 * \brief It gets a value to be printed from this state.
		 *
		 * It gets a value to be printed from this state.
		 *
		 * \return The value at position-th position in this state.
		 */
		virtual double GetPrintableValuesAt(unsigned int position);


		/*!
		 * \brief Add elapsed time to spikes.
		 *
		 * It adds the elapsed time to spikes.
		 *
		 * \param ElapsedTime The time since the last update.
		 */
		virtual void AddElapsedTime(float ElapsedTime);


		/*!
		 * \brief It implements the behaviour when it transmits a spike.
		 *
		 * It implements the behaviour when it transmits a spike. It must be implemented
		 * by any inherited class.
		 */
		virtual void ApplyPresynapticSpike();

		/*!
		 * \brief It implements the behaviour when the target cell fires a spike.
		 *
		 * It implements the behaviour when it the target cell fires a spike. It must be implemented
		 * by any inherited class.
		 */
		virtual void ApplyPostsynapticSpike();

};

#endif /* NEURONSTATE_H_ */

