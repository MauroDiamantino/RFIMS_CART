/*! \file Spectran.h
 *  \brief This header file contains the declarations of the classes which allow the communication with the spectrum analyzer
 *  Aaronia Spectran HF-60105 V4 X.
 *
 *  The classes defined in this header file allows to set up the spectrum analyzer, read its environment variables,
 *  enable/disable the streaming of sweep points, process its responses and to capture and store the sweep points in
 *  an orderly manner.
 *  \author Mauro Diamantino
 */

#ifndef SPECTRAN_H_
#define SPECTRAN_H_


//////////////////SPECIFIC LIBRARIES AND HEADERS/////////////////////

// Inclusion of the header file which has the declarations of the global functions, global classes, etc. and the inclusions of the global libraries.
#include "Basics.h"

// This library represents the D2XX driver provided by FTDI enterprise to communicate with USB devices which use FTDI chips, in this case the USB device is the spectrum analyzer.
#include <ftd2xx.h>
// The library allows to use the ordered, unique-key and associative container `std::map`, where are initially stored the sweep data points.
#include <map>
// This boost library allows to use the bidirectional container _Boost.Bimap_ which is similar to `std::map` but both data types can be used as keys.
#include <boost/bimap.hpp>

/////////////////////////////////////////////////////////////////////


//////////////USER-DEFINED DATA TYPES: ENUM, UNION AND TYPEDEF/////////////

//! An enumeration which contains of the names of all the environment variables of the spectrum analyzer Aaronia Spectran HF-60105 V4 X
/*! Each member (a variable name) has associated an integer number which is equal to its identification index (ID),
 * taking into account the Spectran USB Protocol documentation.
 */
enum class SpecVariable : uint8_t { STARTFREQ=0x01, STOPFREQ, RESBANDW, VIDBANDW, SWEEPTIME, ATTENFAC, REFLEVEL, DISPRANGE,
	DISPUNIT, DETMODE, DEMODMODE, SPECPROC, ANTTYPE, CABLETYPE, RECVCONF, CENTERFREQ=0x1E, SPANFREQ, PREAMPEN=0x10,
	SWPDLYACC, SWPFRQPTS, REFOFFS, USBMEAS=0x20, USBSWPRST, USBSWPID, USBRUNPROG, LOGFILEID=0x30, LOGSAMPCNT,
	LOGTIMEIVL,	SPECDISP=0x41, PEAKDISP, MARKMINPK, RDOUTIDX, MARKCOUNT, LEVELTONE, BACKBBEN, DISPDIS, SPKVOLUME,
	RBWFSTEP=0x60, ANTGAIN,	PEAK1POW=0x80, PEAK2POW, PEAK3POW, PEAK1FREQ=0x84, PEAK2FREQ, PEAK3FREQ, MAXPEAKPOW=0x90,
	STDTONE=0xC0, UNINITIALIZED };


union FloatToBytes
{
	float floatValue; //!< The `float` value which must be split.
	std::uint8_t bytes[4]; //!< The array with the 4 bytes of the inserted `float` value.
}; //!< An union which is used to split a `float` value in its 4 bytes.

//! This typedef simplifies the definitions of containers of type `boost::bimap<float,float>` which is used to store RBW values and its indexes.
typedef boost::bimap<float,float> RBW_bimap;

///////////////////////////////////////////////////////////////////////////


///////////////////////////CONSTANTS///////////////////////////////////////

//! A vector which is initialized with the pairs of values {RBW(Hz), RBW index}. This vector is used to initialize a bidirectional map.
const std::vector<RBW_bimap::value_type> vect( {	{50e6, 0.0}, {3e6, 1.0}, {1e6, 2.0}, {300e3, 3.0}, {100e3, 4.0},
											{30e3, 5.0}, {10e3, 6.0}, {3e3, 7.0}, {1e3, 8.0}, {120e3, 100.0},
											{9e3, 101.0}, {200.0, 102.0}, {5e6, 103.0},	{200e3, 104}, {1.5e6, 105.0} }	);

//! A bidirectional map (bimap) which contains the pairs of values {RBW(Hz), RBW index}.
/*! These pairs of values relates a RBW frequency value with its corresponding index taking into account the Spectran
 * USB protocol. So this bidirectional container allows to get the corresponding index given a frequency value or to
 * get the corresponding frequency value given an index. This allows to work with frequency values of RBW at high
 * level, eliminating the need to learn the protocol's indexes. This container is used by the classes _Command_ and _Reply_.
 */
const RBW_bimap RBW_INDEX( vect.begin(), vect.end() );

///////////////////////////////////////////////////////////////////////////


//////////////////////////////CLASSES//////////////////////////////////////

//! This class builds the corresponding bytes array to send a certain command to a Aaronia Spectran V4 series spectrum analyzer.
/*! The _Command_ class is intended to build the bytes array which will be sent a spectrum analyzer to perform
 * 	one of the following tasks:
 *	- Initialize the communication with the device.
 * 	- Set an environment variable.
 * 	- Get the value of an environment variable.
 * 	- Enable/disable the streaming of sweep points.
 * 	- Close the communication.
 * 	The user have to say to the object which "command type" he wants, which "variable" he wants to write/read and
 * 	which value must be used to set up the specified variable. When the object has the enough data, it will build
 * 	the bytes array which will be sent to the spectrum analyzer via the USB interface.
 * 	The objects of this class are interchanged between the _Spectran configurator_ and the _Spectran interface_.
 */
class Command {
public:
	//Public data types//
	//! An enumeration which contains the command types which can be sent to a spectrum analyzer Aaronia Spectran HF-60105 V4 X.
	/*! This enumeration contains just four commands from the Spectran USB Protocol: *VERIFY*, *LOGOUT*, *GETSTPVAR*
	 *  and *SETSTPVAR*. There are other possible commands which are intended to modify or get information about the internal
	 *  files of the spectrum analyzer, but they are not added because they will not be used. There is an extra command
	 *  type which is *UNINITIALIZED* whose purpose is to state that the object is still incomplete.
	 */
	enum CommandType : char { VERIFY=0x01, LOGOUT, GETSTPVAR=0x20, SETSTPVAR, UNINITIALIZED }; //This enumeration is placed here
		//because a class attribute is defined with this type and it is necessary the enumeration to be defined before.
private:
	//Attributes//
	//Variables
	std::vector<std::uint8_t> bytes; //!< Bytes array (or vector) which will be sent to the spectrum analyzer.
	CommandType commandType; //!< The command type of the object.
	SpecVariable variableName; //!< The variable name which will be modified or read if the command type is *SETSTPVAR* or *GETSTPVAR*.
	float value; //!< The value (as a floating-point number) which will be used to modify a Spectran variable. It is only used with *SETSTPVAR* commands.
	//Private methods//
	//! This method build the corresponding bytes vector when the enough data have been given.
	void FillBytesVector();
public:
	//Class interface//
	//! The default constructor.
	Command();
	//! The most complete constructor which allows to set the internal pointers and optionally the command type.
	Command(const CommandType commType, const SpecVariable variable=SpecVariable::UNINITIALIZED, const float val=0.0);
	//! The copy constructor.
	Command(const Command& anotherComm);
	//! This method is intended to provide to the object the enough data so this can configure itself to be ready to be sent.
	void SetAs(const CommandType commType, const SpecVariable variable=SpecVariable::UNINITIALIZED, const float val=0.0);
	//! This method is intended to set the command's parameters, so it should be used when the command type has already been set.
	void SetParameters(const SpecVariable variable, const float val=0.0);
	//! A method to get the current command type as a value of the enumeration _CommandType_.
	CommandType GetCommandType() const {	return commandType;	}
	//! A method which returns the command type as a `std::string`.
	std::string GetCommTypeString() const;
	//! A method to get the variable name which is going to be modified or read, as a value of the enumeration _SpecVariable_.
	SpecVariable GetVariableName() const {	return variableName;	}
	//! A method which returns the name of the Spectran's variable which is related with the command (_GETSTPVAR_ and _SETSTPVAR_ commands) as a `std::string`.
	std::string GetVariableNameString() const;
	//! A method to get the value which is going to be or has been used to modify a variable.
	float GetValue() const {	return value;	}
	//! A method to obtain the bytes vector like this is implemented internally, a `std::vector` container.
	const std::vector<std::uint8_t>& GetBytesVector() const {	return bytes;	}
	//! A method to obtain the bytes vector but like a C-style array (`std::uint8_t*`).
	/*! This method returns a pointer to `std::uint8_t` so this allows to access directly to the memory addresses where the
	 * vector's bytes are stored. The _Spectran Interface_ object uses this method because it works internally with C-style arrays.
	 */
	const std::uint8_t* GetBytesPointer() const {	return bytes.data();	}
	//! A method which returns the size of the bytes vector.
	unsigned int GetNumOfBytes() const {	return bytes.size();	}
	//! This method allows to reset the object, cleaning the bytes vector, command type, variable name and value.
	void Clear();
	//! An overloading of the assignment operator, adapted for this class.
	const Command& operator=(const Command& command);
};


//! The class *Reply* is intended to receive a bytes vector sent by the spectrum analyzer and to extract its information.
/*! When a command is sent to a Spectran HF-60105 V4 X spectrum analyzer, this will respond with another bytes vector
 * (however some commands do not have reply), so the idea is to insert this in an object of class *Reply*, then the object
 * will process and extract the info (variable id, value, etc.) of the bytes vector and finally the info will be available
 * through the "Get" methods.
 */
class Reply {
public:
	//Public types//
	//! An enumeration which contains the reply types which can be received from a Spectran HF-60105 V4 X spectrum analyzer.
	/*! This enumeration contains just four replies from the Spectran USB Protocol: *VERIFY*, *GETSTPVAR*, *SETSTPVAR*
	 * and *AMPFREQDAT*. The other replies are received when an internal file of the spectrum analyzer have been queried
	 * or modified, and because that will not be done, they are not added. There is an extra command type which is
	 * *UNINITIALIZED* whose purpose is to state that the object is not prepared.
	 */
	enum ReplyType : char { VERIFY=0x01, GETSTPVAR=0x20, SETSTPVAR, AMPFREQDAT, UNINITIALIZED }; //This enumeration is
	//placed here because a class attribute is defined with this type and it is necessary the enumeration to be
	//defined before.
private:
	//Attributes//
	//Variables
	ReplyType replyType; //!< The reply type of the object.
	unsigned int numOfWaitedBytes; //!< The quantity of bytes which are waited taking into account the reply type.
	SpecVariable variableName; //!< The name of the variable which has been queried with a GETSTPVAR command.
	//Private methods//
	//! This method is intended to prepare the object to receive the bytes sent by the spectrum analyzer.
	void PrepareReply();
protected:
	//Protected Attributes//
	std::vector<std::uint8_t> bytes; //!< Bytes array (or vector) which has been received from the spectrum analyzer.
	float value; //!< The value (as a floating-point number) of a queried Spectran variable or a power value (or voltage of field strength). It has sense with *GETSTPVAR* and *AMPFREQDAT* replies.
	//Protected methods//
	//! This method fills correctly the internal bytes vector with the received bytes.
	void FillBytesVector(const std::uint8_t * data);
public:
	//Class interface//
	//! The default constructor.
	Reply();
	//! A constructor which allows to set the reply type and the variable name, in case of _GETSTPVAR_ replies, and prepare the object to receive a spectrum analyzer's reply.
	Reply(const ReplyType type, const SpecVariable variable=SpecVariable::UNINITIALIZED);
	//! The copy constructor.
	Reply(const Reply& anotherReply);
	//! The class destructor which is defined as virtual because there are some classes derived from this one.
	/*! It was necessary to implement the destructor here to define this one as \em virtual. However, it was not necessary to explicitly clean,
	 * 	close and/or destroy any attribute and, because of that, the implementation is empty.
	 */
	virtual ~Reply() {}
	//! This method is intended to set the reply type and variable name, in case of _GETSTPVAR_ replies, and to ask the object to prepare itself to receive the bytes.
	virtual void PrepareTo(const ReplyType type, const SpecVariable variable=SpecVariable::UNINITIALIZED);
	//! This method is intended to insert a reply's bytes and to extract its data.
	virtual void InsertBytes(const std::uint8_t * data);
	//! This method returns the reply type as the corresponding value of the enumeration _ReplyType_.
	ReplyType GetReplyType() const {	return replyType;	}
	//! A method which returns the reply type as a `std::string`.
	std::string GetReplyTypeString() const;
	//! A method which returns the name of the Spectran's variable which is related with the reply (GETSTPVAR reply) as a `std::string`.
	std::string GetVariableNameString() const;
	//! A method to get the bytes vector like this is implemented internally, a `std::vector` container.
	const std::vector<std::uint8_t>& GetBytesVector() const {	return bytes;	}
	//! A method to get a direct pointer to the bytes of the internal vector.
	const std::uint8_t* GetBytesPointer() const {	return bytes.data();	}
	//! A method which allows to know the size of the bytes vector.
	unsigned int GetNumOfBytes() const {	return numOfWaitedBytes; 	}
	//! A method to get the value of the variable which was queried with a *GETSTPVAR* command or a power value (or voltage or field strength) which was received with a *AMPFREQDAT* reply.
	float GetValue() const {	return value;	}
	//! This method states if the received reply is right.
	bool IsRight() const;
	//! This method resets the object.
	virtual void Clear();
	//! An overloading of the assignment operator, adapted to this class.
	virtual const Reply& operator=(const Reply& anotherReply);
};


//! This class derives from the base class *Reply* and is intended to process in a better way replies with sweep points, i.e. _AMPFREQDAT_ replies.
/*! The purpose of this class is to handle the *AMPFREQDAT* replies which carry the sweep points. These specific
 * replies need to be handled in a more complex way, so to simplify the base class ,which handles the others replies,
 * a different class was made with the specific methods to extract the data from the AMPFREQDAT replies.
 */
class SweepReply : public Reply {
	//Attributes//
	unsigned int timestamp; //!< This variable stores the timestamp, internally generated in the spectrum analyzer, of a sweep point.
	std::uint_least64_t frequency; //!< This variable stores the frequency value, in Hertz, of a sweep point.
	float minValue; //!< This variable stores the minimum power value, in case of Min/Max detector is used, or the RMS power value, in case of RMS detector is used. Even, it could be a voltage or field strength value.
	float maxValue; //!< This variable stores the maximum power value, in case of Min/Max detector is used, or the RMS power value, in case of RMS detector is used. Even, it could be a voltage or field strength value.
public:
	//Class Interface//
	//! The default constructor.
	SweepReply(); //defined in Reply.cpp
	//! A more complete constructor which allows to insert the received bytes of a reply.
	SweepReply(const std::uint8_t * bytesPtr) : Reply(Reply::AMPFREQDAT) {	SweepReply::InsertBytes(bytesPtr);	}
	//! An overloading of the method `PrepareTo()` which just leave it without effect because this method has no sense in this derived class.
	void PrepareTo(const ReplyType type, const SpecVariable variable=SpecVariable::UNINITIALIZED) {}
	//! An overloading of the corresponding method of the base class, which allows to insert the bytes of a AMPFREQDAT reply and to extract its data.
	void InsertBytes(const std::uint8_t * data); //defined in Reply.cpp
	//! This method returns the timestamp of the corresponding sweep point.
	unsigned int GetTimestamp() const {	return timestamp;	};
	//! This method returns the frequency value, which is in Hz.
	std::uint_least64_t GetFrequency() const {	return frequency;	};
	//! This method returns the minimum power value, in case of Min/Max detector is used, or the RMS power value, in case of RMS detector is used. Even, it could be a voltage or field strength value.
	float GetMinValue() const {	return minValue;	};
	//! This method returns the maximum power value, in case of Min/Max detector is used, or the RMS power value, in case of RMS detector is used. Even, it could be a voltage or field strength value.
	float GetMaxValue() const {	return maxValue;	};
	//! The aim of this method is to clear the class attributes.
	void Clear(); //defined in Reply.cpp
	//! An overloading of the assignment operator, adapted to this class.
	const SweepReply& operator=(const SweepReply& anotherReply); //defined in Reply.cpp
};


//! The aim of this class is to manage the communication with the Aaronia Spectran device.
/*! This class establishes the communication with the Aaronia Spectran device (VID=0403, PID=E8D8) and allows
 * to write commands to the device and read its replies. Also, it allows to know the available bytes in the
 * input buffer, purge that buffer and reset the communication with the device.
 */
class SpectranInterface {
	//Attributes//
	//Constants
	const DWORD VID = 0x0403; //!< USB Vendor ID of the spectrum analyzer Aaronia Spectran HF-60105 V4 X.
	const DWORD PID = 0xE8D8; //!< USB Product ID of the spectrum analyzer Aaronia Spectran HF-60105 V4 X.
	const std::string DEVICE_DESCRIPTION = "Aaronia SPECTRAN HF-60105 X"; //!< The USB device description which identifies the spectrum analyzer.
	const DWORD BAUD_RATE = FT_BAUD_921600; //!< This constant stores the baud rate of the RS232 interface of the FTDI chip.
	//const DWORD BAUD_RATE = FT_BAUD_115200;
	const DWORD USB_RD_TIMEOUT_MS = 500; //!< This constant represents the timeout which is taken into account in the reading operations, and it is expressed in milliseconds.
	const DWORD USB_WR_TIMEOUT_MS = 1000; //!< This constant represents the timeout which is taken into account in the writing operations, and it is expressed in milliseconds.
	const float DEFAULT_SPK_VOLUME = 0.5; //!< This constant is the internal speaker's default volume of the spectrum analyzer.
	const float LOG_IN_SOUND_DURATION = 100.0; //!< This value represents the duration of the login sounds, which are two pulses.
	const float LOG_OUT_SOUND_DURATION = 500.0; //!< This value represents the duration of the logout sound, which is only one pulse.
	const float NEW_SWEEP_SOUND_DURATION = 100.0; //!< This value represents the duration of the new-sweep sound, which is only one pulse.
	//Variables
	FT_HANDLE ftHandle; //!< This variable is used to handle the communication with the spectrum analyzer.
	bool flagLogIn; //!< This flag registers if the communication has been initiated, i.e. if the interface has logged in with the spectrum analyzer.
	FT_STATUS ftStatus; //!< This variable stores the values returned by some _D2XX_ functions and indicates if the operation was performed correctly or not.
	bool flagSweepsEnabled; //!< A flag which registers if the streaming of sweep points has been enabled or not.
	//Private methods//
	//! This function produces the login sounds.
	void SoundLogIn();
	//! This function produces the logout sound.
	void SoundLogOut();
	//! The aim of this function is to open the communication with the spectrum analyzer and to set up the communication's parameters.
	void OpenAndSetUp();
public:
	//Class Interface//
	//! The default class constructor.
	SpectranInterface();
	//! The class destructor.
	~SpectranInterface();
	//! This method logs in with the spectrum analyzer, once the communication has been opened and its parameters has been set up.
	void Initialize();
	//! This method returns the number of available bytes in the input buffer.
	unsigned int Available();
	//! This method is intended to restore the streaming of sweep points to the first point which corresponds to the initial frequency (Fstart).
	void ResetSweep();
	//! This method allows to enable the streaming of sweep points.
	void EnableSweep();
	//! This method allows to disable the streaming of sweep points.
	void DisableSweep();
	//! This method purges the input buffer.
	void Purge();
	//! The soft reset is performed using the function `FT_ResetDevice()` of the driver D2XX.
	void SoftReset();
	//! The hard reset implies that the spectrum analyzer is turned off and then it is turned on again.
	void HardReset();
	//! This method finishes the communication session with the spectrum analyzer.
	void LogOut();
	//! This method returns the Vendor ID of the spectrum analyzer.
	DWORD GetVID() const {	return VID;	}
	//! This method returns the Product ID of the spectrum analyzer.
	DWORD GetPID() const { 	return PID;	}
	//! This method returns the device description (a string) of the spectrum analyzer.
	std::string GetDevDescription() const {	return DEVICE_DESCRIPTION;	}
	//! This method returns a true value if the communication has been opened and a login has been performed, and a false value otherwise.
	bool IsLogged() const {	return flagLogIn;	}
	//! This method returns a true value if the streaming of sweep points has been enabled and a false otherwise.
	bool IsSweepEnabled() const {	return flagSweepsEnabled;	}
	//! This method allows to perform the a sound to state the capturing of a sweep has finished.
	void SoundNewSweep();
	//Inline methods//
	//! This method is intended to perform all the writing operations.
	/*! First, the method copies the bytes of the command to an auxiliary array, because the function `FT_Write()`
	 * 	destroys the array of bytes which is passed to it, so this is done to avoid the modification of the _Command_
	 * 	object. Then the function `FT_Write()` is called with the corresponding arguments and, finally, it is controlled
	 * 	if there were errors and if all bytes were written.
	 * 	\param [in] command A _Command_ object which contains the bytes must be sent to the spectrum analyzer.
	 */
	void Write(const Command& command)
	{
		DWORD writtenBytes;
		unsigned int numOfBytes = command.GetNumOfBytes();
		std::uint8_t txBuffer[numOfBytes];
		const std::uint8_t * bytesPtr = command.GetBytesPointer();

		for(unsigned int i=0; i<numOfBytes; i++)
			txBuffer[i] = bytesPtr[i];

		ftStatus=FT_Write(ftHandle, txBuffer, numOfBytes, &writtenBytes);
		if (ftStatus!=FT_OK)
			throw rfims_exception("a Spectran command could not be written, the function FT_Write() returned an error value.");
		else if (writtenBytes!=numOfBytes)
			throw rfims_exception("a Spectran command could not be written correctly because not all bytes were written");
	}
	//! This method is intended to perform all the reading operations.
	/*! Before the calling of the function `FT_Read()`, there is a loop where it is queried if the number of waited bytes are available.
	 * 	After each query which states less bytes than what are waited, the method waits 70 ms. Once the desired number bytes are available,
	 * 	the method waits 70 ms to avoid an error which was observed during the tests and which consists in a reading failure when this operation
	 * 	is performed immediately after the needed number of bytes are available. After that waiting, the function `FT_Read()` is called.
	 * 	Later, it is checked if the operation finished with errors and if all bytes were read and, finally, the read bytes are inserted in
	 * 	the _Reply_ object.
	 * 	\param [in,out] reply A _Reply_ object which states the number of bytes must be read and which receives these bytes to the extract the info from them.
	 */
	void Read(Reply& reply)
	{
		const unsigned int WAITING_TIME_MS = 1500; //1.5 s
		const unsigned int DELAY_US = 15000; //10 ms
		const unsigned int NUM_OF_ITERS = ceil( WAITING_TIME_MS / (double(DELAY_US)/1000.0) );
		DWORD receivedBytes;
		unsigned int numOfBytes=reply.GetNumOfBytes();
		std::uint8_t rxBuffer[numOfBytes];

		unsigned int i=0;

//		unsigned int currNumOfBytes=0;
//		while ( currNumOfBytes<numOfBytes && i++<NUM_OF_ITERS )
//		{
//			currNumOfBytes = Available();
//			usleep(DELAY_US);
//		}

		while( Available()<numOfBytes && i++<NUM_OF_ITERS )
			usleep(DELAY_US);

		if(i>=NUM_OF_ITERS)
			throw rfims_exception("in a reading operation, the input bytes were waited too much time.");

		//usleep(200000);

		ftStatus=FT_Read(ftHandle, rxBuffer, numOfBytes, &receivedBytes);
		if (ftStatus!=FT_OK)
			throw rfims_exception("a Spectran reply could not be read, the function FT_Read() returned an error value.");
		else if (receivedBytes!=numOfBytes)
			throw rfims_exception("it was tried to read a Spectran reply but not all bytes were read.");

		reply.InsertBytes(rxBuffer); //The received bytes are inserted in the given Reply object
	}
};


//! The class *SpectranConfigurator* is intended to manage the process of configuring the Aaronia Spectran device.
/*! To do this task, this component give _Command_ objects to the _SpectranInterface_ object, which writes them in the spectrum analyzer
 * 	and then it reads the replies, the Spectran Interface passes them as _Reply_ objects to the Spectran Configurator, so this one can know if
 * 	the last configuration was performed successfully or not.
 */
class SpectranConfigurator
{
public:
	//Public data types//
	//! This structure saves the fixed parameters of the spectrum analyzer, i.e. the parameters which do not change through the entire measurement cycle.
	struct FixedParameters
	{
		int attenFactor; //!< Attenuator factor [dB]: -10=auto, 0=off or a value between 1 to 30.
		unsigned int displayUnit; //!< Measurement unit: 0=dBm (power), 1=dBuV (voltage), 2=V/m (electric field strength) or 3=A/m (magnetic field strength).
		const unsigned int demodMode=0; //!< Demodulator mode: 0=off, 1=am or 2=fm.
		unsigned int antennaType; //!< Antenna type: 0=HL7025, 1=HL7040, 2=HL7060, 3=HL6080 or 4=H60100.
		int cableType; //!< Cable type: -1 is none, 0 is “1m standard cable”.
		const unsigned int recvConf=0; //!< Receiver configuration: 0=spectrum, 1=broadband.
		bool internPreamp; //!< Internal preamplifier enabling: 0=off, 1=on.
		bool sweepDelayAcc; //!< Sweep delay for accuracy mode: 1=enable, 0=disable.
		const bool peakLevelAudioTone=0; //!< Peak Level Audio tone: 0=disable, 1=enable.
		const bool backBBDetector=0; //!< Background Broadband detector: 0=disable, 1=enable.
		float speakerVol; //!< Speaker volume: range from 0.0 to 1.0.
	};
private:
	//Attributes//
	//Constants
	const std::string SPECTRAN_PARAM_PATH = BASE_PATH + "/parameters"; //!< The path where the files with the parameters which are used to configure spectrum analyzer are saved.
	//Variables
	std::ifstream ifs; //!< This object is used to read the parameters from the corresponding files.
	SpectranInterface & interface; //!< A reference to the unique _SpectranInterface_ object, which is responsible for the communication with the spectrum analyzer.
	std::vector<BandParameters> bandsParamVector; //!< This vector saves the parameters of each frequency band.
	std::vector<BandParameters> subBandsParamVector; //!< This vector saves the parameters of each frequency sub-band, which arise because of the limitation of the span to a maximum of 200 MHz.
	unsigned int bandIndex; //!< This variable stores the band index to know what is the next frequency band.
	FixedParameters fixedParam; //!< The fixed parameters are stored in this structure.
	time_t lastWriteTimes[2]; //! This array stores the last-modification times (as seconds measured from the Unix epoch) of the files with parameters.
	std::string paramSetName; //!< This variable stores the name of the current parameters set which are used to set up the spectrum analyzer in each band.
	//Private methods//
	//! This method allows to set an environment variable of the spectrum analyzer.
	void SetVariable(const SpecVariable variable, const float value);
	//! This method checks if the value of a determined variable of the spectrum analyzer is exactly equal to a given value.
	void CheckEqual(const SpecVariable variable, const float value);
	//! This method checks if the value of a determined variable of the spectrum analyzer is approximately equal to a given value.
	void CheckApproxEqual(const SpecVariable variable, float & value);
public:
	//Class' interface//
	//! The default constructor.
	SpectranConfigurator(SpectranInterface & interf);
	//! The destructor which just makes sure the input file stream is closed.
	~SpectranConfigurator() {	ifs.close();	}
	//! This method loads the fixed Spectran's parameters from the corresponding file.
	bool LoadFixedParameters();
	//! This method loads the frequency bands parameters from the corresponding file.
	bool LoadBandsParameters();
	//! This method is intended to configure the spectrum analyzer with the fixed parameters at the beginning of a measurement cycle.
	void InitialConfiguration();
	//! The aim of this method is to configure the spectrum analyzer with parameters of the next frequency band.
	BandParameters ConfigureNextBand();
	//! This method allows to change the parameters of the current frequency band, given a _BandParameters_ structure as parameter.
	/*! \param [in] currBandParam A _BandParameters_ structure with the parameters which it is desired the current frequency band to have.	*/
	void SetCurrBandParameters(const BandParameters & currBandParam) {	subBandsParamVector[bandIndex]=currBandParam;	}
	//! This method returns the fixed parameters.
	const FixedParameters& GetFixedParameters() const {	return fixedParam;	}
	//! This method returns a vector with the parameters of all frequency bands.
	const std::vector<BandParameters> & GetBandsParameters() const {	return bandsParamVector;	}
	//! This method returns a vector with the parameters of all frequency sub-bands.
	const std::vector<BandParameters> & GetSubBandsParameters() const {		return subBandsParamVector;		}
	//! This method returns the parameters of the current frequency band.
	const BandParameters& GetCurrBandParameters() const {	return bandsParamVector[bandIndex];	}
	//! This method returns the number of frequency sub-bands (or bands if they were not split).
	unsigned int GetNumOfBands() const {	return subBandsParamVector.size();	}
	//! This method returns the current value of the band index.
	unsigned int GetBandIndex() const {		return bandIndex;	}
	//! This method returns a `true` if the current band is the last one and a `false` otherwise.
	bool IsLastBand() const {	return ( bandIndex>=bandsParamVector.size() );	}
};

//! The aim of class *SweepBuilder* is to build the complete sweep from the individual sweep points which are delivered by the Spectran Interface.
class SweepBuilder
{
	//Attributes//
	SpectranInterface & interface; //!< A reference to the unique _SpectranInterface_ object, which is responsible for the communication with the spectrum analyzer.
	typedef std::map<std::uint_least64_t,float> SweepMap; //!< This `typedef` simplify the syntaxes of instructions which are related with the `std::map` container.
	SweepMap partialSweep; //!< This is an associative container which is used to build the entire sweep from the points. This is used because it is an ordered and unique-key container.
	Sweep sweep; //!< In this variable is stored the sweep once this has been built. This structure is optimum to perform mathematical operations.
	//Private methods//
	//! A method which build the definite _Sweep_ structure from the partial sweep, which is a `std::map` container.
	void BuildSweep();
public:
	//Class' interface//
	//! The SweepBuilder class's constructor.
	/*! \param [in] interf A reference to the unique _SpectranInterface_ object, which is responsible for the communication with the spectrum analyzer. */
	SweepBuilder(SpectranInterface & interf) : interface(interf) {}
	//! The SweepBuilder class's destructor.
	/*!	Its implementation is empty because the attributes are implicitly destroyed. However, the
	 * destructor is defined here to allow this one to be called explicitly in any part of the code,
	 * what is used by the signals handler to destroy the objects when a signal to finish the execution
	 * of the software is received.
	 */
	~SweepBuilder() {}
	//! The aim of this method is to capture one entire sweep from the spectrum analyzer through the Spectran Interface and returns this one.
	const Sweep& CaptureSweep(BandParameters& bandParam);
	//! This method returns the last captured sweep, as a _Sweep_ structure.
	const Sweep& GetSweep() const {		return sweep;	}
};

#endif /* SPECTRAN_H_ */
