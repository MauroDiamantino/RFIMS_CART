/*
 * SpectranInterface.cpp
 *
 *  Created on: 26/02/2019
 *      Author: new-mauro
 */
#include "SpectranInterface.h"

//! Default constructor
/*! When this constructor is used, the programmer must provide the command data with the method
 * `SetAs(commType,varName,value)`, and even with the method `SetParameters(varName,value)`. Also, the programmer
 * must set the internal pointers with the method `SetPointers`. The internal pointers are used to get info about the
 * variables' IDs and the RBW's indexes.
 */
Command::Command()
{
	value=0.0;
	commandType=UNINITIALIZED;
	variableName=VarName::UNINITIALIZED;
	RBW_INDEX=nullptr;
}

//! A constructor which allows to determine the command type.
/*! When this constructor is used, the programmer must provide the command data with the method
 * `SetAs(commType,varName,value)`, and even with the method `SetParameters(varName,value)`. Also, the programmer
 * must set the internal pointers with the method `SetPointers`. The internal pointers are used to get info about the
 * variables' IDs and the RBW's indexes.
 */
Command::Command(CommandType type)
{
	value=0.0;
	commandType=type;
	variableName=VarName::UNINITIALIZED;
	RBW_INDEX=nullptr;
}

//! A constructor which allows to set the internal pointers.
/*! The internal pointers are used to get info about the variables' IDs and the RBW's indexes.
 * Again, the methods `SetAs()` and/or `SetParameters()` must be used to make the command ready to be sent.
 */
Command::Command(const unordered_map<float,float>& rbw_ind) : RBW_INDEX(&rbw_ind)
{
	value=0.0;
	commandType=UNINITIALIZED;
	variableName=VarName::UNINITIALIZED;
}

//! The most complete constructor which allows to set the internal pointers and the command type.
Command::Command(const unordered_map<float,float>& rbw_ind,	CommandType type) : RBW_INDEX(&rbw_ind)
{
	commandType=type;
	variableName=VarName::UNINITIALIZED;
	value=0.0;
}

//! The Command class destructor.
/*! The destructor just call the method `Clear()`
 */
Command::~Command()
{
	Clear();
}

//! This method build the bytes vector when the enough data have been given.
/*! The method *FillBytesVector* is the heart of this class because this perform the main task of the class, to correctly
 * build the bytes vector with the data given: command type, variable and value. Theses two last data are not taken into
 * for some commands.
 *
 * Once the command object is complete, it is passed to the Spectran Interface which take the command, extract the bytes
 * vector and send it to the spectrum analyzer.
 */
void Command::FillBytesVector()
{
	uint8_t var_id;
	FloatToBytes floatBytes;

	bytes.clear();

	switch(commandType)
	{
	case Command::VERIFY:
		bytes.push_back(0x01);
		bytes.push_back(0xA5);
		bytes.push_back(0x5A);
		bytes.push_back(0xF1);
		bytes.push_back(0x1F);
		break;
	case Command::LOGOUT:
		bytes.push_back(0x02);
		break;
	case Command::GETSTPVAR:
		var_id = uint8_t(variableName);
		bytes.push_back(0x20);
		bytes.push_back(var_id);
		bytes.push_back(0);
		break;
	case Command::SETSTPVAR:
		var_id = uint8_t(variableName);
		if(variableName==VarName::RESBANDW || variableName==VarName::VIDBANDW){
			floatBytes.floatValue = RBW_INDEX->at(value);
		}else if (variableName==VarName::STARTFREQ || variableName==VarName::STOPFREQ ||
				variableName==VarName::CENTERFREQ || variableName==VarName::SPANFREQ){
			floatBytes.floatValue=value/1.0e6;
		}else{
			floatBytes.floatValue=value;
		}
		bytes.push_back(0x21);
		bytes.push_back(var_id);
		bytes.push_back(0);
		bytes.push_back(floatBytes.bytes[0]);
		bytes.push_back(floatBytes.bytes[1]);
		bytes.push_back(floatBytes.bytes[2]);
		bytes.push_back(floatBytes.bytes[3]);
		break;
	default:
		string error="An error occurred when the object tried to build the bytes vector because the command type was wrong";
		throw(error);
	}
}

//! This method is intended to set the internal pointers when they have not been initialized in the constructor.
void Command::SetPointer(const unordered_map<float,float>& rbw_ind)
{
	RBW_INDEX = &rbw_ind;
}

//! This method is intended to provide to the object the enough data so this could configure itself to be ready to be sent.
void Command::SetAs(CommandType commType, VarName varName, float val)
{
	commandType=commType;
	variableName=varName;
	value=val;
	FillBytesVector();
}

//! This method is intended to set the command's parameters, so it should be used when the command type has already been set.
void Command::SetParameters(VarName varName, float val)
{
	variableName=varName;
	value=val;
	FillBytesVector();
}

string Command::GetCommTypeString() const
{
	switch(commandType)
	{
	case Command::VERIFY:
		return "VERIFY";
		break;
	case Command::LOGOUT:
		return "LOGOUT";
		break;
	case Command::GETSTPVAR:
		return "GETSTPVAR";
		break;
	case Command::SETSTPVAR:
		return "SETSTPVAR";
		break;
	default:
		return "UNINITIALIZED";
	}
}

//! A method to obtain the bytes vector like this is implemented internally, a `vector` container.
const vector<uint8_t>& Command::GetBytesVector() const
{
	switch(commandType)
	{
	case CommandType::VERIFY:
	case CommandType::LOGOUT:
	case CommandType::GETSTPVAR:
	case CommandType::SETSTPVAR:
		return bytes;
		break;
	default:
		string error="An error occurred when the bytes vector was required because the command type was wrong";
		throw(error);
	}
}

//! A method to obtain the bytes vector but like a C-style array.
/*! This method returns a pointer to `uint8_t` so this allows to access directly to the memory addresses where the
 * vector's bytes are stored. Because of the Spectran Interface works with C-style arrays, that object uses this method.
 */
const uint8_t* Command::GetBytesPointer() const
{
	switch(commandType)
	{
	case CommandType::VERIFY:
	case CommandType::LOGOUT:
	case CommandType::GETSTPVAR:
	case CommandType::SETSTPVAR:
		return bytes.data();
		break;
	default:
		string error="An error occurred when a pointer to the bytes was required because the command type was wrong";
		throw(error);
	}
}

//! This method allows to reset the object, cleaning the bytes vector, command type, variable name and value.
void Command::Clear()
{
	bytes.clear();
	commandType=UNINITIALIZED;
	variableName=VarName::UNINITIALIZED;
	value=0.0;
}

//! Overloading of the assignment operator.
const Command& Command::operator=(const Command& comm)
{
	bytes=comm.bytes;
	commandType=comm.commandType;
	variableName=comm.variableName;
	value=comm.value;
	return *this;
}
