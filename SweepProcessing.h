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
#include <queue>


//! Class RFPloter
class RFPloter
{
	////////////Class' attributes///////////
	Gnuplot ploter;
	std::string title;
	Sweep sweep;
	///////////Private methods//////////////
	void ConfigureGraph()
	{
		ploter.unset_grid(); ploter.set_legend("inside"); ploter.set_legend("right"); ploter.set_legend("top");
		ploter.set_legend("nobox"); ploter.set_title(title); ploter.set_xlabel("Frequency (Hz)");
	}
public:
	///////////Class interface////////////
	RFPloter(const std::string & titl="") : ploter("lines"), title(titl) {	ConfigureGraph();	}
	~RFPloter() {	ploter.remove_tmpfiles();	}
	void Plot(const FreqValues & curve, const std::string & style = "lines", const std::string & name = "")
	{
		ploter.set_style(style); ploter.set_title(name);
		if( curve.type=="gain" || curve.type=="noise figure" || curve.type=="noise temperature")
		{
			std::vector<double> freqMHz;
			for(const auto f : curve.frequencies)
				freqMHz.push_back( double(f)/1e6 );

			if(curve.type=="gain")
				ploter.set_ylabel("Gain (dB)");
			else if(curve.type=="noise figure")
				ploter.set_ylabel("Noise Figure (dB)");
			else if (curve.type=="noise temperature")
				ploter.set_ylabel("Noise Temperature (°K)");

			ploter.set_xlabel("Frequency (MHz)");
			ploter.plot_xy(freqMHz,curve.values,name);
		}
		else
			ploter.plot_xy(curve.frequencies,curve.values,name);
	}
	void PlotSweep(const Sweep& swp)
	{
		sweep=swp;
		std::ostringstream oss;
		oss << "Calibrated sweep, Azimuth: " << sweep.azimuthAngle;
		oss << " degrees N, Polarization: " << sweep.polarization << ", " << sweep.timeData.GetTimestamp();
		ploter.set_title( oss.str() ); ploter.set_xlabel("Frequency (MHz)");
		ploter.set_ylabel("Power (dBm)"); ploter.set_style("lines");
		std::vector<double> freqMHz;
		for(const auto f : sweep.frequencies)
			freqMHz.push_back( double(f)/1e6 );
		ploter.plot_xy(freqMHz, sweep.values, "Calibrated sweep");
	}
	void PlotRFI(const RFI & rfi)
	{
		if( !rfi.Empty() )
		{
			if( rfi.azimuthAngle==sweep.azimuthAngle && rfi.polarization==sweep.polarization )
			{
				ploter.set_style("points");
				std::vector<double> freqMHz;
				for(const auto f : rfi.frequencies)
					freqMHz.push_back( double(f)/1e6 );
				ploter.plot_xy(freqMHz, rfi.values, "Detected RFI");
			}
			else
			{
				CustomException exc("The RFI which was given to be plotted is not related to the plotted sweep.");
				throw(exc);
			}
		}
	}
	void Clear() {	ploter.reset_all(); ConfigureGraph(); sweep.Clear();	}
};


class CurveAdjuster
{
	/////////Class data types////////
	struct LinearFunction
	{
		float slope;
		float y_intercept;
		//float f_min, f_max;
		std::uint_least64_t f_min, f_max;
		//float Evaluate(float freqValue)
		float Evaluate(const std::uint_least64_t freqValue)
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

//! The aim of this class is to calculate the total gain and total noise figure curves versus frequency of the RF front end
class FrontEndCalibrator
{
	////////Attributes///////////
	//Constants
	const std::string CAL_FILES_PATH = BASE_PATH + "/calibration";
	const float REF_TEMPERATURE = 290.0; // °K
	const float BOLTZMANN_CONST = 1.3806488e-23; // J/°K
	//Variables
	CurveAdjuster & adjuster;
	time_t enrFileLastWriteTime;
	FreqValues correctENR;
	float tsoff;
	FreqValues powerNSoff, powerNSon;
	FreqValues powerNSoff_w, powerNSon_w; //Power values in Watts
	FreqValues noiseTempNSoff, noiseTempNSon;
	std::vector<BandParameters> bandsParameters;
	bool flagNSon;
	FreqValues gain, noiseTemperature, noiseFigure;
	bool flagCalStarted;
	Sweep calSweep;
	FreqValues rbwCurve;
#ifdef DEBUG
	RFPloter auxRFPloter;
#endif
	//////////Private methods//////////////
	void BuildRBWCurve();
public:
	/////////Class Interface//////////
	FrontEndCalibrator(CurveAdjuster & adj);
	FrontEndCalibrator(CurveAdjuster & adj, const std::vector<BandParameters> & bandsParam);
	~FrontEndCalibrator() {}
	void SetBandsParameters(const std::vector<BandParameters> & bandsParam);
	void LoadENR();
	void StartCalibration();
	void TurnOnNS();
	void TurnOffNS()
	{
	#ifdef RASPBERRY_PI
			digitalWrite(piPins.NOISE_SOURCE, LOW); flagNSon = false;
	#endif
	}
	void EndCalibration();
	void SetSweep(const FreqValues & sweep);
	void SetNSoffTemp(const float nsOffTemp) {	tsoff = nsOffTemp;	}
	void EstimateParameters();
	const FreqValues & GetGain() const {	return gain;	}
	const FreqValues & GetNoiseTemp() const {	return noiseTemperature;	}
	const FreqValues & GetNoiseFigure() const {		return noiseFigure;		}
	FreqValues GetENRcorr() const {	return correctENR;		}
	float GetNSoffTemp() const {	return tsoff;	}
	const FreqValues & GetSweepNSoff() const {	return powerNSoff;	}
	const FreqValues & GetSweepNSon() const {	return powerNSon;	}
	bool IsCalibStarted() const {	return flagCalStarted;		}
	bool IsNoiseSourceOff() const {	return !flagNSon;	}
	const Sweep& CalibrateSweep(const Sweep & uncalSweep);
	const Sweep& GetCalSweep() const {	return calSweep;	}
	void LoadDefaultParameters();
	bool AreParamEmpty() {	return( gain.Empty() || noiseTemperature.Empty() || noiseFigure.Empty() );	}
#ifdef DEBUG
	void SetGain(const FreqValues & g) {	gain=g;		}
	void SetNoiseTemp(const FreqValues & nt) {	noiseTemperature=nt;	}
	void SetNoiseFigure(const FreqValues & nf) {	noiseFigure=nf;		}
#endif
};


//! The aim of this class is to compare each calibrated sweep with a threshold curve to determine where there is RF interference.
class RFIDetector
{
	////////////Class' attributes/////////
	//Constants
	const std::string THRESHOLDS_PATH = BASE_PATH + "/thresholds";
	const double ANTENNA_GAIN = 5.0; //dBi
	const double SPEED_OF_LIGHT = 2.99792e8; //m/s
	//Variables
	CurveAdjuster & adjuster;
	std::vector<BandParameters> bandsParameters;
	RFI rfi;
	FreqValues thresholdsCurve;
	time_t threshFileLastWriteTime;
public:
	/////////Class interface////////////
	RFIDetector(CurveAdjuster & adj) : adjuster(adj), thresholdsCurve("threshold curve") { threshFileLastWriteTime=0; }
	~RFIDetector() {}
	void SetThreshNorm(const RFI::ThresholdsNorm thrNorm) {		rfi.threshNorm = thrNorm;	}
	void SetBandsParameters(const std::vector<BandParameters> & bandsParam) {	bandsParameters=bandsParam;	}
	void LoadThreshCurve(const RFI::ThresholdsNorm thrNorm);
	const RFI & DetectRFI(const Sweep & sweep);
	const FreqValues & GetThreshCurve() const {		return thresholdsCurve;		}
	unsigned int GetNumOfRFIBands() const {		return rfi.numOfRFIBands;	}
	const RFI & GetRFI() const {	return rfi;		}
};


class DataLogger
{
	///////////Attributes/////////////
	//Constants
	//const std::string BASE_PATH = "/home/new-mauro/RFIMS-CART";
	const std::string MEASUREMENTS_PATH = BASE_PATH + "/measurements";
	const std::string FRONT_END_PARAM_PATH = BASE_PATH + "/calibration/frontendparam";
	const std::string BANDS_PARAM_CSV_PATH = BASE_PATH + "/parameters/csv";
	const std::string UPLOADS_PATH = BASE_PATH + "/uploads";
	const unsigned int NUM_OF_POSITIONS = 6;
	//Variables
	std::ofstream ofs;
	unsigned int sweepIndex;
	std::string currMeasCycleDate;
	bool flagNewBandsParam;
	bool flagNewFrontEndParam;
	std::queue<std::string> filesToUpload;
public:
	//////////Class interface///////////
	DataLogger();
	~DataLogger();
	void SaveBandsParamAsCSV(const std::vector<BandParameters> & bandsParamVector);
	void SaveFrontEndParam(const FreqValues & gain, const FreqValues & noiseFigure);
	void SaveSweep(const Sweep& sweep);
	void CompressLastFiles();
	void DeleteOldFiles() const;
	void UploadData();
};

#endif /* SWEEPPROCESSING_H_ */
