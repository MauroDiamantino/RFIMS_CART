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
#include <boost/filesystem.hpp>

//! Class Ploter
class RFPloter
{
	Gnuplot ploter;
	void ConfigureGraph();
public:
	RFPloter();
	~RFPloter();
	void PlotSweep(const FreqValueSet& sweep);
	void PlotRFI(const std::vector<FreqValueSet>& rfi);
	void Clear();
};

class DataLogger
{
	///////////Attributes/////////////
	//Constants
	//const boost::filesystem::path MEASUREMENTS_PATH = "/home/pi/RFIMS-CART/measurements"; //Raspberry Pi
	const boost::filesystem::path MEASUREMENTS_PATH = "/home/new-mauro/RFIMS-CART/measurements"; //Notebook
	const unsigned int NUM_OF_POSITIONS = 8;
	//Variables
	FreqValueSet sweep;
	unsigned int sweepIndex;
	float antPosition;
	std::string antPolarization;
	std::ofstream ofs;
	std::string firstSweepDate;
public:
	//////////Class interface///////////
	DataLogger();
	~DataLogger();
	void SetSweep(const FreqValueSet& swp);
	void SetAntennaData(float position, const std::string& polarization) {	antPosition=position; antPolarization=polarization;	}
	bool SaveData();
};

#endif /* SWEEPPROCESSING_H_ */
