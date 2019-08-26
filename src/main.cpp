/*! \file main.cpp
 * 	\brief This file contains the main function of the RFIMS-CART software.
 * 	\author Mauro Diamantino
 */

#include "TopLevel.h"

//#//////////////////////Global variables//////////////////////////

// Global variables which are used by the SignalHandler class
SpectranInterface * SignalHandler::specInterfPtr; //!< The instantiation of the pointer to the _SpectranInterface_ object.
SpectranConfigurator * SignalHandler::specConfiguratorPtr; //!< The instantiation of the pointer to the _SpectranConfigurator_ object.
SweepBuilder * SignalHandler::sweepBuilderPtr; //!< The instantiation of the pointer to the _SweepBuilder_ object.
CurveAdjuster * SignalHandler::adjusterPtr; //!< The instantiation of the pointer to the _CurveAjuster_ object.
FrontEndCalibrator * SignalHandler::calibratorPtr; //!< The instantiation of the pointer to the _FrontEndCalibrator_ object.
RFIDetector * SignalHandler::rfiDetectorPtr; //!< The instantiation of the pointer to the _RFIDetector_ object.
DataLogger * SignalHandler::dataLoggerPtr; //!< The instantiation of the pointer to the _DataLogger_ object.
GPSInterface * SignalHandler::gpsInterfacePtr; //!< The instantiation of the pointer to the _GPSInterface_ object.
AntennaPositioner * SignalHandler::antPositionerPtr; //!< The instantiation of the pointer to the _AntennaPositioner_ object.
RFPlotter * SignalHandler::sweepPlotterPtr; //!< The instantiation of the pointer to the _RFPlotter_ object which is responsible for the plotting of the last captured sweep.
RFPlotter * SignalHandler::gainPlotterPtr; //!< The instantiation of the pointer to the _RFPlotter_ object which is responsible for the plotting of the last estimated gain curve.
RFPlotter * SignalHandler::nfPlotterPtr; //!< The instantiation of the pointer to the _RFPlotter_ object which is responsible for the plotting of the last estimated noise figure curve.

// Flags which are defined by the software arguments and which indicates the way the software must behave.
bool flagCalEnabled; //!< The declaration of an external flag which defines if the calibration of the RF front end must be done or not.
bool flagPlot; //!< The declaration of an external flag which defines if the software has to generate plots.
bool flagInfiniteLoop; //!< The declaration of an external flag which defines if the software has to perform a finite number of measurement cycles or iterate infinitely.
bool flagRFI; //!< The declaration of an external flag which defines if the software has to perform RFI detection or not.
bool flagUpload; //!< The declaration of an external flag which defines if the software has to upload the measurements or not.

//! A variable which saves the number of measurements cycles which left to be done. It is used when the user wishes a finite number of measurements cycles.
unsigned int numOfMeasCycles=0;
//! A variable which saves the norm which defines the harmful RF interference levels: ska-mode1, ska-mode2, itu-ra769-2-vlbi.
RFI::ThresholdsNorm rfiNorm = RFI::SKA_MODE1;
//! A timer which is used to measure the execution time when the number of iterations is finite.
boost::timer::cpu_timer timer;


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

	// A flag which states if the bands parameters have been loaded by first time or they have been reloaded.
	bool flagBandsParamReloaded=false;
	// A flag which states the beginning of a new measurement cycle, and so the need to realize a new front end calibration, to move the antenna to the initial position, etc.
	bool flagNewMeasCycle=true;
	// A flag which is used when the user executes the software to realize a finite number of iterations (measurement cycles), to states the requested measurements cycles have already been done.
	bool flagEndIterations = false;

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
		//Starting timer
		timer.start();

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
		SignalHandler sigHandler;
		sigHandler.SetupSignalHandler(&specInterface, &specConfigurator, &sweepBuilder, &curveAdjuster,
				&frontEndCalibrator, &rfiDetector, &dataLogger, &gpsInterface, &antPositioner,
				&sweepPlotter, &gainPlotter, &nfPlotter);

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
		cout << "The initial azimuth angle is " << antPositioner.GetAzimPosition() << "° N" << endl;

		//#///////////////////////////////END OF THE INITIALIZATION///////////////////////////////////////


		//#/////////////////////////////////////GENERAL LOOP////////////////////////////////////////////

		while(flagInfiniteLoop || !flagEndIterations)
		{
			Sweep uncalSweep;

			if(flagNewMeasCycle)
			{
				//#/////////////////////LOADING OF THE SPECTRAN'S PARAMETERS//////////////////////////

				//Loading by first time or reloading the Spectran parameters
				cout << "\nLoading the Spectran's configuration parameters from the corresponding files" << endl;
				//Loading the fixed parameters
				cout << "Loading the fixed parameters..." << endl;
				if( specConfigurator.LoadFixedParameters() )
				{
					//If the fixed parameters were loaded for the first time or they were reloaded, the initial configuration will be repeated
					cout << "The fixed parameters were loaded by first time or they were reloaded so the Spectran initial configuration will be done" << endl;
					specConfigurator.InitialConfiguration();
					cout << "The initial configuration was carried out successfully" << endl;
				}

				//Loading the frequency bands parameters
				cout << "Loading the frequency bands' parameters..." << endl;
				flagBandsParamReloaded = specConfigurator.LoadBandsParameters();
				cout << "The frequency bands' parameters were loaded successfully" << endl;

				//#///////////////////END OF THE LOADING OF THE SPECTRAN'S PARAMETERS////////////////
			}

			//Determining if the front end calibration must be done or not
			if(flagCalEnabled && flagNewMeasCycle)
			{
				//The front end calibration is enabled and a new measurement cycle is starting
				cout << "\nStarting the front end calibration" << endl;
				frontEndCalibrator.StartCalibration();
			}

			//This flag is pulled down here because it is just not needed from here to down.
			flagNewMeasCycle=false;

			//#//////////////////////////GETTING ANTENNA POSITIONS AND TIME DATA///////////////////////////

			//The timestamp of each sweep is taking at the beginning. Also, the antenna position data are
			//saved in the Sweep object here.
			bool flagSuccess=false;
			unsigned int numOfErrors=0;
			do
			{
				try
				{
					gpsInterface.ReadOneDataSet();
					uncalSweep.timeData = gpsInterface.GetTimeData();
					uncalSweep.azimuthAngle = antPositioner.GetAzimPosition();
					uncalSweep.polarization = antPositioner.GetPolarizationString();
				}
				catch(rfims_exception & exc)
				{
					if( ++numOfErrors < 3)
						cerr << "\nWarning: reading of time data and antenna position failed: " << exc.what() << endl;
					else
						throw rfims_exception("the reading of time data and antenna position failed three times.");
				}
			}while(!flagSuccess);

			//#///////////////////////////////////////////////////////////////////////////////////////////////


			//#////////////////////////////////CAPTURE LOOP OF A WHOLE SWEEP////////////////////////////////////

			cout << "\nStarting the capturing of a whole sweep" << endl;
#ifdef RASPBERRY_PI
			digitalWrite(piPins.LED_SWEEP_CAPTURE, HIGH);
#endif
			//Capturing the sweeps related to each one of the frequency bands, which in conjunction form a whole sweep
			for(unsigned int i=0; i < specConfigurator.GetNumOfBands(); i++)
			{
				BandParameters currBandParam;
				FreqValues currFreqBand;

				currBandParam = specConfigurator.ConfigureNextBand();

				cout << "\nFrequency band N° " << i+1 << endl;
				cout << "Fstart=" << (currBandParam.startFreq/1e6) << " MHz, Fstop=" << (currBandParam.stopFreq/1e6) << " MHz, ";
				cout << "RBW=" << (currBandParam.rbw/1e3) << " KHz, Sweep time=" << currBandParam.sweepTime << " ms" << endl;

				currFreqBand = sweepBuilder.CaptureSweep(currBandParam);

				bool flagLastPointRemoved = uncalSweep.PushBack(currFreqBand);
				if( flagBandsParamReloaded && flagLastPointRemoved )
					currBandParam.samplePoints--;
				
				if(flagBandsParamReloaded)
					specConfigurator.SetCurrBandParameters(currBandParam);
			}

#ifdef RASPBERRY_PI
			digitalWrite(piPins.LED_SWEEP_CAPTURE, LOW);
#endif
			specInterface.SoundNewSweep();
			cout << "\nThe capturing of a whole sweep finished" << endl;

			//#/////////////////////////END OF THE CAPTURE LOOP OF A WHOLE SWEEP////////////////////////////////


			//#///////////TRANSFERRING OF THE NEW FREQ BANDS PARAMETERS AND (RE)LOADING OF OTHER PARAMETERS///////////////

			if(flagBandsParamReloaded)
			{
				//The bands parameters are given to the objects which need them after a sweep was captured to make sure the
				//exact number of samples is known. The parameters which must be adjusted taking into account the bands
				//parameters are reloaded and readjusted each time the bands parameters are changed.
				cout << "\nThe bands parameters changed so these will be transferred to the objects which use them" << endl;
				auto bandsParameters = specConfigurator.GetBandsParameters();
				curveAdjuster.SetBandsParameters(bandsParameters);
				curveAdjuster.SetRefSweep(uncalSweep);
				frontEndCalibrator.SetBandsParameters(bandsParameters);
				cout << "The ENR values curve will be (re)loaded" << endl;
				frontEndCalibrator.LoadENR();
				if(flagRFI)
				{
					cout << "The RFI harmful levels curve will be (re)loaded" << endl;
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


			//#//////////////////////////////SWEEP PROCESSING/////////////////////////////////////

			if( frontEndCalibrator.IsCalibStarted() )
			{
				//#//////////////////////////FRONT END CALIBRATION////////////////////////////////////

				//Estimating the front end parameters, gain and noise figure curves versus frequency////////////////////
				try
				{
					frontEndCalibrator.SetSweep(uncalSweep);

					if( frontEndCalibrator.IsNoiseSourceOff() )
						////////Noise source off////////////
						frontEndCalibrator.TurnOnNS();
					else
					{
						///////Noise source on/////////////
						frontEndCalibrator.EndCalibration();
#ifdef RASPBERRY_PI
						digitalWrite(piPins.LED_SWEEP_PROCESS, HIGH);
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
						digitalWrite(piPins.LED_SWEEP_PROCESS, LOW);
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
				digitalWrite(piPins.LED_SWEEP_PROCESS, HIGH);
#endif

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
				digitalWrite(piPins.LED_SWEEP_PROCESS, LOW);
#endif
				//#/////////////////////////////END OF NORMAL PROCESSING///////////////////////////////////


				//#//////////////////////////////ANTENNA POSITIONING////////////////////////////////////////

				//Checking if the current measurement cycle has finished and if the software should or not starts a new one.
				if( antPositioner.IsLastPosition() && antPositioner.GetPolarization()==Polarization::VERTICAL)
				{
					if( !flagInfiniteLoop && --numOfMeasCycles==0 )
					{
						cout << "\nThe requested number of measurements cycles have been realized" << endl;
						flagEndIterations = true;
					}
					else
						flagNewMeasCycle = true;

					cout << "\nDeleting old files" << endl;
					dataLogger.DeleteOldFiles();
					cout << "The deleting finished" << endl;

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
						cout << "The thread was created, the data to be uploaded will be processed in parallel with the rest of operations" << endl;
					}
				}

				//Changing the antenna position
				cout << "\nThe antenna position will be changed" << endl;
				antPositioner.ChangePolarization();
				if( antPositioner.GetPolarization()==Polarization::HORIZONTAL )
					antPositioner.NextAzimPosition();

				cout << "The new antenna position is: Azimuth=" << antPositioner.GetAzimPosition();
				cout << ", Polarization=" << antPositioner.GetPolarizationString() << endl;

				//#/////////////////////////END OF THE ANTENNA POSITIONING/////////////////////////////////////
			}
		}
		//#///////////////////////////////END OF THE GENERAL LOOP////////////////////////////////////
	}
	catch(rfims_exception & exc)
	{
		cerr << "\nError: " << exc.what() << endl;

		if( !timer.is_stopped() )
		{
			//Showing the elapsed time since the beginning
			timer.stop();
			boost::timer::cpu_times times = timer.elapsed();
			boost::posix_time::time_duration td = boost::posix_time::microseconds(times.wall/1000);
			cout << "\nThe elapsed time since the beginning is: " << boost::posix_time::to_simple_string(td) << endl;
		}

		TurnOffFrontEnd();
		TurnOffLeds();

		cout << "\nExiting" << endl;
		std::exit(EXIT_FAILURE);
	}

	cout << "\nThe sweeps capturing process finished." << endl;

	if( !timer.is_stopped() )
	{
		//Showing the elapsed time since the beginning
		timer.stop();
		boost::timer::cpu_times times = timer.elapsed();
		boost::posix_time::time_duration td = boost::posix_time::microseconds(times.wall/1000);
		cout << "\nThe elapsed time since the beginning is: " << boost::posix_time::to_simple_string(td) << endl;
	}

	TurnOffFrontEnd();
	TurnOffLeds();

	cout << "\nExiting from the software" << endl;
	return 0;
}

//#/////////////////////////END OF MAIN FUNCTION/////////////////////////////////
