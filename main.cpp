/*
 * main.cpp
 *
 *  Created on: 28/04/2019
 *      Author: new-mauro
 */

#include "TopLevel.h"
#include <boost/timer/timer.hpp>

//! Global variables which are used by the SignalHandler class
SpectranInterface * SignalHandler::specInterfPtr;
SpectranConfigurator * SignalHandler::specConfiguratorPtr;
SweepBuilder * SignalHandler::sweepBuilderPtr;
CurveAdjuster * SignalHandler::adjusterPtr;
FrontEndCalibrator * SignalHandler::calibratorPtr;
DataLogger * SignalHandler::dataLoggerPtr;
RFPloter * SignalHandler::sweepPloterPtr;
RFPloter * SignalHandler::gainPloterPtr;
RFPloter * SignalHandler::nfPloterPtr;
AntennaPositioner * SignalHandler::antPositionerPtr;

int main(int argc, char * argv[])
{
	struct
	{
		const int LED_SWEEP_CAPTURE = 9;
		const int LED_SWEEP_PROCESS = 10;
	} piPins;
	boost::timer::cpu_timer timer;
	bool flagBandsParamReloaded=false;
	bool flagEndMeasCycle = false;
	bool flagCalEnabled=true, flagPlot=false, flagRFI=false, flagInfIterations=true;
	unsigned int numOfIter=0;
	bool flagStartCal=true;

	cout << "\n\t\t\t\tRF Interference Monitoring System (RFIMS)" << endl;
	cout << "\t\t\t\tChina-Argentina Radio Telescope (CART)\n" << endl;

	//Checking of the program's arguments
	if(argc>1)
	{
		unsigned int argc_aux=argc;
		std::vector<std::string> argVector;
		for(unsigned int i=0; i<(argc_aux-1); i++)
			argVector.push_back( argv[i] );

		unsigned int i=1;
		while( i<argc_aux && argVector[i]!="--no-frontend-cal" )	i++;
		if(i<argc_aux)	flagCalEnabled=false;

		i=1;
		while( i<argc_aux && argVector[i]!="--plot" )	i++;
		if(i<argc_aux)	flagPlot=true;

		i=1;
		while( i<argc_aux && argVector[i]!="--detect-rfi" )	i++;
		if(i<argc_aux)	flagRFI=true;

		i=1;
		size_t equalSignPos=0;
		while( i<argc_aux && ( equalSignPos=argVector[i].find("--iter=") )==std::string::npos )	i++;
		if(i<argc_aux)
		{
			flagInfIterations=false;
			std::istringstream iss;
			std::string numString = argVector[i].substr( equalSignPos+1, (argVector[i].size()-equalSignPos-1) );
			iss.str(numString);
			iss >> numOfIter;
		}
	}

	try
	{
		///////////////////////Instantiations & Initializations//////////////////////////////////
#ifdef RASPBERRY_PI
		//Initializing the Wiring Pi library
		wiringPiSetup();
		pinMode(piPins.LED_SWEEP_CAPTURE, OUTPUT);
		pinMode(piPins.LED_SWEEP_PROCESS, OUTPUT);
		digitalWrite(piPins.LED_SWEEP_CAPTURE, LOW);
		digitalWrite(piPins.LED_SWEEP_PROCESS, LOW);
#endif
		SpectranInterface specInterface;
		SpectranConfigurator specConfigurator(specInterface);
		SweepBuilder sweepBuilder(specInterface);
		CurveAdjuster curveAdjuster;
		FrontEndCalibrator frontEndCalibrator(curveAdjuster);
		DataLogger dataLogger;
		RFPloter sweepPloter;
		RFPloter gainPloter, nfPloter;
		GPSInterface gpsInterface;
		AntennaPositioner antPositioner(gpsInterface);

		//Setting of pointers to objects which are used by SignalHandler class
		SignalHandler sigHandler;
		sigHandler.SetupSignalHandler(&specInterface, &specConfigurator, &sweepBuilder, &curveAdjuster,
				&frontEndCalibrator, &dataLogger, &sweepPloter, &gainPloter, &nfPloter, &antPositioner);

		cout << "\nInitializing communication with Aaronia Spectran device..." << endl;
		specInterface.Initialize();
		cout << "The communication was established successfully" << endl;

		//Initializing the communication with the GPS receiver
		cout << "\nInitializing the GPS receiver..." << endl;
		gpsInterface.Initialize();
		cout << "The device was initialize successfully" << endl;

		//Loading the Spectran parameters
		cout << "\nLoading the Spectran's configuration parameters from the corresponding files" << endl;
		cout << "Loading the fixed parameters..." << endl;
		if( specConfigurator.LoadFixedParameters() )
		{
			//If the fixed parameters were loaded for the first time or they were reloaded, the initial configuration will be repeated
			cout << "The fixed parameters were loaded by first time or they were reloaded so the Spectran initial configuration will be done" << endl;
			specConfigurator.InitialConfiguration();
			cout << "The initial configuration was carried out successfully" << endl;
		}

		cout << "Loading the frequency bands' parameters..." << endl;
		flagBandsParamReloaded = specConfigurator.LoadBandsParameters();
		cout << "The frequency bands' parameters were loaded successfully" << endl;

		//Putting the antenna in the initial position and polarization
		antPositioner.Initialize();

		//////////////////////////////////General Loop///////////////////////////////////

		while(!flagEndMeasCycle)
		{
			Sweep uncalSweep;

			if(flagCalEnabled)
			{
				if(flagStartCal)
				{
					cout << "\nStarting the front end calibration" << endl;
					frontEndCalibrator.StartCalibration();
					cout << "\nTurn off the noise source, switch the input to this one and press Enter to continue..." << endl;
					WaitForKey();
					flagStartCal=false;
				}
			}
			else
			{
				if( frontEndCalibrator.AreParamEmpty() )
					frontEndCalibrator.LoadDefaultParameters();
			}

			//The timestamp of each sweep is taking at the beginning. Also the antenna position data are saved in the Sweep object.
			gpsInterface.ReadOneDataSet();
			uncalSweep.timeData = gpsInterface.GetTimeData();
			uncalSweep.azimuthAngle = antPositioner.GetAzimPosition();
			uncalSweep.polarization = antPositioner.GetPolarizationString();

			cout << "\nStarting the capturing of a whole sweep" << endl;
#ifdef RASPBERR_PI
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
			
			//Transferring the bands parameters to the objects which need them
			if(flagBandsParamReloaded)
			{
				//The bands parameters are given to the objects after a sweep was captured to make sure the exact number of samples is known
				cout << "\nThe bands parameters are transferred to the objects now that the number of samples is exactly known" << endl;
				auto bandsParameters = specConfigurator.GetBandsParameters();
				curveAdjuster.SetBandsParameters(bandsParameters);
				curveAdjuster.SetRefSweep(uncalSweep);
				frontEndCalibrator.SetBandsParameters(bandsParameters);
				frontEndCalibrator.LoadENR();

				flagBandsParamReloaded=false;
			}

			/////////////////////Sweep processing//////////////////////

			if( frontEndCalibrator.IsCalibStarted() )
			{
				/////////////////////Front-end calibration: estimating its parameters, gain and noise figure///////////////////////

				try
				{
					frontEndCalibrator.SetSweep(uncalSweep);

					if( frontEndCalibrator.IsNoiseSourceOff() )
					{
						//if(flagPlot)
							//calPloter.Plot(uncalSweep, "lines", "Sweep with noise source off");
						frontEndCalibrator.TurnOnNS();
						cout << "\nTurn on the noise source and press Enter to continue..." << endl;
						WaitForKey();
					}
					else
					{
						frontEndCalibrator.EndCalibration();
						cout << "\nTurn off the noise source, switch the input to the antenna and press Enter to continue..." << endl;
						WaitForKey();

#ifdef RASPBERRY_PI
						digitalWrite(piPins.LED_SWEEP_PROCESS, HIGH);
#endif
						frontEndCalibrator.EstimateParameters();

						TimeData timeData = gpsInterface.GetTimeData();
						frontEndCalibrator.SaveFrontEndParam(timeData);

						if(flagPlot)
						{
							//calPloter.Plot(uncalSweep, "lines", "Sweep with noise source on");
							//FreqValues calSweepNSoff = frontEndCalibrator.CalibrateSweep( frontEndCalibrator.GetSweepNSoff() );
							//calPloter.Plot(calSweepNSoff, "lines", "Calibrated sweep with NS off (50ohm load)");

							gainPloter.Clear();
							gainPloter.Plot( frontEndCalibrator.GetGain(), "lines", "Total receiver gain");

							nfPloter.Clear();
							nfPloter.Plot( frontEndCalibrator.GetNoiseFigure(), "lines", "Total receiver noise figure");
						}
#ifdef RASPBERRY_PI
						digitalWrite(piPins.LED_SWEEP_PROCESS, LOW);
#endif
					}
				}
				catch(std::exception & exc)
				{
					cerr << "\nWarning: error during front end calibration: " << exc.what() << '.' << endl;
					if( frontEndCalibrator.AreParamEmpty() )
					{
						cerr << "The default front end parameters will be loaded." << endl;
						frontEndCalibrator.LoadDefaultParameters();
					}
					else
						cerr << "The last estimated front end parameters will be used." << endl;
				}
			}
			else
			{
				///////////////Sweeps captured with the antenna connected to the input////////////

#ifdef RASPBERRY_PI
				digitalWrite(piPins.LED_SWEEP_PROCESS, HIGH);
#endif

				//Sweep calibration, taking into account the total gain curve
				Sweep calSweep = frontEndCalibrator.CalibrateSweep(uncalSweep);

				if(flagPlot)
				{
					//Ploting the actual sweep
					sweepPloter.Clear();
					sweepPloter.PlotSweep(calSweep);
				}

				//Transferring the ready-to-save sweep to the data logger in order to this component saves the data in memory
				dataLogger.SaveData(calSweep);

#ifdef RASPBERRY_PI
				digitalWrite(piPins.LED_SWEEP_PROCESS, LOW);
#endif


				if( antPositioner.IsLastPosition() && antPositioner.GetPolarization()==Polarization::VERTICAL)
				{
					if( !flagInfIterations && --numOfIter==0 )
						flagEndMeasCycle = true;
					else
						flagStartCal = true;
				}

				antPositioner.ChangePolarization();
				if( antPositioner.GetPolarization()==Polarization::HORIZONTAL)
					antPositioner.NextAzimPosition();

				cout << "\nThe new antenna position is: Azimutal=" << antPositioner.GetAzimPosition();
				cout << ", Polarization=" << antPositioner.GetPolarizationString() << endl;
			}
		}
		////////////////////End Loop/////////////////////

		cout << "\nThe sweeps capturing process finished." << endl;
		timer.stop();
		boost::timer::cpu_times times = timer.elapsed();

		double hours = double(times.wall)/(1e9*3600.0);
		cout << "\nThe elapsed time since the beginning is: " << hours << " hours" << endl;
	}
	catch(CustomException & exc)
	{
		cerr << "\nError: " << exc.what() << endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}
