/*
 * RFPloter.cpp
 *
 *  Created on: 02/04/2019
 *      Author: new-mauro
 */

#include "SweepProcessing.h"

RFPloter::RFPloter()
{
	ConfigureGraph();
}

RFPloter::~RFPloter()
{
	ploter.remove_tmpfiles();
}

void RFPloter::PlotSweep(const FreqValueSet& sweep)
{
	ploter.set_style("lines");
	std::vector<float> freqMHz;
	for(auto f : sweep.frequencies)
		freqMHz.push_back( f/1e6 );

	ploter.plot_xy(freqMHz, sweep.values, "Sweep");
}

void RFPloter::PlotRFI(const std::vector<FreqValueSet>& rfiVector)
{
	ploter.set_style("points");
	std::ostringstream oss;
	for(auto rfi : rfiVector)
	{
		std::vector<float> freqMHz;
		for(auto f : rfi.frequencies)
			freqMHz.push_back( f/1e6 );

		oss.clear();
		oss << "RFI N° " << rfi.index;
		ploter.plot_xy(freqMHz, rfi.values, oss.str() );
	}
}

void RFPloter::Clear()
{
	ploter.reset_all();

	ConfigureGraph();
}

void RFPloter::ConfigureGraph()
{
	ploter.unset_grid();
	ploter.set_legend("inside");
	ploter.set_legend("right");
	ploter.set_legend("top");
	ploter.set_legend("nobox");
	ploter.set_title("Complete captured sweep");
	ploter.set_xlabel("Frequency (MHz)");
	ploter.set_ylabel("Power (dBm)");
}

