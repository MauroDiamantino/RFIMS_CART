/*
 * TestBenchComm&Replies.cpp
 *
 *  Created on: 22/11/2018
 *      Author: new-mauro
 */
#include "SNDKInterface.h"


int main(){
	string replyStr="AINFO:921.0 MHz;-51.8 dBm;05.11.2014 15:23:54;05.11.2014 16:21:11\n";
	string asweepStr="AINFO:12-29-00.621 10.11.2014$12-29-00.621 10.11.2014$-115.244#-117.947#-116.362#-115.634$860 MHz#861.5 MHz#977 MHz#978.5 MHz\n";
	Command comm;
	Reply rep(replyStr);
	ASWEEPReply sweep;
	string aux;
	unsigned int n;

	cout << "Test Bench de las clases Command y Reply, y la subclase ASWEEPReply\n\n";

	//Test Command
	comm.SetSpectranVariable("STOPFRQ","900"); //Stop freq=900MHz
	cout << "\nSe configuro un comando para configurar la variable STOPFRQ con el valor 900 MHz\n";
	aux=comm.GetString();
	cout << "El string del comando es el siguiente:\n" << aux << '\n';
	cout << "Los campos del comando son los siguientes:\n";
	n=comm.GetNumOfFields();
	for (unsigned int i=0;i<n;i++){
		cout << comm.GetField(i) << '\n';
	}

	//Test ASWEEPReply
	sweep.SetUpReply(asweepStr);
	cout << "\nSe configuro un objeto ASWEEPReply con una respuesta de ese tipo.\n";
	cout << "El string de la respuesta es el siguiente\n:" << sweep.GetString() << '\n';
	cout << "Los campos de la respuesta son los siguientes:\n";
	n=sweep.GetNumOfFields();
	for(unsigned int i=0;i<n;i++){
		cout << sweep.GetField(i) << '\n';
	}
	cout << "Los subcampos del campo correspondiente (2ro) son los siguientes:\n";
	n=sweep.GetNumOfSubFields();
	for(unsigned int i=0;i<n;i++){
		cout << sweep.GetSubfield(i) << '\n';
	}
	cout << "Los valores del barrido son los siguientes:\n";
	vector<string> aux_vector;
	n=sweep.GetNumOfSweepValues();
	aux_vector=sweep.GetFreqValues();
	for(unsigned int i=0;i<n;i++){
		cout << aux_vector << '\t';
	}
	cout << '\n';
	aux_vector=sweep.GetPowerValues();
	for(unsigned int i=0;i<n;i++){
		cout << aux_vector << '\t';
	}

	return 0;
}
