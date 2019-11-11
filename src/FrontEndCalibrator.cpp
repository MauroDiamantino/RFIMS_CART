/*! \file FrontEndCalibrator.cpp
 * 	\brief This file contains the definitions of several methods of the class _FrontEndCalibrator_.
 * 	\author Mauro Diamantino
 */

#include "SweepProcessing.h"

bool CheckNoFiniteAndNegValues(const std::vector<FreqValues::value_type> & values)
{
	bool flagWrongValue = false;
	auto valueIter = values.begin();
	while( valueIter!=values.end() && flagWrongValue==false )
	{
		if( !isfinite(*valueIter) || *valueIter<=0 )
			flagWrongValue = true;
		valueIter++;
	}

	if(flagWrongValue)
		return true;

	return false;
}

bool CheckNoFiniteValues(const std::vector<FreqValues::value_type> & values)
{
	bool flagWrongValue = false;
	auto valueIter = values.begin();
	while( valueIter!=values.end() && flagWrongValue==false )
	{
		if( !isfinite(*valueIter) )
			flagWrongValue = true;
		valueIter++;
	}

	if(flagWrongValue)
		return true;

	return false;
}

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
		powerNSon("sweep"), powerNSoff_pw("sweep"), powerNSon_pw("sweep"), gain("gain"),
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
		adjuster(adj), correctENR("enr"), powerNSoff("sweep"), powerNSon("sweep"), powerNSoff_pw("sweep"),
		powerNSon_pw("sweep"), bandsParameters(bandsParam), gain("gain"), noiseTemperature("noise temperature"),
		noiseFigure("noise figure"), rbwCurve("rbw values curve"),
		auxRFPloter("Sweeps captured with a 50 ohm load at the input")
#else
/*! \param [in] adj A reference to a _CurveAdjuster_ object, which will be used to adjust some internal curves.
 * 	\param [in] bandsParam A vector with the parameters of all frequency bands.
 */
FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj, const std::vector<BandParameters> & bandsParam) :
		adjuster(adj), correctENR("enr"), powerNSoff("sweep"), powerNSon("sweep"), powerNSoff_pw("sweep"),
		powerNSon_pw("sweep"), bandsParameters(bandsParam), gain("gain"), noiseTemperature("noise temperature"),
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
	if( bandsParameters.empty() )
		throw rfims_exception("the front end calibrator could not build the RBW curve because it has not received the bands' parameters.");

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

	try
	{
		rbwCurve = adjuster.AdjustCurve(rbwCurve);
	}
	catch(rfims_exception & exc)
	{
		exc.Prepend("the adjustment of the RBW curve failed");
		throw;
	}
}

void FrontEndCalibrator::CorrectNoFiniteAndNegVal(std::vector<FreqValues::value_type> & values)
{
	//Correcting the wrong values of the noise figure curve
	bool flagWrongPrevVal = false;
	bool flagBeginWrongVal = false;
	auto meanValue = values.front();
	auto lastCorrNumIter = values.begin();

	auto nfIter = values.begin();
	for( ; nfIter!=values.end(); nfIter++)
	{
		if( !isfinite(*nfIter) || *nfIter < 0 )
		{
			if( nfIter==values.begin() )
				flagBeginWrongVal = true;
			else
			{
				if( !flagWrongPrevVal )
					lastCorrNumIter = nfIter - 1;

				if( nfIter==values.end()-1 )
				{
					auto auxIter = lastCorrNumIter + 1;
					for( ; auxIter!=values.end(); auxIter++)
						*auxIter = *lastCorrNumIter;
				}
			}

			flagWrongPrevVal = true;
		}
		else
		{
			if(flagWrongPrevVal)
			{
				if(flagBeginWrongVal)
				{
					auto auxIter = values.begin();
					for( ; auxIter!=nfIter; auxIter++)
						*auxIter = *nfIter;
					flagBeginWrongVal = false;
				}
				else
				{
					meanValue = (*nfIter + *lastCorrNumIter) / 2.0;
					auto auxIter = lastCorrNumIter + 1;
					for( ; auxIter!=nfIter; auxIter++)
						*auxIter = meanValue;
				}

				flagWrongPrevVal = false;
			}
		}
	}
}

void FrontEndCalibrator::CorrectNoFiniteVal(std::vector<FreqValues::value_type> & values)
{
	//Correcting the wrong values of the noise figure curve
	bool flagWrongPrevVal = false;
	bool flagBeginWrongVal = false;
	auto meanValue = values.front();
	auto lastCorrNumIter = values.begin();

	auto nfIter = values.begin();
	for( ; nfIter!=values.end(); nfIter++)
	{
		if( !isfinite(*nfIter) )
		{
			if( nfIter==values.begin() )
				flagBeginWrongVal = true;
			else
			{
				if( !flagWrongPrevVal )
					lastCorrNumIter = nfIter - 1;

				if( nfIter==values.end()-1 )
				{
					auto auxIter = lastCorrNumIter + 1;
					for( ; auxIter!=values.end(); auxIter++)
						*auxIter = *lastCorrNumIter;
				}
			}

			flagWrongPrevVal = true;
		}
		else
		{
			if(flagWrongPrevVal)
			{
				if(flagBeginWrongVal)
				{
					auto auxIter = values.begin();
					for( ; auxIter!=nfIter; auxIter++)
						*auxIter = *nfIter;
					flagBeginWrongVal = false;
				}
				else
				{
					meanValue = (*nfIter + *lastCorrNumIter) / 2.0;
					auto auxIter = lastCorrNumIter + 1;
					for( ; auxIter!=nfIter; auxIter++)
						*auxIter = meanValue;
				}

				flagWrongPrevVal = false;
			}
		}
	}
}

/*!	After the bands' parameters are stored, the RBW curve is built, taking into account those parameters.
 * \param [in] bandsParam A vector with the parameters of all frequency bands.
 */
void FrontEndCalibrator::SetBandsParameters(const std::vector<BandParameters> & bandsParam)
{
	bandsParameters = bandsParam;
}

/*!	The ENR values versus frequency of the noise generator are loaded from the file
 * [BASE_PATH](\ref BASE_PATH)/calibration/enr.txt, then those values are corrected taking into account
 * a statistical mean physical temperature of the noise source at time of the factory calibration.
 * That technique is exposed in the Application Note "Noise Figure Measurement Accuracy – The Y-Factor
 * Method" of Keysight Technologies. To finish, the corrected ENR curve is adjusted to be used in the
 * mathematical operations with the captured sweeps.
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
		try
		{
			correctENR = adjuster.AdjustCurve(correctENR);
		}
		catch(rfims_exception & exc)
		{
			exc.Prepend("the adjustment of the ENR curve failed");
			throw;
		}
	}
}

void FrontEndCalibrator::StartCalibration()
{
#ifdef MANUAL
	cout << "Turn off the noise source, switch the input to the noise source and " << endl;

	#ifdef BUTTON
		cout << "press the button to continue..." << endl;
		while( digitalRead(piPins.BUTTON_ENTER)==pinsValues.BUTTON_OFF );
	#else
		cout << "press enter to continue..." << endl;
		WaitForEnter();
	#endif
#endif

	TurnOffNS();

#ifdef RASPBERRY_PI
	digitalWrite(piPins.SWITCH, pinsValues.SWITCH_TO_NS);
#endif

	flagCalStarted=true;
}

void FrontEndCalibrator::TurnOnNS()
{
#ifdef MANUAL
	cout << "\nTurn on the noise source and " << endl;

	#ifdef BUTTON
		cout << "press the button to continue..." << endl;
		while( digitalRead(piPins.BUTTON_ENTER)==pinsValues.BUTTON_OFF );
	#else
		cout << "press enter to continue..." << endl;
		WaitForEnter();
	#endif
#endif

#ifdef RASPBERRY_PI
	digitalWrite(piPins.NOISE_SOURCE, pinsValues.NS_ON);
#endif

	flagNSon = true;
}

void FrontEndCalibrator::TurnOffNS()
{
#ifdef RASPBERRY_PI
	digitalWrite(piPins.NOISE_SOURCE, pinsValues.NS_OFF);
#endif

	flagNSon = false;
}

void FrontEndCalibrator::EndCalibration()
{
#ifdef MANUAL
	cout << "\nTurn off the noise source, switch the input to the antenna and " << endl;

	#ifdef BUTTON
		cout << "press the button to continue..." << endl;
		while( digitalRead(piPins.BUTTON_ENTER)==pinsValues.BUTTON_OFF );
	#else
		cout << "press enter to continue..." << endl;
		WaitForEnter();
	#endif
#endif

	TurnOffNS();

#ifdef RASPBERRY_PI
	digitalWrite(piPins.SWITCH, pinsValues.SWITCH_TO_ANT);
#endif

	flagCalStarted=false;
}

void FrontEndCalibrator::SetSweep(const FreqValues & sweep_dbm)
{
	if(flagNSon)
	{
		powerNSon = sweep_dbm;
		//powerNSon_w = pow(10.0, sweep_dbm/10.0 ) * 1e-3; //The power values are converted from dBm to Watts
		powerNSon_pw = pow(10.0, (sweep_dbm + 90.0)/10.0); //The power values are converted from dBm to pW
	}
	else
	{
		powerNSoff = sweep_dbm;
		//powerNSoff_w = pow(10.0, sweep_dbm/10.0 ) * 1e-3; //The power values are converted from dBm to Watts
		powerNSoff_pw = pow(10.0, (sweep_dbm + 90.0)/10.0); //The power values are converted from dBm to pW
	}
}

/*! This method must be called once the calibration process finished, i.e. the method `EndCalibration()` must be called first.
 * 	The front end parameters, total gain and total noise figure, are estimated using the Y-Factor method, which is exposed in
 * 	the Application Note "Noise Figure Measurement Accuracy – The Y-Factor Method" of Keysight Technologies.
 *
 * 	To perform the estimation of the front end parameters the following equations are used:
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
 *
 * Once the parameters' curves have been estimated, their mean values are checked to be reasonable.
 */
void FrontEndCalibrator::EstimateParameters()
{
	//if( powerNSon_w.Empty() || powerNSoff_w.Empty() )
	if( powerNSon_pw.Empty() || powerNSoff_pw.Empty() )
		throw rfims_exception("the front end calibrator could not estimate the parameters because one sweep (or both) was lacking.");

	if( correctENR.Empty() )
		throw rfims_exception("the front end calibrator could not estimate the parameters because the ENR curve is empty, remember to ask the calibrator to load that curve before the calibration.");

	if( rbwCurve.Empty() )
		throw rfims_exception("the front end calibrator could not estimate the parameters because the RBW curve is empty, remember to ask the calibrator to build that curve before the calibration.");

	FreqValues tson("noise temperature"), yFactor("y-factor"), gainPowersRatio("gain");

	//Calculating the front end's noise figure curve
	tson = (REF_TEMPERATURE * correctENR) + tsoff;

	//////////7
//	if( CheckNoFiniteAndNegValues(tson.values) )
//		cout << "Hay valores incorrectos en tson" << endl;
	///////////

	//yFactor = powerNSon_w / powerNSoff_w;
	yFactor = powerNSon_pw / powerNSoff_pw;

	//auxPlotter2.Plot(yFactor, "lines", "yFactor");

	//////////7
//	if( CheckNoFiniteAndNegValues(yFactor.values) )
//		cout << "Hay valores incorrectos en yFactor" << endl;
	///////////

	noiseTemperature = (tson - (tsoff * yFactor)) / (yFactor - 1.0); //Aqui se producen los valores erroneos (nan)

	//////////7
//	if( CheckNoFiniteAndNegValues(noiseTemperature.values) )
//		cout << "Hay valores incorrectos en noiseTemperature" << endl;
	///////////

	noiseFigure = 10.0*log10( 1.0 + noiseTemperature / REF_TEMPERATURE );

	//////////7
//	if( CheckNoFiniteAndNegValues(noiseFigure.values) )
//		cout << "Hay valores incorrectos en noiseFigure" << endl;
	///////////

	//Correcting the wrong values of the noise figure curve
	CorrectNoFiniteAndNegVal(noiseFigure.values);
	
	//Checking the mean value of noise figure
	auto meanNoiseFig = noiseFigure.MeanValue();
	if( 0.5 > meanNoiseFig || meanNoiseFig > 20.0 )
	{
		std::ostringstream oss;
		oss << "a ridiculous mean noise, " << meanNoiseFig << " dB, was got during estimation.";
		throw rfims_exception( oss.str() );
	}

	//Calculating the front end's gain curve
	//gainPowersRatio = 0.5/(BOLTZMANN_CONST * rbwCurve) * ( powerNSoff_w/(tsoff + noiseTemperature) + powerNSon_w/(tson + noiseTemperature) );
	gainPowersRatio = 0.5/(BOLTZMANN_CONST * rbwCurve) * ( 1e-12*powerNSoff_pw/(tsoff + noiseTemperature) + 1e-12*powerNSon_pw/(tson + noiseTemperature) );

	//////////7
//	if( CheckNoFiniteAndNegValues(gainPowersRatio.values) )
//		cout << "Hay valores incorrectos en gainPowersRatio" << endl;
	///////////

	gain = 10.0*log10(gainPowersRatio);

	//////////7
//	if( CheckNoFiniteAndNegValues(gain.values) )
//		cout << "Hay valores incorrectos en gain" << endl;
	///////////

	//Correcting the wrong values of the gain curve
	CorrectNoFiniteAndNegVal(gain.values);

	//Checking the mean value of gain
	auto meanGain = gain.MeanValue();
	if( 10.0 > meanGain || meanGain > 100.0 )
	{
		std::ostringstream oss;
		oss << "a ridiculous mean gain, " << meanGain << " dB, was got during estimation.";
		throw rfims_exception( oss.str() );
	}

	//Setting correctly the time data
	//noiseTemperature.timeData = noiseFigure.timeData = gain.timeData = powerNSoff_w.timeData;
	noiseTemperature.timeData = noiseFigure.timeData = gain.timeData = powerNSoff_pw.timeData;
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
	if( gain.Empty() || noiseTemperature.Empty() )
		throw rfims_exception("the front end calibrator could not calibrate the sweep because the front end parameters have not been estimated (or loaded) before.");

#ifdef DEBUG
	auxRFPloter.Clear();
	auxRFPloter.Plot(powerOut, "lines", "Uncalibrated sweep");
#endif
	Sweep powerInEff, powerInEff_pw;
	Sweep powerIn, powerIn_pw;
	FreqValues frontEndNoise_pw("noise");
	try
	{
		//Calculating the effective input power (Pin_eff) which contains the antenna power and the
		//internal noise generated in the receiver
		powerInEff = powerOut - gain;

		//////////7
//		if( CheckNoFiniteValues(powerInEff.values) )
//			cout << "Hay valores incorrectos en powerInEff" << endl;
		///////////

#ifdef DEBUG
		auxRFPloter.Plot(powerInEff, "lines", "Effective input power (Pant + Nreceiver)");
#endif
		//powerInEff_w = pow(10.0, powerInEff/10.0) * 1e-3; //The power values are converted from dBm to Watts
		powerInEff_pw = pow(10.0, (powerInEff + 90.0)/10.0); //The power values are converted from dBm to pW

		//////////7
//		if( CheckNoFiniteAndNegValues(powerInEff_pw.values) )
//			cout << "Hay valores incorrectos en powerInEff_pw" << endl;
		///////////

		//Calculating the input power (Pin) which represents only the antenna power, without the receiver noise
		//powerIn_w = powerInEff_w - BOLTZMANN_CONST * rbwCurve * noiseTemperature;
		frontEndNoise_pw = 1e12 * BOLTZMANN_CONST * rbwCurve * noiseTemperature;
		powerIn_pw = powerInEff_pw - frontEndNoise_pw;

		//////////7
//		if( CheckNoFiniteAndNegValues(powerIn_pw.values) )
//			cout << "Hay valores incorrectos en powerIn_pw" << endl;
		///////////

		//powerIn = 10.0*log10(powerIn_w) + 30.0; //Converting the input power from Watts to dBm
		powerIn = 10.0*log10(powerIn_pw) - 90.0; //Converting the input power from pW to dBm

		//////////7
//		if( CheckNoFiniteValues(powerIn.values) )
//			cout << "Hay valores incorrectos en powerIn" << endl;
		///////////

#ifdef DEBUG
		auxRFPloter.Plot(powerIn, "lines", "Input power (Pant)");
#endif

		//The input power is saved as the calibrated sweep
		calSweep = powerIn;

		//Correcting wrong power values (nan)
		CorrectNoFiniteVal(calSweep.values);

		//Setting correctly the auxiliary data
		calSweep.azimuthAngle = powerOut.azimuthAngle;
		calSweep.polarization = powerOut.polarization;
		calSweep.timeData = powerOut.timeData;
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

		//Extracting the frequency values of noise figure curve
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

		try
		{
			gain = adjuster.AdjustCurve(defaultGain);
		}
		catch(rfims_exception & exc)
		{
			exc.Prepend("the adjustment of the estimated gain curve failed");
			throw;
		}

		try
		{
			noiseFigure = adjuster.AdjustCurve(defaultNoiseFig);
		}
		catch(rfims_exception & exc)
		{
			exc.Prepend("the adjustment of the estimated noise figure curve failed");
			throw;
		}

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
