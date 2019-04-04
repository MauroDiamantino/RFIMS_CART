/*
 * SweepProcessing.h
 *
 *  Created on: 02/04/2019
 *      Author: new-mauro
 */

#ifndef SWEEPPROCESSING_H_
#define SWEEPPROCESSING_H_

#include "RFIMS_CART.h"

#include "gnuplot_i.hpp" //A C++ interface to gnuplot

//! Class Ploter
class RFPloter
{
	Gnuplot ploter;
	void ConfigureGraph();
public:
	RFPloter();
	~RFPloter();
	void PlotSweep(const FreqValueSet& sweep);
	void PlotRFI(const vector<FreqValueSet>& rfi);
	void Clear();
};


//class DataLogger
//{
//
//};

#endif /* SWEEPPROCESSING_H_ */
