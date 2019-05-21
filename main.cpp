/*
 * main.cpp
 *
 *  Created on: 28/04/2019
 *      Author: new-mauro
 */

#include "Spectran.h"
#include "SweepProcessing.h"
#include "AntennaPositioning.h"
#include <boost/timer/timer.hpp>

void WaitForKey();

int main()
{
	struct
	{
		const int LED_SWEEP_CAPTURE = 9;
		const int LED_SWEEP_PROCESS = 10;
	} piPins;
	
	boost::timer::cpu_timer timer;
	bool flagBandsParamReloaded=false;
	bool flagEndMeasCycle = false;

	cout << "\n\t\t\t\tRF Interference Monitoring System (RFIMS)" << endl;
	cout << "\t\t\t\tChina-Argentina Radio Telescope (CART)\n" << endl;

	try
	{
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
		//RFPloter sweepPloter[12], calPloter;
		GPSInterface gpsInterface;
		AntennaPositioner antPositioner(gpsInterface);

		cout << "\nInitializing communication with Aaronia Spectran device..." << endl;
		specInterface.Initialize();
		cout << "The communication was established successfully" << endl;

		//Initializing the communication with the GPS receiver
		cout << "\nInitializing the communication with the GPS receiver..." << endl;
		gpsInterface.Initialize();
		cout << "The communication was established successfully" << endl;

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

		//Front-end calibration
		cout << "\nStarting the front end calibration" << endl;
		frontEndCalibrator.StartCalibration();
		cout << "\nTurn off the noise source, switch the input to this one and press Enter to continue..." << endl;
		WaitForKey();

		//unsigned int i=0;
		Sweep wholeSweep;
		while(!flagEndMeasCycle)
		{
			wholeSweep.Clear();

			//The timestamp of each sweep is taking at the beginning
			gpsInterface.ReadOneDataSet();
			wholeSweep.timeData = gpsInterface.GetTimeData();

			cout << "\nStarting the capturing of a whole sweep" << endl;
#ifdef RASPBERR_PI
			digitalWrite(piPins.LED_SWEEP_CAPTURE, HIGH);
#endif

			//Capturing the sweeps related to each one of the frequency bands, which in conjunction form a whole sweep
			for(unsigned int i=0; i < specConfigurator.GetNumOfBands(); i++)
			{
				BandParameters currBandParam;
				FreqValues currFreqBand;

				cout << "\nFrequency band N° " << i+1 << endl;
				currBandParam = specConfigurator.ConfigureNextBand();
				cout << "Fstart=" << (currBandParam.startFreq/1e6) << " MHz, Fstop=" << (currBandParam.stopFreq/1e6) << " MHz, ";
				cout << "RBW=" << (currBandParam.rbw/1e3) << " KHz, Sweep time=" << currBandParam.sweepTime << " ms" << endl;

				currFreqBand = sweepBuilder.CaptureSweep(currBandParam);

				bool flagLastPointRemoved = wholeSweep.PushBack(currFreqBand);
				if( flagLastPointRemoved && flagBandsParamReloaded )
					currBandParam.samplePoints--;
				
				if(flagBandsParamReloaded)
					specConfigurator.SetCurrBandParameters(currBandParam);
			}
#ifdef RASPBERRY_PI
			digitalWrite(piPins.LED_SWEEP_CAPTURE, LOW);
#endif
			specInterface.SoundNewSweep();
			cout << "\nThe capturing of a whole sweep (1 GHz to 8 GHz) finished" << endl;
			
			//Transferring the bands parameters to the objects which need them
			if(flagBandsParamReloaded)
			{
				//The bands parameters are given to the objects after a sweep was captured to make sure the exact number of samples is known
				cout << "\nThe bands parameters are transferred to the objects now that the number of samples is exactly known" << endl;
				std::vector<BandParameters> bandsParameters;
				bandsParameters = specConfigurator.GetBandsParameters();
				curveAdjuster.SetBandsParameters(bandsParameters);
				curveAdjuster.SetRefSweep(wholeSweep);
				frontEndCalibrator.SetBandsParameters(bandsParameters);
				frontEndCalibrator.LoadENR();

				flagBandsParamReloaded=false;
			}

			////////Sweep processing///////////
			if( frontEndCalibrator.IsCalibStarted() )
			{
				////////////Front End Calibration/////////////
				frontEndCalibrator.SetSweep(wholeSweep);

				if( !frontEndCalibrator.IsNoiseSourceOn() )
				{
					//calPloter.Plot(wholeSweep, "lines", "Sweep with noise source off");
					frontEndCalibrator.TurnOnNS();
					cout << "\nTurn on the noise source and press Enter to continue..." << endl;
					WaitForKey();
				}
				else
				{
					//calPloter.Plot(wholeSweep, "lines", "Sweep with noise source on");
					frontEndCalibrator.EndCalibration();
					//frontEndCalibrator.TurnOffNS();
					cout << "\nTurn off the noise source, switch the input to the antenna and press Enter to continue..." << endl;
					WaitForKey();

#ifdef RASPBERRY_PI
					digitalWrite(piPins.LED_SWEEP_PROCESS, HIGH);
#endif
					frontEndCalibrator.EstimateParameters();

//					FreqValues totalGain("gain");
//					totalGain.frequencies = frontEndParam.frequency;
//					totalGain.values = frontEndParam.gain_dB;

					//FreqValues calSweepNSoff = frontEndCalibrator.CalibrateSweep( frontEndCalibrator.GetSweepNSoff() );
					//calPloter.Plot(calSweepNSoff, "lines", "Calibrated sweep with NS off (50ohm load)");

					TimeData timeData = gpsInterface.GetTimeData();
					frontEndCalibrator.SaveFrontEndParam(timeData);

//					gainPloter.set_title("Ganancia total del front end");
//					gainPloter.set_xlabel("Frecuencia (Hz)");
//					gainPloter.set_ylabel("Ganancia (dB)");
//					gainPloter.plot_xy(frontEndParam.frequency, frontEndParam.gain_dB, "Ganancia");
//
//					nfPloter.set_title("Figura de ruido total del front end");
//					nfPloter.set_xlabel("Frecuencia (Hz)");
//					nfPloter.set_ylabel("Figura de ruido (dB)");
//					nfPloter.plot_xy(frontEndParam.frequency, frontEndParam.noiseFigure, "Figura de ruido");
#ifdef RASPBERRY_PI
					digitalWrite(piPins.LED_SWEEP_PROCESS, LOW);
#endif

				}
			}
			else
			{
				///////////Sweeps captured with the antenna connected to the input////////////

#ifdef RASPBERRY_PI
				digitalWrite(piPins.LED_SWEEP_PROCESS, HIGH);
#endif

				//Sweep calibration, taking into account the total gain curve
				Sweep calSweep = frontEndCalibrator.CalibrateSweep(wholeSweep);

				//Ploting the actual sweep
				//sweepPloter[i++].PlotSweep(calSweep);

				//Transferring the ready-to-save sweep and the antenna position data to the data
				//logger in order to this component saves the data in memory
				calSweep.azimuthAngle = antPositioner.GetAzimPosition();
				calSweep.polarization = antPositioner.GetPolarizationString();
				dataLogger.SaveData(calSweep);

#ifdef RASPBERRY_PI
				digitalWrite(piPins.LED_SWEEP_PROCESS, LOW);
#endif

				Polarization currPolarization = antPositioner.GetPolarization();
				if( antPositioner.IsLastPosition() && currPolarization==Polarization::VERTICAL)
					flagEndMeasCycle = true;
				else
				{
					antPositioner.ChangePolarization();
					if( antPositioner.GetPolarization()==Polarization::HORIZONTAL)
						antPositioner.NextAzimPosition();

					cout << "\nThe new antenna position is: Azimutal=" << antPositioner.GetAzimPosition();
					cout << ", Polarization=" << antPositioner.GetPolarizationString() << endl;
				}
			}
		}

		cout << "\nThe measurement cycle finished" << endl;
		timer.stop();
		boost::timer::cpu_times times = timer.elapsed();

		double hours = double(times.wall)/(1e9*3600.0);
		cout << "\nThe elapsed time since the beginning is: " << hours << " hours" << endl;
		cout << "\nPress Enter to terminate the program..." << endl;
		WaitForKey();
	}
	catch(CustomException & exc)
	{
		cerr << "\nError: " << exc.what() << endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}
