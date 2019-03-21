/*
 * Reply.cpp
 *
 *  Created on: 03/03/2019
 *      Author: new-mauro
 */

#include "SpectranInterface.h"

////////////////////Implementations of some Reply class' methods//////////////////

//! Default constructor
Reply::Reply()
{
	replyType=Reply::UNINITIALIZED;
	variableName=VarName::UNINITIALIZED;
	value=0.0;
	numOfWaitedBytes=0;
}

//! A constructor which allows to set the reply type.
Reply::Reply(ReplyType type, VarName varName)
{
	replyType=type;
	variableName=varName;
	value=0.0;
	PrepareReply();
}

//! The copy constructor
Reply::Reply(const Reply& anotherReply)
{
	bytes=anotherReply.bytes;
	replyType=anotherReply.replyType;
	numOfWaitedBytes=anotherReply.numOfWaitedBytes;
	variableName=anotherReply.variableName;
	value=anotherReply.value;
}

//! This method is intended to prepare the object to receive the bytes sent by the spectrum analyzer.
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

//! This method must be used to set the reply type and to ask the object to prepare itself to receive the bytes.
/*! The tasks performed by this methods are: to clear the object, to set the reply type and then to call the
 * private method `PrepareReply()`. The variable name must be set for the GETSTPVAR replies, for other cases it
 * is not important.
 */
void Reply::PrepareTo(ReplyType type, VarName varName)
{
	replyType=type;
	variableName=varName;
	PrepareReply();
}

void Reply::FillBytesVector(uint8_t * data)
{
	bytes.insert(bytes.begin(), data, data+numOfWaitedBytes);
}

//! The bytes vector must be inserted in the Reply object with this method to extract the reply data then.
/*! The reply object must have been prepared before inserting the bytes vector.
 */
void Reply::InsertBytes(uint8_t* data)
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

		if( variableName==VarName::RESBANDW || variableName==VarName::VIDBANDW )
		{
			value = RBW_INDEX.right.at( floatBytes.floatValue ); //The obtained value is the RBW index so it is changed
																//for the actual frequency value in Hz
		}else if ( variableName==VarName::STARTFREQ || variableName==VarName::STOPFREQ ||
				variableName==VarName::CENTERFREQ || variableName==VarName::SPANFREQ )
		{
			value = floatBytes.floatValue*1.0e6; //The measurement unit is converted from MHz to Hz
		}else
		{
			value = floatBytes.floatValue;
		}
		break;
	case ReplyType::AMPFREQDAT:
		floatBytes.bytes[0] = bytes.at(9);
		floatBytes.bytes[1] = bytes.at(10);
		floatBytes.bytes[2] = bytes.at(11);
		floatBytes.bytes[3] = bytes.at(12);
		value = floatBytes.floatValue;
		break;
	default:
		value=0.0;
	}
}

//! A method which returns the reply type but as a string.
string Reply::GetReplyTypeString() const
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

//! This method states if the received reply is right.
/*! To do so, the method checks if the reply type (determined when the reply object was prepared) is equal to first
 * received byte and then, if the reply type is VERIFY it checks if the following bytes are correct, or if the reply
 * type is GETSTPVAR or SETSTPVAR it checks the "status" byte, which must be zero when all is right.
 */
bool Reply::IsRight() const
{
	bool flagRight;

	switch(replyType){
	case ReplyType::VERIFY:
		if ( bytes[0]==ReplyType::VERIFY && bytes[1]==0x51 && bytes[2]==0x1A
				&& bytes[3]==0xF5 && bytes[4]==0xAF ){
			flagRight=true;
		}else{
			flagRight=false;
		}
		break;
	case ReplyType::GETSTPVAR:
		if( bytes[0]==ReplyType::GETSTPVAR && bytes[1]==0){
			flagRight=true;
		}else{
			flagRight=false;
		}
		break;
	case ReplyType::SETSTPVAR:
		if( bytes[0]==ReplyType::SETSTPVAR && bytes[1]==0){
			flagRight=true;
		}else{
			flagRight=false;
		}
		break;
	case ReplyType::AMPFREQDAT:
		if( bytes[0]==ReplyType::AMPFREQDAT ){
			flagRight=true;
		}else{
			flagRight=false;
		}
		break;
	default:
		//! By default the method indicates that the reply is incorrect.
		flagRight=false;
	}

	return flagRight;
}

//! The method to reset the object.
void Reply::Clear()
{
	replyType=ReplyType::UNINITIALIZED;
	numOfWaitedBytes=0;
	variableName=VarName::UNINITIALIZED;
	bytes.clear();
	value=0.0;
}

//! The overloading of the assignment operator.
const Reply& Reply::operator =(const Reply& anotherReply)
{
	bytes=anotherReply.bytes;
	replyType=anotherReply.replyType;
	numOfWaitedBytes=anotherReply.numOfWaitedBytes;
	value=anotherReply.value;
	return *this;
}


/////////////////////Implementation of some SweepRely class' methods/////////////////

SweepReply::SweepReply() : Reply(Reply::AMPFREQDAT)
{
	timestamp=0;
	frequency=0.0;
	minValue=0.0;
	maxValue=0.0;
}

SweepReply::SweepReply(uint8_t* bytesPtr) : Reply(Reply::AMPFREQDAT)
{
	SweepReply::InsertBytes(bytesPtr);
}

void SweepReply::InsertBytes(uint8_t * data)
{
	union UnIntToBytes{
		unsigned int intValue;
		uint8_t bytes[4];
	}unsIntBytes;

	FloatToBytes floatBytes;

	FillBytesVector(data);

	/*! Timestamp extraction: this value is received as a 4-bytes unsigned integer and it represents the count of Spectran's
	 * an internal timer. The timer period is approximately 3.5 nS and it is a 32-bit timer.
	 */
	unsIntBytes.bytes[0]=bytes.at(1);
	unsIntBytes.bytes[1]=bytes.at(2);
	unsIntBytes.bytes[2]=bytes.at(3);
	unsIntBytes.bytes[3]=bytes.at(4);
	timestamp=unsIntBytes.intValue;

	/*! Frequency extraction: this value is received as a 4-bytes unsigned integer in Hz/10. */
	unsIntBytes.bytes[0]=bytes.at(5);
	unsIntBytes.bytes[1]=bytes.at(6);
	unsIntBytes.bytes[2]=bytes.at(7);
	unsIntBytes.bytes[3]=bytes.at(8);
	frequency=float(unsIntBytes.intValue*10);

	/*! Min power extraction: this value is received as a 4-bytes floating point value, measured in dBm. */
	floatBytes.bytes[0]=bytes.at(9);
	floatBytes.bytes[1]=bytes.at(10);
	floatBytes.bytes[2]=bytes.at(11);
	floatBytes.bytes[3]=bytes.at(12);
	value=minValue=floatBytes.floatValue;

	/*! Max power extraction: this value is received as a 4-bytes floating point value, measured in dBm. */
	floatBytes.bytes[0]=bytes.at(13);
	floatBytes.bytes[1]=bytes.at(14);
	floatBytes.bytes[2]=bytes.at(15);
	floatBytes.bytes[3]=bytes.at(16);
	maxValue=floatBytes.floatValue;
}

void SweepReply::Clear()
{
	bytes.clear();
	value=0.0;
	timestamp=0.0;
	frequency=0.0;
	minValue=0.0;
	maxValue=0.0;
}

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
