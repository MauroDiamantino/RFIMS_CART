/*
 * FrontEndCalibrator.cpp
 *
 *  Created on: 18/04/2019
 *      Author: new-mauro
 */

#include "SweepProcessing.h"

#ifdef DEBUG
FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj) : adjuster(adj), correctENR("enr"), powerNSoff("sweep"),
		powerNSon("sweep"), powerNSoff_w("sweep"), powerNSon_w("sweep"), noiseTempNSoff("noise temperature"),
		noiseTempNSon("noise temperature"), gain("gain"), noiseTemperature("noise temperature"),
		noiseFigure("noise figure"), rbwCurve("rbw values curve"),
		auxRFPloter("Sweeps captured with a 50 ohm load at the input")
#else
FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj) : adjuster(adj), correctENR("enr"), powerNSoff("sweep"),
		powerNSon("sweep"), powerNSoff_w("sweep"), powerNSon_w("sweep"), noiseTempNSoff("noise temperature"),
		noiseTempNSon("noise temperature"), gain("gain"), noiseTemperature("noise temperature"),
		noiseFigure("noise figure"), rbwCurve("rbw values curve")
#endif
{
	enrFileLastWriteTime = -100;
	tsoff = REF_TEMPERATURE;
	flagNSon = false;
	flagCalStarted = false;
}

#ifdef DEBUG
FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj, const std::vector<BandParameters> & bandsParam) :
		adjuster(adj), correctENR("enr"), powerNSoff("sweep"), powerNSon("sweep"), powerNSoff_w("sweep"),
		powerNSon_w("sweep"), noiseTempNSoff("noise temperature"), noiseTempNSon("noise temperature"),
		bandsParameters(bandsParam), gain("gain"), noiseTemperature("noise temperature"), noiseFigure("noise figure"),
		rbwCurve("rbw values curve"), auxRFPloter("Sweeps captured with a 50 ohm load at the input")
#else
FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj, const std::vector<BandParameters> & bandsParam) :
		adjuster(adj), correctENR("enr"), powerNSoff("sweep"), powerNSon("sweep"), powerNSoff_w("sweep"),
		powerNSon_w("sweep"), noiseTempNSoff("noise temperature"), noiseTempNSon("noise temperature"),
		bandsParameters(bandsParam), gain("gain"), noiseTemperature("noise temperature"), noiseFigure("noise figure"),
		rbwCurve("rbw values curve")
#endif
{
	enrFileLastWriteTime = -100;
	tsoff = REF_TEMPERATURE;
	flagNSon = false;
	flagCalStarted = false;
}

//! This method build a curve with RBW values versus frequency.
/*! The aim of the RBW curve is to simplify the syntax of the equations which are used in the methods
 * `FrontEndCalibrator :: EstimateParameters ()` and `FrontEndCalibrator :: CalibrateSweep ()`.
 * Firstly, the RBW curve is loaded with two points for each frequency band: one point with the RBW value
 * of that band and the start frequency (Fstart) and the other point with the same RBW value and the stop
 * frequency (Fstop). Then, the RBW curve is adjusted (interpolated) using the CurveAdjuster object and,
 * because the way the first values ​​were loaded, the end RBW curve will be formed by steps, i.e. all the
 * frequencies which corresponds to the same band will have the same RBW value.
 */
void FrontEndCalibrator::BuildRBWCurve()
{
	const float deltaFreq=100e3; //This delta frequency is summed to the start frequencies to avoid the discontinuity

	//Building the curve with RBW values vs frequency
	rbwCurve.Clear();
	for(auto itBand = bandsParameters.begin() ; itBand != bandsParameters.end(); itBand++)
	{
		rbwCurve.frequencies.push_back(itBand->startFreq + deltaFreq);
		rbwCurve.values.push_back(itBand->rbw);

		rbwCurve.frequencies.push_back(itBand->stopFreq);
		rbwCurve.values.push_back(itBand->rbw);
	}

	rbwCurve = adjuster.AdjustCurve(rbwCurve);
}

void FrontEndCalibrator::SetBandsParameters(const std::vector<BandParameters> & bandsParam)
{
	bandsParameters = bandsParam;
	BuildRBWCurve();
}

void FrontEndCalibrator::LoadENR()
{
	boost::filesystem::path pathAndFilename(CAL_FILES_PATH);
	pathAndFilename /= "enr.txt";

	if( enrFileLastWriteTime < boost::filesystem::last_write_time(pathAndFilename) )
	{
		enrFileLastWriteTime = boost::filesystem::last_write_time(pathAndFilename);

		std::ifstream ifs( pathAndFilename.string() );
		std::string line, deviceName;
		size_t devNamePos;

		//Extracting the device name
		std::getline(ifs, line);
		devNamePos = line.find('=');
		if( devNamePos++ == std::string::npos )
			throw( CustomException("No device name in ENR file") );

		deviceName = line.substr(devNamePos, line.size()-devNamePos);
		boost::algorithm::to_lower(deviceName);

		FreqValues enr("enr");
		float freqGHz, enrValuedB;
		char delimiter;

		//Checking if there is a comment line and extracting it
		line.clear();
		std::getline(ifs, line);
		if( line.find("//")==std::string::npos )
		{
			//No comment was found so a line with frequency and power values was extracted
			std::istringstream iss(line);
			iss >> freqGHz >> delimiter >> enrValuedB;
			//enr.frequencies.push_back( freqGHz*1e9 );
			enr.frequencies.push_back( (std::uint_least64_t) (freqGHz*1e9) );
			enr.values.push_back( pow(10.0, enrValuedB/10.0) );
		}

		//Extracting the ENR and frequency values
		do
		{
			ifs >> freqGHz >> delimiter >> enrValuedB;
			ifs.get();
			//enr.frequencies.push_back( freqGHz*1e9 );
			enr.frequencies.push_back( (std::uint_least64_t) (freqGHz*1e9) );
			enr.values.push_back( pow(10.0, enrValuedB/10.0) );
			ifs.peek();
		}while( !ifs.eof() );

		//Correcting the ENR values
		if(deviceName=="346c" || deviceName=="n4002a")
			correctENR = enr + (REF_TEMPERATURE - 304.8)/REF_TEMPERATURE;
		else if(deviceName=="n4000a" || deviceName=="n4001a" || deviceName=="346a" || deviceName=="346b")
			correctENR = enr + (REF_TEMPERATURE - 302.8)/REF_TEMPERATURE;
		else
			correctENR = enr;

		//Adjusting the curve (interpolation, extrapolation and/or decimation)
		correctENR = adjuster.AdjustCurve(correctENR);
	}
}

void FrontEndCalibrator::StartCalibration()
{
#ifdef DEBUG
{
	cout << "Turn noise source off, switch the input to the noise source and ";
	#ifdef BUTTON //implied that RASPBERRY_PI is defined
		cout << "press the button to continue..." << endl;
		while( digitalRead(piPins.BUTTON_ENTER)==HIGH );
	#else
		cout << "press enter to continue..." << endl;
		WaitForKey();
	#endif
}
#else
{
	#ifdef RASPBERRY_PI
		digitalWrite(piPins.NOISE_SOURCE, LOW); digitalWrite(piPins.SWITCH, SWITCH_TO_NS);
	#endif
}
#endif
	flagNSon = false; flagCalStarted=true;
}

void FrontEndCalibrator::TurnOnNS()
{
#ifdef DEBUG
	cout << "\nTurn on the noise source and ";
	#ifdef BUTTON //implied that RASPBERRY_PI is defined
		cout << "press the button to continue..." << endl;
		while( digitalRead(piPins.BUTTON_ENTER)==HIGH );
	#else
		cout << "press Enter to continue..." << endl;
		WaitForKey();
	#endif
#else
	#ifdef RASPBERRY_PI
		digitalWrite(piPins.NOISE_SOURCE, HIGH);
	#endif
#endif
	flagNSon = true;
}

void FrontEndCalibrator::EndCalibration()
{
#ifdef DEBUG
	cout << "\nTurn off the noise source, switch the input to the antenna and ";
	#ifdef BUTTON //implied that RASPBERRY_PI is defined
		cout << "press the button to continue..." << endl;
		while( digitalRead(piPins.BUTTON_ENTER)==HIGH );
	#else
		cout << "press Enter to continue..." << endl;
		WaitForKey();
	#endif
#else
	TurnOffNS();
	#ifdef RASPBERRY_PI
		digitalWrite(piPins.SWITCH, SWITCH_TO_ANTENNA);
	#endif
#endif

	flagCalStarted=false;
}

void FrontEndCalibrator::SetSweep(const FreqValues & sweep)
{
	if(flagNSon)
	{
		powerNSon = sweep;
		powerNSon_w = pow(10.0, sweep/10.0 ) * 1e-3; //The power values are converted from dBm to Watts
	}
	else
	{
		powerNSoff = sweep;
		powerNSoff_w = pow(10.0, sweep/10.0 ) * 1e-3; //The power values are converted from dBm to Watts
	}
}


void FrontEndCalibrator::EstimateParameters()
{
	if( powerNSon_w.Empty() || powerNSoff_w.Empty() )
		throw( CustomException("The Front End calibrator cannot calculate parameters because one sweep (or both) is lacking.") );

	FreqValues tson("noise temperature"), yFactor("y-factor"), gainPowersRatio("gain");

	//Calculating the front end's noise figure curve
	tson = (REF_TEMPERATURE * correctENR) + tsoff;
	yFactor = powerNSon_w / powerNSoff_w;
	noiseTemperature = (-(tsoff * yFactor) + tson) / (yFactor - 1.0);
	noiseFigure = 10.0*log10( 1.0 + noiseTemperature / REF_TEMPERATURE );

	//Checking the mean value of noise figure
	float meanNoiseFig = noiseFigure.MeanValue();
	if( 0.5 > meanNoiseFig || meanNoiseFig > 20.0 )
		throw( CustomException("A ridiculous mean noise figure was got during estimation.") );

	//Correcting negative values of
	for(auto nf : noiseFigure.values)
		if(nf < 0)
			nf = 3.5;

	//Calculating the front end's gain curve
	gainPowersRatio = 0.5/(BOLTZMANN_CONST * rbwCurve) * ( powerNSoff_w/(tsoff + noiseTemperature) + powerNSon_w/(tson + noiseTemperature) );
	gain = 10.0*log10(gainPowersRatio);
	float meanGain = gain.MeanValue();

	//Checking the mean value of gain
	if( 10.0 > meanGain || meanGain > 100.0 )
		throw( CustomException("A ridiculous mean gain was got during estimation.") );
}

const Sweep& FrontEndCalibrator::CalibrateSweep(const Sweep& powerOut)
{
#ifdef DEBUG
	auxRFPloter.Clear();
	auxRFPloter.Plot(powerOut, "lines", "Uncalibrated sweep");
#endif
	Sweep powerInEff, powerInEff_w;
	Sweep powerIn, powerIn_w;
	try
	{
		//Calculating the effective input power (Pin_eff) which contains the antenna power and the
		//internal noise generated in the receiver
		powerInEff = powerOut - gain;
#ifdef DEBUG
		auxRFPloter.Plot(powerInEff, "lines", "Effective input power (Pant + Nreceiver)");
#endif
		powerInEff_w = pow(10.0, powerInEff/10.0) * 1e-3; //The power values are converted from dBm to Watts

		//Calculating the input power (Pin) which represents only the antenna power, without the receiver noise
		powerIn_w = powerInEff_w - BOLTZMANN_CONST * rbwCurve * noiseTemperature;

		//Converting the input power from Watts to dBm
		powerIn = 10.0*log10(powerIn_w) + 30.0;

#ifdef DEBUG
		auxRFPloter.Plot(powerIn, "lines", "Input power (Pant)");
#endif

		//The input power is saved as the calibrated sweep
		calSweep = powerIn;
	}
	catch(CustomException & exc)
	{
		CustomException exc1("A sweep could not be calibrated: ");
		exc1.Append( exc.what() );
		throw exc1;
	}

	return(calSweep);
}

void FrontEndCalibrator::LoadDefaultParameters()
{
	boost::filesystem::path pathGain(CAL_FILES_PATH), pathNoiseFig(CAL_FILES_PATH);
	pathGain /= "frontendparam";
	pathGain /= "default";
	pathGain /= "gain_default.csv";
	pathNoiseFig /= "frontendparam";
	pathNoiseFig /= "default";
	pathNoiseFig /= "noisefigure_default.csv";

	if( boost::filesystem::exists(pathGain) && boost::filesystem::exists(pathNoiseFig) )
	{
		FreqValues defaultGain("gain"), defaultNoiseFig("noise figure");
		double freqMHz;
		float gainValue, noiseFigValue;
		char delimiter=',', aux;
		std::ifstream ifs( pathGain.string() );

		ifs.exceptions( std::ifstream::badbit );

		//Exctracting the frequency values gain curve
		while( ifs.get()!=',' );
		do
		{
			ifs >> freqMHz >> delimiter;
			defaultGain.frequencies.push_back( std::uint_least64_t(freqMHz*1e6) );
		}while( delimiter==',' );

		//Extracting the gain values
		while( ifs.get()!=',' );
		do
		{
			ifs >> gainValue >> delimiter;
			defaultGain.values.push_back(gainValue);
			aux = ifs.peek();
		}while( delimiter==',' && !ifs.eof() && aux!='\r' && aux!='\n' );

		ifs.close();

		ifs.open( pathNoiseFig.string() );

		//Exctracting the frequency values of noise figure curve
		while( ifs.get()!=',' );
		do
		{
			ifs >> freqMHz >> delimiter;
			defaultNoiseFig.frequencies.push_back( std::uint_least64_t(freqMHz*1e6) );
		}while( delimiter==',' );

		//Extracting the noiseFigure values
		while( ifs.get()!=',' );
		do
		{
			ifs >> noiseFigValue >> delimiter;
			defaultNoiseFig.values.push_back(noiseFigValue);
			aux=ifs.peek();
		}while( delimiter==',' && !ifs.eof() && aux!='\r' && aux!='\n' );

		ifs.close();

		gain.Clear();
		noiseFigure.Clear();

		gain = adjuster.AdjustCurve(defaultGain);
		noiseFigure = adjuster.AdjustCurve(defaultNoiseFig);

		FreqValues noiseFactor("noise factor");
		noiseFactor = pow(10.0, noiseFigure/10.0);
		noiseTemperature = (noiseFactor - 1.0) * REF_TEMPERATURE;
	}
	else
	{
		CustomException exc("At least one of the files with the default front end parameters were not found.");
		throw(exc);
	}
}
