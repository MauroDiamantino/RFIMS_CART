/*
 * SweepProcessing.h
 *
 *  Created on: 02/04/2019
 *      Author: new-mauro
 */

#ifndef SWEEPPROCESSING_H_
#define SWEEPPROCESSING_H_

#include "Basics.h"
#include "gnuplot_i.hpp" //A C++ interface to gnuplot

class CurveAdjuster
{
	/////////Class types////////
	struct LinearFunction
	{
		float slope;
		float y_intercept;
		//float f_min, f_max;
		std::uint_least64_t f_min, f_max;
		//float Evaluate(float freqValue)
		float Evaluate(std::uint_least64_t freqValue)
		{
			if( f_min<=freqValue && freqValue<=f_max )
				return( slope*freqValue + y_intercept );
			else
				throw( CustomException("Out-of-range calculation on a linear function.") );
		}
	};
	///////Attributes////////
	std::vector<BandParameters> bandsParameters;
	Sweep refSweep;
	std::vector<LinearFunction> lines;
	FreqValues adjCurve;
	/////////Private methods///////
	void BuildLines(const FreqValues& curve);
public:
	///////Class interface/////////
	CurveAdjuster() : adjCurve("") {}
	~CurveAdjuster() {}
	void SetBandsParameters(const std::vector<BandParameters> & bandsParam) {	bandsParameters=bandsParam;	}
	void SetRefSweep(const Sweep & swp) {	refSweep = swp;		}
	const FreqValues& AdjustCurve(const FreqValues & curve);
	const FreqValues& GetAdjustedCurve() const {	return adjCurve;	}
};

//struct FrontEndParameters
//{
//	std::vector<float> gain_dB;
//	std::vector<float> noiseTemperature;
//	std::vector<float> noiseFigure;
//	std::vector<float> frequency;
//	std::string timestamp;
//};

//! The aim of this class is to calculate the total gain and total noise figure curves versus frequency of the RF front end
class FrontEndCalibrator
{
	////////Attributes///////////
	//Constants
#ifdef RASPBERRY_PI
	struct
	{
		const uint8_t SWITCH = 7;
		const uint8_t NOISE_SOURCE = 8;
	}piPins;
	const int SWITCH_TO_NS = HIGH;
	const int SWITCH_TO_ANTENNA = LOW;
	const std::string FILES_PATH = "/home/pi/RFIMS-CART/calibration";
#else
	const std::string FILES_PATH = "/home/new-mauro/RFIMS-CART/calibration";
#endif
	const float REF_TEMPERATURE = 290.0; // °K
	const float BOLTZMANN_CONST = 1.3806488e-23; // J/°K
	//Variables
	time_t enrFileLastWriteTime;
	FreqValues correctENR;
	float tsoff;
	FreqValues powerNSoff, powerNSon; //Power values in Watts
	FreqValues noiseTempNSoff, noiseTempNSon;
	std::vector<BandParameters> bandsParameters;
	bool flagNSon;
	//FrontEndParameters frontEndParam;
	FreqValues gain, noiseTemperature, noiseFigure;
	CurveAdjuster & adjuster;
	bool flagCalStarted;
	Sweep calSweep;
	FreqValues rbwCurve;
	///////Private Methods///////
	//void CalculateOutNoiseTemps();
public:
	/////////Class Interface//////////
	FrontEndCalibrator(CurveAdjuster & adj);
	FrontEndCalibrator(CurveAdjuster & adj, std::vector<BandParameters> & bandsParam);
	~FrontEndCalibrator() {}
	void SetBandsParameters(const std::vector<BandParameters> & bandsParam) {	bandsParameters = bandsParam;	}
	void LoadENR();
#ifdef RASPBERRY_PI
	//void SwitchToNS() { digitalWrite(piPins.NOISE_SOURCE, LOW); digitalWrite(piPins.SWITCH, SWITCH_TO_NS);	}
	void StartCalibration()
	{
		digitalWrite(piPins.NOISE_SOURCE, LOW); digitalWrite(piPins.SWITCH, SWITCH_TO_NS);
		flagNSon = false; flagCalStarted=true;
	}
	void TurnOnNS() {	digitalWrite(piPins.NOISE_SOURCE, HIGH); flagNSon = true;	}
	void TurnOffNS() {	digitalWrite(piPins.NOISE_SOURCE, LOW); flagNSon = false;	}
	void EndCalibration()
	{
		TurnOffNS();
		digitalWrite(piPins.SWITCH, SWITCH_TO_ANTENNA);
		flagCalStarted=false;
	}
#else
	void StartCalibration() {	flagNSon = false; flagCalStarted=true;	}
	void TurnOnNS() {	flagNSon = true;	}
	void TurnOffNS() {	flagNSon = false;	}
	void EndCalibration() {		flagNSon = false; flagCalStarted=false;	}
#endif
	void SetSweep(const FreqValues & sweep);
	void SetNSoffTemp(const float nsOffTemp) {	tsoff = nsOffTemp;	}
	//const FrontEndParameters& CalculateParameters();
	void EstimateParameters();
	void SaveFrontEndParam(const TimeData & timeData);
	//const FrontEndParameters& GetFrontEndParam() const {	return frontEndParam;	}
	const FreqValues & GetGain() const {	return gain;	}
	const FreqValues & GetNoiseTemp() const {	return noiseTemperature;	}
	const FreqValues & GetNoiseFigure() const {		return noiseFigure;		}
	FreqValues GetENRcorr() const {	return correctENR;		}
	float GetNSoffTemp() const {	return tsoff;	}
	const FreqValues & GetSweepNSoff() {	return powerNSoff;	}
	const FreqValues & GetSweepNSon() {	return powerNSon;	}
	bool IsCalibStarted() const {	return flagCalStarted;		}
	bool IsNoiseSourceOff() const {	return !flagNSon;	}
	const Sweep& CalibrateSweep(const Sweep& uncalSweep);
	const Sweep& GetCalSweep() {	return calSweep;	}
	void LoadDefaultParameters();
	bool AreParamEmpty() {	return( gain.Empty() || noiseTemperature.Empty() || noiseFigure.Empty() );	}
	///////////////
	void SetGain(const FreqValues & g) {	gain=g;		}
	void SetNoiseTemp(const FreqValues & nt) {	noiseTemperature=nt;	}
	void SetNoiseFigure(const FreqValues & nf) {	noiseFigure=nf;		}
};


//! Class RFPloter
class RFPloter
{
	Gnuplot ploter;
	std::string title;
	void ConfigureGraph()
	{
		ploter.unset_grid(); ploter.set_legend("inside"); ploter.set_legend("right"); ploter.set_title(title);
		ploter.set_legend("top"); ploter.set_legend("nobox"); ploter.set_xlabel("Frequency (Hz)");
	}
public:
	RFPloter(std::string titl="") : title(titl) {	ConfigureGraph();	}
	~RFPloter() {	ploter.remove_tmpfiles();	}
	void Plot(const FreqValues & curve, std::string style, std::string name = "")
	{
		ploter.set_style(style);
		ploter.plot_xy(curve.frequencies, curve.values,  name);
	}
	void PlotSweep(const Sweep& sweep)
	{
		std::ostringstream oss;
		oss << "Complete sweep, Azimuth: " << sweep.azimuthAngle;
		oss << " \xF8 N, Polarization: " << sweep.polarization << ", " << sweep.timeData.timestamp();
		ploter.set_title( oss.str() ); ploter.set_xlabel("Frequency (MHz)");
		ploter.set_ylabel("Power (dBm)"); ploter.set_style("lines");
		std::vector<float> freqMHz;
		for(auto f : sweep.frequencies)
			freqMHz.push_back( f/1e6 );
		ploter.plot_xy(freqMHz, sweep.values, "Sweep");
	}
	void PlotRFI(const std::vector<FreqValues>& rfiVector)
	{
		ploter.set_style("points");
		for(auto rfi : rfiVector)
		{
			std::vector<float> freqMHz;
			for(auto f : rfi.frequencies)
				freqMHz.push_back( f/1e6 );

			ploter.plot_xy(freqMHz, rfi.values, "RFI" );
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
	Sweep sweep;
	unsigned int sweepIndex;
	std::ofstream ofs;
	std::string firstSweepDate;
public:
	//////////Class interface///////////
	DataLogger();
	~DataLogger();
	void SaveData(const Sweep& swp);
	//void SentDataToRemote()
};

#endif /* SWEEPPROCESSING_H_ */