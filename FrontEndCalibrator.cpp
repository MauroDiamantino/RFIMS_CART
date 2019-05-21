/*
 * FrontEndCalibrator.cpp
 *
 *  Created on: 18/04/2019
 *      Author: new-mauro
 */

#include "SweepProcessing.h"

FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj) : correctENR("enr"), powerNSoff("sweep"), powerNSon("sweep"),
		noiseTempNSoff("noise temperature"), noiseTempNSon("noise temperature"), adjuster(adj), rbwCurve("rbw values curve")
{
	enrFileLastWriteTime = -100;
	tsoff = REF_TEMPERATURE;
#ifdef RASPBERRY_PI
	pinMode(piPins.NOISE_SOURCE, OUTPUT);
	pinMode(piPins.SWITCH, OUTPUT);
	digitalWrite(piPins.NOISE_SOURCE, LOW);
	digitalWrite(piPins.SWITCH, SWITCH_TO_ANTENNA);
#endif
	flagNSon = false;
	flagCalStarted = false;
}

FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj, std::vector<BandParameters> & bandsParam) : correctENR("enr"),
		powerNSoff("sweep"), powerNSon("sweep"), noiseTempNSoff("noise temperature"), noiseTempNSon("noise temperature"),
		 bandsParameters(bandsParam), adjuster(adj), rbwCurve("rbw values curve")
{
	enrFileLastWriteTime = -100;
	tsoff = REF_TEMPERATURE;
#ifdef RASPBERRY_PI
	digitalWrite(piPins.NOISE_SOURCE, LOW);
	digitalWrite(piPins.SWITCH, SWITCH_TO_ANTENNA);
#endif
	flagNSon = false;
	flagCalStarted = false;
}

void FrontEndCalibrator::LoadENR()
{
	boost::filesystem::path pathAndFilename(FILES_PATH);
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
			enr.frequencies.push_back( freqGHz*1e9 );
			enr.values.push_back( pow(10.0, enrValuedB/10.0) );
		}

		//Extracting the ENR and frequency values
		do
		{
			ifs >> freqGHz >> delimiter >> enrValuedB;
			ifs.get();
			enr.frequencies.push_back( freqGHz*1e9 );
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

void FrontEndCalibrator::SetSweep(const FreqValues & sweep)
{
	if(flagNSon)
		powerNSon = pow(10.0, sweep/10.0 ) * 1e-3; //The power values are converted from dBm to Watts
	else
		powerNSoff = pow(10.0, sweep/10.0 ) * 1e-3; //The power values are converted from dBm to Watts
}

//const FrontEndParameters& FrontEndCalibrator::EstimateParameters()
//{
//	if( powerNSon.Empty() || powerNSoff.Empty() )
//		throw( CustomException("The Front End calibrator cannot calculate parameters because one sweep (or both) is lacking.") );
//
//	FreqValues tson("noise temperature"), yFactor("y-factor"), frontEndNoiseTemp("noise temperature");
//	FreqValues frontEndNoiseFigure("noise figure"), frontEndGaindB("gain_dB");
//
//	//Calculating the front end's noise figure curve
//	tson = (REF_TEMPERATURE * correctENR) + tsoff;
//	yFactor = powerNSon / powerNSoff;
//	frontEndNoiseTemp = (tson - tsoff * yFactor) / (yFactor - 1.0);
//	frontEndNoiseFigure = 10.0*log10( 1.0 + frontEndNoiseTemp / REF_TEMPERATURE );
//
//	//Calculating the front end's gain curve
//	auto itPowerNSoff = powerNSoff.values.begin();
//	auto itPowerNSon = powerNSon.values.begin();
//	auto itTson = tson.values.begin();
//	auto itFENoiseTemp = frontEndNoiseTemp.values.begin();
//	auto itBand = bandsParameters.begin();
//	auto itFreq = powerNSoff.frequencies.begin();
//	float gain;
//
//	for( ; itPowerNSoff!=powerNSoff.values.end(), itPowerNSon!= powerNSon.values.end(), itTson!=tson.values.end(),
//		itFENoiseTemp!=frontEndNoiseTemp.values.end(), itBand!=bandsParameters.end(), itFreq!=powerNSoff.frequencies.end();
//		itPowerNSoff++, itPowerNSon++, itTson++, itFENoiseTemp++, itFreq++)
//	{
//		if( *itFreq > itBand->stopFreq )
//			itBand++;
//
//		gain = 0.5/(BOLTZMANN_CONST * itBand->rbw) * ( (*itPowerNSoff)/(tsoff + *itFENoiseTemp) + (*itPowerNSon)/(*itTson + *itFENoiseTemp) );
//		frontEndGaindB.values.push_back( 10.0*log10(gain) );
//	}
//
//	//Saving the estimated parameters in a structure
//	frontEndParam.noiseTemperature = frontEndNoiseTemp.values;
//	frontEndParam.noiseFigure = frontEndNoiseFigure.values;
//	frontEndParam.gain_dB = frontEndGaindB.values;
//	frontEndParam.frequency = frontEndNoiseTemp.frequencies;
//
//	return frontEndParam;
//}

void FrontEndCalibrator::EstimateParameters()
{
	if( powerNSon.Empty() || powerNSoff.Empty() )
		throw( CustomException("The Front End calibrator cannot calculate parameters because one sweep (or both) is lacking.") );

	FreqValues tson("noise temperature"), yFactor("y-factor");

	//Calculating the front end's noise figure curve
	tson = (REF_TEMPERATURE * correctENR) + tsoff;
	yFactor = powerNSon / powerNSoff;
	noiseTemperature = (tson - tsoff * yFactor) / (yFactor - 1.0);
	noiseFigure = 10.0*log10( 1.0 + noiseTemperature / REF_TEMPERATURE );

	//Building the rbw values curve
	auto itBand = bandsParameters.begin();
	for(auto itFreq = powerNSoff.frequencies.begin(); itFreq!=powerNSoff.frequencies.end(); itFreq++)
	{
		if( *itFreq > itBand->stopFreq )
			itBand++;
		rbwCurve.values.push_back( itBand->rbw );
	}
	rbwCurve.frequencies=powerNSoff.frequencies;
	rbwCurve.timeData=powerNSoff.timeData;

	//Calculating the front end's gain curve
	gain = 0.5/(BOLTZMANN_CONST * rbwCurve) * ( (powerNSoff)/(tsoff + noiseTemperature) + (powerNSon)/(tson + noiseTemperature) );
}

void FrontEndCalibrator::SaveFrontEndParam(const TimeData & timeData)
{
	boost::filesystem::path filePath(FILES_PATH);
	filePath /= "frontendparam";
	if( !boost::filesystem::exists(filePath) )
		boost::filesystem::create_directory(filePath);

	std::string filename("noisefigure_");
	filename += timeData.date() + ".csv";
	filePath /= filename;
	std::ofstream ofs( filePath.string() );
	ofs.setf(std::ios::fixed, std::ios::floatfield);

	//Saving the noise figure data
	ofs << "Timestamp";
	for(auto& freq : noiseFigure.frequencies)
		ofs << ',' << std::setprecision(3) << (freq/1e6);
	ofs << "\r\n";
	ofs << timeData.timestamp();
	for(auto& nf : noiseFigure.values)
		ofs << ',' << std::setprecision(1) << nf;
	ofs << "\r\n";

	ofs.close();

	filename = "gain_" + timeData.date() + ".csv";
	filePath.remove_filename();
	filePath /= filename;
	ofs.open( filePath.string() );

	//Saving the gain data
	ofs << "Timestamp";
	for(auto& freq : gain.frequencies)
		ofs << ',' << std::setprecision(3) << (freq/1e6);
	ofs << "\r\n";
	ofs << timeData.timestamp();
	for(auto& g : gain.values)
		ofs << ',' << std::setprecision(1) << g;
	ofs << "\r\n";

	ofs.close();
}

const Sweep& FrontEndCalibrator::CalibrateSweep(const Sweep& powerOut)
{
	///////
	RFPloter rfPloter("Sweeps captured with a 50 ohm load at the input");
	rfPloter.Plot(powerOut, "lines", "Uncalibrated sweep");
	/////////
	Sweep powerInEff, powerInEff_mw;
	Sweep powerIn, powerIn_mw;
	try
	{
		//Calculating the effective input power (Pin_eff) which contains the antenna power and the internal noise
		//generated in the receiver
		powerInEff = powerOut + gain;
		/////////
		rfPloter.Plot(powerInEff, "lines", "Effective input power (Pant + Nreceiver)");
		/////////
		powerInEff_mw = pow(10.0, powerInEff/10.0);

		//Calculating the input power (Pin) which represents only the antenna power, without the receiver noise
//		auto itPowerInEff = powerInEff.values.begin();
//		auto itFrontEndTemp = frontEndParam.noiseTemperature.begin();
//		auto itFreq = powerInEff_mw.frequencies.begin();
//		auto itBand = bandsParameters.begin();
//		float powerInValue_mw;
//
//		for( ; itPowerInEff!=powerInEff.values.end(), itFrontEndTemp!=frontEndParam.noiseTemperature.end(),
//			itFreq!=powerInEff_mw.frequencies.end(), itBand!=bandsParameters.end(); itPowerInEff++,
//			itFrontEndTemp++, itFreq++)
//		{
//			if( *itFreq > itBand->stopFreq )
//				itBand++;
//
//			powerInValue_mw = *itPowerInEff - BOLTZMANN_CONST * itBand->rbw * *itFrontEndTemp;
//			powerIn_mw.values.push_back(powerInValue_mw);
//		}
//		powerIn_mw.azimuthAngle = powerInEff.azimuthAngle;
//		powerIn_mw.frequencies = powerInEff.frequencies;
//		powerIn_mw.polarization = powerInEff.polarization;
//		powerIn_mw.timeData = powerInEff.timeData;

		//////////////
		powerIn_mw = powerInEff_mw - BOLTZMANN_CONST * rbwCurve * noiseTemperature;
		//////////////

		//Converting the input power to dBm
		powerIn = 10.0*log10(powerIn_mw);
		////////
		rfPloter.Plot(powerIn, "lines", "Input power (Pant)");
		////////

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
