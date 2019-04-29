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

class CurveAdjuster
{
	/////////Class types////////
	struct LinearFunction
	{
		float slope;
		float y_intercept;
		float f_min, f_max;
		float Evaluate(float freqValue)
		{
			if( f_min<=freqValue && freqValue<=f_max )
				return( slope*freqValue + y_intercept );
			else
				throw( CustomException("Out-of-range calculation on a linear function.") );
		}
	};
	///////Attributes////////
	std::vector<BandParameters> bandsParameters;
	std::vector<LinearFunction> lines;
	FreqValueSet adjCurve;
	/////////Private methods///////
	void BuildLines(const FreqValueSet& curve);
public:
	///////Class interface/////////
	CurveAdjuster() : adjCurve("", 0) {}
	void SetBandsParameters(const std::vector<BandParameters> & bandsParam) {	bandsParameters=bandsParam;	}
	const FreqValueSet& AdjustCurve(const FreqValueSet & curve);
	const FreqValueSet& GetAdjustedCurve() const {	return adjCurve;	}
};

//!The aim of this class is to calibrate the sweeps taking into account the total gain curve of the RF front end.
class SweepCalibrator
{
	////////Attributes///////////
	//Constants
#ifdef RASPBERRY_PI
	const boost::filesystem::path FILES_PATH = "/home/pi/RFIMS_CART/calibration/";
#else
	const boost::filesystem::path FILES_PATH = "/home/new-mauro/RFIMS_CART/calibration/";
#endif
	//Variables
	//CurveAdjuster & adjuster;
	//FreqValueSet totalGain;
	FreqValueSet calCurve;
	FreqValueSet calSweep;
public:
	///////Class' interface/////////
	//SweepCalibrator(CurveAdjuster & adj) : adjuster(adj) {}
	//void SetTotalGainCurve(const FreqValueSet& totGain) {	totalGain=totGain;	}
	void BuildCalCurve(const FreqValueSet& totalGain)
	{
		if( !totalGain.Empty() )
		{
			calCurve = -totalGain;
			//ajuste de la curva
		}
		else
			throw( CustomException("The total gain curve is empty.") );
	}
	const FreqValueSet& CalibrateSweep(const FreqValueSet& uncalSweep)
	{
		try
		{
			calSweep = uncalSweep + calCurve;
		}
		catch(CustomException & exc)
		{
			CustomException exc1("A sweep could not be calibrated: ");
			exc1.Append( exc.what() );
			throw exc1;
		}
		return(calSweep);
	}
	const FreqValueSet& GetCalSweep() const {	return calSweep;	}
	const FreqValueSet& GetCalCurve() const {	return calCurve;	}
};

struct FrontEndParameters
{
	std::vector<float> gain_dB;
	std::vector<float> noiseTemperature;
	std::vector<float> noiseFigure;
	std::vector<float> frequency;
	std::string timestamp;
};


//! The aim of this class is to calculate the total gain and total noise figure curves versus frequency of the RF front end
class FrontEndCalibrator
{
	////////Attributes///////////
	//Constants
#ifdef RASPBERRY_PI
	struct
	{
		const uint8_t SWITCH = 10;
		const uint8_t NOISE_SOURCE = 12;
	}piPins;
	const uint8_t SWITCH_TO_NS = HIGH;
	const uint8_t SWITHC_TO_ANTENNA = LOW;
	const std::string FILES_PATH = "/home/pi/RFIMS-CART/calibration";
#else
	const std::string FILES_PATH = "/home/new-mauro/RFIMS-CART/calibration";
#endif
	const float REF_TEMPERATURE = 290.0; // °K
	const float BOLTZMANN_CONST = 1.3806488e-23; // J/°K
	//Variables
	time_t enrFileLastWriteTime;
	FreqValueSet correctENR;
	float tsoff;
	//FreqValueSet sweepNSoff, sweepNSon;
	FreqValueSet powerNSoff, powerNSon; //Power values in Watts
	FreqValueSet noiseTempNSoff, noiseTempNSon;
	std::vector<BandParameters> bandsParameters;
	bool flagNSon;
	FrontEndParameters frontEndParam;
	CurveAdjuster & adjuster;
	bool flagCalStarted;
	///////Private Methods///////
	//void CalculateOutNoiseTemps();
public:
	/////////Class Interface//////////
	FrontEndCalibrator(CurveAdjuster & adj);
	FrontEndCalibrator(CurveAdjuster & adj, std::vector<BandParameters> & bandsParam);
	void SetBandsParameters(const std::vector<BandParameters> & bandsParam) {	 bandsParameters = bandsParam;	}
	void LoadENR();
#ifdef RASPBERRY_PI
	//void SwitchToNS() { digitalWrite(piPins.NOISE_SOURCE, LOW); digitalWrite(piPins.SWITCH, SWITCH_TO_NS);	}
	void StartCalibration()
	{
		digitalWrite(piPins.NOISE_SOURCE, LOW); digitalWrite(piPins.SWITCH, SWITCH_TO_NS);
		flagNSon = false;
	}
	void TurnOnNS() {	digitalWrite(piPins.NOISE_SOURCE, HIGH); flagNSon = true;	}
	void EndCalibration()
	{
		digitalWrite(piPins.NOISE_SOURCE, LOW); digitalWrite(piPins.SWITCH, SWITCH_TO_ANTENNA);
		flagNSon = false;
	}
#else
	void StartCalibration() {	flagNSon = false; flagCalStarted=true;	}
	void TurnOnNS() {	flagNSon = true;	}
	void EndCalibration() {		flagNSon = false; flagCalStarted=false;	}
#endif
	void SetSweep(const FreqValueSet & sweep);
	void SetNSoffTemp(const float nsOffTemp) {	tsoff = nsOffTemp;	}
	const FrontEndParameters& CalculateParameters();
	void SaveFrontEndParam(const TimeData & timeData);
	const FrontEndParameters& GetFrontEndParam() const {	return frontEndParam;	}
	FreqValueSet GetENRcorr() const {	return correctENR;		}
	float GetNSoffTemp() const {	return tsoff;	}
	bool IsCalibStarted() const {	return flagCalStarted;		}
	bool IsNoiseSourceOn() const {	return flagNSon;	}
};


//! Class Ploter
class RFPloter
{
	Gnuplot ploter;
	void ConfigureGraph()
	{
		ploter.unset_grid(); ploter.set_legend("inside"); ploter.set_legend("right");
		ploter.set_legend("top"); ploter.set_legend("nobox"); ploter.set_xlabel("Frequency (Hz)");
	}
public:
	RFPloter() {	ConfigureGraph();	}
	~RFPloter() {	ploter.remove_tmpfiles();	}
	void Plot(const FreqValueSet & curve, std::string style, std::string name = "")
	{
		ploter.set_title(name); ploter.set_style(style);
		ploter.plot_xy(curve.frequencies, curve.values,  name);
	}
	void PlotSweep(const FreqValueSet& sweep)
	{
		ploter.set_title("Complete sweep"); ploter.set_xlabel("Frequency (MHz)");
		ploter.set_ylabel("Power (dBm)"); ploter.set_style("lines");
		std::vector<float> freqMHz;
		for(auto f : sweep.frequencies)
			freqMHz.push_back( f/1e6 );
		ploter.plot_xy(freqMHz, sweep.values, "Sweep");
	}
	void PlotRFI(const std::vector<FreqValueSet>& rfiVector)
	{
		ploter.set_style("points");
		std::ostringstream oss;
		for(auto rfi : rfiVector)
		{
			std::vector<float> freqMHz;
			for(auto f : rfi.frequencies)
				freqMHz.push_back( f/1e6 );
			oss.str("");
			oss << "RFI N° " << rfi.index;
			ploter.plot_xy(freqMHz, rfi.values, oss.str() );
		}
	}
	void Clear() {	ploter.reset_all(); ConfigureGraph();	}
};


class DataLogger
{
	///////////Attributes/////////////
	//Constants
#ifdef RASPBERRY_PI
	const boost::filesystem::path MEASUREMENTS_PATH = "/home/pi/RFIMS-CART/measurements"; //Raspberry Pi
#else
	const boost::filesystem::path MEASUREMENTS_PATH = "/home/new-mauro/RFIMS-CART/measurements"; //Notebook
#endif
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
	void SaveData();
	//void SentDataToRemote()
};

#endif /* SWEEPPROCESSING_H_ */
