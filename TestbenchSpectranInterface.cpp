/*
 * TestbenchSpectranInterface.cpp
 *
 *  Created on: 06/03/2019
 *      Author: new-mauro
 */

#include "SpectranInterface.h"
#include <map>

const DWORD VID = 0x0403;
const DWORD PID = 0xE8D8;
const string DEVICE_DESCRIPTION = "Aaronia SPECTRAN HF-60105 X";
//const unsigned int WAITING_TIME = 200000;
//Parametros para la captura de un barrido
//const float CENTER_FREQ = 800e6;
const float START_FREQ = 750e6;
const float STOP_FREQ = 760e6;
const float SPAN = (STOP_FREQ - START_FREQ);
const float SWEEP_TIME = 1e3;
const float RBW = 1e6;
const float VBW = 3e6;
const float SAMPLE_POINTS = 2*SPAN/RBW + 1;
//const float DELTA_FREQ = SPAN/(SAMPLE_POINTS);

void SetAndReadVariables(SpectranInterface& interface, VarName var1, VarName var2, float value)
{
	Command comm;
	Reply reply;
	unsigned int i;

	for (i=(unsigned int)var1; i<=(unsigned int)var2; i++)
	{
		//usleep(WAITING_TIME);

		cout << "Configuracion de la variable ";
		printf("%02X", i);
		cout << endl;
		comm.SetAs(Command::SETSTPVAR, VarName(i), value);
		interface.Write(comm);
		reply.PrepareTo(Reply::SETSTPVAR);
		interface.Read(reply);
		assert( reply.IsRight()==true );
		cout << "La configuracion fue exitosa" << endl;
		comm.Clear();
		reply.Clear();

		//usleep(WAITING_TIME);

		cout << "Lectura de la variable ";
		printf("%02X", i);
		cout << endl;
		comm.SetAs(Command::GETSTPVAR, VarName(i));
		interface.Write(comm);
		reply.PrepareTo(Reply::GETSTPVAR, VarName(i));
		interface.Read(reply);
		assert( reply.IsRight()==true );
		assert( reply.GetValue()==value );
		cout << "El valor de la variable es " << reply.GetValue() << endl;
		comm.Clear();
		reply.Clear();
	}
}

void SetAndReadVariables(SpectranInterface& interface, VarName var1, float value){ SetAndReadVariables(interface, var1, var1, value);	}


int main(){
	Command comm;
	Reply reply;

	cout.precision(1);
	cout.setf(ios::fixed, ios::floatfield);

	cout << "\t\t\tTestbench de la clase SpectranInterface\n" << endl;

	try
	{
		cout << "Abriendo comunicacion con el dispositivo Spectran" << endl;
		SpectranInterface interface;
		assert(interface.GetVID()==VID);
		assert(interface.GetPID()==PID);
		assert(interface.GetDevDescription()==DEVICE_DESCRIPTION);
		cout << "La comunicacion fue abierta con exito" << endl;

		cout << "Iniciando la sesion con el dispositivo" << endl;
		unsigned int errorCounter=0;

		interface.Initialize();
		assert( interface.IsLogged()==true );
		cout << "La sesion fue iniciada con exito\n" << endl;

//		cout << "Configuracion de la variable STARTFREQ en 800 MHz" << endl;
//		SetAndReadVariables(interface, VarName::STARTFREQ, 800e6);
//
//		cout << "Configuracion de la variable STOPFREQ en 900 MHz" << endl;
//		SetAndReadVariables(interface, VarName::STOPFREQ, 900e6);
//
//		SetAndReadVariables(interface, VarName::RESBANDW, VarName::SWEEPTIME, 200.0);
//
//		SetAndReadVariables(interface, VarName::ATTENFAC, VarName::REFLEVEL, 10.0);
//
//		SetAndReadVariables(interface, VarName::DISPUNIT, VarName::DEMODMODE, 0.0);
//
//		SetAndReadVariables(interface, VarName::ANTTYPE, VarName::RECVCONF, 0.0);
//
//		SetAndReadVariables(interface, VarName::CENTERFREQ, 500e6);
//
//		SetAndReadVariables(interface, VarName::SPANFREQ, 100e6);
//
//		SetAndReadVariables(interface, VarName::PREAMPEN, VarName::SWPDLYACC, 0.0);

		//SetAndReadVariables(interface, VarName::SWPFRQPTS, SAMPLE_POINTS);

		//SetAndReadVariables(interface, VarName::ANTGAIN, 5.0);

		//Configuracion de las variables STARTFREQ, STOPFREQ, SWEEPTIME y RBW para luego capturar
		//un barrido con una cantidad accesible de puntos
		SetAndReadVariables(interface, VarName::STARTFREQ, START_FREQ);
		SetAndReadVariables(interface, VarName::STOPFREQ, STOP_FREQ);
		SetAndReadVariables(interface, VarName::SWEEPTIME, SWEEP_TIME);
		SetAndReadVariables(interface, VarName::RESBANDW, RBW);
		SetAndReadVariables(interface, VarName::VIDBANDW, VBW);

		cout << "\nCaptura de un barrido y presentacion de los valores en linea" << endl;
		typedef map<float,float> SweepMap;
		SweepMap sweep;
		pair< SweepMap::iterator, bool> mapReply;
		SweepReply swReply;
		bool flagSweepReady=false;
		float frequency, power;
		unsigned int i=0;

		//Reinicio del barrido actual
//		comm.Clear();
//		comm.SetAs(Command::SETSTPVAR, VarName::USBSWPRST, 1.0);
//		interface.Write(comm);
//		reply.Clear();
//		reply.PrepareTo(Reply::SETSTPVAR);
//		interface.Read(reply);
//		assert( reply.IsRight()==true );

		cout << "Habilitacion del envio de barridos y reinicio del barrido actual\n" << endl;
		comm.Clear();
		reply.Clear();
		comm.SetAs(Command::SETSTPVAR, VarName::USBMEAS, 1.0);
		interface.Write(comm);
		reply.PrepareTo(Reply::SETSTPVAR);
		interface.Read(reply);
		assert( reply.IsRight()==true );
		//interface.Purge();

		cout << "\tIndice\t\tFrecuencia(Hz)\t\tPotencia(dBm)" << endl;
		while (flagSweepReady==false)
		{
			swReply.Clear();
			interface.Read(swReply);
			frequency=swReply.GetFrequency();
			power=swReply.GetValue();
			mapReply = sweep.insert( SweepMap::value_type(frequency, power) );
			flagSweepReady = !mapReply.second;
			i++;
			cout << '\t' << i << "\t\t" << frequency << "\t\t" << power << endl;
			usleep( (__useconds_t) SWEEP_TIME/51 ); //tiempo teorico entre muestras dado un sweep time de 5s y que la cantidad de muestras es 51
		}
		cout << "La cantidad de puntos capturados es: " << sweep.size() << endl;
//		assert( sweep.size()==SAMPLE_POINTS );
//		SweepMap::iterator it = sweep.begin();
//		float freq1, freq2, delta;
//		freq1=it->first;
//		it++;
//		freq2=it->first;
//		delta=freq2-freq1;
//		cout << "El delta de frecuencia es: (" << freq2 << " Hz - " << freq1 << " Hz) = " << delta << " Hz" << endl;
//		assert( delta==DELTA_FREQ );

		cout << "\nDeshabilitacion del envio de barridos" << endl;
		Command comm(Command::SETSTPVAR, VarName::USBMEAS, 0.0);
		Reply reply;
		errorCounter=0;

		for(unsigned int i=0; i<2; i++)
		{
			//usleep(WAITING_TIME);

			try
			{
				interface.Write(comm);
				reply.PrepareTo(Reply::SETSTPVAR);
				interface.Read(reply);
				interface.Purge();
				if(reply.IsRight()==false){
					CustomException except("Warning: la respuesta de uno de los comandos para deshabilitar el envio de barridos fue incorrecta.");
					throw(except);
				}
			}
			catch(exception& except)
			{
				cerr << except.what() << endl;
				errorCounter++;
			}
		}
		if(errorCounter>=2){
			CustomException except("Error: fallo la deshabilitacion de barridos");
			throw(except);
		}
		cout << "La deshabilitacion fue realizada con exito" << endl;

		cout << "Limpieza de los buffers de entrada y salida" << endl;
		interface.Purge();
		cout << "Limpieza exitosa" << endl;

		sleep(1);

		//El envio del comando LOGOUT y el cierre de la comunicacion con el dispositivo seran realizados por el destructor

	}
	catch(exception& exc)
	{
		cerr << '\n' << exc.what() << endl;
		exit(EXIT_FAILURE);
	}

	cout << "\nEl testbench de la clase SpectranInterface finalizo con resultados exitosos. Sigue asi!!" << endl;

	return 0;
}
