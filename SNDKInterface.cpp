/*
 * SNDKInterface.cpp
 *
 *  Created on: 22/11/2018
 *      Author: new-mauro
 */
#include "SNDKInterface.h"

//Implementations of Command class' methods
Command::Command(){
	variable="";
	argument=-100; //a number which is not possible as argument based on the Spectran USB protocol.
	fields.clear();
}

Command::~Command(){
	fields.clear();
}

void Command::SetSpectranVariable(string var,string arg){
	fields.push_back("SPECTRAN");
	fields.push_back("CTRL");
	variable=var;
	argument=arg;
	fields.push_back(variable+' '+argument);
}

void Command::ToAuthenticate(){
	const string username="aaronia";
	const string method="AD4";
	const string password="5b8a605dc30d376b1faf7d3d817e0593161f88554af2ec9139b717c3fdae58a7";

	fields.push_back("AUTHENTICATION");
	fields.push_back(username+'&'+method+'&'+password);
}

void Command::ToServer(string comm){
	fields.push_back("SERVER");
	boost::to_upper(comm);
	fields.push_back(comm);
}

void Command::GetInfoSpectran(string comm){
	fields.push_back("SPECTRAN");
	fields.push_back("INFO");
	boost::to_upper(comm);
	fields.push_back(comm);
}

void Command::SetUpSpectranFcn(string fcn){
	fields.push_back("SPECTRAN");
	fields.push_back("CALC");
	boost::to_upper(fcn);
	fields.push_back(fcn);
}

string Command::GetString(){
	string aux=fields[0];
	for (unsigned int i=0;i<fields.size();i++){
		aux+=':'+fields[i];
	}
	aux+='\n';
	return aux;
}

string Command::GetField(unsigned int i){
	if (i<fields.size()){
		return fields[i];
	}else{
		//ERROR
		return "";
	}
}

Reply::Reply(){
	fields.clear();
	subfields.clear();
	subfieldSeparator='';
}

Reply::Reply(string rply){
	SetUpReply(rply);
}

Reply::~Reply(){
	fields.clear();
	subfields.clear();
}

void Reply::SetUpReply(string rply){
	istringstream iss(rply);
	string aux;

	if(rply.back()=='\n'){
		//Respusta completa. Se descarta el caracter final, '\n'
		rply.pop_back();

		//Separacion de los campos de la respuesta
		getline(iss,aux,':');
		fields.push_back(aux);

		if(fields[0]=="DEVICE_SETUP"){
			getline(iss,aux,':');
			fields.push_back(aux);
			getline(iss,aux);
			fields.push_back(aux);
		}else{
			while ( !iss.eof() && !iss.fail() ){
				getline(iss,aux,':');
				fields.push_back(aux);
			}
		}

		//Separacion de los subcampos del campo correspondiente (solo es valido para ciertas respuestas)
		unsigned int i=1;
		bool flagFound;

		//Se busca en cada campo de la respuesta los posibles separados de subcampos
		while( i<fields.size() ){

			flagFound=fields[i].find('|')!=string::npos;
			if( flagFound ){	subfieldSeparator='|'; break;	}

			flagFound=fields[i].find(',')!=string::npos;
			if( flagFound ){	subfieldSeparator=','; break;	}

			flagFound=fields[i].find(';')!=string::npos;
			if( flagFound ){	subfieldSeparator=';'; break;	}

			flagFound=fields[i].find('$')!=string::npos;
			if( flagFound ){	subfieldSeparator='$'; break;	}

			i++;
		}

		//Si se encontro algun campo que tiene subcampos, los mismos son extraidos y almacenados en forma separada
		if(flagFound==true){
			istringstream iss(fields[i]);
			string aux;

			while( !iss.eof() && !iss.fail() ){
				getline(iss,aux,subfieldSeparator);
				subfields.push_back(aux);
			}
		}
	}else{
		//Error
		cerr << "Error: se recibio una respuesta incompleta.\n";
		cerr << '\t' << rply;
	}
}

string Reply::GetString(){
	string aux=fields[0];

	for(unsigned int i=0;i<fields.size();i++){
		aux+=':'+fields[i];
	}
	aux+='\n';
	return aux;
}

string Reply::GetField(unsigned int i){
	if(i<fields.size()){
		return fields[i];
	}else{
		//ERROR
		return "";
	}
}

string Reply::GetSubfield(unsigned int i){
	if(i<subfields.size()){
		return subfields[i];
	}else{
		//ERROR
		return "";
	}
}

ASWEEPReply::ASWEEPReply(){
	Reply::Reply();
	valueSeparator='';
	powerValues.clear();
	freqValues.clear();
}

ASWEEPReply::ASWEEPReply(string str){
	ASWEEPReply::SetUpReply(str);
}

ASWEEPReply::~ASWEEPReply(){
	Reply::~Reply();
	powerValues.clear();
	freqValues.clear();
}

void ASWEEPReply::SetUpReply(string str){

	Reply::SetUpReply(str);

	string powerSubfield = Reply::GetSubfield(2);
	string freqSubfield = Reply::GetSubfield(3);
	istringstream iss1(powerSubfield);
	istringstream iss2(freqSubfield);
	string aux;

	if( powerSubfield.size()==freqSubfield.size() ){

		while( !iss1.eof() && !iss1.fail() ){
			getline(iss1,aux,'#');
			powerValues.push_back(aux);
		}

		while( !iss2.eof() && !iss2.fail() ){
			getline(iss2,aux,'#');
			freqValues.push_back(aux);
		}
	}else{
		//Error
		cerr << "Error: se obtuvo un barrido incompleto.\n";
	}
}
