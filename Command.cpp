/*
 * Command.cpp
 *
 *  Created on: 26/02/2019
 *      Author: new-mauro
 */

#include "Spectran.h"

//////////////////////Implementations of some Command class' methods//////////////////////////

//! Default constructor
/*! When this constructor is used, the programmer must provide the command data with the method
 * `SetAs(commType,variable,value)`, and even with the method `SetParameters(variable,value)`. Also, the programmer
 * must set the internal pointers with the method `SetPointers`. The internal pointers are used to get info about the
 * variables' IDs and the RBW's indexes.
 */
Command::Command()
{
	commandType=UNINITIALIZED;
	variableName=SpecVariable::UNINITIALIZED;
	value=0.0;
}

//! The most complete constructor which allows to set the internal pointers and optionally the command type.
/*! If this constructor is used providing both parameters, then the method `SetParamerts` should be used to set the
 * variable name (GETSTPVAR and SETSTPVAR commands) and its value (just SETSTPVAR command). If it used providing just
 * the first parameter, then the method `SetAs` should be used to set command type, variable name and value.
 */
Command::Command(CommandType type, SpecVariable variable, float val)
{
	commandType=type;
	variableName=variable;
	value=val;
	FillBytesVector();
}

//! The copy constructor
Command::Command(const Command& anotherComm)
{
	bytes=anotherComm.bytes;
	commandType=anotherComm.commandType;
	variableName=anotherComm.variableName;
	value=anotherComm.value;
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
	std::uint8_t var_id;
	FloatToBytes floatBytes;

	switch(commandType)
	{
	case Command::VERIFY:
		bytes.reserve(5);
		bytes.push_back( std::uint8_t(Command::VERIFY) );
		bytes.push_back(0xA5);
		bytes.push_back(0x5A);
		bytes.push_back(0xF1);
		bytes.push_back(0x1F);
		break;
	case Command::LOGOUT:
		bytes.push_back( std::uint8_t(Command::LOGOUT) );
		break;
	case Command::GETSTPVAR:
		var_id = std::uint8_t(variableName);
		bytes.reserve(3);
		bytes.push_back( std::uint8_t(Command::GETSTPVAR) );
		bytes.push_back(var_id);
		bytes.push_back(0);
		break;
	case Command::SETSTPVAR:
		var_id = std::uint8_t(variableName);
		if( variableName==SpecVariable::RESBANDW || variableName==SpecVariable::VIDBANDW )
		{
			floatBytes.floatValue = RBW_INDEX.left.at(value); //The given value is the actual frequency value but the
															//corresponding RBW (and VBW) index must be sent, so here
															//this conversion is made
		}else if ( variableName==SpecVariable::STARTFREQ || variableName==SpecVariable::STOPFREQ ||
				variableName==SpecVariable::CENTERFREQ || variableName==SpecVariable::SPANFREQ )
		{
			floatBytes.floatValue=value/1.0e6; //The given value is in Hz but it must be sent in MHz, so it is divided by
												//one million (1e6) to change it.
		}else
		{
			floatBytes.floatValue=value;
		}
		bytes.reserve(7);
		bytes.push_back( std::uint8_t(Command::SETSTPVAR) );
		bytes.push_back(var_id);
		bytes.push_back(0);
		bytes.push_back(floatBytes.bytes[0]);
		bytes.push_back(floatBytes.bytes[1]);
		bytes.push_back(floatBytes.bytes[2]);
		bytes.push_back(floatBytes.bytes[3]);
		break;
	default:
		//A LOGOUT command is built by default
		bytes.push_back( std::uint8_t(Command::LOGOUT) );
		break;
	}
}

//! This method is intended to provide to the object the enough data so this could configure itself to be ready to be sent.
void Command::SetAs(CommandType commType, SpecVariable variable, float val)
{
	commandType=commType;
	variableName=variable;
	value=val;
	FillBytesVector();
}

//! This method is intended to set the command's parameters, so it should be used when the command type has already been set.
void Command::SetParameters(SpecVariable variable, float val)
{
	variableName=variable;
	value=val;
	FillBytesVector();
}

//! A method which returns the command type as a std::string.
std::string Command::GetCommTypeString() const
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

//! A method which returns the name, as a std::string, of the Spectran's variable which is related with the command (GETSTPVAR and SETSTPVAR commands)
std::string Command::GetVariableNameString() const
{
	switch(variableName)
	{
	case SpecVariable::ANTGAIN:
		return "antenna gain";
	case SpecVariable::ANTTYPE:
		return "antenna type";
	case SpecVariable::ATTENFAC:
		return "attenuator factor";
	case SpecVariable::BACKBBEN:
		return "background BB detector";
	case SpecVariable::CABLETYPE:
		return "cable type";
	case SpecVariable::CENTERFREQ:
		return "center frequency";
	case SpecVariable::DEMODMODE:
		return "demodulator mode";
	case SpecVariable::DETMODE:
		return "detector mode";
	case SpecVariable::DISPDIS:
		return "display update";
	case SpecVariable::DISPRANGE:
		return "display range";
	case SpecVariable::DISPUNIT:
		return "display unit";
	case SpecVariable::LEVELTONE:
		return "peak level audio tone";
	case SpecVariable::LOGFILEID:
		return "log file id";
	case SpecVariable::LOGSAMPCNT:
		return "log samples count";
	case SpecVariable::LOGTIMEIVL:
		return "log time interval";
	case SpecVariable::MARKCOUNT:
		return "marker count";
	case SpecVariable::MARKMINPK:
		return "marker minimum peak";
	case SpecVariable::MAXPEAKPOW:
		return "global maximal peak power";
	case SpecVariable::PEAK1FREQ:
		return "peak 1 frequency";
	case SpecVariable::PEAK1POW:
		return "peak 1 power";
	case SpecVariable::PEAK2FREQ:
		return "peak 2 frequency";
	case SpecVariable::PEAK2POW:
		return "peak 2 power";
	case SpecVariable::PEAK3FREQ:
		return "peak 3 frequency";
	case SpecVariable::PEAK3POW:
		return "peak 3 power";
	case SpecVariable::PEAKDISP:
		return "peak display mode";
	case SpecVariable::PREAMPEN:
		return "internal preamplifier enabling";
	case SpecVariable::RBWFSTEP:
		return "rbw frequency (read only)";
	case SpecVariable::RDOUTIDX:
		return "vertical line marker";
	case SpecVariable::RECVCONF:
		return "receiver configuration";
	case SpecVariable::REFLEVEL:
		return "reference level";
	case SpecVariable::REFOFFS:
		return "manual calibration value";
	case SpecVariable::RESBANDW:
		return "rbw";
	case SpecVariable::SPANFREQ:
		return "span";
	case SpecVariable::SPECDISP:
		return "spectrum display mode";
	case SpecVariable::SPECPROC:
		return "spectrum processing mode";
	case SpecVariable::SPKVOLUME:
		return "speaker volume";
	case SpecVariable::STARTFREQ:
		return "start frequency";
	case SpecVariable::STDTONE:
		return "output standard speaker tone";
	case SpecVariable::STOPFREQ:
		return "stop frequency";
	case SpecVariable::SWEEPTIME:
		return "sweep time";
	case SpecVariable::SWPDLYACC:
		return "sweep delay for accuracy mode";
	case SpecVariable::SWPFRQPTS:
		return "sweep frequency points";
	case SpecVariable::USBMEAS:
		return "measurements data to USB enabling";
	case SpecVariable::USBRUNPROG:
		return "run spectran program";
	case SpecVariable::USBSWPID:
		return "usb sweep id request";
	case SpecVariable::USBSWPRST:
		return "reset current sweep";
	case SpecVariable::VIDBANDW:
		return "vbw";
	default:
		return "unknown";
	}
}

//! This method allows to reset the object, cleaning the bytes vector, command type, variable name and value.
void Command::Clear()
{
	bytes.clear();
	commandType=UNINITIALIZED;
	variableName=SpecVariable::UNINITIALIZED;
	value=0.0;
}

//! The overloading of the assignment operator.
const Command& Command::operator=(const Command& anotherComm)
{
	bytes=anotherComm.bytes;
	commandType=anotherComm.commandType;
	variableName=anotherComm.variableName;
	value=anotherComm.value;
	return *this;
}
