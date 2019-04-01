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
#include <cassert> //To use assert() function to debug the code
#include <ftd2xx.h> //The library which allows to communicate with the FTDI driver
#include <exception>
#include <cstdlib> //exit, EXIT_SUCCESS, EXIT_FAILURE
#include <sstream> //stringstream
#include <unistd.h> //usleep
#include <fstream> //ifstream
#include <boost/bimap.hpp> //Bidirectional container
#include <boost/algorithm/string.hpp> //to_lower(string)
#include <boost/filesystem/operations.hpp> //last_write_time(path)

using namespace std;


//////////////User-defined global types///////////
//! An enumeration which contains the names of all the environment variables of the Spectran HF-60105 V4 X spectrum analyzer.
enum class SpecVariable : uint8_t { STARTFREQ=0x01, STOPFREQ, RESBANDW, VIDBANDW, SWEEPTIME, ATTENFAC, REFLEVEL, DISPRANGE,
	DISPUNIT, DETMODE, DEMODMODE, SPECPROC, ANTTYPE, CABLETYPE, RECVCONF, CENTERFREQ=0x1E, SPANFREQ, PREAMPEN=0x10,
	SWPDLYACC, SWPFRQPTS, REFOFFS, USBMEAS=0x20, USBSWPRST, USBSWPID, USBRUNPROG, LOGFILEID=0x30, LOGSAMPCNT,
	LOGTIMEIVL,	SPECDISP=0x41, PEAKDISP, MARKMINPK, RDOUTIDX, MARKCOUNT, LEVELTONE, BACKBBEN, DISPDIS, SPKVOLUME,
	RBWFSTEP=0x60, ANTGAIN,	PEAK1POW=0x80, PEAK2POW, PEAK3POW, PEAK1FREQ=0x84, PEAK2FREQ, PEAK3FREQ, MAXPEAKPOW=0x90,
	STDTONE=0xC0, UNINITIALIZED };

union FloatToBytes {
	float floatValue;
	uint8_t bytes[4];
};//!< An union which is used to split a float value in its 4 bytes

typedef boost::bimap<float,float> RBW_bimap;


///////////////Constants///////////////////////
//! A vector which is initialized with the pairs of values {RBW(Hz), RBW index}. The vector is used to initialize a bidirectional map.
const vector<RBW_bimap::value_type> vect( {	{50e6, 0.0}, {3e6, 1.0}, {1e6, 2.0}, {300e3, 3.0}, {100e3, 4.0},
											{30e3, 5.0}, {10e3, 6.0}, {3e3, 7.0}, {1e3, 8.0}, {120e3, 100.0},
											{9e3, 101.0}, {200.0, 102.0}, {5e6, 103.0},	{200e3, 104}, {1.5e6, 105.0} }	);

//! A bidirectional map (bimap) which contains the pairs of values {RBW(Hz), RBW index}
/*! These pairs of values relates a RBW frequency value with its corresponding index taking into account the Spectran
 * USB protocol. So this bidirectional container allows to get the corresponding index given a frequency value or to
 * get the corresponding frequency value given an index. This allows to work with frequency values of RBW at high
 * level, eliminating the need to learn the protocol's indexes. This container is used by the classes Command and Reply.
 */
const RBW_bimap RBW_INDEX( vect.begin(), vect.end() );


/////////////////////Classes///////////////////////

//!Class CustomException derived from standard class exception.
class CustomException : public exception
{
	string message;
public:
	CustomException(const string& msg="Error") : message(msg) {}
	void SetMessage(const string& msg) {	message=msg;	}
	void Append(const string& msg){		message+=msg;	}
	virtual const char * what() const throw()
	{
		return message.c_str();
	}
};

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
class Command {
public:
	///////Public types///////////
	//! An enumeration which contains the command types which can be sent to a Spectran HF-60105 V4 X spectrum analyzer.
	/*! This enumeration contains just four commands from the Spectran USB Protocol: *VERIFY*, *LOGOUT*, *GETSTPVAR*
	 *  and *SETSTPVAR*. The other commands are intended to modify or get information about the internal files of the
	 *  spectrum analyzer, and because those will not be used they are not added. There is an extra command
	 *  type which is *UNINITIALIZED* whose purpose is to state that the object is still incomplete.
	 */
	enum CommandType : char { VERIFY=0x01, LOGOUT, GETSTPVAR=0x20, SETSTPVAR, UNINITIALIZED }; //This enumeration is
		//placed here because a class attribute is defined with this type and it is necessary the enumeration to be
		//defined before.
private:
	////////Attributes//////////
	//Constants
	//! This container allows to obtain the index (an integer value) which represent a given RBW (resolution bandwidth) value (in Hz), as it is defined in the Spectran USB Protocol.
	/*! The container *RBW_INDEX* can also be used to obtain the index of a given VBW (video bandwidth) value (in Hz),
	 * because both variables have the same indexes.
	 */
	//const unordered_map<float,float> RBW_INDEX;
	//const RBW_bimap* RBW_INDEX;
	//Variables
	vector<uint8_t> bytes; //!< Bytes array (or vector) which will be sent by the Spectran Interface.
	CommandType commandType; //!< The command type of the object.
	SpecVariable variableName; //!< The variable name which will be modified or read if the command type is *GETSTPVAR* or *SETSTPVAR*
	float value; //!< The value (as a float number) which will be used to modify a Spectran variable. It is just used with *SETSTPVAR* command.
	//////////Private methods/////////
	void FillBytesVector();
public:
	//////////Class interface//////////
	Command();
	Command(CommandType commType, SpecVariable variable=SpecVariable::UNINITIALIZED, float val=0.0);
	Command(const Command& anotherComm);
	void SetAs(CommandType commType, SpecVariable variable=SpecVariable::UNINITIALIZED, float val=0.0);
	void SetParameters(SpecVariable variable, float val=0.0);
	//! A method to get the current command type.
	CommandType GetCommandType() const {	return commandType;	}
	string GetCommTypeString() const;
	//! A method to get the variable name which is going to be or has been modified or read.
	SpecVariable GetVariableName() const {	return variableName;	}
	//! A method to get the value which is going to be or has been used to modify a variable.
	float GetValue() const {	return value;	}
	//! A method to obtain the bytes vector like this is implemented internally, a `vector` container.
	const vector<uint8_t>& GetBytesVector() const {	return bytes;	}
	//! A method to obtain the bytes vector but like a C-style array.
	/*! This method returns a pointer to `uint8_t` so this allows to access directly to the memory addresses where the
	 * vector's bytes are stored. Because of the Spectran Interface works with C-style arrays, that object uses this method.
	 */
	const uint8_t* GetBytesPointer() const {	return bytes.data();	}
	//! A method which returns the size of the bytes vector.
	unsigned int GetNumOfBytes() const {	return bytes.size();	}
	void Clear();
	const Command& operator=(const Command& command);
};


//! The class *Reply* is intended to receive a bytes vector sent by the spectrum analyzer and to extract its info.
/*! When a command is sent to a Spectran HF-60105 V4 X spectrum analyzer, this will respond with another bytes vector
 * (however some commands do not have reply), so the idea is to insert this in an object of class *Reply*, then the object
 * will process and extract the info (variable id, value, etc.) of the bytes vector and finally the info will be available
 * through the "Get" methods.
 */
class Reply {
public:
	//////////Public types////////////
	//! An enumeration which contains the reply types which can be received from a Spectran HF-60105 V4 X spectrum analyzer.
	/*! This enumeration contains just four commands from the Spectran USB Protocol: *VERIFY*, *GETSTPVAR*, *SETSTPVAR*
	 * and *AMPFREQDAT*. The other replies are received when an internal file of the spectrum analyzer have been queried
	 * or modified, and because that will not be done they are not added. There is an extra command type which is
	 * *UNINITIALIZED* whose purpose is to state that the object is not prepared.
	 */
	enum ReplyType : char { VERIFY=0x01, GETSTPVAR=0x20, SETSTPVAR, AMPFREQDAT, UNINITIALIZED }; //This enumeration is
	//placed here because a class attribute is defined with this type and it is necessary the enumeration to be
	//defined before.
private:
	//////////Attributes////////////
	//Constants
	//! This container allows to obtain the index (an integer value) which represent a given RBW (resolution bandwidth) value (in Hz), as it is defined in the Spectran USB Protocol.
	/*! The container *RBW_INDEX* can also be used to obtain the index of a given VBW (video bandwidth) value (in Hz),
	 * because both variables have the same indexes.
	 */
	//const RBW_bimap * RBW_INDEX;
	//Variables
	ReplyType replyType; //!< The reply type of the object.
	unsigned int numOfWaitedBytes; //!< The quantity of bytes which are waited taking into account the reply type.
	SpecVariable variableName; //!< The name of the variable which has been queried with a GETSTPVAR command.
	//////////Private methods///////////
	void PrepareReply();
protected:
	///////////Protected Attributes/////////////
	vector<uint8_t> bytes; //!< Bytes array (or vector) which has been received from a spectrum analyzer.
	float value; //!< The value (as a float number) of the queried Spectran variable or a power value. It has sense with *GETSTPVAR* and *AMPFREQDAT* replies.
	///////////Protected methods//////////////
	void FillBytesVector(uint8_t * data);
public:
	//////////Class interface///////////
	Reply();
	Reply(ReplyType type, SpecVariable variable=SpecVariable::UNINITIALIZED);
	Reply(const Reply& anotherReply);
	virtual ~Reply() {}
	virtual void PrepareTo(ReplyType type, SpecVariable variable=SpecVariable::UNINITIALIZED);
	virtual void InsertBytes(uint8_t * data);
	//! A Get method which returns the reply type as the corresponding value of the class' internal enumerator.
	ReplyType GetReplyType() const {	return replyType;	}
	string GetReplyTypeString() const;
	string GetVariableName() const;
	//! A method to get the bytes vector like this is implemented internally, a `vector` container.
	const vector<uint8_t>& GetBytesVector() const {	return bytes;	}
	//! A method to get a direct pointer to the bytes of the internal vector.
	const uint8_t* GetBytesPointer() const {	return bytes.data();	}
	//! A method which allows to know the size of the bytes vector.
	unsigned int GetNumOfBytes() const {	return numOfWaitedBytes; 	}
	//! A method to get the value of the variable which was queried with a *GETSTPVAR* command, or one of the power values of a *AMPFREQDAT* reply.
	float GetValue() const {	return value;	}
	bool IsRight() const;
	virtual void Clear();
	virtual const Reply& operator=(const Reply& anotherReply);
};


//! The class *SweepReply* derives from the base class *Reply*
/*! The purpose of this class is to handle the *AMPFREQDAT* replies which carry out the frequency points. These specific
 * replies need to be handled in a more complex way, so to simplify the base class ,which handles the others replies,
 * a different class was made with the specific methods to extract the data from the AMPFREQDAT replies.
 */
class SweepReply : public Reply {
	////////////Attributes/////////////
	unsigned int timestamp;
	float frequency;
	float minValue;
	float maxValue;
public:
	///////////Class Interface//////////
	SweepReply();
	SweepReply(uint8_t * bytesPtr);
	~SweepReply() {}
	void PrepareTo(ReplyType type, SpecVariable variable=SpecVariable::UNINITIALIZED) {}
	void InsertBytes(uint8_t * b);
	unsigned int GetTimestamp() const {	return timestamp;	};
	float GetFrequency() const {	return frequency;	};
	float GetMinValue() const {	return minValue;	};
	float GetMaxValue() const {	return maxValue;	};
	void Clear();
	const SweepReply& operator=(const SweepReply& anotherReply);
};


//! The class *SpectranInterface* is intended to manage the communication with the Aaronia Spectran device.
/*! This class establishes the communication with the Aaronia Spectran device (VID=0403, PID=E8D8) and allows
 * to write commands to the device and read its replies. Also, it allows to know the available bytes in the
 * input buffer, purge that buffer and reset the communication with the device.
 */
class SpectranInterface {
	////////////Attributes/////////////
	//Constants
	const DWORD VID = 0x0403;
	const DWORD PID = 0xE8D8;
	const string DEVICE_DESCRIPTION = "Aaronia SPECTRAN HF-60105 X";
	const DWORD USB_RD_TIMEOUT_MS = 500;
	const DWORD USB_WR_TIMEOUT_MS = 1000;
	const float SPK_VOLUME = 0.5;
	const float LOG_IN_SOUND_DURATION = 100.0;
	const float LOG_OUT_SOUND_DURATION = 500.0;
	const unsigned int WAITING_TIME_US = 50000;
	//Variables
	FT_HANDLE ftHandle;
	bool flagLogIn;
	FT_STATUS ftStatus;
	////////////Private methods/////////////
	void SoundLogIn();
	void SoundLogOut();
	void OpenAndSetUp();
public:
	////////////Class Interface/////////////
	SpectranInterface();
	~SpectranInterface();
	void Initialize();
	void Write(const Command& command);
	void Read(Reply& reply);
	unsigned int Available();
	void Purge();
	void Reset();
	void LogOut();
	DWORD GetVID() const {	return VID;	}
	DWORD GetPID() const { 	return PID;	}
	string GetDevDescription() const {	return DEVICE_DESCRIPTION;	}
	bool IsLogged() const {	return flagLogIn;	}
};

//! The class *SpectranConfigurator* is intended to manage the process of configuring the Aaronia Spectran device.
class SpectranConfigurator
{
	////////Private class' types/////////
	struct VarParameters
	{
		bool flagEnable; //it determines if the band is used or not
		float startFreq;
		float stopFreq;
		float rbw; //resolution bandwidth
		float vbw; //video bandwidth
		unsigned long int sweepTime;
		unsigned int samplePoints; //number of samples
		unsigned int detector; //”rms”(0) or “min/max”(1)
	};
	struct FixedParameters
	{
		int attenFactor;
		unsigned int displayUnit; //”dBm” or “dBuV” or “V/m” or “A/m”
		unsigned int demodMode; //”off” or “am” or “fm”
		unsigned int antennaType;
		int cableType; //-1 is none, 0 is “1m standard cable”
		unsigned int recvConf; //0=spectrum, 1=broadband
		bool internPreamp; //0=off, 1=on
		bool sweepDelayAcc; //Sweep delay for accuracy. 1=enable, 0=disable
		bool peakLevelAudioTone; //0=disable, 1=enable
		bool backBBDetector; //0=disable, 1=enable
		float speakerVol; //range from 0.0 to 1.0
		float antennaGain; //Nominal antenna gain in dB
	};
	///////////Attributes///////////////
	//Constants
	const string FILES_PATH = "/home/new-mauro/RFIMS-CART/parameters/";
	//Variables
	ifstream ifs;
	SpectranInterface & interface;
	vector<VarParameters> bands;
	unsigned int bandIndex;
	FixedParameters fixedParam;
	time_t lastWriteTimes[2];
	bool flagSweepsEnabled;
	//////////Private methods//////////////
	void SetAndCheckVariable(SpecVariable variable, float value);
public:
	/////////////////Class' interface//////////////
	SpectranConfigurator(SpectranInterface& interf);
	~SpectranConfigurator();
	bool LoadParameters();
	void InitialConfiguration();
	unsigned int ConfigureNextBand();
	void EnableSweep();
	void DisableSweep();
	vector<VarParameters> GetBandsParam() const {	return bands;	}
	unsigned int GetNumOfBands() const {	return bands.size();	}
	unsigned int GetBandIndex() const {		return bandIndex;	}
	bool IsLastBand() const {	return ( bandIndex>=bands.size() );	}
	bool IsSweepEnabled() const {	return flagSweepsEnabled;	}
};

#endif /* SPECTRANINTERFACE_H_ */
