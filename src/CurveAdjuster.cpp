/*! \file CurveAdjuster.cpp
 * 	\brief This file contains the definitions of several methods of the class _CurveAdjuster_.
 * 	\author Mauro Diamantino
 */

#include "SweepProcessing.h"

/*!	The interpolation and extrapolation of the curve are performed in this method, to get
 *	the set of linear functions in the frequency domain which model the given the curve.
 *	\param [in] curve The frequency curve to be adjusted.
 */
void CurveAdjuster::BuildLines(const FreqValues & curve)
{
	LinearFunction auxLine;
	lines.clear();
	auto itFreq = curve.frequencies.begin();
	auto itValue = curve.values.begin();
	
	if( *itFreq > bandsParameters.front().startFreq )
	{
		auxLine.f_min = bandsParameters.front().startFreq * 0.9;
		auxLine.f_max = *itFreq;
		auxLine.slope = 0.0;
		auxLine.y_intercept = *itValue;
		lines.push_back(auxLine);
	}
	itFreq++; itValue++;

	for( ; itFreq!=curve.frequencies.end(); itFreq++, itValue++)
	{
		auxLine.f_min = *(itFreq-1);
		auxLine.f_max = *itFreq;
		auxLine.slope = ( *itValue - *(itValue-1) ) / ( *itFreq - *(itFreq-1) );
		auxLine.y_intercept = *(itValue-1) - *(itFreq-1) * auxLine.slope;
		lines.push_back(auxLine);
	}

	if( curve.frequencies.back() < bandsParameters.back().stopFreq )
	{
		auxLine.f_min = curve.frequencies.back();
		auxLine.f_max = bandsParameters.back().stopFreq * 1.1;
		auxLine.slope = 0.0;
		auxLine.y_intercept = curve.values.back();
		lines.push_back(auxLine);
	}
}

/*! This method takes the set of linear functions, which model the given in the frequency
 * 	domain, and it generates the adjusted curve evaluating each linear function, in its
 * 	range, taking into account the frequency values of the reference sweep.
 *
 * 	The frequency curve must be a _FreqValues_ structure or any of the structure derived
 * 	from that one.
 * 	\param [in] curve The frequency curve to be adjusted.
 */
const FreqValues & CurveAdjuster::AdjustCurve(const FreqValues & curve)
{
	BuildLines(curve);

	adjCurve.Clear();

	auto itLine=lines.begin();
	//The first linear function, taking into account the bands parameters, is found
	while( bandsParameters.front().startFreq >= itLine->f_max )
		++itLine;

	for(auto & freq : refSweep.frequencies)
	{
		while(freq > itLine->f_max)
			++itLine;

		adjCurve.frequencies.push_back(freq);
		adjCurve.values.push_back( itLine->Evaluate(freq) );
	}

	return adjCurve;
}
