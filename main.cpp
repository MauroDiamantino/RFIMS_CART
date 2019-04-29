/*
 * main.cpp
 *
 *  Created on: 28/04/2019
 *      Author: new-mauro
 */

#include "Spectran.h"
#include "SweepProcessing.h"
#include "AntennaPositioning.h"

void WaitForKey()
{
    cin.clear();
    cin.ignore(std::cin.rdbuf()->in_avail());
    cin.get();
}

int main()
{
	bool flagBandsParamReloaded=false;
	bool flagEndMeasCycle = false;
	bool flag50ohmLoadSweep = false;

	cout << "\n\t\t\tRF Interference Monitoring System (RFIMS)" << endl;
	cout << "\t\t\tChina-Argentina Radio Telescope (CART)\n" << endl;

	try
	{
		SpectranInterface specInterface;

		cout << "\nInitializing communication with Aaronia Spectran device..." << endl;
		specInterface.Initialize();
		cout << "The communication was established successfully" << endl;

		SpectranConfigurator specConfigurator(specInterface);
		SweepBuilder sweepBuilder(specInterface);
		CurveAdjuster curveAdjuster;
		FrontEndCalibrator frontEndCalibrator(curveAdjuster);
		SweepCalibrator sweepCalibrator;
		DataLogger dataLogger;
		RFPloter sweepPloter, calPloter;
		GPSInterface gpsInterface;
		AntennaPositioner antPositioner;

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

		while(!flagEndMeasCycle)
		{
			FreqValueSet wholeSweep;

			//Capturing the sweeps related to each one of the frequency bands, which in conjunction form a whole sweep
			cout << "\nStarting the capturing of a whole sweep\n" << endl;
			for(unsigned int i=0; i < specConfigurator.GetNumOfBands(); i++)
			{
				BandParameters currBandParam;
				FreqValueSet currFreqBand;

				cout << "\nFrequency band N° " << i+1 << endl;
				currBandParam = specConfigurator.ConfigureNextBand();
				cout << "Fstart= " << (currBandParam.startFreq/1e6) << " MHz, Fstop= " << (currBandParam.stopFreq/1e6) << " MHz, ";
				cout << "RBW= " << (currBandParam.rbw/1e3) << " KHz, Sweep time= " << currBandParam.sweepTime << " ms" << endl;

				currFreqBand = sweepBuilder.CaptureSweep(currBandParam);

				wholeSweep.PushBack(currFreqBand);
			}

			//Transferring the bands parameters to the objects which need them
			if(flagBandsParamReloaded)
			{
				//The bands parameters are given to the objects after a sweep was captured to make sure the exact number of samples is known
				std::vector<BandParameters> bandsParameters;
				bandsParameters = specConfigurator.GetBandsParameters();
				curveAdjuster.SetBandsParameters(bandsParameters);
				frontEndCalibrator.SetBandsParameters(bandsParameters);
				frontEndCalibrator.LoadENR();

				flagBandsParamReloaded=false;
			}

			if( frontEndCalibrator.IsCalibStarted() )
			{
				////////////Front End Calibration/////////////
				frontEndCalibrator.SetSweep(wholeSweep);

				if( !frontEndCalibrator.IsNoiseSourceOn() )
				{
					calPloter.Plot(wholeSweep, "lines", "Sweep noise source off");
					frontEndCalibrator.TurnOnNS();
					cout << "\nTurn on the noise source and press Enter to continue..." << endl;
					WaitForKey();
				}
				else
				{
					calPloter.Plot(wholeSweep, "lines", "Sweep noise source on");
					frontEndCalibrator.EndCalibration();
					cout << "\nTurn off the noise source and press Enter to continue..." << endl;
					WaitForKey();
					flag50ohmLoadSweep=true;
				}
			}
			else
			{
				//Sweep calibration, taking into account the total gain curve
				FreqValueSet calSweep = sweepCalibrator.CalibrateSweep(wholeSweep);

				if(flag50ohmLoadSweep)
				{
					/////////Sweep captured with a 50 ohm load connected to the input////////////
					calPloter.Plot(calSweep, "lines", "Calibrated sweep with 50ohm load");
					cout << "Switch the input to the antenna and press Enter to continue..." << endl;
					WaitForKey();
					flag50ohmLoadSweep=false;
				}
				else
				{
					///////////Sweeps captured with the antenna connected to the input////////////

					//Ploting the actual sweep
					sweepPloter.Clear();
					sweepPloter.PlotSweep(calSweep);

					//Taking the time data from the GPS and transferring these to the sweep structure
					calSweep.timeData = gpsInterface.GetTimeData();

					//Transferring the ready-to-save sweep and the antenna position data to the data
					//logger in order to this component saves the data in memory
					dataLogger.SetSweep(calSweep);
					dataLogger.SetAntennaData(antPositioner.GetAzimPosition(), antPositioner.GetPolarization());
					dataLogger.SaveData();

					if( antPositioner.IsLastPosition() )
						flagEndMeasCycle = true;
					else
					{
						if( antPositioner.GetPolarization()=="horizontal" )
							antPositioner.ChangePolarization();
						else
							antPositioner.NextAzimPosition();
					}
				}
			}
		}

		cout << "\nThe measurement cycle finished" << endl;
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
