/*!	\file SpectranInterface.cpp
 * 	\brief This file contains the definitions of several methods of the class _SpectranInterface_.
 * 	\author Mauro Diamantino
 */

#include "Spectran.h"

/*! This constructor initializes the internal flags, flagLogIn and flagSweepsEnabled, as false, includes the VID and PID of the
 * 	spectrum analyzer in the list of possible values and, finally, it calls the method `OpenAndSetUp()`.
 */
SpectranInterface::SpectranInterface()
{
	flagLogIn=false;
	flagSweepsEnabled=false;

	//The pair of values (VID,PID) of the Spectran HF-60105 V4 X are included in the list of possible values.
	ftStatus=FT_SetVIDPID(VID, PID);

	if (ftStatus!=FT_OK)
	{
		rfims_exception except("the Spectran Interface could not include the pair of values (PID,VID) of the Spectran device in the list of possible values.");
		throw(except);
	}
	else
		OpenAndSetUp();
}

/*!	The communication with the spectrum analyzer is opened, taken into account the device description, and then, the communication's
 * 	parameters are set up:
 * 	- baud rate, with the maximum value 921600 bits/s, taking into account the constants defined in the header file ftd2xx.h.
 * 	- reading and writing timeouts, taking into account the constants USB_RD_TIMEOUT_MS and USB_WR_TIMEOUT_MS.
 * 	- data flow control: none.
 * 	- data characteristics: 8 bits per word, 2 stop bits and no parity.
 * 	- the latency timer: 2 ms.
 * 	- special characters (event and error characters): disabled.
 * 	- Transfer size for USB IN request: 4096 bytes.
 */
void SpectranInterface::OpenAndSetUp()
{
	ftStatus=FT_OpenEx((PVOID)DEVICE_DESCRIPTION.c_str(), FT_OPEN_BY_DESCRIPTION, &ftHandle);

	if (ftStatus!=FT_OK)
	{
		rfims_exception except;
		switch(ftStatus)
		{
		case 2:
			except.SetMessage("the Spectran device was not found.");
			throw(except);
		case 3:
			except.SetMessage("the Spectran device could not be opened.");
			throw(except);
		default:
			std::ostringstream oss;
			oss << "the function FT_OpenEx(), of the D2XX library, returned a ftStatus value of " << ftStatus << '.';
			except.SetMessage( oss.str() );
			throw(except);
		}
	}
	else
	{
		//Setting up the FTDI IC//
		ftStatus=FT_SetTimeouts(ftHandle, USB_RD_TIMEOUT_MS, USB_WR_TIMEOUT_MS);
		if (ftStatus!=FT_OK)
			throw rfims_exception("the read and write timeouts could not be set up.");

		ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0);
		if(ftStatus!=FT_OK)
			throw rfims_exception("the flow control could not be set up.");

		ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
		if(ftStatus!=FT_OK)
			throw rfims_exception("the data characteristics could not be set up.");

		ftStatus = FT_SetBaudRate(ftHandle, BAUD_RATE);
		if(ftStatus!=FT_OK)
			throw rfims_exception("the baud rate could not be set up.");

		ftStatus = FT_SetLatencyTimer(ftHandle, 2);
		if(ftStatus!=FT_OK)
			throw rfims_exception("the latency timer could not be set up to 2ms.");

		ftStatus = FT_SetChars(ftHandle, 0, 0, 0, 0);
		if(ftStatus!=FT_OK)
			throw rfims_exception("the special characters could not be disabled.");

		ftStatus = FT_SetUSBParameters(ftHandle, 4096, 0);
		if(ftStatus!=FT_OK)
			throw rfims_exception("the USB request transfer size could not be set up.");
	}
}

/*! The destructor carry out the logging out and the communication closing. In each operation,
 * this method produces messages in the `stdout` if there were errors, but it never produces exceptions.
 */
SpectranInterface::~SpectranInterface()
{
	cout << "\nClosing the communication with the Spectran device." << endl;

	try
	{
		LogOut();
	}
	catch(std::exception & exc)
	{
		cerr << "\nError: the logging out failed during destruction of object SpectranInterface: " << exc.what() << endl;
	}

	ftStatus = FT_Close(ftHandle);
	if(ftStatus!=FT_OK)
		cerr << "Error: the communication with the Spectran device could not be closed." << endl;
}

/*! Firstly, this method sends two VERIFY commands to log in with the spectrum analyzer. If that operation failed, it resets the
 *	spectrum analyzer in a hard way (turn it off and then turn it on), then it tries again to send two VERIFY commands and if that
 *	failed it retries the reset and repeat this up to three times.
 *
 *	Secondly, it makes sure the streaming of sweep points is disabled. And finally, it set up the speaker volume, produces the login sounds
 *	and purge the input and output buffers.
 */
void SpectranInterface::Initialize()
{
	Reply reply;
	unsigned int errorCounter=0, retryCounter=0;
	bool flagSuccess=false;
	Command command(Command::VERIFY);
	do
	{
		//Sending of two VERIFY commands
		do
		{
			try
			{
				Write(command);
				reply.Clear();
				reply.PrepareTo(Reply::VERIFY);
				Read(reply);
				if( reply.IsRight() )
					flagSuccess=true;
			}
			catch (std::exception& exc)
			{
				//cerr << "Warning: one of the two commands VERIFY to initialize the communication failed." << endl;
				errorCounter++;
			}
		}while(flagSuccess==false && errorCounter<2); //Just one error is accepted

		if(flagSuccess==false)
		{
			if(++retryCounter<3) //Just three retry are accepted
			{
				cerr << "Warning: Two commands VERIFY failed but the Spectran interface will reset the device and try again." << endl;
				//Turn off the device then turn it back on
				HardReset();
			}
			else
				throw rfims_exception("the Spectran interface could not initialize the communication with the Spectran device.");
		}
	}while(flagSuccess==false);

	//It is tried to disable again the transmission of measurements from the spectrum analyzer.
	DisableSweep();

	//Setting the speaker volume to its default value.
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
		cerr << "Warning: the setting of the speaker volume to the value of " << DEFAULT_SPK_VOLUME << " failed: " << exc.what() << endl;
	}

	if (reply.IsRight()!=true)
		cerr << "Warning: the reply to the command to set the speaker volume was wrong." << endl;

	flagLogIn=true; //To remember the session has been started

	SoundLogIn(); //To state that the communication was initialized

	//The input buffer is purged
	try
	{
		Purge();
	}
	catch(rfims_exception & exc)
	{
		exc.Prepend("the purging failed during initialization");
		throw;
	}
}

unsigned int SpectranInterface::Available()
{
	DWORD numOfInputBytes;
	ftStatus=FT_GetQueueStatus(ftHandle, &numOfInputBytes);
	//DWORD numOfOutputBytes, events;
	//ftStatus=FT_GetStatus(ftHandle, &numOfInputBytes, &numOfOutputBytes, &events);
	if (ftStatus!=FT_OK)
		throw rfims_exception("the Spectran interface could not read the number of bytes in the receive queue.");

	return numOfInputBytes;
}

/*!	The reset of the current sweep has sense when the streaming of sweep points is enabled.
 */
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
			throw rfims_exception("a wrong reply was received.");
	}
	catch(rfims_exception & exc)
	{
		exc.Prepend("the command to reset the current sweep failed");
		throw;
	}
}

/*!	Take into account that once the streaming the sweep points is enabled, the spectrum analyzer will send these points autonomously,
 * 	i.e. the communication will not follow anymore the master-slave paradigm where the slave only sends data to the master when this
 * 	sends a request. So the software must be prepared to receive and process the data stream.
 */
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
			throw rfims_exception("the reply to the command to enable the sending of measurements via USB was wrong.");
	}
	catch(rfims_exception & exc)
	{
		exc.Prepend("the enabling of the sending of measurements via USB failed");
		throw;
	}

	flagSweepsEnabled=true;
}

/*! To ensure the streaming is disabled at least two commands  (USMEAS, 0) must be sent because the first one will
 * 	receive a wrong reply as there is a delay until the spectrum analyzer stops sending sweep points (AMPFREQDAT),
 * 	so the reply to first command will be mixed with the sweep points and the software will read it with errors.
 * 	Because of that, this method is implemented with a loop where it is tried to disable the streaming times,
 * 	up to 4 times. After the streaming is disabled successfully, the input buffer is purged.
 */
void SpectranInterface::DisableSweep()
{
	Command comm(Command::SETSTPVAR, SpecVariable::USBMEAS, 0.0);
	Reply reply;
	unsigned int errorCounter=0;
	bool flagSuccess=false;

	do
	{
		try
		{
			Write(comm);
			reply.Clear();
			reply.PrepareTo(Reply::SETSTPVAR);
			Read(reply);
			if(reply.IsRight()!=true)
				throw rfims_exception("the reply to the command to disable the sending of measurements via USB was wrong.");

			flagSuccess=true;
		}
		catch(rfims_exception & exc)
		{
			//cerr << "Warning: one of the commands to disable the sending of measurements via USB failed." << endl;

			if(++errorCounter < 5)
			{
				Purge();
				usleep(500000);
			}
			else
			{
				exc.Prepend("the disabling of the sending of measurements via USB failed");
				throw;
			}
		}
	}while(flagSuccess==false);

	Purge();
	usleep(500000);

	flagSweepsEnabled=false;
}

void SpectranInterface::Purge()
{
	//The input buffer is purged
	ftStatus=FT_Purge(ftHandle, FT_PURGE_RX);
	if (ftStatus!=FT_OK)
		throw rfims_exception("the Spectran Interface failed when it tried to purge the input buffer.");
}

/*! First, a logout is performed, then the communication is restarted using the function FT_ResetDevice() and,
 * 	finally, the method sleeps 3 seconds. After calling this method the communication must be initialized again
 * 	with the method Initialize().
 */
void SpectranInterface::SoftReset()
{
	LogOut();

	ftStatus=FT_ResetDevice(ftHandle);
	if(ftStatus!=FT_OK)
		throw rfims_exception("the USB device could not be restarted.");

	sleep(3);
}

/*!	To start, a logout is performed, then the communication is closed, the entire RF front end is turned off sequentially,
 * 	then it is waited 10 seconds, the entire front end is turned on again sequentially, and finally the communication is
 * 	opened again. After the calling of this method, a login must be performed calling the method Initialize().
 */
void SpectranInterface::HardReset()
{
	LogOut();

	ftStatus = FT_Close(ftHandle);
	if(ftStatus!=FT_OK)
		cerr << "Error: The communication with the Spectran device could not be closed." << endl;

	TurnOffFrontEnd();

	sleep(10);

	TurnOnFrontEnd();

	OpenAndSetUp();
}

/*! To ensure the logout can be carried out successfully, first, the streaming of sweep points is disabled, then
 * 	the logout sound is performed and, finally, the command to logout is sent to the spectrum analyzer.
 */
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
	catch(rfims_exception& exc)
	{
		exc.Prepend("the command LOGOUT failed");
		throw;
	}

	flagLogIn=false;
}

/*!	The login sounds are two short consecutive beeps and its duration is determined by the constant LOG_IN_SOUND_DURATION.
 * 	This sounds are produced by the internal speaker of the spectrum analyzer.
 */
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

/*!	The logout sound is just one long beep and its duration is determined by the constant LOG_OUT_SOUND_DURATION.
 * 	This sound is produced by the internal speaker of the spectrum analyzer.
 */
void SpectranInterface::SoundLogOut()
{
	Command command(Command::SETSTPVAR, SpecVariable::STDTONE, LOG_OUT_SOUND_DURATION);
	Reply reply(Reply::SETSTPVAR);
	try
	{
		Write(command);
		Read(reply);
		if( reply.IsRight()!=true )
			throw rfims_exception("the reply to the command to produce the beep was wrong.");
	}
	catch(std::exception& exc)
	{
		cerr << "Warning: the command to produce the log-out sound failed: " << exc.what() << endl;
	}
}

/*! The new-sweep sound is compound of just one pulse whose duration is determined by the constant NEW_SWEEP_SOUND_DURATION.
 * 	This method should be called by the object which is responsible for the capture of the sweep points and the building of
 * 	the entire sweep.
 */
void SpectranInterface::SoundNewSweep()
{
	Command comm(Command::SETSTPVAR, SpecVariable::STDTONE, NEW_SWEEP_SOUND_DURATION);
	Reply reply(Reply::SETSTPVAR);
	try
	{
		Write(comm);
		Read(reply);
		if(reply.IsRight()!=true)
			throw rfims_exception("the reply to the command to make the beep was wrong.");
	}
	catch(rfims_exception & exc)
	{
		cerr << "Warning: the command to produce the new-sweep sound failed: " << exc.what() << endl;
	}
}
