/*! \file FrontEndCalibrator.cpp
 * 	\brief This file contains the definitions of several methods of the class _FrontEndCalibrator_.
 * 	\author Mauro Diamantino
 */

#include "SweepProcessing.h"


#ifdef DEBUG
/*! At instantiation, the programmer must provide a reference to a _CurveAdjuster_ object.
 * \param [in] adj A reference to a _CurveAdjuster_ object, which will be used to adjust some internal curves.
 */
FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj) : adjuster(adj), correctENR("enr"), powerNSoff("sweep"),
		powerNSon("sweep"), powerNSoff_w("sweep"), powerNSon_w("sweep"), gain("gain"),
		noiseTemperature("noise temperature"), noiseFigure("noise figure"), rbwCurve("rbw values curve"),
		auxRFPloter("Sweeps captured with a 50 ohm load at the input")
#else
/*! At instantiation, the programmer must provide a reference to a _CurveAdjuster_ object.
 * \param [in] adj A reference to a _CurveAdjuster_ object, which will be used to adjust some internal curves.
 */
FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj) : adjuster(adj), correctENR("enr"), powerNSoff("sweep"),
		powerNSon("sweep"), powerNSoff_w("sweep"), powerNSon_w("sweep"), gain("gain"),
		noiseTemperature("noise temperature"), noiseFigure("noise figure"), rbwCurve("rbw values curve")
#endif
{
	enrFileLastWriteTime = -100;
	tsoff = REF_TEMPERATURE;
	flagNSon = false;
	flagCalStarted = false;
}

#ifdef DEBUG
/*! \param [in] adj A reference to a _CurveAdjuster_ object, which will be used to adjust some internal curves.
 * 	\param [in] bandsParam A vector with the parameters of all frequency bands.
 */
FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj, const std::vector<BandParameters> & bandsParam) :
		adjuster(adj), correctENR("enr"), powerNSoff("sweep"), powerNSon("sweep"), powerNSoff_w("sweep"),
		powerNSon_w("sweep"), bandsParameters(bandsParam), gain("gain"), noiseTemperature("noise temperature"),
		noiseFigure("noise figure"), rbwCurve("rbw values curve"),
		auxRFPloter("Sweeps captured with a 50 ohm load at the input")
#else
/*! \param [in] adj A reference to a _CurveAdjuster_ object, which will be used to adjust some internal curves.
 * 	\param [in] bandsParam A vector with the parameters of all frequency bands.
 */
FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj, const std::vector<BandParameters> & bandsParam) :
		adjuster(adj), correctENR("enr"), powerNSoff("sweep"), powerNSon("sweep"), powerNSoff_w("sweep"),
		powerNSon_w("sweep"), bandsParameters(bandsParam), gain("gain"), noiseTemperature("noise temperature"),
		noiseFigure("noise figure"), rbwCurve("rbw values curve")
#endif
{
	enrFileLastWriteTime = -100;
	tsoff = REF_TEMPERATURE;
	flagNSon = false;
	flagCalStarted = false;
}

/*! The aim of the RBW curve is to simplify the syntax of the equations which are used in the methods
 * `FrontEndCalibrator::EstimateParameters()` and `FrontEndCalibrator::CalibrateSweep()`.
 * Firstly, the RBW curve is loaded with two points for each frequency band: one point with the RBW value
 * of that band and its start frequency (Fstart) and the other point with the same RBW value and its stop
 * frequency (Fstop). Then, the RBW curve is adjusted using the _CurveAdjuster_ object and, because of
 * the way the first values ​​were loaded, the end RBW curve will be formed by steps, i.e. all the frequencies
 * which corresponds to the same band will have the same RBW value.
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

/*!	After the bands' parameters are stored, the RBW curve is built, taking into account those parameters.
 * \param [in] bandsParam A vector with the parameters of all frequency bands.
 */
void FrontEndCalibrator::SetBandsParameters(const std::vector<BandParameters> & bandsParam)
{
	bandsParameters = bandsParam;
	BuildRBWCurve();
}

/*!	The ENR values versus frequency of the noise generator are loaded from the file
 * [BASE_PATH](\ref BASE_PATH)/calibration/enr.txt, then those values are corrected taking into account
 * a statistical mean physical temperature of the noise source at time of the factory calibration.
 * That technique is exposed in the Application Note "Noise Figure Measurement Accuracy – The Y-Factor
 * Method" of Keysight Technologies. To finish, the corrected ENR curve is adjusted to be used in the
 * mathematical operations with the captured sweeps.
 *
 * To perform the estimation of the front end parameters the following equations are used:
 * \f[
 * 		ENR_{CORR}=ENR_{CAL}+\frac{T_{O}-T_{CORR}}{T_{O}}
 * \f]
 * \f[
 * 		T_{SON}=T_{O}*ENR_{CORR}+T_{SOFF}
 * \f]
 * \f[
 * 		Y=\frac{N_{OUT_{ON}}}{N_{OUT_{OFF}}}
 * \f]
 * \f[
 * 		T_{receiver}=\frac{T_{SON}-Y*T_{SOFF}}{Y-1}
 * \f]
 * \f[
 * 		F_{receiver}=1+\frac{T_{receiver}}{T_O}
 * \f]
 * \f[
 * 		NF_{receiver}=10*\log_{10}(F_{receiver})
 * \f]
 * \f[
 * 		G_{receiver}=\frac{1}{2*k*RBW}*\left[\frac{N_{OUT_{ON}}}{T_{SON}+T_{receiver}}+\frac{N_{OUT_{OFF}}}{T_{SOFF}+T_{receiver}}\right]
 * \f]
 */
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
			throw rfims_exception("no device name in ENR file");

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

/*! This method must be called once the calibration process finished, i.e. the method `EndCalibration()` must be called first.
 * 	The front end parameters, total gain and total noise figure, are estimated using the Y-Factor method, which is exposed in
 * 	the Application Note "Noise Figure Measurement Accuracy – The Y-Factor Method" of Keysight Technologies.
 *
 * 	Once the parameters' curves have been estimated, their mean values are checked to be reasonable.
 */
void FrontEndCalibrator::EstimateParameters()
{
	if( powerNSon_w.Empty() || powerNSoff_w.Empty() )
		throw rfims_exception("the front end calibrator could not estimate the parameters because one sweep (or both) was lacking.");

	FreqValues tson("noise temperature"), yFactor("y-factor"), gainPowersRatio("gain");

	//Calculating the front end's noise figure curve
	tson = (REF_TEMPERATURE * correctENR) + tsoff;
	yFactor = powerNSon_w / powerNSoff_w;
	noiseTemperature = (-(tsoff * yFactor) + tson) / (yFactor - 1.0);
	noiseFigure = 10.0*log10( 1.0 + noiseTemperature / REF_TEMPERATURE );

	//Checking the mean value of noise figure
	float meanNoiseFig = noiseFigure.MeanValue();
	if( 0.5 > meanNoiseFig || meanNoiseFig > 20.0 )
		throw rfims_exception("a ridiculous mean noise figure was got during estimation.");

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
		throw rfims_exception("a ridiculous mean gain was got during estimation.");
}

/*!	The calibration of sweeps with output power values implies the following tasks:
 * - Referencing the sweep to the input of the front end (antenna's output), what is done subtracting
 * the total gain curve to the sweep, taking the power values in dBm and the gain values in dB.
 * - Removing of the internal noise added to the sweep by the front end, what is done taking into
 * account its total equivalent noise temperature.
 * To perform these tasks the following equation are used:
 * \f[
 * 		P_{IN_{eff}[dBm]}=P_{OUT[dBm]}-G_{receiver[dB]}
 * \f]
 * \f[
 * 		P_{IN_{eff}[W]}=10^{\frac{P_{IN_{eff}[dBm]}}{10}}*10^{-3}
 * \f]
 * \f[
 * 		P_{IN[W]}=P_{IN_{eff}[W]}-N_{receiver[W]}=P_{IN_{eff}[W]}-k_{[J/°K]}*RBW_{[Hz]}*T_{receiver[°K]}
 * \f]
 * \f[
 * 		P_{IN[dBm]}=10*\log_{10}(P_{IN[W]})+30
 * \f]
 * \param [in] powerOut A _Sweep_ structure which stores an uncalibrated sweep.
 * \return The calibrated sweep.
 */
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
	catch(rfims_exception & exc)
	{
		exc.Prepend("a sweep could not be calibrated");
		throw;
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
		rfims_exception exc("at least one of the files with the default front end parameters were not found.");
		throw(exc);
	}
}
