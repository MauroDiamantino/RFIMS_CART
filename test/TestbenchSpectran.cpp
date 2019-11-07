/*
 * TestbenchSpectran.cpp
 *
 *  Created on: 03/11/2019
 *      Author: new-mauro
 */

#include "../src/TopLevel.h"

//#//////////////////////GLOBAL VARIABLES////////////////////////

unsigned int numOfFiles = 1;
unsigned int numOfSweepsPerFile = 10;
bool flagCalibrateSweeps = true;
bool flagTestPlot=false;

//#//////////////////////////////////////////////////////////////


bool ProcessTestArguments(int argc, char * argv[]);


//#//////////////////MAIN FUNCTION///////////////////////////////

int main(int argc, char * argv[])
{
	//#///////Local variables///////////
	SignalHandler signalHandler;
	bool flagEndIterations=false;
	bool flagSetBandsParamAndRefSweep=true;
	unsigned int fileIndex=0;
	unsigned int sweepIndex=0;
	//#/////////////////////////////////

	//Checking of the software's arguments
	if( !ProcessTestArguments(argc, argv) )
		std::exit(EXIT_FAILURE);

	cout << "\n\t\t\t\tTestbench of the spectrum analyzer Aaronia Spectran HF-60105 V4 X" << endl;

	//A timer is initiated to know the elapsed time when the software finish
	timer.start();

	//Plotters
	RFPlotter sweepPlotter, gainPlotter, nfPlotter;

	try
	{
		//#///////////////////////////////////INITIALIZATIONS//////////////////////////////////////////

		SpectranInterface specInterface;
		SpectranConfigurator specConfigurator(specInterface);
		SweepBuilder sweepBuilder(specInterface);
		CurveAdjuster curveAdjuster;
		FrontEndCalibrator frontEndCalibrator(curveAdjuster);
		DataLogger dataLogger;

		//Setting of pointers to objects which are used by SignalHandler class
		SignalHandler::specInterfPtr = &specInterface;
		SignalHandler::specConfiguratorPtr = &specConfigurator;
		SignalHandler::sweepBuilderPtr = &sweepBuilder;
		SignalHandler::adjusterPtr = &curveAdjuster;
		SignalHandler::calibratorPtr = &frontEndCalibrator;
		SignalHandler::dataLoggerPtr = &dataLogger;
		SignalHandler::sweepPlotterPtr = &sweepPlotter;
		SignalHandler::gainPlotterPtr = &gainPlotter;
		SignalHandler::nfPlotterPtr = &nfPlotter;

		//Initializing the spectrum analyzer
		cout << "\nInitializing the spectrum analyzer Aaronia Spectran HF-60105 V4 X..." << endl;
		specInterface.Initialize();
		cout << "The spectrum analyzer was initialized successfully" << endl;

		//Setting the number of sweeps file in the object data logger
		dataLogger.SetNumOfSweeps(numOfSweepsPerFile);

		//Setting the timestamp source of the filename where the sweeps will be stored
		dataLogger.SetFilenameTimestampSrc(DataLogger::SWEEP);

		//#///////////////////////////////END OF THE INITIALIZATION///////////////////////////////////////


		//#/////////////////////LOADING OF THE SPECTRAN'S PARAMETERS//////////////////////////

		//Loading the Spectran parameters
		cout << "\nLoading the Spectran's configuration parameters from the corresponding files" << endl;

		//Loading the fixed parameters
		cout << "Loading the fixed parameters..." << endl;
		if( specConfigurator.LoadFixedParameters() )
		{
			//If the fixed parameters were loaded for the first time or they were reloaded, the initial
			//configuration will be repeated
			cout << "The fixed parameters were loaded, so the initial configuration of the Spectran will be performed" << endl;
			specConfigurator.InitialConfiguration();
			cout << "The initial configuration was carried out successfully" << endl;
		}

		//Loading the frequency bands parameters
		cout << "Loading the frequency bands' parameters..." << endl;
		specConfigurator.LoadBandsParameters();
		cout << "The frequency bands' parameters were loaded successfully" << endl;

		//Passing the bands parameters to the objects which need them
		auto bandsParameters = specConfigurator.GetBandsParameters();
		curveAdjuster.SetBandsParameters(bandsParameters);
		frontEndCalibrator.SetBandsParameters(bandsParameters);

		//#///////////////////END OF THE LOADING OF THE SPECTRAN'S PARAMETERS////////////////

		if(flagCalibrateSweeps)
		{
			//The front end calibration is performed once at the beginning
			cout << "\nStarting the front end calibration" << endl;
			frontEndCalibrator.StartCalibration();
		}


		//#/////////////////////////////////////GENERAL LOOP////////////////////////////////////////////

		while(!flagEndIterations)
		{
			Sweep uncalSweep;

			//The timestamp, azimuth angle and antenna polarization of each sweep are configured here with
			//ridiculous values because these parameters are not important in this software.
			//The timestamp define the name of the files where the sweeps are stored, and because of that
			//the seconds are set with the file index to avoid the files to be overwritten.
			uncalSweep.timeData.year=0; uncalSweep.timeData.month=0; uncalSweep.timeData.day=0;
			uncalSweep.timeData.hour=0; uncalSweep.timeData.minute=0; uncalSweep.timeData.second=fileIndex;
			uncalSweep.azimuthAngle = 0.0;
			uncalSweep.polarization = "";

			//#////////////////////////////////CAPTURE LOOP OF A WHOLE SWEEP////////////////////////////////////

			if( frontEndCalibrator.IsCalibStarted() )
				cout << "\n\nStarting the capturing of a sweep for the calibration" << endl;
			else
				cout << "\n\nStarting the capturing of the sweep " << (sweepIndex + 1) << '/' << numOfSweepsPerFile << ", in file " << (fileIndex + 1) << '/' << numOfFiles << endl;

			//Capturing the sweeps related to each one of the frequency bands, which in conjunction form a whole sweep
			for(unsigned int i=0; i < specConfigurator.GetNumOfBands(); i++)
			{
				BandParameters currBandParam;
				FreqValues currFreqBand;

				currBandParam = specConfigurator.ConfigureNextBand();

				cout << "\nFrequency band N° " << (i+1) << '/' << specConfigurator.GetNumOfBands() << endl;
				cout << "Fstart=" << (currBandParam.startFreq/1e6) << " MHz, Fstop=" << (currBandParam.stopFreq/1e6) << " MHz, ";
				cout << "RBW=" << (currBandParam.rbw/1e3) << " KHz, Sweep time=" << currBandParam.sweepTime << " ms" << endl;

				currFreqBand = sweepBuilder.CaptureSweep(currBandParam);

				bool flagLastPointRemoved = uncalSweep.PushBack(currFreqBand);

				if(flagLastPointRemoved)
					currBandParam.samplePoints--;
				specConfigurator.SetCurrBandParameters(currBandParam);
			}

			cout << "\nThe capturing of a whole sweep finished" << endl;
			specInterface.SoundNewSweep();

			if(flagSetBandsParamAndRefSweep)
			{
				curveAdjuster.SetRefSweep(uncalSweep);

				if(flagCalibrateSweeps)
				{
					//Loading the ENR values, now that the bands parameters are known
					cout << "\nThe ENR values curve will be loaded" << endl;
					frontEndCalibrator.LoadENR();

					frontEndCalibrator.BuildRBWCurve();
				}

				flagSetBandsParamAndRefSweep=false;
			}

			//Showing the max power in the input of the spectrum analyzer
			auto sweepIter = std::max_element( uncalSweep.values.begin(), uncalSweep.values.end() );
			auto maxElemPos = std::distance( uncalSweep.values.begin(), sweepIter );
			cout << "\nThe max power in the input of the spectrum analyzer (after the internal preamp) was: " << *sweepIter << " dBm at the frequency of ";
			cout << std::setprecision(4) << uncalSweep.frequencies.at(maxElemPos) / 1e6 << " MHz (max value: +20 dBm)" << endl;

			//#/////////////////////////END OF THE CAPTURE LOOP OF A WHOLE SWEEP////////////////////////////////


			//#///////////////////////////////////////SWEEP PROCESSING////////////////////////////////////////////

			if( frontEndCalibrator.IsCalibStarted() )
			{
				//#//////////////////////////FRONT END CALIBRATION////////////////////////////////////
				//Estimating the front end parameters, gain and noise figure curves versus frequency////////////////////
				try
				{
					frontEndCalibrator.SetSweep(uncalSweep);

					if( frontEndCalibrator.IsNoiseSourceOff() )
						frontEndCalibrator.TurnOnNS();
					else
					{
						frontEndCalibrator.EndCalibration();
						frontEndCalibrator.EstimateParameters();

						dataLogger.SaveFrontEndParam( frontEndCalibrator.GetGain(), frontEndCalibrator.GetNoiseFigure() );

						if(flagTestPlot)
							try
							{
								gainPlotter.Clear();
								gainPlotter.Plot( frontEndCalibrator.GetGain(), "lines", "Total receiver gain");
								nfPlotter.Clear();
								nfPlotter.Plot( frontEndCalibrator.GetNoiseFigure(), "lines", "Total receiver noise figure");
							}
							catch(std::exception & exc)
							{
								cerr << "\nWarning: " << exc.what() << endl;
							}
					}
				}
				catch(rfims_exception & exc)
				{
					frontEndCalibrator.EndCalibration();
					exc.Prepend("error during front end calibration");
					throw;
				}
				//#////////////////////////END OF THE FRONT END CALIBRATION////////////////////////////////
			}
			else
			{
				//#////////////////////////////////NORMAL PROCESSING///////////////////////////////////////

				//#/////Processing of sweeps which were captured with the antenna connected to the input////////////

				//Calibration of the captured sweep
				Sweep calSweep;
				if(flagCalibrateSweeps)
				{
					//Sweep calibration, taking into account the total gain curve
					cout << "\nThe captured sweep is being calibrated" << endl;
					calSweep = frontEndCalibrator.CalibrateSweep(uncalSweep);
					cout << "The sweep calibration finished" << endl;
				}
				else
					calSweep = uncalSweep;

				//Plotting of the captured sweep
				if(flagTestPlot)
					try
					{
						//Plotting the current sweep and the detected RFI
						cout << "\nThe calibrated sweep will be plotted" << endl;
						sweepPlotter.Clear();
						sweepPlotter.PlotSweep(calSweep);
					}
					catch(std::exception & exc)
					{
						cerr << "\nWarning: " << exc.what();
					}

				//Transferring the sweep and detected RFI to the data logger in order to this component saves the data in memory
				dataLogger.SaveSweep(calSweep);

				//#/////////////////////////////END OF NORMAL PROCESSING///////////////////////////////////


				//Checking if the software must continue or not
				if(++sweepIndex >= numOfSweepsPerFile)
				{
					sweepIndex=0;
					if(++fileIndex >= numOfFiles)
						flagEndIterations=true;
				}
			}
		}
	}
	catch(std::exception & exc)
	{
		cerr << "\nError: " << exc.what() << endl;

		cout << "\nThe elapsed time since the beginning is: " << GetTimeAsString(timer) << endl;

		cout << "\nExiting from the software..." << endl;
//		cout << "\nPress enter to finish the rfims software..." << endl;
//		WaitForKey();

		std::exit(EXIT_FAILURE);
	}

	cout << "\nThe sweeps capturing process finished." << endl;

	cout << "\nThe elapsed time since the beginning is: " << GetTimeAsString(timer) << endl;

	cout << "\nExiting from the software..." << endl;
//	cout << "\nPress enter to finish the rfims software..." << endl;
//	WaitForKey();

	return 0;
}

//#/////////////////////////////////////////////////////////////


//#/////////////////////FUNCTIONS///////////////////////////////

void PrintTestHelp()
{
	cout << "Usage: test-spectran [--num-files='number'] [--num-sweeps-file] [--uncal-sweeps] [--plot] [--help | -h]" << endl;

	cout << "\nThis software was designed to test the capture of sweeps with the spectrum analyzer Aaronia Spectran HF-60105 V4 X." << endl;
	cout << "It is intended to capture sweeps with the Spectran device connected to a RF front end, which can be composed of just LNAs" << endl;
	cout << "and RF connectors and cables or it can contains and RF switch and a noise source to estimate the front end's parameters" << endl;
	cout << "and then calibrate the sweeps captured with the antenna connected to the input. The calibration can be avoided " << endl;
	cout << "with an argument and in that case the front end's parameters are not estimated and the sweeps are stored uncalibrated." << endl;
	cout << "The power management of the RF switch and the noise source is considered to be performed manually." << endl;

	cout << "\nThe arguments' descriptions are presented in the following:" << endl;

	cout << "\n\t--num-files='number'\t\t\t\tDetermine the number of files with sweeps which must be generated by the" << endl;
	cout << "\t\t\t\t\t\t\tsoftware. The default number is 1." << endl;

	cout << "\n\t--num-sweeps-file='number'\t\t\tDetermine the number of sweeps per file which must be captured by the" << endl;
	cout << "\t\t\t\t\t\t\tsoftware. The default number is 10." << endl;

	cout << "\n\t--uncal-sweeps\t\t\t\t\tIndicate the software does not have to perform the front end calibrate," << endl;
	cout << "\t\t\t\t\t\t\tso it just has to capture sweeps and store them uncalibrated. The default" << endl;
	cout << "\t\t\t\t\t\t\tbehavior is to calibrate the sweeps." << endl;

	cout << "\n\t--plot\t\t\t\t\t\tEnable the plotting of the different RF data which are got by the software." << endl;
	cout << "\t\t\t\t\t\t\tIf this argument is not given no plot is produced." << endl;

	cout << "\n\t-h, --help\t\t\t\t\tShow this help and finish there." << endl;
}


bool ProcessTestArguments(int argc, char * argv[])
{
	if(argc>1)
	{
		unsigned int argc_aux=argc;
		std::list<std::string> argList;
		for(unsigned int i=1; i<argc_aux; i++)
			argList.push_back( argv[i] );

		//Searching for the argument --help
		auto argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="--help" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			PrintTestHelp();
			return false;
		}

		//Searching for the argument -h
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="-h" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			PrintTestHelp();
			return false;
		}

		//Searching for the argument --num-files=xx
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && argIter->find("--num-files=")==std::string::npos )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			auto equalSignPos = argIter->find('=');
			std::istringstream iss( argIter->substr(equalSignPos+1) );
			iss >> numOfFiles;
			argList.erase(argIter);
		}

		//Searching for the argument --num-sweeps-file=xx
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && argIter->find("--num-sweeps-file=")==std::string::npos )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			auto equalSignPos = argIter->find('=');
			std::istringstream iss( argIter->substr(equalSignPos+1) );
			iss >> numOfSweepsPerFile;
			argList.erase(argIter);
		}

		//Searching for the argument --uncal-sweeps
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="--uncal-sweeps" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagCalibrateSweeps = false;
			argList.erase(argIter);
		}

		//Searching the argument --plot
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="--plot" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagTestPlot=true;
			argList.erase(argIter);
		}

		//Checking if there were arguments which were not recognized
		if( !argList.empty() )
		{
			cout << "test-spectran: the following arguments were not recognized:";
			for(argIter = argList.cbegin(); argIter != argList.cend(); argIter++)
				cout << " \'" << *argIter << '\'';
			cout << endl;

			cout << "Usage: test-spectran [--num-files='number'] [--num-sweeps-file] [--uncal-sweeps] [--plot] [--help | -h]" << endl;
			return false;
		}
	}
	return true;
}

//#/////////////////////////////////////////////////////////////
