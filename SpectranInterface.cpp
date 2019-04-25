/*
 * SpectranInterface.cpp
 *
 *  Created on: 06/03/2019
 *      Author: new-mauro
 */

#include "Spectran.h"

SpectranInterface::SpectranInterface()
{
	flagLogIn=false;
	flagSweepsEnabled=false;

	//The pair of values (VID,PID) of the Spectran HF-60105 V4 X are included in the list of possible values.
	ftStatus=FT_SetVIDPID(VID, PID);

	if (ftStatus!=FT_OK){
		CustomException except("The Spectran Interface could not include the pair of values (PID,VID) of the Spectran device in the list of possible values.");
		throw(except);

	}else{

		OpenAndSetUp();

	}
}

void SpectranInterface::OpenAndSetUp()
{
	ftStatus=FT_OpenEx((PVOID)DEVICE_DESCRIPTION.c_str(), FT_OPEN_BY_DESCRIPTION, &ftHandle);

	if (ftStatus!=FT_OK){
		CustomException except;
		switch(ftStatus)
		{
		case 2:
			except.SetMessage("The Spectran device was not found.");
			throw(except);
		case 3:
			except.SetMessage("The Spectran device could not be opened.");
			throw(except);
		default:
			std::ostringstream oss;
			oss << "The function FT_OpenEx(), of the D2XX library, returned a ftStatus value of " << ftStatus;
			except.SetMessage( oss.str() );
			throw(except);
		}
	}else{
		//////////////Setting up the FTDI IC///////////////
		ftStatus=FT_SetTimeouts(ftHandle, USB_RD_TIMEOUT_MS, USB_WR_TIMEOUT_MS);
		if (ftStatus!=FT_OK){
			CustomException except("The read and write timeouts could not be set up.");
			throw(except);
		}

		///////////////////Configuracion obtenida del Foro////////////////////////
		ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0);
		if(ftStatus!=FT_OK)
		{
			CustomException exc("The flow control could not be set up.");
			throw(exc);
		}

		ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
		if(ftStatus!=FT_OK)
		{
			CustomException exc("The data characteristics could not be set up.");
			throw(exc);
		}

		ftStatus = FT_SetBaudRate(ftHandle, FT_BAUD_921600);
		//ftStatus = FT_SetBaudRate(ftHandle, FT_BAUD_115200);
		if(ftStatus!=FT_OK)
		{
			CustomException exc("The baud rate could not be set up.");
			throw(exc);
		}

		ftStatus = FT_SetChars(ftHandle, 0, 0, 0, 0);
		if(ftStatus!=FT_OK)
		{
			CustomException exc("The special characters could not be disabled.");
			throw(exc);
		}

		ftStatus = FT_SetUSBParameters(ftHandle, 0x1000, 0x000);
		if(ftStatus!=FT_OK)
		{
			CustomException exc("The USB request transfer size could not be set up.");
			throw(exc);
		}
	}
}

SpectranInterface::~SpectranInterface()
{
	try
	{
		LogOut();
	}
	catch(std::exception & exc)
	{
		cerr << "Error: " << exc.what() << " During destruction of class SpectraInterface." << endl;
	}

	ftStatus = FT_Close(ftHandle);
	if(ftStatus!=FT_OK)
	{
		cerr << "Error: The communication with the Spectran device could not be closed." << endl;
	}
}

void SpectranInterface::Initialize()
{
	Reply reply;
	unsigned int errorCounter=0, retryCounter=0;
	bool flagSuccess=false;

	Command command(Command::VERIFY);

	do
	{
		//Sending a LOGOUT command to make sure the device will reply correctly to the VERIFY commands
		LogOut();

		//Sending of two VERIFY commands
		do
		{
			try
			{
				Write(command);
				reply.Clear();
				reply.PrepareTo(Reply::VERIFY);
				Read(reply);
				flagSuccess=true;
			}
			catch (std::exception& exc)
			{
				cerr << "Warning: one of the two commands VERIFY to initialize the communication failed." << endl;
				errorCounter++;
			}
		}while(flagSuccess==false && errorCounter<2); //Just one error is accepted

		if(flagSuccess==false)
		{
			retryCounter++;
			if(retryCounter<3) //Just three retry are accepted
			{
				cerr << "Warning: Two commands VERIFY failed but the Spectran interface will reset the device and try again." << endl;
				//Turn off the device then turn it back on
			}
			else
			{
				CustomException exc("The Spectran interface could not initiliaze the communicaiton with the Spectran device.");
				throw(exc);
			}
		}
	}while(flagSuccess==false);


	//Disabling the transmission of measurements from the spectrum analyzer. Two commands are sent.
	DisableSweep();

	//Setting the speaker volume
	command.Clear();
	command.SetAs(Command::SETSTPVAR, SpecVariable::SPKVOLUME, DEFAULT_SPK_VOLUME);
	try
	{
		Write(command);
		reply.Clear();
		reply.PrepareTo(Reply::SETSTPVAR);
		Read(reply);
	}
	catch (std::exception& exc)
	{
		CustomException except("There was an error when the Spectran interface tried to set up the speaker volume.");
		throw(except);
	}

	if (reply.IsRight()!=true){
		std::ostringstream oss;
		oss << "The Spectran Interface tried to set the speaker volume to " << DEFAULT_SPK_VOLUME << " but the reply was not right.";
		CustomException except( oss.str() );
		throw(except);
	}

	flagLogIn=true; //To remember the session has been started

	SoundLogIn(); //To state that the communication was initialized

	//The input and output buffers are purged
	Purge();
}

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

unsigned int SpectranInterface::Available()
{
	DWORD numOfInputBytes;
	ftStatus=FT_GetQueueStatus(ftHandle, &numOfInputBytes);
	//DWORD numOfOutputBytes, events;
	//ftStatus=FT_GetStatus(ftHandle, &numOfInputBytes, &numOfOutputBytes, &events);
	if (ftStatus!=FT_OK){
		CustomException except("The Spectran interface could not read the number of bytes in the receive queue.");
		throw(except);
	}

	return numOfInputBytes;
}


void SpectranInterface::ResetSweep()
{
	//Reseting the current sweep
	Command comm(Command::SETSTPVAR, SpecVariable::USBSWPRST, 1.0);
	Reply reply(Reply::SETSTPVAR);
	try
	{
		Write(comm);
		Read(reply);
		if(!reply.IsRight())
			throw( CustomException("A wrong reply was received.") );
	}
	catch(CustomException & exc)
	{
		exc.Append("\nThe command to reset the current sweep failed.");
		throw;
	}
}

//! The aim of this method is to enable the sending of measurements via USB
void SpectranInterface::EnableSweep()
{
	Command comm(Command::SETSTPVAR, SpecVariable::USBMEAS, 1.0);
	Reply reply;
	try
	{
		Write(comm);
		reply.Clear();
		reply.PrepareTo(Reply::SETSTPVAR);
		Read(reply);
		if(reply.IsRight()!=true)
		{
			CustomException exc("The reply to the command to enable the sending of measurements via USB was wrong.");
			throw(exc);
		}
	}
	catch(CustomException & exc)
	{
		exc.Append("\nThe enabling of the sending of measurements via USB failed.");
		throw;
	}

	flagSweepsEnabled=true;
}

//! This method allows to disable the sending of measurements via USB
void SpectranInterface::DisableSweep()
{
	Command comm(Command::SETSTPVAR, SpecVariable::USBMEAS, 0.0);
	Reply reply;
	unsigned int errorCounter=0;

	for(unsigned int i=0; i<3; i++)
	{
		try
		{
			Write(comm);
			reply.Clear();
			reply.PrepareTo(Reply::SETSTPVAR);
			Read(reply);
			if(reply.IsRight()!=true)
			{
				CustomException exc("The reply to the command to disable the sending of measurements via USB was wrong.");
				throw(exc);
			}
		}
		catch(CustomException & exc)
		{
			cerr << "Warning: one of the commands to disable the sending of measurements via USB failed." << endl;

			if(++errorCounter < 3)
			{
				Purge();
				usleep(500000);
			}
			else
			{
				exc.Append("\nThe disabling of the sending of measurements via USB failed.");
				throw;
			}
		}
	}

	Purge();
	usleep(500000);

	flagSweepsEnabled=false;
}

void SpectranInterface::Purge()
{
	//The input buffer is purged
	ftStatus=FT_Purge(ftHandle, FT_PURGE_RX);
	if (ftStatus!=FT_OK){
		CustomException except("The Spectran Interface failed when it tried to purge the input buffer.");
		throw(except);
	}
}

//! The USB device is restarted.
/*! First, a logout is performed, then the communication is restarted and finally the Spectran interface
 * initialize the communication again.
 */
void SpectranInterface::ResetDevice()
{
	LogOut();

	ftStatus=FT_ResetDevice(ftHandle);
	if(ftStatus!=FT_OK){
		CustomException except("The USB device could not be restarted.");
		throw(except);
	}

	sleep(3);

	Initialize();
}

void SpectranInterface::LogOut()
{
	Command command;
	Reply reply;

	if(flagLogIn==true)
	{
		//Disabling the transmission of measurements
		DisableSweep();

		//Sound to indicate the session has been finished
		SoundLogOut();
	}

	//Sending of the LOGOUT command
	command.Clear();
	command.SetAs(Command::LOGOUT);
	try
	{
		Write(command);
	}
	catch(CustomException& exc)
	{
		exc.Append("\nThe command LOGOUT failed.");
		throw;
	}

	flagLogIn=false;
}

void SpectranInterface::SoundLogIn()
{
	Command command(Command::SETSTPVAR, SpecVariable::STDTONE, LOG_IN_SOUND_DURATION);

	for (auto i=0; i<2; i++)
	{
		Reply reply(Reply::SETSTPVAR);
		try
		{
			Write(command);
			Read(reply);
			if (reply.IsRight()!=true)
			{
				cerr << "Warning: the reply of one of the commands to produce one of the beeps which sound in the login was wrong" << endl;
			}
		}
		catch(std::exception& exc)
		{
			cerr << "Warning: one of the commands to produce one of the beeps which sound in the login failed." << endl;
		}
	}
}

void SpectranInterface::SoundLogOut()
{
	Command command(Command::SETSTPVAR, SpecVariable::STDTONE, LOG_OUT_SOUND_DURATION);
	Reply reply(Reply::SETSTPVAR);
	try
	{
		Write(command);
		Read(reply);
		if (reply.IsRight()!=true)
		{
			cerr << "Warning: The reply of the command to produce the beep which sound in the logout was wrong." << endl;
		}
	}
	catch(std::exception& exc)
	{
		cerr << "Warning: " << exc.what() << endl;
		cerr << "There was a failure with the command to produce the beep which sound in the logout failed." << endl;
	}
}
