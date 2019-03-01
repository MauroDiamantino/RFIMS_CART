/*
 * Testbench1.cpp
 *
 *  Created on: 27/02/2019
 *      Author: new-mauro
 */
#include "SpectranInterface.h"

void PrintBytes(const Command& comm)
{
	const uint8_t* ptr = comm.GetBytesPointer();

	cout << "El vector de bytes obtenido es el siguiente:" << endl;
	for(unsigned int i=0; i<comm.GetNumOfBytes(); i++)
	{
		printf("\t0x%02X",ptr[i]);
	}
	cout << endl;
}

int main()
{
	Command comm(RBW_INDEX);

	cout << "Configuracion de un comando VERIFY" << endl;
	comm.SetAs(Command::VERIFY);
	PrintBytes(comm);
	comm.Clear();

	cout << "\nConfiguracion de un comando LOGOUT" << endl;
	comm.SetAs(Command::LOGOUT);
	PrintBytes(comm);
	comm.Clear();

	for (char j=Command::GETSTPVAR; j<=Command::SETSTPVAR; j++){

		for (uint8_t i=0x01; i<=0x0F; i++){
			cout << "\nConfiguracion de un comando 0x";
			printf("%02X", j);
			cout << " para configurar variable 0x";
			printf("%02X", i);
			cout << endl;
			comm.SetAs(Command::CommandType(j), VarName(i), 1.0e6);
			PrintBytes(comm);
			comm.Clear();
		}

		for (uint8_t i=0x1E; i<=0x1F; i++){
			cout << "\nConfiguracion de un comando 0x";
			printf("%02X", j);
			cout << " para configurar variable 0x";
			printf("%02X", i);
			cout << endl;
			comm.SetAs(Command::CommandType(j), VarName(i), 2.0e6);
			PrintBytes(comm);
			comm.Clear();
		}

		for (uint8_t i=0x10; i<=0x13; i++){
			cout << "\nConfiguracion de un comando 0x";
			printf("%02X", j);
			cout << " para configurar variable 0x";
			printf("%02X", i);
			cout << endl;
			comm.SetAs(Command::CommandType(j), VarName(i), 3.0e6);
			PrintBytes(comm);
			comm.Clear();
		}

		for (uint8_t i=0x20; i<=0x23; i++){
			cout << "\nConfiguracion de un comando 0x";
			printf("%02X", j);
			cout << " para configurar variable 0x";
			printf("%02X", i);
			cout << endl;
			comm.SetAs(Command::CommandType(j), VarName(i), 4.0e6);
			PrintBytes(comm);
			comm.Clear();
		}

		for (uint8_t i=0x30; i<=0x32; i++){
			cout << "\nConfiguracion de un comando 0x";
			printf("%02X", j);
			cout << " para configurar variable 0x";
			printf("%02X", i);
			cout << endl;
			comm.SetAs(Command::CommandType(j), VarName(i), 5.0e6);
			PrintBytes(comm);
			comm.Clear();
		}

		for (uint8_t i=0x41; i<=0x49; i++){
			cout << "\nConfiguracion de un comando 0x";
			printf("%02X", j);
			cout << " para configurar variable 0x";
			printf("%02X", i);
			cout << endl;
			comm.SetAs(Command::CommandType(j), VarName(i), 6.0e6);
			PrintBytes(comm);
			comm.Clear();
		}

		for (uint8_t i=0x60; i<=0x61; i++){
			cout << "\nConfiguracion de un comando 0x";
			printf("%02X", j);
			cout << " para configurar variable 0x";
			printf("%02X", i);
			cout << endl;
			comm.SetAs(Command::CommandType(j), VarName(i), 7.0e6);
			PrintBytes(comm);
			comm.Clear();
		}

		for (uint8_t i=0x80; i<=0x86; i++){
			cout << "\nConfiguracion de un comando 0x";
			printf("%02X", j);
			cout << " para configurar variable 0x";
			printf("%02X", i);
			cout << endl;
			comm.SetAs(Command::CommandType(j), VarName(i), 8.0e6);
			PrintBytes(comm);
			comm.Clear();
		}

		cout << "\nConfiguracion de un comando 0x";
		printf("%02X", j);
		cout << " para configurar variable 0x90" << endl;
		comm.SetAs(Command::CommandType(j), VarName::MAXPEAKPOW, 9.0e6);
		PrintBytes(comm);
		comm.Clear();

		cout << "\nConfiguracion de un comando 0x";
		printf("%02X", j);
		cout << " para configurar variable 0xC0" << endl;
		comm.SetAs(Command::CommandType(j), VarName::STDTONE, 10.0e6);
		PrintBytes(comm);
		comm.Clear();
	}

	return 0;
}
