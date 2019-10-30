/*
 * TopLevel.cpp
 *
 *  Created on: 30/10/2019
 *      Author: new-mauro
 */

#include "TopLevel.h"

/////////////Global variables which are used by the SignalHandler class///////////////
//! The instantiation of the pointer to the _SpectranInterface_ object.
SpectranInterface * SignalHandler::specInterfPtr=nullptr;
//! The instantiation of the pointer to the _SpectranConfigurator_ object.
SpectranConfigurator * SignalHandler::specConfiguratorPtr=nullptr;
//! The instantiation of the pointer to the _SweepBuilder_ object.
SweepBuilder * SignalHandler::sweepBuilderPtr=nullptr;
//! The instantiation of the pointer to the _CurveAjuster_ object.
CurveAdjuster * SignalHandler::adjusterPtr=nullptr;
//! The instantiation of the pointer to the _FrontEndCalibrator_ object.
FrontEndCalibrator * SignalHandler::calibratorPtr=nullptr;
//! The instantiation of the pointer to the _RFIDetector_ object.
RFIDetector * SignalHandler::rfiDetectorPtr=nullptr;
//! The instantiation of the pointer to the _DataLogger_ object.
DataLogger * SignalHandler::dataLoggerPtr=nullptr;
//! The instantiation of the pointer to the _GPSInterface_ object.
GPSInterface * SignalHandler::gpsInterfacePtr=nullptr;
//! The instantiation of the pointer to the _AntennaPositioner_ object.
AntennaPositioner * SignalHandler::antPositionerPtr=nullptr;
//! The instantiation of the pointer to the _RFPlotter_ object which is responsible for the plotting of the last captured sweep.
RFPlotter * SignalHandler::sweepPlotterPtr=nullptr;
//! The instantiation of the pointer to the _RFPlotter_ object which is responsible for the plotting of the last estimated gain curve.
RFPlotter * SignalHandler::gainPlotterPtr=nullptr;
//! The instantiation of the pointer to the _RFPlotter_ object which is responsible for the plotting of the last estimated noise figure curve.
RFPlotter * SignalHandler::nfPlotterPtr=nullptr;

void PrintHelp()
{
	cout << "Usage: rfmis-cart [--plot] [--no-frontend-cal] [--rfi={ska-mode1,ska-mode2,itu-ra769}] [--num-meas-cycles='number'] [--no-upload] [--num-azim-pos='number'] [--help | -h]" << endl;
	cout << "\nThis software was designed to capture RF power measurements from a spectrum analyzer Aaronia Spectran V4, using an antenna" << endl;
	cout << "which could be rotated to point the horizon in different azimuth angles and whose polarization could be changed between" << endl;
	cout << "horizontal and vertical. A sweep from 1 GHz (or maybe less) to 9.4 GHz is captured in each antenna position and then it is calibrated," << endl;
	cout << "it is processed to identify RF interference (RFI), it is saved into memory, it is plotted with the detected RFI and finally" << endl;
	cout << "the measurements are sent to a remote server. The software's arguments can be put in any order." << endl;
	cout << "\nThe arguments' descriptions are presented in the following:" << endl;
	cout << "\t--plot\t\t\t\t\t\tEnable the plotting of the different RF data which are got by the software." << endl;
	cout << "\t\t\t\t\t\t\t  If this argument is not given no plot is produced." << endl;
	cout << "\t--no-frontend-cal\t\t\t\tDisable the front end calibration, i.e. the estimation of the front end's parameters," << endl;
	cout <<	"\t\t\t\t\t\t\t  total gain and total noise figure, using a noise generator. Instead of that," << endl;
	cout << "\t\t\t\t\t\t\t  default front end's parameters curves are used to calibrated the sweeps." << endl;
	cout << "\t\t\t\t\t\t\t  If this argument is not given the front end calibration is performed normally," << endl;
	cout << "\t\t\t\t\t\t\t  turning the noise generator on and off." << endl;
	cout << "\t--rfi={ska-mode1,ska-mode2,itu-ra769-2-vlbi}\tEnable the identifying of RF interference (RFI). The user has to provide the norm" << endl;
	cout << "\t\t\t\t\t\t\t  (or protocol) which must be taken into account to define the harmful levels of RFI:" << endl;
	cout << "\t\t\t\t\t\t\t  The SKA protocol Mode 1, The SKA protocol Mode 2 or the ITU's recommendation." << endl;
	cout << "\t\t\t\t\t\t\t  RA.769-2. If this argument is not given the RFI identifying is not performed." << endl;
	cout << "\t--num-meas-cycles='number'\t\t\tDetermine the number of measurements cycles which must be performed. A measurement" << endl;
	cout << "\t\t\t\t\t\t\t  cycle is formed by all the sweeps which are captured while the antenna goes over" << endl;
	cout << "\t\t\t\t\t\t\t  the 360° of azimuth angle. If this argument is not given the measurement" << endl;
	cout << "\t\t\t\t\t\t\t  cycles are performed indefinitely." << endl;
	cout << "\t--no-upload\t\t\t\t\tDisable the uploading of data, i.e., the sending of collected data to the remote server." << endl;
	cout << "\t--num-azim-pos='number'\t\t\t\tDetermine the number of azimuth positions, which defined the azimuth rotation angle." << endl;
	cout << "\t\t\t\t\t\t\t  The number of sweeps which will be captured during a measurement cycle is the number" << endl;
	cout << "\t\t\t\t\t\t\t  of azimuth positions multiplied by 2. If this argument is not given, by default the number" << endl;
	cout << "\t\t\t\t\t\t\t  of positions is 6." << endl;
	cout << "\t-h, --help\t\t\t\t\tShow this help and finish there." << endl;
}

/*! \details This function determines the values of the behavior flags (flagCalEnabled, flagPlot, flagRFI, etc.) taking
 * into account the arguments that were received and its values. The function returns a `true` value if the arguments
 * were processed correctly and a `false` value if there was an argument which could not be recognized, and in that case
 * it presents a message, in the `stdout`, explaining the correct use of the software arguments.
 * \param [in] argc The number of arguments that were received by the software.
 * \param [in] argv An array of C strings (`char*`) where each one is a software's argument.
 */
bool ProcessMainArguments (int argc, char * argv[])
{
	if(argc>1)
	{
		unsigned int argc_aux=argc;
		std::list<std::string> argList;
		for(unsigned int i=1; i<argc_aux; i++)
			argList.push_back( argv[i] );

		//Searching the argument --help
		auto argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="--help" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			PrintHelp();
			return false;
		}

		//Searching the argument -h
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="-h" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			PrintHelp();
			return false;
		}

		//Searching the argument --no-frontend-cal
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="--no-frontend-cal" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagCalEnabled=false;
			argList.erase(argIter);
		}

		//Searching the argument --plot
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="--plot" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagPlot=true;
			argList.erase(argIter);
		}

		//Searching the argument --rfi=xxxxx
		argIter = argList.cbegin();
		size_t equalSignPos=0;
		while( argIter!=argList.cend() && argIter->find("--rfi=")==std::string::npos )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagRFI=true;
			equalSignPos = argIter->find('=');
			std::string rfiNormStr = argIter->substr(equalSignPos+1);
			if( rfiNormStr=="ska-mode1" )
				rfiNorm = RFI::SKA_MODE1;
			else if( rfiNormStr=="ska-mode2" )
				rfiNorm = RFI::SKA_MODE2;
			else if( rfiNormStr=="itu-ra769-2-vlbi" )
				rfiNorm = RFI::ITU_RA769_2_VLBI;
			else
			{
				cout << "rfims-cart: unrecognized argument '" << *argIter << '\'' << endl;
				cout << "Usage: rfmis-cart [--plot] [--no-frontend-cal] [--rfi={ska-mode1,ska-mode2,itu-ra769}] [--num-meas-cycles='number'] [--no-upload] [--num-azim-pos='number'] [--help | -h]" << endl;
				return false;
			}

			argList.erase(argIter);
		}

		//Searching the argument --num-meas-cycles=xx
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && argIter->find("--num-meas-cycles=")==std::string::npos )		argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagInfiniteLoop=false;
			equalSignPos = argIter->find('=');
			std::istringstream iss;
			std::string numString = argIter->substr(equalSignPos+1);
			iss.str(numString);
			iss >> numOfMeasCycles;
			argList.erase(argIter);
		}

		//Searching the argument --no-upload
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && argIter->find("--no-upload")==std::string::npos )		argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagUpload=false;
			argList.erase(argIter);
		}

		//Searching the argument --num-azim-pos=xx
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && argIter->find("--num-azim-pos=")==std::string::npos )		argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			equalSignPos = argIter->find('=');
			std::istringstream iss;
			std::string numString = argIter->substr(equalSignPos+1);
			iss.str(numString);
			iss >> numOfAzimPos;
			argList.erase(argIter);
		}


		//Checking if there were arguments which were not recognized
		if(!argList.empty())
		{
			cout << "rfims-cart: the following arguments were not recognized:";
			for(argIter = argList.cbegin(); argIter != argList.cend(); argIter++)
				cout << " \'" << *argIter << '\'';
			cout << endl;
			cout << "Usage: rfmis-cart [--plot] [--no-frontend-cal] [--rfi={ska-mode1,ska-mode2,itu-ra769}] [--num-meas-cycles='number'] [--no-upload] [--num-azim-pos='number'] [--help | -h]" << endl;
			return false;
		}
	}
	return true;
}
