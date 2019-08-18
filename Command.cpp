/*
 * Command.cpp
 *
 *  Created on: 26/02/2019
 *      Author: new-mauro
 */

/*! \file Command.cpp
 * 	\brief This file contains the definitions of several of the methods of the class _Command_.
 * 	\author Mauro Diamantino
 */

#include "Spectran.h"

//////////////////////Implementations of some Command class' methods//////////////////////////

/*! When this constructor is used, the programmer must provide the command data with the method
 * `SetAs(commType,variable,value)`, and even with the method `SetParameters(variable,value)`.
 */
Command::Command()
{
	commandType=UNINITIALIZED;
	variableName=SpecVariable::UNINITIALIZED;
	value=0.0;
}

/*! If this constructor is used just providing the first parameter, _command type_, then the method `SetParameters()` should be used
 * 	to set the variable name (for _GETSTPVAR_ and _SETSTPVAR_ commands) and the value that must be used to write it (just for _SETSTPVAR_
 * 	commands).
 * 	\param [in] type The command type: VERIFY, LOGOUT, GETSTPVAR or SETSTPVAR.
 * 	\param [in] variable An optional argument which determines which variable will be read or written.
 * 	\param [in] val An optional argument which represents the value which will be used to write the given variable.
 */
Command::Command(const CommandType type, const SpecVariable variable, const float val)
{
	commandType=type;
	variableName=variable;
	value=val;
	FillBytesVector();
}

/*! This method takes an object of the same class as argument, and it copies its attributes.
 * \param [in] anotherComm A _Command_ object given to copy its attributes.
 */
Command::Command(const Command& anotherComm)
{
	bytes=anotherComm.bytes;
	commandType=anotherComm.commandType;
	variableName=anotherComm.variableName;
	value=anotherComm.value;
}

/*! This method is the heart of this class because this perform its main task: to build the bytes vector with the given data, this is
 * the command type, variable name and value. Theses two last data are not taken into account for some commands.
 *
 * Once the command object is complete, it is passed to the _Spectran Interface_ which takes the command, extracts the bytes
 * vector and it sends this to the spectrum analyzer.
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
			floatBytes.floatValue = RBW_INDEX.left.at(value); //The given value is the actual frequency value, in Hertz, but the
															//corresponding RBW (and VBW) index must be sent, so here this conversion is made.
		}else if ( variableName==SpecVariable::STARTFREQ || variableName==SpecVariable::STOPFREQ ||
				variableName==SpecVariable::CENTERFREQ || variableName==SpecVariable::SPANFREQ )
		{
			floatBytes.floatValue=value/1e6; //The given value is in Hz but it must be sent in MHz, so it is divided by
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

/*! \param [in] commType The command type: VERIFY, LOGOUT, GETSTPVAR or SETSTPVAR.
 * 	\param [in] variable An optional argument which determines which variable will be read or written.
 * 	\param [in] val An optional argument which represents the value which will be used to write the given variable.
 */
void Command::SetAs(const CommandType commType, const SpecVariable variable, const float val)
{
	commandType=commType;
	variableName=variable;
	value=val;
	FillBytesVector();
}

/*! \param [in] variable The variable which will be read or written, with the commands GETSTPVAR and SETSTPVAR.
 * 	\param [in] val An optional argument which determines the value which will be used to write the given variable.
 */
void Command::SetParameters(const SpecVariable variable, const float val)
{
	variableName=variable;
	value=val;
	FillBytesVector();
}

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

void Command::Clear()
{
	bytes.clear();
	commandType=UNINITIALIZED;
	variableName=SpecVariable::UNINITIALIZED;
	value=0.0;
}

/*!	\param [in] anotherComm A _Command_ object given to copy its attributes.
 */
const Command& Command::operator=(const Command& anotherComm)
{
	bytes=anotherComm.bytes;
	commandType=anotherComm.commandType;
	variableName=anotherComm.variableName;
	value=anotherComm.value;
	return *this;
}
