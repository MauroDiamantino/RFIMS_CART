/*! \file Reply.cpp
 * 	\brief This file contains the definitions of several methods of the classes _Reply_ and _SweepReply_.
 * 	\author Mauro Diamantino
 */

#include "Spectran.h"

////////////////////Implementations of some Reply class' methods//////////////////

Reply::Reply()
{
	replyType=Reply::UNINITIALIZED;
	variableName=SpecVariable::UNINITIALIZED;
	value=0.0;
	numOfWaitedBytes=0;
}

/*! \param [in] type The reply type: VERIFY, GETSTPVAR, SETSTPVAR or AMPFREQDAT.
 * 	\param [in] variable An optional argument which indicates the variable whose value will be received by a GETSTPVAR reply.
 */
Reply::Reply(const ReplyType type, const SpecVariable variable)
{
	replyType=type;
	variableName=variable;
	value=0.0;
	PrepareReply();
}

/*! \param [in] anotherReply A _Reply_ object which is given to copy its attributes.
 */
Reply::Reply(const Reply& anotherReply)
{
	bytes=anotherReply.bytes;
	replyType=anotherReply.replyType;
	numOfWaitedBytes=anotherReply.numOfWaitedBytes;
	variableName=anotherReply.variableName;
	value=anotherReply.value;
}

/*! To prepare the object this method determines the number of waited bytes taking into account the given reply type
 * and then it asks the vector to reserve the enough memory to save those bytes. Moreover, once the object has been prepared
 * and passed to the Spectran Interface, the number of waited bytes is used by the interface to determine how many bytes
 * it must read. This method is private so it is used internally.
 */
void Reply::PrepareReply()
{
	switch(replyType){
	case ReplyType::VERIFY:
		numOfWaitedBytes=5;
		break;
	case ReplyType::GETSTPVAR:
		numOfWaitedBytes=6;
		break;
	case ReplyType::SETSTPVAR:
		numOfWaitedBytes=2;
		break;
	case ReplyType::AMPFREQDAT:
		numOfWaitedBytes=17;
		break;
	default:
		numOfWaitedBytes=0;
	}

	bytes.reserve(numOfWaitedBytes);
}

/*! The tasks performed by this methods are: to clear the object, to set the reply type and then to call the
 * private method `PrepareReply()`. The variable name must be set for the GETSTPVAR replies, for other cases it
 * is not important.
 * \param [in] type The reply type: VERIFY, GETSTPVAR, SETSTPVAR or AMPFREQDAT.
 * \param [in] variable An optional argument which indicates the variable whose value will be received by a GETSTPVAR reply.
 */
void Reply::PrepareTo(const ReplyType type, const SpecVariable variable)
{
	replyType=type;
	variableName=variable;
	PrepareReply();
}

/*!	\param [in] data A pointer to the bytes which contain the information of the reply sent by the spectrum analyzer.
 */
void Reply::FillBytesVector(const std::uint8_t * data)
{
	bytes.insert(bytes.begin(), data, data+numOfWaitedBytes);
}

/*! The reply object must have been prepared before inserting the bytes vector.
 * 	\param [in] data A pointer to the bytes which must be inserted in the object and from which the info must be extracted.
 */
void Reply::InsertBytes(const std::uint8_t* data)
{
	FloatToBytes floatBytes;

	FillBytesVector(data);

	switch(replyType)
	{
	case ReplyType::GETSTPVAR:
		floatBytes.bytes[0] = bytes.at(2);
		floatBytes.bytes[1] = bytes.at(3);
		floatBytes.bytes[2] = bytes.at(4);
		floatBytes.bytes[3] = bytes.at(5);

		if( variableName==SpecVariable::RESBANDW || variableName==SpecVariable::VIDBANDW )
		{
			value = RBW_INDEX.right.at( floatBytes.floatValue ); //The obtained value is the RBW index so it is changed
																//for the actual frequency value in Hz
		}else if ( variableName==SpecVariable::STARTFREQ || variableName==SpecVariable::STOPFREQ ||
				variableName==SpecVariable::CENTERFREQ || variableName==SpecVariable::SPANFREQ )
		{
			value = floatBytes.floatValue*1.0e6; //The measurement unit is converted from MHz to Hz
		}else
		{
			value = floatBytes.floatValue;
		}
		break;
	case ReplyType::AMPFREQDAT:
		//Power value, maximum value or RMS value. Even, it could be a voltage value or a field strength value.
		floatBytes.bytes[0] = bytes.at(13);
		floatBytes.bytes[1] = bytes.at(14);
		floatBytes.bytes[2] = bytes.at(15);
		floatBytes.bytes[3] = bytes.at(16);
		value = floatBytes.floatValue;
		break;
	default:
		value=0.0;
	}
}

std::string Reply::GetReplyTypeString() const
{
	switch(replyType){
	case ReplyType::VERIFY:
		return "VERIFY";
		break;
	case ReplyType::GETSTPVAR:
		return "GETSTPVAR";
		break;
	case ReplyType::SETSTPVAR:
		return "SETSTPVAR";
		break;
	case ReplyType::AMPFREQDAT:
		return "AMPFREQDAT";
		break;
	default:
		return "UNINITIALIZED";
	}
}

std::string Reply::GetVariableNameString() const
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

/*! To do so, the method checks if the reply type (determined when the reply object was prepared) is equal to first
 * 	received byte and if the reply type is VERIFY it checks if the following bytes are correct, or if the reply
 * 	type is GETSTPVAR or SETSTPVAR it checks the "status" byte, which must be zero when all is right.
 */
bool Reply::IsRight() const
{
	bool flagRight;

	switch(replyType)
	{
	case ReplyType::VERIFY:
		if ( bytes[0]==ReplyType::VERIFY && bytes[1]==0x51 && bytes[2]==0x1A
				&& bytes[3]==0xF5 && bytes[4]==0xAF )
			flagRight=true;
		else
			flagRight=false;
		break;
	case ReplyType::GETSTPVAR:
		if( bytes[0]==ReplyType::GETSTPVAR && bytes[1]==0)
			flagRight=true;
		else
			flagRight=false;
		break;
	case ReplyType::SETSTPVAR:
		if( bytes[0]==ReplyType::SETSTPVAR && bytes[1]==0)
			flagRight=true;
		else
			flagRight=false;
		break;
	case ReplyType::AMPFREQDAT:
		if( bytes[0]==ReplyType::AMPFREQDAT )
			flagRight=true;
		else
			flagRight=false;
		break;
	default:
		//! By default the method indicates that the reply is incorrect.
		flagRight=false;
	}

	return flagRight;
}

void Reply::Clear()
{
	replyType=ReplyType::UNINITIALIZED;
	numOfWaitedBytes=0;
	variableName=SpecVariable::UNINITIALIZED;
	bytes.clear();
	value=0.0;
}

/*!	\param [in] anotherReply A _Reply_ object which is given to copy its attributes.
 *
 */
const Reply& Reply::operator =(const Reply& anotherReply)
{
	bytes=anotherReply.bytes;
	replyType=anotherReply.replyType;
	numOfWaitedBytes=anotherReply.numOfWaitedBytes;
	value=anotherReply.value;
	return *this;
}


/////////////////////Implementation of some SweepRely class' methods/////////////////

/*! This constructor clears the class attributes and calls the _Reply_ constructor with an argument to
 * set the reply type to AMPFREQDAT.
 */
SweepReply::SweepReply() : Reply(Reply::AMPFREQDAT)
{
	timestamp=0;
	frequency=0;
	minValue=0.0;
	maxValue=0.0;
}

/*! This method extracts the timestamp, frequency value and power values of a AMPFREQDAT reply.
 * 	\param [in] data A pointer to the bytes which contains the timestamp and the frequency and power values.
 */
void SweepReply::InsertBytes(const std::uint8_t * data)
{
	union UnIntToBytes{
		unsigned int intValue;
		std::uint8_t bytes[4];
	}unsIntBytes;

	FloatToBytes floatBytes;

	FillBytesVector(data);

	/*! Timestamp extraction: this value is received as a 4-byte unsigned integer and it represents the count of a
	 *  internal timer of the spectrum analyzer. The timer period is approximately 3.5 nS and it is a 32-bit timer.
	 */
	unsIntBytes.bytes[0]=bytes.at(1);
	unsIntBytes.bytes[1]=bytes.at(2);
	unsIntBytes.bytes[2]=bytes.at(3);
	unsIntBytes.bytes[3]=bytes.at(4);
	timestamp=unsIntBytes.intValue;

	/*! Frequency extraction: this value is received as a 4-byte unsigned integer in Hz/10 and it is stored as an unsigned integer in Hz. */
	unsIntBytes.bytes[0]=bytes.at(5);
	unsIntBytes.bytes[1]=bytes.at(6);
	unsIntBytes.bytes[2]=bytes.at(7);
	unsIntBytes.bytes[3]=bytes.at(8);
	frequency = ( (std::uint_least64_t) unsIntBytes.intValue )*10;

	/*! Min/RMS power extraction: this value is received as a 4-byte floating point value, measured in dBm. Even, it could be a voltage value or a field strength value.*/
	floatBytes.bytes[0]=bytes.at(9);
	floatBytes.bytes[1]=bytes.at(10);
	floatBytes.bytes[2]=bytes.at(11);
	floatBytes.bytes[3]=bytes.at(12);
	minValue=floatBytes.floatValue;

	/*! Max/RMS power extraction: this value is received as a 4-bytes floating point value, measured in dBm. Even, it could be a voltage value or a field strength value.*/
	floatBytes.bytes[0]=bytes.at(13);
	floatBytes.bytes[1]=bytes.at(14);
	floatBytes.bytes[2]=bytes.at(15);
	floatBytes.bytes[3]=bytes.at(16);
	value=maxValue=floatBytes.floatValue;
}

void SweepReply::Clear()
{
	bytes.clear();
	value=0.0;
	timestamp=0.0;
	frequency=0;
	minValue=0.0;
	maxValue=0.0;
}

/*!	\param [in] anotherReply A _Reply_ object which is given to copy its attributes.
 */
const SweepReply& SweepReply::operator=(const SweepReply& anotherReply)
{
	bytes=anotherReply.bytes;
	value=anotherReply.value;
	timestamp=anotherReply.timestamp;
	frequency=anotherReply.frequency;
	minValue=anotherReply.minValue;
	maxValue=anotherReply.maxValue;
	return *this;
}
