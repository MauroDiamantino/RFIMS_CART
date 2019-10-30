/*
 * TestbenchGPSLogger.cpp
 *
 *  Created on: 25/10/2019
 *      Author: new-mauro
 */

#include "../src/TopLevel.h"
#include <cctype>

//#//////Constants/////////////
const unsigned int DEF_NUM_AZIM_POS = 6;
const unsigned int DELAY_US = 1000000;
const unsigned int WAITING_TIME_S = 20;
const unsigned int NUM_OF_TRIES = WAITING_TIME_S / (double(DELAY_US) / 1e6);

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
bool flagCalEnabled = true; //!< The declaration of a flag which defines if the calibration of the RF front end must be done or not. By default the calibration is enabled.
bool flagPlot = false; //!< The declaration of a flag which defines if the software has to generate plots or not. By default the plotting is not performed.
bool flagInfiniteLoop = true; //!< The declaration of a flag which defines if the software has to perform a finite number of measurement cycles or iterate infinitely. By default the software iterates infinitely.
bool flagRFI = false; //!< The declaration of a flag which defines if the software has to perform RFI detection or not. By this task is not performed.
bool flagUpload = true; //!< The declaration of a flag which defines if the software has to upload the measurements or not. By default the uploading is performed.

//! A variable which saves the number of measurements cycles which left to be done. It is used when the user wishes a finite number of measurements cycles.
unsigned int numOfMeasCycles = 0;
unsigned int numOfAzimPos = DEF_NUM_AZIM_POS;
//! A variable which saves the norm which defines the harmful RF interference levels: ska-mode1, ska-mode2, itu-ra769-2-vlbi.
RFI::ThresholdsNorm rfiNorm = RFI::SKA_MODE1;
//! A timer which is used to measure the execution time when the number of iterations is finite.
boost::timer::cpu_timer timer;

int main()
{
	cout << "\n\t\t\tTestbench del dispositivo Aaronia GPS Logger " << endl;

	cout << "\nNotas:" << endl;
	cout << "- Asegurese de realizar una calibracion del dispositivo (ver manual en el maletin) antes de realizar este test." << endl;
	cout << "- Cuando se le solicite alguna accion al usuario, el programa esperara a lo sumo " << WAITING_TIME_S << "s a que el usuario haga lo solicitado." << endl;

	try
	{
		unsigned int i=0;
		GPSInterface gpsInterface;

		cout << "\nInicializando la comunicacion con el receptor GPS..." << endl;
		gpsInterface.Initialize();
		cout << "La comunicacion fue inicializada con exito" << endl;

		cout << "\nPresione una tecla para comenzar..." << endl;
		WaitForKey();

		cout << "A continuacion se probara la recepcion a pedido de los datos de los sensores (streaming deshabilitado)" << endl;

		//////////////////////PRUEBA 1//////////////////////
		cout << "\nUbique el dispositivo sobre una superficie plana..." << endl;

		//Analisis del angulo roll
		do
		{
			usleep(DELAY_US);
			gpsInterface.UpdateRoll();
		}
		while( (gpsInterface.GetRoll() < -5.0 || gpsInterface.GetRoll() > 5.0) && ++i < NUM_OF_TRIES );

		if(i>=NUM_OF_TRIES)
			throw rfims_exception("se espero demasiado tiempo que el angulo roll este alrededor de cero.");

		//Analisis del angulo pitch
		do
		{
			usleep(DELAY_US);
			gpsInterface.UpdatePitch();
		}
		while( (gpsInterface.GetPitch() < -5.0 || gpsInterface.GetPitch() > 5.0) && ++i < NUM_OF_TRIES );

		if(i>=NUM_OF_TRIES)
			throw rfims_exception("se espero demasiado tiempo que el angulo pitch este alrededor de cero.");

		cout << "Excelente!" << endl;
		/////////////////////////////////////////////////////

		/////////////////////PRUEBA 2////////////////////////
		cout << "\nRote el dispositivo sobre uno de sus costados, de modo que el angulo roll este alrededor de 90 grados (en valor absoluto)..." << endl;

		//Analisis del angulo roll
		do
		{
			usleep(DELAY_US);
			gpsInterface.UpdateRoll();
		}
		while( ( fabs(gpsInterface.GetRoll()) < 85.0 || fabs(gpsInterface.GetRoll()) > 95.0 ) && ++i < NUM_OF_TRIES );

		if(i>=NUM_OF_TRIES)
			throw rfims_exception("se espero demasiado tiempo que el angulo roll este alrededor de 90 grados.");

		cout << "Excelente!" << endl;
		////////////////////////////////////////////////////

		////////////////////PRUEBA 3///////////////////////
		cout << "\nColoque el dispositivo en forma vertical sobre el cable USB, de modo que el angulo pitch este alrededor de 90 grados (en valor absoluto)..." << endl;

		//Analisis del angulo pitch
		do
		{
			usleep(DELAY_US);
			gpsInterface.UpdatePitch();
		}
		while( ( fabs(gpsInterface.GetPitch()) < 85.0 || fabs(gpsInterface.GetPitch()) > 95.0) && ++i < NUM_OF_TRIES );

		if(i>=NUM_OF_TRIES)
			throw rfims_exception("se espero demasiado tiempo que el angulo pitch este alrededor de 90 grados.");

		cout << "Excelente!" << endl;
		//////////////////////////////////////////////////////

		///////////////////PRUEBA 4///////////////////////////
		cout << "\nColoque el dispositivo sobre una superficie plana apuntando hacia el Este..." << endl;

		//Analisis del angulo yaw
		do
		{
			usleep(DELAY_US);
			gpsInterface.UpdateYaw();
		}
		while( (gpsInterface.GetYaw() < 80.0 || gpsInterface.GetYaw() > 100.0) && ++i < NUM_OF_TRIES );

		if(i>=NUM_OF_TRIES)
			throw rfims_exception("se espero demasiado tiempo que el angulo yaw este alrededor de 90 grados.");

		cout << "Excelente!" << endl;
		//////////////////////////////////////////////////////

		///////////////////PRUEBA 5///////////////////////////
		cout << "\nColoque el dispositivo sobre una superficie plana apuntando hacia el Sur..." << endl;

		//Analisis del angulo yaw
		do
		{
			usleep(DELAY_US);
			gpsInterface.UpdateYaw();
		}
		while( (gpsInterface.GetYaw() < 170.0 || gpsInterface.GetYaw() > 190.0) && ++i < NUM_OF_TRIES );

		if(i>=NUM_OF_TRIES)
			throw rfims_exception("se espero demasiado tiempo que el angulo yaw este alrededor de 180 grados.");

		cout << "Excelente!" << endl;
		//////////////////////////////////////////////////////

		/////////////////PRUEBA 6/////////////////////////////
		cout << "\nA continuacion se presenta la fecha y hora obtenida del GPS:" << endl;
		char answer;
		unsigned int numOfTries=0;
		bool flagSuccess = false;
		do
		{
			auto timeData = gpsInterface.UpdateTimeData();
			cout << '\t' << timeData.GetTimestamp() << endl;
			cout << "Es correcta la fecha y la hora? (Ingrese 's' por si o cualquier otra letra por no): ";
			cin >> answer;

			if( tolower(answer) == 's' )
				flagSuccess = true;
			else
				if(++numOfTries < 5)
					cout << "Se obtendra nuevamente la fecha y la hora del GPS: " << endl;
				else
					throw rfims_exception("se intento demasiadas veces obtener una fecha y hora correctas del GPS.");
		}
		while(!flagSuccess);

		cout << "Excelente!" << endl;
		//////////////////////////////////////////////////////

		/////////////////////PRUEBA 7/////////////////////////
		cout << "\nLa cantidad de satelites con los cuales el receptor GPS ha establecido conexion es: ";
		cout << gpsInterface.UpdateNumOfSatellites() << endl;
		//////////////////////////////////////////////////////

		/////////////////////PRUEBA 8////////////////////////
		cout << "\nA continuacion se probara el streaming de datos del dispositivo" << endl;
		cout << "La tasa de actualizacion para los datos de los sensores (no para los datos del GPS) sera: " << gpsInterface.GetDataRate() << " datos/s" << endl;

		cout << "Presione una tecla para comenzar..." << endl;
		WaitForKey();

		gpsInterface.EnableStreaming();
		cout << "\nStreaming habilitado!" << endl;

		cout << "\nDatos del GPS: 5 registros" << endl;

		for(int i=0; i<5; i++)
		{
			while( !gpsInterface.NewGPSData() );
			cout << "\n\tTimestamp: " << gpsInterface.GetTimeData().GetTimestamp() << endl;
			auto coordinates = gpsInterface.GetCoordinates();
			cout << "\tLatitud: " << coordinates.latitude << " °" << endl;
			cout << "\tLongitud: " << coordinates.longitude << " °" << endl;
			cout << "\tNumero de satelites: " << gpsInterface.GetNumOfSatellites() << endl;
			cout << "\tElevacion (GPS): " << gpsInterface.GetGPSElevation() << " m" << endl;
		}

		Data3D data;
		cout << "\nDatos del giroscopio: 10 registros" << endl;
		for(int i=0; i<10; i++)
		{
			while( !gpsInterface.NewGyroData() );
			data = gpsInterface.GetGyroData();
			cout << "\tx: " << data.x << " °/s\ty: " << data.y << " °/s\tz: " << data.z << " °/s" << endl;
		}

		cout << "\nDatos del acelerometro: 10 registros" << endl;
		for(int i=0; i<10; i++)
		{
			while( !gpsInterface.NewAccelerData() );
			data = gpsInterface.GetAccelerData();
			cout << "\tx: " << data.x << " g\ty: " << data.y << " g\tz: " << data.z << " g" << endl;
		}

		cout << "\nDatos del magnetometro: 10 registros" << endl;
		for(int i=0; i<10; i++)
		{
			while( !gpsInterface.NewCompassData() );
			data = gpsInterface.GetCompassData();
			cout << "\tx: " << data.x << " Gauss\ty: " << data.y << " Gauss\tz: " << data.z << " Gauss" << endl;
		}

		cout << "\nPresion y elevacion obtenida a partir del barometro: 10 registros" << endl;
		for(int i=0; i<10; i++)
		{
			while( !gpsInterface.NewPressure() );
			cout << "\tPresion: " << gpsInterface.GetPressure() << " hPa\tElevacion: " << gpsInterface.GetPressElevation() << " m" << endl;
		}

		cout << "\nAngulos de navegacion: 40 registros" << endl;
		for(int i=0; i<40; i++)
		{
			while( !gpsInterface.NewYaw() || !gpsInterface.NewRoll() || !gpsInterface.NewPitch() );
			cout << "\tyaw: " << gpsInterface.GetYaw() << " °\tpitch: " << gpsInterface.GetPitch() << " °\troll: " << gpsInterface.GetRoll() << " °" << endl;
		}

		gpsInterface.DisableStreaming();
		cout << "\nStreaming deshabilitado!" << endl;

		/////////////////////////////////////////////////////
	}
	catch(std::exception & exc)
	{
		cerr << "\nError: " << exc.what() << endl;
		cout << "\nTip: realice el test nuevamente y si no obtiene mejoras pruebe con realizar la calibracion manual nuevamente y asegurese de que la bateria este bien cargada." << endl;
		exit(EXIT_FAILURE);
	}

	cout << "\nLa prueba del dispositivo Aaronia GPS Logger fue exitosa!!" << endl;
	return 0;
}