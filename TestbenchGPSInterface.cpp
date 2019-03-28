/*
 * TestbenchGPSInterface.cpp
 *
 *  Created on: 24/03/2019
 *      Author: new-mauro
 */

#include "AntennaPositioning.h"
//#include <cstdlib>

int main()
{
	cout << "\t\tTestbench of class GPS Interface" << endl;
	try
	{
		cout << "Creando objeto interfaz GPS" << endl;
		GPSInterface gpsInterface;
		cout << "El objeto fue creado exitosamente" << endl;

		cout << "\nInicializando comunicacion con el receptor GPS" << endl;
		gpsInterface.Initialize();
		cout << "La comunicacion fue inicializada con exito" << endl;

		cout << "A continuacion se irán capturando mediciones con el metodo ReadOneDataSet() y mostrandolas..." << endl;
		sleep(2);
		GPSCoordinates coordinates;
		Data3D data;
		for(unsigned int i=0; i<50; i++)
		{
			gpsInterface.ReadOneDataSet();

			//system("clear");

			cout << "\nDatos del GPS:" << endl;
			cout << "\tTimestamp: " << gpsInterface.GetTimestamp() << endl;
			coordinates = gpsInterface.GetCoordinates();
			cout << "\tLatitud: " << coordinates.latitude << " °" << endl;
			cout << "\tLongitud: " << coordinates.longitude << " °" << endl;
			cout << "\tNumero de satelites: " << gpsInterface.GetNumOfSatellites() << endl;
			cout << "\tElevacion: " << gpsInterface.GetGPSElevation() << " m" << endl;

			cout << "\nDatos del Giroscopio:" << endl;
			data = gpsInterface.GetGyroData();
			cout << "\tx: " << data.x << " °/s\ty: " << data.y << " °/s\tz: " << data.z << " °/s" << endl;

			cout << "\nDatos del magnetometro:" << endl;
			data = gpsInterface.GetCompassData();
			cout << "\tx: " << data.x << "\ty: " << data.y << "\tz: " << data.z << endl;

			cout << "\nDatos del acelerometro:" << endl;
			data = gpsInterface.GetAccelerData();
			cout << "\tx: " << data.x << " g\ty: " << data.y << " g\tz: " << data.z << " g" << endl;

			cout << "\nPresion y elevacion obtenida a partir del barometro:" << endl;
			cout << "\tPresion: " << gpsInterface.GetPressure() << " hPa\tElevacion: " << gpsInterface.GetPressureElevation() << " m" << endl;

			cout << "\nAngulos de navegacion:" << endl;
			cout << "\tyaw: " << gpsInterface.GetYaw() << " °\tpitch: " << gpsInterface.GetPitch() << " °\troll: " << gpsInterface.GetRoll() << " °" << endl;

			//usleep(500000);
		}
	}
	catch(exception& exc)
	{
		cerr << exc.what() << endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}


