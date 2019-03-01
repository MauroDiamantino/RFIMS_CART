/*
 * SpectranInterface.h
 *
 *  Created on: 26/02/2019
 *      Author: new-mauro
 */

/*! \file SpectranInterface.h
 *  \brief This header file contains the definition of the needed classes for the communication with a Aaronia
 *  Spectran HF-60105 V4 X spectrum analyzer.
 *
 *  This file includes the following libraries for the stated purposes:
 *  - iostream: cout, cin
 *  - vector: to use this container
 *  - string: to use this container
 *  - unordered_map: to use this container
 *
 *  The namespace *std* is used to simplify the uses of objects like cout and cin.
 */

#ifndef SPECTRANINTERFACE_H_
#define SPECTRANINTERFACE_H_

/////////////Libraries//////////////////
#include <iostream> //cout, cin
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

//////////////User-defined global types///////////
//! An enumeration which contains the names of all the environment variables of the Spectran HF-60105 V4 X spectrum analyzer.
enum VarName : uint8_t { STARTFREQ=0x01, STOPFREQ, RESBANDW, VIDBANDW, SWEEPTIME, ATTENFAC, REFLEVEL, DISPRANGE,
	DISPUNIT, DETMODE, DEMODMODE, SPECPROC, ANTTYPE, CABLETYPE, RECVCONF, CENTERFREQ=0x1E, SPANFREQ, PREAMPEN=0x10,
	SWPDLYACC, SWPFRQPTS, REFOFFS, USBMEAS=0x20, USBSWPRST, USBSWPID, USBRUNPROG, LOGFILEID=0x30, LOGSAMPCNT,
	LOGTIMEIVL,	SPECDISP=0x41, PEAKDISP, MARKMINPK, RDOUTIDX, MARKCOUNT, LEVELTONE, BACKBBEN, DISPDIS, SPKVOLUME,
	RBWFSTEP=0x60, ANTGAIN,	PEAK1POW=0x80, PEAK2POW, PEAK3POW, PEAK1FREQ=0x84, PEAK2FREQ, PEAK3FREQ, MAXPEAKPOW=0x90,
	STDTONE=0xC0, UNINITIALIZED };

///////////////Constants///////////////////////
const unordered_map<float,float> RBW_INDEX( {	{100e6, 0.0}, {3e6, 1.0}, {1e6, 2.0}, {300e3, 3.0}, {100e3, 4.0},
												{30e3, 5.0}, {10e3, 6.0}, {3e3, 7.0}, {1e3, 8.0}, {120e3, 100.0},
												{9e3, 101.0}, {200.0, 102.0}, {5e6, 103.0},	{200e3, 104},
												{1.5e6, 105.0} } );

/////////////////Classes/////////////////

//! This class builds the corresponding bytes array to send a certain command to a Aaronia Spectran V4 series spectrum analyzer.
/*! The "Command" class is intended to build the bytes array which will be sent a spectrum analyzer to perform
 * one of the following tasks:
 *	- initialize the communication with the device
 * 	- set an environment variable
 * 	- get the value of an environment variable
 * 	- close the communication
 * The user have to say to the object which "command type" he wants, which "variable" he wants to change/read,
 * which value must be used to set up the specified variable. When the object has the enough data, it will build
 * the bytes array which will be sent to the spectrum analyzer via the USB interface.
 * The objects of this class will be interchanged between the "Spectran configurator" and the "Spectran interface".
 */
class Command{
public:
	///////Public types///////////
	//! An enumeration which contains the commands types which can be sent to a Spectran HF-60105 V4 X spectrum analyzer.
	/*! This enumeration contains just four commands from the Spectran USB Protocol: *VERIFY*, *LOGOUT*, *GETSTPVAR*
	 *  and *SETSTPVAR*. The other commands are intended to modify or get information about the internal files of the
	 *  spectrum analyzer, and because that will not be used those commands are not added. There is an extra command
	 *  type which is *UNINITIALIZED* whose purpose is to state that the object is still incomplete.
	 */
	enum CommandType : char { VERIFY, LOGOUT, GETSTPVAR, SETSTPVAR, UNINITIALIZED };
	//This enumeration is placed here because a class attribute is defined with this type and it is necessary the enumeration
	//to be defined before
private:
	///////Private types//////////
	union FloatToBytes{
		float floatValue;
		uint8_t bytes[4];
	};//!< An union which is used to split a float value in its 4 bytes
	////////Attributes//////////
	//Constants
	//! This container allows to obtain the index (an integer value) which represent the given RBW value (in Hz), as it is defined in the Spectran USB Protocol.
	/*! The container *RBW_INDEX* can also be used to obtain the index of a given VBW value (in Hz), because both variables
	 * have the same indexes.
	 */
	const unordered_map<float,float>* RBW_INDEX;
	//Variables
	vector<uint8_t> bytes; //!< Bytes array (or vector) which will be sent by the Spectran Interface.
	CommandType commandType; //!< The command type of the object.
	VarName variableName; //!< The variable name which will be modified or read if the command type is *GETSTPVAR* or *SETSTPVAR*
	float value; //!< The value (as a float number) of the read Spectran variable or the value which will be used to modify a Spectran variable.
	////Private methods////
	void FillBytesVector();
public:
	////Class interface////
	Command();
	Command(CommandType type);
	Command(const unordered_map<float,float>& rbw_ind);
	Command(const unordered_map<float,float>& rbw_ind, CommandType commType);
	~Command();
	void SetPointer(const unordered_map<float,float>& rbw_ind);
	void SetAs(CommandType commType, VarName varName=VarName::UNINITIALIZED, float val=0.0);
	void SetParameters(VarName varName, float val=0.0);
	//! A method to get the current command type.
	CommandType GetCommandType() const {	return commandType;	}
	string GetCommTypeString() const;
	//! A method to get the variable name which is going to be or has been modified or read.
	VarName GetVariableName() const {	return variableName;	}
	//! A method to get the value which is going to be or has been used to modify a variable.
	float GetValue() const {	return value;	}
	const vector<uint8_t>& GetBytesVector() const;
	const uint8_t* GetBytesPointer() const;
	//! A method which returns the size of the bytes vector.
	unsigned int GetNumOfBytes() const {	return bytes.size();	}
	void Clear();
	const Command& operator=(const Command& command);
};


#endif /* SPECTRANINTERFACE_H_ */
