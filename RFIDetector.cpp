/*
 * RFIDetector.cpp
 *
 *  Created on: 01/06/2019
 *      Author: new-mauro
 */

#include "SweepProcessing.h"

/////////////////Implementations of the RFIDetector class' methods//////////////////////////////

void RFIDetector::LoadThreshCurve(const RFI::ThresholdsNorm thrNorm)
{
	boost::filesystem::path pathAndFilename(THRESHOLDS_PATH);

	rfi.threshNorm = thrNorm;

	switch(rfi.threshNorm)
	{
	case RFI::ThresholdsNorm::ITU_RA769_2_VLBI:
		pathAndFilename /= "itu_ra769-2_vlbi.txt";
		break;
	case RFI::ThresholdsNorm::SKA_MODE1:
		pathAndFilename /= "ska_mode1.txt";
		break;
	case RFI::ThresholdsNorm::SKA_MODE2:
	default:
		pathAndFilename /= "ska_mode2.txt";
	}

	if( thresholdsCurve.Empty() || threshFileLastWriteTime < boost::filesystem::last_write_time(pathAndFilename) )
	{
		//The threshold curve must be loaded by first time or the corresponding file has been changed

		//Saving the last write time of the corresponding file
		threshFileLastWriteTime = boost::filesystem::last_write_time(pathAndFilename);

		FreqValues fluxDensityThrCurve;
		std::ifstream ifs( pathAndFilename.string() );
		std::string line;
		float freqMHz, fluxDensity;
		char delimiter;
		bool flagHeader=true;

		//Extracting the header
		while(flagHeader)
		{
			std::getline(ifs, line);
			boost::algorithm::to_lower(line);
			if( line.find("norm") == std::string::npos )
				if( line.find("//") != std::string::npos )
					flagHeader=false;
		}

		//////Values extraction loop///////
		do
		{
			ifs >> freqMHz >> delimiter;
			if( ifs.peek()==' ' )
				ifs >> delimiter;
			ifs >> fluxDensity;

			//The frequency values are saved in Hz as an integer value
			fluxDensityThrCurve.frequencies.push_back( (std::uint_least64_t) (freqMHz*1e6) );
			//The flux density values are saved as-is, in dB[W*m^-2*Hz^-1]
			fluxDensityThrCurve.values.push_back(fluxDensity);

			ifs.get(); //The character '\n' is extracted
			ifs.peek(); //This function ensures the 'eofbit' is updated
		}while( !ifs.eof() );

		//Adjusting the thresholds curve taking into account the bands parameters
		fluxDensityThrCurve = adjuster.AdjustCurve(fluxDensityThrCurve);

		//Converting from flux density (dB[W*m^-2*Hz^-1]) to power (dBm)
		double antAperture, threshPower_w;
		auto itFrequency = fluxDensityThrCurve.frequencies.begin();
		auto itFluxDensity = fluxDensityThrCurve.values.begin();
		auto itBandParam = bandsParameters.begin();
		for( ; itFrequency != fluxDensityThrCurve.frequencies.end(), itFluxDensity != fluxDensityThrCurve.values.end();
			++itFrequency, ++itFluxDensity)
		{
			if( *itFrequency > itBandParam->stopFreq )
				if( ++itBandParam == bandsParameters.end() )
					--itBandParam;

			antAperture = pow(10.0, ANTENNA_GAIN/10.0) * pow(SPEED_OF_LIGHT, 2) / ( 4.0 * M_PI * pow(*itFrequency, 2) ); //m²
			threshPower_w = pow(10.0, *itFluxDensity/10.0) * antAperture * itBandParam->rbw; //Watts
			thresholdsCurve.values.push_back( 10.0*log10(threshPower_w) + 30.0 ); //Watts to dBm
		}
	}
}

const RFI & RFIDetector::DetectRFI(const Sweep & sweep)
{
	rfi.Clear();

	rfi.azimuthAngle=sweep.azimuthAngle;
	rfi.polarization=sweep.polarization;
	rfi.timeData=sweep.timeData;

	bool flagPreviousDetection=false;
	auto itFrequency = sweep.frequencies.begin();
	auto itPower = sweep.values.begin();
	auto itThreshold = thresholdsCurve.values.begin();

	for( ; itFrequency!=sweep.frequencies.end(), itPower!=sweep.values.end(), itThreshold!=thresholdsCurve.values.end();
			itFrequency++, itPower++, itThreshold++)
	{
		if( *itPower > *itThreshold)
		{
			rfi.frequencies.push_back( *itFrequency );
			rfi.values.push_back( *itPower );

			if(!flagPreviousDetection)
			{
				++rfi.numOfRFIBands;
				flagPreviousDetection=true;
			}
		}
		else
			flagPreviousDetection=false;
	}

	return rfi;
}
