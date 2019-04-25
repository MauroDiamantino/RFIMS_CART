/*
 * TestbenchSweepCapturing.cpp
 *
 *  Created on: 31/03/2019
 *      Author: new-mauro
 */

#include "Spectran.h"
#include <time.h>

#define MEDIR_TIEMPOS(description,modo)	{\
	\
	clock_gettime(CLOCK_REALTIME, &endtime);\
	if(modo){\
		timeinterval = double((endtime.tv_sec - starttime.tv_sec) * 1000000000 + endtime.tv_nsec - starttime.tv_nsec);\
		std::cout << description  << "\t" << (double) timeinterval/1000000 << " ms" << std::endl;\
	}\
	starttime = endtime;\
}

void WaitForKey()
{
    cin.clear();
    cin.ignore(std::cin.rdbuf()->in_avail());
    cin.get();
}

int main()
{
	struct timespec starttime;
	struct timespec endtime;
	double timeinterval;

	cout << "\t\tTestbench de las clases SpectranInterface, SpectranConfigurator, SweepBuilder, RFPloter y DataLogger" << endl;

	try
	{
		SpectranInterface specInterface;

		cout << "\nIniciando la comunicacion con el dispositivo Spectran HF-60105 V4 X" << endl;
		specInterface.Initialize();
		cout << "La sesion fue iniciada con exito" << endl;

		SpectranConfigurator specConfigurator(specInterface);
		SweepBuilder swBuilder(specInterface);

		//Cargando los parametros
		cout << "\nCargando los parametros de configuracion del dispositivo Spectran desde los archivos correspondientes" << endl;
		if( specConfigurator.LoadFixedParameters() )
		{
			//If the fixed parameters were loaded for the first time or they were reloaded, the initial configuration will be repeated
			cout << "Los parametros fijos fueron cargados por primera vez o fueron recargados por lo que se efectuara la configuracion inicial" << endl;
			specConfigurator.InitialConfiguration();
			cout << "La configuracion inicial fue realizada con exito" << endl;
		}

		cout << "\nCargando los parametros de las bandas frecuenciales" << endl;
		specConfigurator.LoadBandsParameters();
		cout << "\nLos parametros de las bandas fueron cargados exitosamente" << endl;

		FreqValueSet wholeSweep;

		MEDIR_TIEMPOS("",0);

		//Capturando los barridos de cada una de las bandas que componen el barrido completo
		for(unsigned int i=0; i < specConfigurator.GetNumOfBands(); i++)
		{
			BandParameters currBandParam;
			FreqValueSet currFreqBand;

			cout << "\nSe inicia el proceso de captura de un nuevo barrido, correspondiente a la banda frecuencial N° " << i+1 << endl;
			cout << "Configurando el dispositivo Spectran con los parametros de la siguiente banda frecuencial" << endl;
			currBandParam = specConfigurator.ConfigureNextBand();

			cout << "La frecuencias de los extremos son: fstart= " << (currBandParam.startFreq/1e6) << " MHz, fstop= " << (currBandParam.stopFreq/1e6) << " MHz" << endl;

			cout << "Capturando los puntos de un barrido" << endl;
			currFreqBand = swBuilder.CaptureSweep(currBandParam);
			specConfigurator.SetCurrBandParameters(currBandParam);
			cout << "La captura de un barrido finalizo" << endl;

			cout << "\nLa cantidad de puntos del barrido es: " << currBandParam.samplePoints << endl;

			cout << "Agregando el barrido de la banda frecuencial actual al final del barrido completo" << endl;
			wholeSweep.PushBack(currFreqBand);
		}

		MEDIR_TIEMPOS("\nTiempo consumido para la captura de un barrido completo ",1);

		//Setting the timestamp
		wholeSweep.timeData.year=2019; wholeSweep.timeData.month=4; wholeSweep.timeData.day=24;
		wholeSweep.timeData.hour=8; wholeSweep.timeData.minute=11; wholeSweep.timeData.second=0;

//		cout << "\nPresione cualquier tecla para terminar..." << endl;
//		WaitForKey();
	}
	catch(std::exception & exc)
	{
		cerr << "\nError: " << exc.what() << endl;

		WaitForKey();

		exit(EXIT_FAILURE);
	}

	return 0;
}
