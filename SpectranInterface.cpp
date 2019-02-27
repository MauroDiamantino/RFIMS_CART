/*
 * SpectranInterface.cpp
 *
 *  Created on: 26/02/2019
 *      Author: new-mauro
 */
#include "SpectranInterface.h"

Command::Command(const unordered_map<string,unsigned int>& var_id,
		const unordered_map<float,float>& rbw_ind):VARIABLE_ID(*var_id),RBW_INDEX(*rbw_ind)
{
	value=0.0;
	cout << "An object of class Command has been constructed" << endl;
}

Command::Command(const unordered_map<string,unsigned int>& var_id,
		const unordered_map<float,float>& rbw_ind,
		string& commType):VARIABLE_ID(var_id),RBW_INDEX(rbw_ind)
{
	commandType=commType;
	value=0.0;
	cout << "An object of class Command has been constructed" << endl;
}

Command::~Command()
{
	Clear();
	cout << "An object of class Command has been destroyed" << endl;
}

void Command::FillBytesVector()
{
	uint8_t var_id;
	union FloatToBytes{
		float floatValue;
		uint8_t bytes[4];
	}floatBytes;

	bytes.clear();

	switch(commandType)
	{
	case "Verify":
		bytes.push_back(0x01);
		bytes.push_back(0xA5);
		bytes.push_back(0x5A);
		bytes.push_back(0xF1);
		bytes.push_back(0x1F);
		break;
	case "LogOut":
		bytes.push_back(0x02);
		break;
	case "GetSTPVar":
		var_id = VARIABLE_ID[variableName];
		bytes.push_back(0x20);
		bytes.push_back(var_id);
		bytes.push_back(0);
		break;
	case "SetSTPVar":
		var_id = VARIABLE_ID[variableName];
		floatBytes.floatValue=value;
		bytes.push_back(0x21);
		bytes.push_back(var_id);
		bytes.push_back(0);
		bytes.push_back(floatBytes.bytes[0]);
		bytes.push_back(floatBytes.bytes[1]);
		bytes.push_back(floatBytes.bytes[2]);
		bytes.push_back(floatBytes.bytes[3]);
		break;
	default:
		bytes.push_back(0x02);
	}
}

void Command::SetAs(string commType, string varName, float val=0.0)
{
	commandType=commType;
	variableName=varName;
	value=val;

	FillBytesVector();
}

void Command::SetParameters(string varName, float val=0.0)
{
	variableName=varName;
	value=val;

	FillBytesVector();
}

void Command::Clear()
{
	bytes.clear();
	commandType.clear();
	variableName.clear();
	value=0.0;
}

Command& Command::operator=(Command& comm)
{
	bytes=comm.GetBytesVector();
	commandType=comm.GetCommandType();
	variableName=comm.GetVariableName();
	value=comm.GetValue();
	return *this;
}
