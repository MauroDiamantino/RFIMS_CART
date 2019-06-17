/*
 * Spectran.h
 *
 *  Created on: 26/02/2019
 *      Author: new-mauro
 */

/*! \file Spectran.h
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

#ifndef SPECTRAN_H_
#define SPECTRAN_H_

/////////////Libraries//////////////////
#include <ftd2xx.h>
#include <map>
#include <boost/bimap.hpp> //Bidirectional container

///////////Other header files/////////////
#include "Basics.h"

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
	std::uint8_t bytes[4];
};//!< An union which is used to split a float value in its 4 bytes

typedef boost::bimap<float,float> RBW_bimap;

///////////////Constants///////////////////////
//! A vector which is initialized with the pairs of values {RBW(Hz), RBW index}. The vector is used to initialize a bidirectional map.
const std::vector<RBW_bimap::value_type> vect( {	{50e6, 0.0}, {3e6, 1.0}, {1e6, 2.0}, {300e3, 3.0}, {100e3, 4.0},
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
	//Variables
	std::vector<std::uint8_t> bytes; //!< Bytes array (or vector) which will be sent by the Spectran Interface.
	CommandType commandType; //!< The command type of the object.
	SpecVariable variableName; //!< The variable name which will be modified or read if the command type is *GETSTPVAR* or *SETSTPVAR*
	float value; //!< The value (as a float number) which will be used to modify a Spectran variable. It is just used with *SETSTPVAR* command.
	//////////Private methods/////////
	void FillBytesVector();
public:
	//////////Class interface//////////
	Command();
	Command(const CommandType commType, const SpecVariable variable=SpecVariable::UNINITIALIZED, const float val=0.0);
	Command(const Command& anotherComm);
	void SetAs(const CommandType commType, const SpecVariable variable=SpecVariable::UNINITIALIZED, const float val=0.0);
	void SetParameters(const SpecVariable variable, const float val=0.0);
	//! A method to get the current command type.
	CommandType GetCommandType() const {	return commandType;	}
	std::string GetCommTypeString() const;
	std::string GetVariableNameString() const;
	//! A method to get the variable name which is going to be or has been modified or read.
	SpecVariable GetVariableName() const {	return variableName;	}
	//! A method to get the value which is going to be or has been used to modify a variable.
	float GetValue() const {	return value;	}
	//! A method to obtain the bytes vector like this is implemented internally, a `std::vector` container.
	const std::vector<std::uint8_t>& GetBytesVector() const {	return bytes;	}
	//! A method to obtain the bytes vector but like a C-style array.
	/*! This method returns a pointer to `std::uint8_t` so this allows to access directly to the memory addresses where the
	 * vector's bytes are stored. Because of the Spectran Interface works with C-style arrays, that object uses this method.
	 */
	const std::uint8_t* GetBytesPointer() const {	return bytes.data();	}
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
	//Variables
	ReplyType replyType; //!< The reply type of the object.
	unsigned int numOfWaitedBytes; //!< The quantity of bytes which are waited taking into account the reply type.
	SpecVariable variableName; //!< The name of the variable which has been queried with a GETSTPVAR command.
	//////////Private methods///////////
	void PrepareReply();
protected:
	///////////Protected Attributes/////////////
	std::vector<std::uint8_t> bytes; //!< Bytes array (or vector) which has been received from a spectrum analyzer.
	float value; //!< The value (as a float number) of the queried Spectran variable or a power value. It has sense with *GETSTPVAR* and *AMPFREQDAT* replies.
	///////////Protected methods//////////////
	void FillBytesVector(const std::uint8_t * data);
public:
	//////////Class interface///////////
	Reply();
	Reply(const ReplyType type, const SpecVariable variable=SpecVariable::UNINITIALIZED);
	Reply(const Reply& anotherReply);
	virtual ~Reply() {}
	virtual void PrepareTo(const ReplyType type, const SpecVariable variable=SpecVariable::UNINITIALIZED);
	virtual void InsertBytes(const std::uint8_t * data);
	//! A Get method which returns the reply type as the corresponding value of the class' internal enumerator.
	ReplyType GetReplyType() const {	return replyType;	}
	std::string GetReplyTypeString() const;
	std::string GetVariableNameString() const;
	//! A method to get the bytes vector like this is implemented internally, a `std::vector` container.
	const std::vector<std::uint8_t>& GetBytesVector() const {	return bytes;	}
	//! A method to get a direct pointer to the bytes of the internal vector.
	const std::uint8_t* GetBytesPointer() const {	return bytes.data();	}
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
	std::uint_least64_t frequency;
	//float frequency;
	float minValue;
	float maxValue;
public:
	///////////Class Interface//////////
	SweepReply();
	SweepReply(const std::uint8_t * bytesPtr) : Reply(Reply::AMPFREQDAT) {	SweepReply::InsertBytes(bytesPtr);	}
	//~SweepReply() {}
	void PrepareTo(const ReplyType type, const SpecVariable variable=SpecVariable::UNINITIALIZED) {}
	void InsertBytes(const std::uint8_t * data);
	unsigned int GetTimestamp() const {	return timestamp;	};
	//float GetFrequency() const {	return frequency;	};
	std::uint_least64_t GetFrequency() const {	return frequency;	};
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
	const std::string DEVICE_DESCRIPTION = "Aaronia SPECTRAN HF-60105 X";
	const DWORD USB_RD_TIMEOUT_MS = 500;
	const DWORD USB_WR_TIMEOUT_MS = 1000;
	const float DEFAULT_SPK_VOLUME = 0.5;
	const float LOG_IN_SOUND_DURATION = 100.0;
	const float LOG_OUT_SOUND_DURATION = 500.0;
	//const unsigned int WAITING_TIME_US = 50000;
	//Variables
	FT_HANDLE ftHandle;
	bool flagLogIn;
	FT_STATUS ftStatus;
	bool flagSweepsEnabled;
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
	void ResetSweep();
	void EnableSweep();
	void DisableSweep();
	void Purge();
	void SoftReset();
	void HardReset();
	void LogOut();
	DWORD GetVID() const {	return VID;	}
	DWORD GetPID() const { 	return PID;	}
	std::string GetDevDescription() const {	return DEVICE_DESCRIPTION;	}
	bool IsLogged() const {	return flagLogIn;	}
	bool IsSweepEnabled() const {	return flagSweepsEnabled;	}
	void SoundNewSweep();
};
//////////////////Inline SpectranInterace's methods/////////////////////
inline void SpectranInterface::Write(const Command& command)
{
	DWORD writtenBytes;
	unsigned int numOfBytes = command.GetNumOfBytes();
	std::uint8_t txBuffer[numOfBytes];
	const std::uint8_t * bytesPtr = command.GetBytesPointer();

	for(unsigned int i=0; i<numOfBytes; i++){
		txBuffer[i] = bytesPtr[i];
	}

	ftStatus=FT_Write(ftHandle, txBuffer, numOfBytes, &writtenBytes);
	if (ftStatus!=FT_OK){
		CustomException except("The Spectran Interface could not write a command, the function FT_Write() returned an error value.");
		throw(except);
	}else if (writtenBytes!=numOfBytes){
		CustomException except("The Spectran Interface could not write a command correctly, not all bytes were written");
		throw(except);
	}
}

inline void SpectranInterface::Read(Reply& reply)
{
	DWORD receivedBytes;
	unsigned int numOfBytes=reply.GetNumOfBytes();
	std::uint8_t rxBuffer[numOfBytes];

	unsigned int i=0;
	unsigned int currNumOfBytes=0;
	while ( currNumOfBytes<numOfBytes && i++<20 )
	{
		currNumOfBytes = Available();
		usleep(70000);
	}
	if(i>=20)
	{
		CustomException exc("In a reading operation, the input bytes were waited too much time.");
		throw(exc);
	}

//	usleep(200000);

	ftStatus=FT_Read(ftHandle, rxBuffer, numOfBytes, &receivedBytes);
	if (ftStatus!=FT_OK){
		CustomException except("The Spectran interface could not read a Spectran reply, the function FT_Read returned an error value.");
		throw(except);
	}else if (receivedBytes!=numOfBytes){
		CustomException except("The Spectran interface tried to read a Spectran reply but not all bytes were read.");
		throw(except);
	}
	reply.InsertBytes(rxBuffer); //The received bytes are inserted in the given Reply object
}
//////////////////////////////////////////////////////////////////////////


//! The class *SpectranConfigurator* is intended to manage the process of configuring the Aaronia Spectran device.
class SpectranConfigurator
{
public:
	/////////////Public class's data types/////////////////
	struct FixedParameters
	{
		int attenFactor;
		unsigned int displayUnit; //”dBm” or “dBuV” or “V/m” or “A/m”
		const unsigned int demodMode=0; //0=off or 1=am or 2=fm
		unsigned int antennaType; //0=HL7025, 1=HL7040, 2=HL7060, 3=HL6080 or 4=H60100
		int cableType; //-1 is none, 0 is “1m standard cable”
		const unsigned int recvConf=0; //0=spectrum, 1=broadband
		bool internPreamp; //0=off, 1=on
		bool sweepDelayAcc; //Sweep delay for accuracy. 1=enable, 0=disable
		const bool peakLevelAudioTone=0; //0=disable, 1=enable
		const bool backBBDetector=0; //0=disable, 1=enable
		float speakerVol; //range from 0.0 to 1.0
	};
private:
	///////////Attributes///////////////
	//Constants
	const std::string SPEC_PARAM_PATH = BASE_PATH + "/parameters";
	//Variables
	std::ifstream ifs;
	SpectranInterface & interface;
	std::vector<BandParameters> bandsParamVector;
	std::vector<BandParameters> subBandsParamVector;
	unsigned int bandIndex;
	FixedParameters fixedParam;
	time_t lastWriteTimes[2];
	//////////Private methods//////////////
	void SetVariable(const SpecVariable variable, const float value);
	void CheckEqual(const SpecVariable variable, const float value);
	void CheckApproxEqual(const SpecVariable variable, float & value);
public:
	/////////////////Class' interface//////////////
	SpectranConfigurator(SpectranInterface & interf);
	//! The destructor of class SpectranConfigurator, which just makes sure the input file stream (ifs) is closed.
	~SpectranConfigurator() {	ifs.close();	}
	bool LoadFixedParameters();
	bool LoadBandsParameters();
	void InitialConfiguration();
	BandParameters ConfigureNextBand();
	void SetCurrBandParameters(const BandParameters & currBandParam) {	subBandsParamVector[bandIndex]=currBandParam;	}
	const FixedParameters& GetFixedParameters() const {	return fixedParam;	}
	const std::vector<BandParameters> & GetBandsParameters() const {	return bandsParamVector;	}
	const std::vector<BandParameters> & GetSubBandsParameters() const {		return subBandsParamVector;		}
	const BandParameters& GetCurrBandParameters() const {	return bandsParamVector[bandIndex];	}
	unsigned int GetNumOfBands() const {	return subBandsParamVector.size();	}
	unsigned int GetBandIndex() const {		return bandIndex;	}
	bool IsLastBand() const {	return ( bandIndex>=bandsParamVector.size() );	}
};

//! The aim of class *SweepBuilder* is to build the complete sweep from the individual sweep points which are delivered by the Spectran Interface
class SweepBuilder
{
	//////////Attributes////////////
	SpectranInterface & interface;
	//typedef std::map<float,float> SweepMap;
	typedef std::map<std::uint_least64_t,float> SweepMap;
	SweepMap partialSweep;
	Sweep sweep;
	////////////Private methods/////////
	void BuildSweep();
public:
	/////////Class' interface/////////
	//! The SweepBuilder class's constructor
	SweepBuilder(SpectranInterface & interf) : interface(interf) {}
	~SweepBuilder() {}
	const Sweep& CaptureSweep(BandParameters& bandParam);
	const Sweep& GetSweep() const {	return sweep;	}
};

#endif /* SPECTRAN_H_ */
