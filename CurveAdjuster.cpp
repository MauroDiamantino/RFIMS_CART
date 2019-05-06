/*
 * CurveAdjuster.cpp
 *
 *  Created on: 18/04/2019
 *      Author: new-mauro
 */

#include "SweepProcessing.h"

void CurveAdjuster::BuildLines(const FreqValues & curve)
{
	LinearFunction auxLine;
	lines.clear();
	auto itFreq = curve.frequencies.begin();
	auto itValue = curve.values.begin();
	if( *itFreq > bandsParameters.front().startFreq )
	{
		auxLine.f_min = bandsParameters.front().startFreq;
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
		auxLine.f_max = bandsParameters.back().stopFreq;;
		auxLine.slope = 0.0;
		auxLine.y_intercept = curve.values.back();
		lines.push_back(auxLine);
	}
}

//const FreqValues & CurveAdjuster::AdjustCurve(const FreqValues & curve)
//{
//	const float TOLERANCE = 5e3;
//	BuildLines(curve);
//
//	adjCurve.Clear();
//
//	auto itLine=lines.begin();
//	//The first linear function, taking into account the bands parameters, is found
//	while( bandsParameters.front().startFreq >= itLine->f_max )
//		++itLine;
//
//	unsigned int numOfAccumPoints=0;
//
//	for(auto itBand=bandsParameters.begin(); itBand!=bandsParameters.end(); itBand++)
//	{
//		//Checking if the current band is enabled. If not, the iterator is incremented up to find an enabled band.
//		while( itBand->flagEnable == false )
//			itBand++;
//
//		numOfAccumPoints += itBand->samplePoints;
//
//		//Generating the frequency points for the current band (the last point is generated later)
//		float deltaFreq = (itBand->stopFreq - itBand->startFreq) / (itBand->samplePoints - 1);
//		float freq = itBand->startFreq;
//		for( ; freq < (itBand->stopFreq-TOLERANCE); freq += deltaFreq)
//		{
//			if(freq > itLine->f_max)
//				++itLine;
//
//			adjCurve.frequencies.push_back(freq);
//			adjCurve.values.push_back( itLine->Evaluate(freq) );
//		}
//
//		//Checking if there is a next band, and if it is true, it is checked if its Fstart matches the Fstop of the
//		//current band. If there is not a next band or if the frequencies do not match, the frequency point
//		//corresponding to Fstop is generated.
//		//if( (itBand+1)==bandsParameters.end() || itBand->stopFreq!=(itBand+1)->startFreq )
//		if( adjCurve.frequencies.size()<numOfAccumPoints )
//		{
//			adjCurve.frequencies.push_back( itBand->stopFreq );
//			adjCurve.values.push_back( itLine->Evaluate( itBand->stopFreq ) );
//		}
//	}
//
//	return adjCurve;
//}

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
		if(freq > itLine->f_max)
			++itLine;

		adjCurve.frequencies.push_back(freq);
		adjCurve.values.push_back( itLine->Evaluate(freq) );
	}

	return adjCurve;
}
