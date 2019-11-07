/*! \file main.cpp
 * 	\brief This file contains the main function of the RFIMS-CART software.
 * 	\author Mauro Diamantino
 */

#include "TopLevel.h"


//#////////////////////////////MAIN FUNCTION//////////////////////////////////

//! The main function of the RFISM-CART software.
/*!	This function instantiates all the needed objects and performs the software tasks in the corresponding order.
 * The objects and their relations can be observed in the following components diagram:
 * \image html software_components_diagram.png "Software components diagram"
 * \image latex software_components_diagram.eps "Software components diagram"
 * The order of the operations which are performed in the main function follows the following flow diagram:
 * \image html high_level_flow_diagram.png "High-level flow diagram"
 * \image latex high_level_flow_diagram.eps "High-level flow diagram" width=13cm
 * \param [in] argc The number of arguments that were received by the software.
 * \param [in] argv An array of C strings (`char*`) where each one is a software's argument.
 */
int main(int argc, char * argv[])
{
	//#////////Local variables/////////////
	// An object which is responsible of the handling of the received signals.
	SignalHandler signalHandler;
	// A flag which states if the bands parameters have been loaded by first time or they have been reloaded.
	bool flagBandsParamReloaded=false;
	// A flag which states the beginning of a new measurement cycle, and so the need to realize a new front end calibration, to move the antenna to the initial position, etc.
	bool flagNewMeasCycle=true;
	// A flag which is used when the user executes the software to realize a finite number of iterations (measurement cycles), to states the requested measurements cycles have already been done.
	bool flagEndIterations = false;
	// A variable which represents the number of the current sweep (like an index but this starts in one). The sweeps got for the calibration are not taking into account.
	unsigned int sweepNumber = 1;
	unsigned int measCycleIndex = 0;
	//#/////////////////////////////////////

	// The checking of the program's arguments.
	if( !ProcessMainArguments(argc, argv) )
		std::exit(EXIT_FAILURE);

	cout << "\n\t\t\t\tRF Interference Monitoring System (RFIMS)" << endl;
	cout << "\t\t\t\tChina-Argentina Radio Telescope (CART)\n" << endl;

	// Initializing the GPIO pins.
	InitializeGPIO();

	TurnOnFrontEnd();

	if(!flagInfiniteLoop)
		timer.start(); //Starting timer

	RFPlotter sweepPlotter, gainPlotter, nfPlotter;

	try
	{
		//#///////////////////////////////////INITIALIZATIONS//////////////////////////////////////////

		SpectranInterface specInterface;
		SpectranConfigurator specConfigurator(specInterface);
		SweepBuilder sweepBuilder(specInterface);
		CurveAdjuster curveAdjuster;
		FrontEndCalibrator frontEndCalibrator(curveAdjuster);
		RFIDetector rfiDetector(curveAdjuster);
		DataLogger dataLogger;
		GPSInterface gpsInterface;
		AntennaPositioner antPositioner(gpsInterface);

		//Setting of pointers to objects which are used by SignalHandler class
		signalHandler.SetAllPointers(&specInterface, &specConfigurator, &sweepBuilder, &curveAdjuster, &frontEndCalibrator,
				&rfiDetector, &dataLogger, &gpsInterface, &antPositioner, &sweepPlotter, &gainPlotter, &nfPlotter);

		//Initializing the spectrum analyzer
		cout << "\nInitializing the spectrum analyzer Aaronia Spectran HF-60105 V4 X..." << endl;
		specInterface.Initialize();
		cout << "The spectrum analyzer was initialized successfully" << endl;

		//Initializing the GPS receiver
		cout << "\nInitializing the Aaronia GPS receiver..." << endl;
		gpsInterface.Initialize();
		cout << "The GPS receiver was initialize successfully" << endl;

		//Putting the antenna in the initial position and polarization
		antPositioner.Initialize();
		cout << "\nThe initial azimuth angle is " << antPositioner.GetAzimPosition() << "° N" << endl;
		
		//Setting the total number of sweeps per measurement cycle in the data logger
		dataLogger.SetNumOfSweeps(2*numOfAzimPos);

		//Setting the number of azimuth positions in the antenna positioner
		antPositioner.SetNumOfAzimPos(numOfAzimPos);

		//#///////////////////////////////END OF THE INITIALIZATION///////////////////////////////////////


		//#/////////////////////////////////////GENERAL LOOP////////////////////////////////////////////

		while(flagInfiniteLoop || !flagEndIterations)
		{
			Sweep uncalSweep;

			if(flagNewMeasCycle)
			{
				sweepNumber = 1;

				//#/////////////////////LOADING OF THE SPECTRAN'S PARAMETERS//////////////////////////

				//Loading by first time or reloading the Spectran parameters
				cout << "\nLoading the Spectran's configuration parameters from the corresponding files" << endl;
				
				//Loading the fixed parameters
				cout << "Loading the fixed parameters..." << endl;
				if( specConfigurator.LoadFixedParameters() )
				{
					//If the fixed parameters were loaded for the first time or they were reloaded, the initial
					//configuration will be repeated
					cout << "The fixed parameters were loaded by first time or they were reloaded, ";
					cout << "so the initial configuration of the Spectran will be performed" << endl;
					specConfigurator.InitialConfiguration();
					cout << "The initial configuration was carried out successfully" << endl;
				}

				//Loading the frequency bands parameters
				cout << "Loading the frequency bands' parameters..." << endl;
				flagBandsParamReloaded = specConfigurator.LoadBandsParameters();
				cout << "The frequency bands' parameters were loaded successfully" << endl;

				//#///////////////////END OF THE LOADING OF THE SPECTRAN'S PARAMETERS////////////////

				//Determining if the front end calibration must be done or not
				if(flagCalEnabled)
				{
					//The front end calibration is enabled and a new measurement cycle is starting
					cout << "\nStarting the front end calibration" << endl;
					frontEndCalibrator.StartCalibration();
				}
	
				//This flag is pulled down here because it is just not needed from here to down.
				flagNewMeasCycle=false;
			}


			//#//////////////////////////CAPTURING THE ANTENNA'S POSITION AND THE TIME DATA///////////////////////////

			//The timestamp of each sweep is taking at the beginning. Also, the antenna position data are
			//saved in the Sweep object here.
			uncalSweep.timeData = gpsInterface.UpdateTimeData();
			uncalSweep.azimuthAngle = antPositioner.GetAzimPosition();
			uncalSweep.polarization = antPositioner.GetPolarizationString();

			//#///////////////////////////////////////////////////////////////////////////////////////////////


			//#////////////////////////////////CAPTURE LOOP OF A WHOLE SWEEP////////////////////////////////////

			if( frontEndCalibrator.IsCalibStarted() )
				cout << "\nStarting the capturing of a sweep for the calibration" << endl;
			else
				if(flagInfiniteLoop)
					cout << "\nStarting the capturing of the sweep " << sweepNumber++ << '/' << (numOfAzimPos*2) << endl;
				else
					cout << "\nStarting the capturing of the sweep " << sweepNumber++ << '/' << (numOfAzimPos*2) << ", in the measurement cycle " << (measCycleIndex + 1) << '/' << numOfMeasCycles << endl;

#ifdef RASPBERRY_PI
			digitalWrite(piPins.LED_SWEEP_CAPTURE, pinsValues.LED_SWP_CAPT_ON);
#endif

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
				
				if(flagBandsParamReloaded)
				{
					if(flagLastPointRemoved)
						currBandParam.samplePoints--;
					specConfigurator.SetCurrBandParameters(currBandParam);
				}
			}

#ifdef RASPBERRY_PI
			digitalWrite(piPins.LED_SWEEP_CAPTURE, pinsValues.LED_SWP_CAPT_OFF);
#endif

			specInterface.SoundNewSweep();
			cout << "\nThe capturing of a whole sweep finished" << endl;

			//Showing the max power in the input of the spectrum analyzer
			auto sweepIter = std::max_element( uncalSweep.values.begin(), uncalSweep.values.end() );
			auto maxElemPos = std::distance( uncalSweep.values.begin(), sweepIter );
			cout << "\nThe max power in the input of the spectrum analyzer (after the internal preamp) was: " << *sweepIter << " dBm at the frequency of ";
			cout << std::setprecision(4) << uncalSweep.frequencies.at(maxElemPos) / 1e6 << " MHz (max value: +20 dBm)" << endl;

			//#/////////////////////////END OF THE CAPTURE LOOP OF A WHOLE SWEEP////////////////////////////////


			//#///////////TRANSFERRING OF THE NEW FREQ BANDS PARAMETERS AND (RE)LOADING OF OTHER PARAMETERS///////////////

			if(flagBandsParamReloaded)
			{
				//The bands parameters are given to the objects which need them after a sweep was captured to make sure the
				//exact number of samples is known. The parameters which must be adjusted taking into account the bands
				//parameters are reloaded and readjusted each time the bands parameters are changed.
				auto bandsParameters = specConfigurator.GetBandsParameters();
				curveAdjuster.SetBandsParameters(bandsParameters);
				frontEndCalibrator.SetBandsParameters(bandsParameters);

				curveAdjuster.SetRefSweep(uncalSweep);

				cout << "\nThe ENR values curve will be (re)loaded" << endl;
				frontEndCalibrator.LoadENR();

				frontEndCalibrator.BuildRBWCurve();

				if(flagRFI)
				{
					cout << "\nThe RFI harmful levels curve will be (re)loaded" << endl;
					rfiDetector.SetBandsParameters(bandsParameters);
					rfiDetector.LoadThreshCurve(rfiNorm);
				}

				//Saving the bands parameters in a CSV file
				dataLogger.SaveBandsParamAsCSV(bandsParameters);

				if(!flagCalEnabled)
				{
					cout << "\nThe front end calibration is not enabled so the default front end parameters will be loaded..." << endl;
					frontEndCalibrator.LoadDefaultParameters();
					cout << "The default front end parameters were loaded successfully" << endl;

					if(flagPlot)
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
			//#///////////////////////////END OF TRANSFERRING OF THE BANDS PARAMETERS//////////////////////////////


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
#ifdef RASPBERRY_PI
						digitalWrite(piPins.LED_SWEEP_PROCESS, pinsValues.LED_SWP_PROC_ON);
#endif
						frontEndCalibrator.EstimateParameters();

						dataLogger.SaveFrontEndParam( frontEndCalibrator.GetGain(), frontEndCalibrator.GetNoiseFigure() );

						if(flagPlot)
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
#ifdef RASPBERRY_PI
						digitalWrite(piPins.LED_SWEEP_PROCESS, pinsValues.LED_SWP_PROC_OFF);
#endif
					}
				}
				catch(std::exception & exc)
				{
					cerr << "\nWarning: error during front end calibration: " << exc.what() << '.' << endl;

					frontEndCalibrator.EndCalibration();

					if( frontEndCalibrator.AreParamEmpty() || flagBandsParamReloaded )
					{
						cout << "The default front end parameters will be loaded." << endl;
						frontEndCalibrator.LoadDefaultParameters();
					}
					else
						cout << "The last estimated front end parameters will be used." << endl;
				}
				//#////////////////////////END OF THE FRONT END CALIBRATION////////////////////////////////
			}
			else
			{
				//#////////////////////////////////NORMAL PROCESSING///////////////////////////////////////

				//Processing of sweeps which were captured with the antenna connected to the input////////////

#ifdef RASPBERRY_PI
				digitalWrite(piPins.LED_SWEEP_PROCESS, pinsValues.LED_SWP_PROC_ON);
#endif
				//This flag is pulled down here because it is just not needed from here to down.
				flagBandsParamReloaded=false;

				//Sweep calibration, taking into account the total gain curve
				cout << "\nThe captured sweep is being calibrated" << endl;
				Sweep calSweep = frontEndCalibrator.CalibrateSweep(uncalSweep);
				cout << "The sweep calibration finished" << endl;

				RFI detectedRFI;
				if(flagRFI)
				{
					//Detecting RFI
					cout << "\nThe RFI which is present in the current calibrated sweep is being detected" << endl;
					detectedRFI = rfiDetector.DetectRFI(calSweep);
					if( rfiDetector.GetNumOfRFIBands()==0 )
						cout << "No RFI was detected" << endl;
					else
						cout << "It were detected " << rfiDetector.GetNumOfRFIBands() << " RFI bands." << endl;
				}

				if(flagPlot)
					try
					{
						//Plotting the current sweep and the detected RFI
						cout << "\nThe calibrated sweep will be plotted" << endl;
						sweepPlotter.Clear();
						sweepPlotter.PlotSweep(calSweep);
						if(flagRFI && rfiDetector.GetNumOfRFIBands()!=0)
						{
							cout << "The detected RFI will be plotted" << endl;
							sweepPlotter.PlotRFI(detectedRFI);
						}
					}
					catch(std::exception & exc)
					{
						cerr << "\nWarning: " << exc.what();
					}

				//Transferring the sweep and detected RFI to the data logger in order to this component saves the data in memory
				dataLogger.SaveSweep(calSweep);
				if(flagRFI)
					dataLogger.SaveRFI(detectedRFI);

#ifdef RASPBERRY_PI
				digitalWrite(piPins.LED_SWEEP_PROCESS, pinsValues.LED_SWP_PROC_OFF);
#endif
				//#/////////////////////////////END OF NORMAL PROCESSING///////////////////////////////////


				//Checking if the current measurement cycle has finished and if the software should or not starts a new one.
				if( antPositioner.IsLastPosition() && antPositioner.GetPolarization()==Polarization::VERTICAL)
				{
					if( !flagInfiniteLoop && ++measCycleIndex >= numOfMeasCycles )
					{
						cout << "\nThe " << numOfMeasCycles << " measurements cycles have been realized" << endl;
						flagEndIterations = true;
					}
					else
						flagNewMeasCycle = true;

					//Uploading
					if(flagUpload)
					{
						cout << "\nCreating a thread to prepare and upload data in parallel" << endl;
						try
						{
							dataLogger.PrepareAndUploadData();
						}
						catch(std::exception & exc)
						{
							cerr << "\nWarning: " << exc.what() << endl;
						}
					}

					//Deleting old files
					cout << "\nDeleting old files" << endl;
					try
					{
						dataLogger.DeleteOldFiles();
					}
					catch(std::exception & exc)
					{
						cerr << "\nWarning: " << exc.what() << endl;
					}
				}

				//#//////////////////////////////ANTENNA POSITIONING////////////////////////////////////////

				//Changing the antenna position
				cout << "\nThe antenna position will be changed" << endl;
				antPositioner.ChangePolarization();
				if( antPositioner.GetPolarization()==Polarization::HORIZONTAL )
					antPositioner.NextAzimPosition();

				cout << "The new antenna position is:" << endl;
				cout << "\tPosition number: " << (antPositioner.GetPositionIndex() + 1) << '/' << numOfAzimPos << endl;
				cout <<	"\tAzimuth: " << antPositioner.GetAzimPosition() << "° N" << endl;
				cout << "\tPolarization: " << antPositioner.GetPolarizationString() << endl;

				//#/////////////////////////END OF THE ANTENNA POSITIONING/////////////////////////////////////
			}
		}
		//#///////////////////////////////END OF THE GENERAL LOOP////////////////////////////////////
	}
	catch(std::exception & exc)
	{
		cerr << "\nError: " << exc.what() << endl;

		if( !timer.is_stopped() )
			cout << "\nThe elapsed time since the beginning is: " << GetTimeAsString(timer) << endl;

		TurnOffFrontEnd();
		TurnOffLeds();

		//cout << "\nExiting from the rfims software..." << endl;
		cout << "\nPress enter to finish the rfims software..." << endl;
		WaitForKey();

		std::exit(EXIT_FAILURE);
	}

	cout << "\nThe sweeps capturing process finished." << endl;

	if( !timer.is_stopped() )
		cout << "\nThe elapsed time since the beginning is: " << GetTimeAsString(timer) << endl;

	TurnOffFrontEnd();
	TurnOffLeds();

	//cout << "\nExiting from the rfims software..." << endl;
	cout << "\nPress enter to finish the rfims software..." << endl;
	WaitForKey();

	return 0;
}

//#/////////////////////////END OF MAIN FUNCTION/////////////////////////////////
