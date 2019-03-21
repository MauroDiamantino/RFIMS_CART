/*
 * SpectranInterface.cpp
 *
 *  Created on: 06/03/2019
 *      Author: new-mauro
 */

#include "SpectranInterface.h"

SpectranInterface::SpectranInterface()
{
	flagLogIn=false;

	//The pair of values (VID,PID) of the Spectran HF-60105 V4 X are included in the list of possible values.
	ftStatus=FT_SetVIDPID(VID, PID);

	if (ftStatus!=FT_OK){
		CustomException except("Error when the Spectran Interface tried to include the pair of values (PID,VID) of the Spectran device in the list of possible values.");
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
			except.SetMessage("Error: the Spectran device was not found.");
			throw(except);
		case 3:
			except.SetMessage("Error: the Spectran device could not be opened.");
			throw(except);
		default:
			stringstream ss;
			ss << "Error: the function FT_OpenEx() returned a ftStatus value of " << ftStatus;
			except.SetMessage( ss.str() );
			throw(except);
		}
	}else{
		//////////////Setting up the FTDI IC///////////////
		ftStatus=FT_SetTimeouts(ftHandle, USB_RD_TIMEOUT_MS, USB_WR_TIMEOUT_MS);
		if (ftStatus!=FT_OK){
			CustomException except("Error: the read and write timeouts could not be set up.");
			throw(except);
		}

		////////////Configuracion obtenida de la documentacion del GPS//////////////////
//		ftStatus=FT_SetBaudRate(ftHandle, 625000);
//		if(ftStatus!=FT_OK)
//		{
//			if(ftStatus==7){
//				CustomException except("Error: the baud rate could not be set up because the given value is not invalid.");
//				throw(except);
//			}else{
//				stringstream ss;
//				ss << "Error: the baud rate could not be set up. ftStatus = " << ftStatus;
//				CustomException except( ss.str() );
//				throw(except);
//			}
//		}
//
//		ftStatus=FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
//		if(ftStatus!=FT_OK)
//		{
//			CustomException exc("Error: the data format could not be set up.");
//			throw(exc);
//		}
//
//		ftStatus=FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0);
//		if(ftStatus!=FT_OK)
//		{
//			CustomException exc("Error: the flow control could not be set up.");
//			throw(exc);
//		}

		///////////////////Configuracion obtenida del Foro////////////////////////
		ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0);
		if(ftStatus!=FT_OK)
		{
			CustomException exc("Error: the flow control could not be set up.");
			throw(exc);
		}

		ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
		if(ftStatus!=FT_OK)
		{
			CustomException exc("Error: the data characteristics could not be set up.");
			throw(exc);
		}

		ftStatus = FT_SetBaudRate(ftHandle, FT_BAUD_921600);
		if(ftStatus!=FT_OK)
		{
			CustomException exc("Error: the baud rate could not be set up.");
			throw(exc);
		}

		ftStatus = FT_SetChars(ftHandle, 0, 0, 0, 0);
		if(ftStatus!=FT_OK)
		{
			CustomException exc("Error: the special characters could not be disabled.");
			throw(exc);
		}

		ftStatus = FT_SetUSBParameters(ftHandle, 0x1000, 0x000);
		if(ftStatus!=FT_OK)
		{
			CustomException exc("Error: the USB request transfer size could not be set up.");
			throw(exc);
		}
	}
}

SpectranInterface::~SpectranInterface()
{
	LogOut();

	FT_Close(ftHandle);
}

void SpectranInterface::Initialize()
{
	Reply reply;
	unsigned int errorCounter=0;

	//if (flagLogIn==true)
		//LogOut();

	//Sending of two VERIFY commands
	Command command(Command::VERIFY);
	for (auto i=0; i<2; i++)
	{
		try
		{
			Write(command);
			reply.Clear();
			reply.PrepareTo(Reply::VERIFY);
			Read(reply);
		}
		catch (exception& exc)
		{
			//cerr << "Initialize()->" << exc.what() << endl;
			cerr << "Warning: one of the two commands VERIFY to initialize the communication failed." << endl;
			errorCounter++;
			//Just one error is accepted
			if (errorCounter>1){
				CustomException except("Initialize(). Error sending the two commands VERIFY. Both commands received a wrong reply or no reply.");
				throw(except);
			}
		}
	}

	//Disabling the transmission of measurements from the spectrum analyzer. Two commands are sent.
	command.Clear();
	command.SetAs(Command::SETSTPVAR, VarName::USBMEAS, 0.0);
	Write(command); //First command to set USBMEAS to 0
	reply.Clear();
	reply.PrepareTo(Reply::SETSTPVAR);
	Read(reply);
	try
	{
		Write(command); //Second command to set USBMEAS to 0
		reply.Clear();
		reply.PrepareTo(Reply::SETSTPVAR);
		Read(reply);
	}
	catch(exception& exc)
	{
		CustomException except("Error: there was an error when the Spectran interface tried to disable the transmission of measurements.");
		throw(except);
	}

	if(reply.IsRight()==false)
	{
		CustomException except("Error: the reply to the 2nd command to disable the transmission of measurements was wrong.");
		throw(except);
	}

	//Setting the speaker volume
	command.Clear();
	command.SetAs(Command::SETSTPVAR, VarName::SPKVOLUME, SPK_VOLUME);
	try
	{
		Write(command);
		reply.Clear();
		reply.PrepareTo(Reply::SETSTPVAR);
		Read(reply);
	}
	catch (exception& exc)
	{
		CustomException except("Error: there was an error when the Spectran interface tried to set up the speaker volume.");
		throw(except);
	}

	if (reply.IsRight()!=true){
		stringstream ss;
		ss << "Error: the Spectran Interface tried to set the speaker volume to " << SPK_VOLUME << " but the reply was not right.";
		CustomException except( ss.str() );
		throw(except);
	}

	flagLogIn=true; //To remember the session has been started

	SoundLogIn(); //To state that the communication was initialized

	//The input and output buffers are purged
	Purge();
}

void SpectranInterface::Write(const Command& command)
{
	DWORD writtenBytes;
	unsigned int numOfBytes = command.GetNumOfBytes();
	uint8_t txBuffer[numOfBytes];
	const uint8_t * bytesPtr = command.GetBytesPointer();

	for(unsigned int i=0; i<numOfBytes; i++){
		txBuffer[i] = bytesPtr[i];
	}

	ftStatus=FT_Write(ftHandle, txBuffer, numOfBytes, &writtenBytes);
	if (ftStatus!=FT_OK){
		CustomException except("Write(). Error when the Spectran Interface tried to write a command, the function FT_Write() returned an error value.");
		throw(except);
	}else if (writtenBytes!=numOfBytes){
		CustomException except("Write(). Error when the Spectran Interface tried to write a command, not all bytes were written");
		throw(except);
	}
}

void SpectranInterface::Read(Reply& reply)
{
	DWORD receivedBytes;
	unsigned int numOfBytes=reply.GetNumOfBytes();
	uint8_t rxBuffer[numOfBytes];

	usleep(200000);

	ftStatus=FT_Read(ftHandle, rxBuffer, numOfBytes, &receivedBytes);
	if (ftStatus!=FT_OK){
		CustomException except("Read(). Error when the Spectran interface tried to read a Spectran reply, the function FT_Read returned an error value.");
		throw(except);
	}else if (receivedBytes!=numOfBytes){
		CustomException except("Read(). Error when the Spectran interface tried to read a Spectran reply, not all bytes were read.");
		throw(except);
	}
	reply.InsertBytes(rxBuffer); //The received bytes are inserted in the given Reply object
}

unsigned int SpectranInterface::Available()
{
	DWORD numOfInputBytes;
	ftStatus=FT_GetQueueStatus(ftHandle, &numOfInputBytes);
	if (ftStatus!=FT_OK){
		CustomException except("Available(). Error when the Spectran interface tried to read the number of bytes in the receive queue.");
		throw(except);
	}

	return numOfInputBytes;
}

void SpectranInterface::Purge()
{
	//The input and output buffers are purged
	ftStatus=FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
	if (ftStatus!=FT_OK){
		CustomException except("Error when the Spectran Interface tried to purge the input and output buffers.");
		throw(except);
	}
}

//! The USB device is restarted
void SpectranInterface::Reset()
{
	LogOut();

	ftStatus=FT_ResetDevice(ftHandle);
	if(ftStatus!=FT_OK){
		CustomException except("Error: the USB device could not be restarted.");
		throw(except);
	}

	sleep(3);

	ftStatus=FT_Close(ftHandle);
	if(ftStatus!=FT_OK){
		CustomException except("Error: the USB could not be closed to reset the communication.");
		throw(except);
	}

	OpenAndSetUp();

}

void SpectranInterface::LogOut()
{
	Command command;
	Reply reply;

	if(flagLogIn==true)
	{
		//Disabling the transmission of measurements
		command.SetAs(Command::SETSTPVAR, VarName::USBMEAS, 0.0);
		Write(command); //First command to set USBMEAS to 0
		reply.PrepareTo(Reply::SETSTPVAR);
		Read(reply);
		try
		{
			Write(command); //Second command to set USBMEAS to 0
			reply.Clear();
			reply.PrepareTo(Reply::SETSTPVAR);
			Read(reply);
		}
		catch(exception& exc)
		{
			CustomException except("Error: there was an error when the Spectran interface tried to disable the transmission of measurements.");
			throw(except);
		}

		if(reply.IsRight()==false)
		{
			CustomException except("Error: the reply to the 2nd command to disable the transmission of measurements was wrong.");
			throw(except);
		}

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
	catch(exception& exc)
	{
		string str( exc.what() );
		str.insert(0, "LogOut()->");
		CustomException exc2(str);
		throw(exc2);
	}

	flagLogIn=false;
}

void SpectranInterface::SoundLogIn()
{
	Command command(Command::SETSTPVAR, VarName::STDTONE, LOG_IN_SOUND_DURATION);

	for (auto i=0; i<2; i++)
	{
		Reply reply(Reply::SETSTPVAR);
		try
		{
			Write(command);
			Read(reply);
		}
		catch(exception& exc)
		{
			string str(exc.what());
			str.insert(0, "SoundLogIn()->");
			CustomException except(str);
			throw(except);
		}
		if (reply.IsRight()!=true){
			CustomException except("Error when the Spectran Interface tried to make the Log-In sound.");
			throw(except);
		}
	}
}

void SpectranInterface::SoundLogOut()
{
	Command command(Command::SETSTPVAR, VarName::STDTONE, LOG_OUT_SOUND_DURATION);
	Reply reply(Reply::SETSTPVAR);
	try
	{
		Write(command);
		Read(reply);
	}
	catch(exception& exc)
	{
		string str(exc.what());
		str.insert(0, "SoundLogOut()->");
		CustomException except(str);
		throw(except);
	}
	if (reply.IsRight()!=true){
		CustomException except("Error when the Spectran Interface tried to make the Log-Out sound.");
		throw(except);
	}
}
