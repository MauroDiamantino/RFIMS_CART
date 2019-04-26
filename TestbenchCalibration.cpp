/*
 * main.cpp
 *
 *  Created on: 21/04/2019
 *      Author: new-mauro
 */

#include "Spectran.h"
#include "SweepProcessing.h"

#ifdef RASPBERRY_PI
const unsigned int BUTTON = 10;
#endif

void WaitForKey()
{
    cin.clear();
    cin.ignore(std::cin.rdbuf()->in_avail());
    cin.get();
}

int main()
{
	cout << "\t\tTestbench de la calibracion del front end" << endl;

	try
	{
		SpectranInterface specInterface;
		SpectranConfigurator specConfigurator(specInterface);
		SweepBuilder sweepBuilder(specInterface);
		CurveAdjuster adjuster;
		FrontEndCalibrator frontCalibrator(adjuster);

		cout << "\nIniciando la comunicacion con el dispositivo Spectran HF-60105 V4 X" << endl;
		specInterface.Initialize();
		cout << "La sesion fue iniciada con exito" << endl;

		//Cargando los parametros
		cout << "\nCargando los parametros de configuracion del dispositivo Spectran desde los archivos correspondientes" << endl;
		if( specConfigurator.LoadFixedParameters() )
		{
			//If the fixed parameters were loaded for the first time or they were reloaded, the initial configuration will be repeated
			cout << "\nLos parametros fijos fueron cargados por primera vez o fueron recargados por lo que se efectuara la configuracion inicial" << endl;
			specConfigurator.InitialConfiguration();
			cout << "La configuracion inicial fue realizada con exito" << endl;
		}

		cout << "\nCargando los parametros de las bandas frecuenciales" << endl;
		specConfigurator.LoadBandsParameters();
		cout << "Los parametros de las bandas fueron cargados exitosamente" << endl;

		cout << "\nIniciando el proceso de calibracion del front end" << endl;
		frontCalibrator.StartCalibration();

		RFPloter rfPloter;

		FreqValueSet wholeSweep;

		//Capturando los barridos de cada una de las bandas que componen el barrido completo
		for(unsigned int i=0; i < specConfigurator.GetNumOfBands(); i++)
		{
			BandParameters currBandParam;
			FreqValueSet currFreqBand;

			cout << "\nSe inicia el proceso de captura de un nuevo barrido, correspondiente a la banda frecuencial N° " << i+1 << endl;
			cout << "Configurando el dispositivo Spectran con los parametros de la siguiente banda frecuencial" << endl;
			currBandParam = specConfigurator.ConfigureNextBand();

			cout << "Capturando los puntos de un barrido" << endl;
			currFreqBand = sweepBuilder.CaptureSweep(currBandParam);
			cout << "La captura de un barrido finalizo" << endl;

			cout << "Agregando el barrido de la banda frecuencial actual al final del barrido completo" << endl;
			wholeSweep.PushBack(currFreqBand);
		}

		cout << "\nFinalizo la captura de un barrido con NS apagado. Sera cargado en el calibrador del front end." << endl;
		frontCalibrator.SetSweep( wholeSweep );

		rfPloter.Plot(wholeSweep, "lines", "Sweep NS off");
		frontCalibrator.TurnOnNS();

		cout << "\nAlimente el generador de ruido y presione una tecla para continuar..." << endl;
		WaitForKey();

		wholeSweep.Clear();

		//Capturando los barridos de cada una de las bandas que componen el barrido completo
		for(unsigned int i=0; i < specConfigurator.GetNumOfBands(); i++)
		{
			BandParameters currBandParam;
			FreqValueSet currFreqBand;

			cout << "\nSe inicia el proceso de captura de un nuevo barrido, correspondiente a la banda frecuencial N° " << i+1 << endl;
			cout << "Configurando el dispositivo Spectran con los parametros de la siguiente banda frecuencial" << endl;
			currBandParam = specConfigurator.ConfigureNextBand();

			cout << "Capturando los puntos de un barrido" << endl;
			currFreqBand = sweepBuilder.CaptureSweep(currBandParam);
			cout << "La captura de un barrido finalizo" << endl;

			cout << "Agregando el barrido de la banda frecuencial actual al final del barrido completo" << endl;
			wholeSweep.PushBack(currFreqBand);
		}

		cout << "\nFinalizo la captura de un barrido con NS on. Sera cargado en el calibrador del front end." << endl;
		frontCalibrator.SetSweep( wholeSweep );

		rfPloter.Plot(wholeSweep, "lines", "Sweep NS on");

		cout << "\nApague el generador de ruido y presione una tecla para continuar..." << endl;
		WaitForKey();

		cout << "Cargando los parametros de las bandas (ya con valores correctos de numero de muestras) en el FrontEndCalibrator y CurveAdjuster" << endl;
		std::vector<BandParameters> bandsParameters;
		bandsParameters = specConfigurator.GetBandsParameters();
		adjuster.SetBandsParameters(bandsParameters);
		frontCalibrator.SetBandsParameters(bandsParameters);

		cout << "Cargando los valores del parametero ENR del generador de ruido" << endl;
		frontCalibrator.LoadENR();

		frontCalibrator.EndCalibration();

		FrontEndParameters frontEndParam = frontCalibrator.CalculateParameters();

		TimeData timeData;
		timeData.year=2019; timeData.month=4; timeData.day=25;
		timeData.hour=19; timeData.minute=0; timeData.second=0;
		frontCalibrator.SaveFrontEndParam(timeData);

		Gnuplot gainPloter, nfPloter;
		gainPloter.set_style("lines"); nfPloter.set_style("lines");

		gainPloter.plot_xy(frontEndParam.frequency, frontEndParam.gain_dB, "Ganancia");
		gainPloter.set_title("Ganancia total del front end");
		gainPloter.set_xlabel("Frecuencia (Hz)");
		gainPloter.set_ylabel("Ganancia (dB)");

		nfPloter.plot_xy(frontEndParam.frequency, frontEndParam.noiseFigure, "Figura de ruido");
		nfPloter.set_title("Figura de ruido total del front end");
		nfPloter.set_xlabel("Frecuencia (Hz)");
		nfPloter.set_ylabel("Figura de ruido (dB)");

		SweepCalibrator sweepCalibrator;
		FreqValueSet totalGain("gain");
		totalGain.frequencies = frontEndParam.frequency;
		totalGain.values = frontEndParam.gain_dB;
		sweepCalibrator.BuildCalCurve(totalGain);

		wholeSweep.Clear();

		//Capturando los barridos de cada una de las bandas que componen el barrido completo
		for(unsigned int i=0; i < specConfigurator.GetNumOfBands(); i++)
		{
			BandParameters currBandParam;
			FreqValueSet currFreqBand;

			cout << "\nSe inicia el proceso de captura de un nuevo barrido, correspondiente a la banda frecuencial N° " << i+1 << endl;
			cout << "Configurando el dispositivo Spectran con los parametros de la siguiente banda frecuencial" << endl;
			currBandParam = specConfigurator.ConfigureNextBand();

			cout << "Capturando los puntos de un barrido" << endl;
			currFreqBand = sweepBuilder.CaptureSweep(currBandParam);
			cout << "La captura de un barrido finalizo" << endl;

			cout << "Agregando el barrido de la banda frecuencial actual al final del barrido completo" << endl;
			wholeSweep.PushBack(currFreqBand);
		}

		FreqValueSet calSweep = sweepCalibrator.CalibrateSweep(wholeSweep);

		rfPloter.Plot(calSweep, "lines", "Calibrated sweep NS on");

		cout << "\nPresione una tecla para terminar..." << endl;
		WaitForKey();
	}
	catch(std::exception & exc)
	{
		cerr << "\Error: " << exc.what() << endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}
