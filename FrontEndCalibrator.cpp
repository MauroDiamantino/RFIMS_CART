/*
 * FrontEndCalibrator.cpp
 *
 *  Created on: 18/04/2019
 *      Author: new-mauro
 */

#include "SweepProcessing.h"

FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj) : corrENR("enr"), sweepNSoff("sweep"), sweepNSon("sweep"),
		noiseTempNSoff("noise temperature"), noiseTempNSon("noise temperature"), adjuster(adj)
{
	enrFileLastWriteTime = -100;
	tsoff = REF_TEMPERATURE;
#ifdef RASPBERRY_PI
	digitalWrite(piPins.NOISE_SOURCE, LOW);
	digitalWrite(piPins.SWITCH, SWITCH_TO_ANTENNA);
#endif
	flagNSon = false;
}

FrontEndCalibrator::FrontEndCalibrator(CurveAdjuster & adj, std::vector<BandParameters> & bandsParam) : corrENR("enr"),
		sweepNSoff("sweep"), sweepNSon("sweep"), noiseTempNSoff("noise temperature"), noiseTempNSon("noise temperature"),
		 bandsParameters(bandsParam), adjuster(adj)
{
	enrFileLastWriteTime = -100;
	tsoff = REF_TEMPERATURE;
#ifdef RASPBERRY_PI
	digitalWrite(piPins.NOISE_SOURCE, LOW);
	digitalWrite(piPins.SWITCH, SWITCH_TO_ANTENNA);
#endif
	flagNSon = false;
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

		//Checking if there is a comment line and extracting it
		if( ifs.peek()=='/' )
		{
			char aux[50];
			ifs.getline(aux, 50);
		}

		//Extracting the ENR and frequency values
		FreqValueSet enr("enr");
		float frequency, enrValue;
		char delimiter;
		do
		{
			ifs >> frequency >> delimiter >> enrValue;
			ifs.get();
			enr.frequencies.push_back( frequency*1e6 );
			enr.values.push_back( pow(10.0, enrValue/10.0) );
			ifs.peek();
		}while( !ifs.eof() );

		//Correcting the ENR values
		if(deviceName=="346c" || deviceName=="n4002a")
			corrENR = enr + (REF_TEMPERATURE - 304.8)/REF_TEMPERATURE;
		else if(deviceName=="n4000a" || deviceName=="n4001a" || deviceName=="346a" || deviceName=="346b")
			corrENR = enr + (REF_TEMPERATURE - 302.8)/REF_TEMPERATURE;
		else
			corrENR = enr;

		//Adjusting the curve (interpolation, extrapolation and/or decimation)
		corrENR = adjuster.AdjustCurve(corrENR);
	}
}

void FrontEndCalibrator::SetSweep(const FreqValueSet & sweep)
{
	if(flagNSon)
		sweepNSon = pow(10.0, (1/10.0)*sweep );
	else
		sweepNSoff = pow(10.0, (1/10.0)*sweep );
}

void FrontEndCalibrator::CalculateOutNoiseTemps()
{
	//Calculating the noise temperature of the noise measured by the spectrum analyzer when the Noise Source is turned off
	for(auto itBand=bandsParameters.begin(); itBand!=bandsParameters.end(); itBand++)
	{
		while( itBand->flagEnable==false )
			itBand++;

		noiseTempNSoff = sweepNSoff * ( 1.0 / ( 1e3 * BOLTZMANN_CONST * itBand->rbw ) );
	}

	//Calculating the noise temperature of the noise measured by the spectrum analyzer when the Noise Source is turned on
	for(auto itBand=bandsParameters.begin(); itBand!=bandsParameters.end(); itBand++)
	{
		while( itBand->flagEnable==false )
			itBand++;

		noiseTempNSon = sweepNSon * ( 1.0 / ( 1e3 * BOLTZMANN_CONST * itBand->rbw ) );
	}
}

const FrontEndParameters& FrontEndCalibrator::CalculateParameters()
{
	if( sweepNSon.Empty() || sweepNSoff.Empty() )
		throw( CustomException("The Front End calibrator cannot calculate parameters because one sweep (or both) is lacking.") );

	FreqValueSet tson("noise temperature"), yFactor("y-factor"), frontEndNoiseTemp("noise temperature");
	FreqValueSet frontEndNoiseFigure("noise figure"), frontEndGain("gain");

	tson = (REF_TEMPERATURE * corrENR) + tsoff;
	yFactor = sweepNSon / sweepNSoff;
	frontEndNoiseTemp = (tson - tsoff * yFactor) / (yFactor - 1.0); //- tson;
	//frontEndNoiseTemp = frontEndNoiseTemp - tson;
	frontEndNoiseFigure = 10.0*log10( 1.0 + (1/REF_TEMPERATURE) * frontEndNoiseTemp );
	CalculateOutNoiseTemps();
	frontEndGain = 0.5*( noiseTempNSon / (tson + frontEndNoiseTemp) + noiseTempNSoff / (tsoff + frontEndNoiseTemp) );
	//frontEndGain = noiseTempNSon / (tson + frontEndNoiseTemp);

	frontEndParam.noiseTemperature = frontEndNoiseTemp.values;
	frontEndParam.noiseFigure = frontEndNoiseFigure.values;
	FreqValueSet frontEndGaindB = 10.0*log10(frontEndGain);
	frontEndParam.gain_dB = frontEndGaindB.values;
	frontEndParam.frequency = frontEndNoiseTemp.frequencies;

	return frontEndParam;
}

void FrontEndCalibrator::SaveFrontEndParam(const TimeData & timeData)
{
	boost::filesystem::path filePath(FILES_PATH);
	filePath /= "frontendparam";
	if( !boost::filesystem::exists(filePath) )
		boost::filesystem::create_directory(filePath);

	std::string filename("noisefigure");
	filename += timeData.date() + ".csv";
	filePath /= filename;
	std::ofstream ofs( filePath.string() );

	//Saving the noise figure data
	ofs << "Timestamp";
	for(auto& freq : frontEndParam.frequency)
		ofs << ',' << freq;
	ofs << "\r\n";
	ofs << timeData.timestamp();
	for(auto& nf : frontEndParam.noiseFigure)
		ofs << ',' << nf;
	ofs << "\r\n";

	ofs.close();

	filename = "gain" + timeData.date() + ".csv";
	filePath.remove_filename();
	filePath /= filename;
	ofs.open( filePath.string() );

	//Saving the gain data
	ofs << "Timestamp";
	for(auto& freq : frontEndParam.frequency)
		ofs << ',' << freq;
	ofs << "\r\n";
	ofs << timeData.timestamp();
	for(auto& gain : frontEndParam.gain_dB)
		ofs << ',' << gain;
	ofs << "\r\n";

	ofs.close();
}
