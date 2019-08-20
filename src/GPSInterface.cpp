/*! \file GPSInterface.cpp
 * 	\brief This file contains the definitions of several methods of the class _GPSInterface_.
 * 	\author Mauro Diamantino
 */

#include "AntennaPositioning.h"

///////////////Implementations of the GPSInterface class' methods///////////////

/*! The constructor has to include the custom VID and PID combination of Aaronia GPS receiver within the allowed
 * values, then it has to open the communication with the GPS receiver and set up the UART port on the
 * FTDI chip.
 */
GPSInterface::GPSInterface()
{
	//Initialization of attributes
	ftHandle=0;
	ftStatus=0;
	flagConnected=false;
	coordinates={ 0.0, 0.0 };
	numOfSatellites=0;
	gpsElevation=0.0;
	gyroData=compassData=accelData = { Data3D::UNINITIALIZED, "", 0.0, 0.0, 0.0 };
	yaw=0.0;
	pitch=0.0;
	roll=0.0;
	pressure=0.0;
	presElevation=0.0;

	ftStatus=FT_SetVIDPID(VID, PID);
	if(ftStatus!=FT_OK)
	{
		CustomException exc("The custom VID and PID combinations of the Aaronia GPS receiver could not be included.");
		throw(exc);
	}

	ftStatus=FT_OpenEx((PVOID)DEVICE_DESCRIPTION.c_str(), FT_OPEN_BY_DESCRIPTION, &ftHandle);
	if (ftStatus!=FT_OK)
	{
		CustomException exc;
		switch(ftStatus)
		{
		case 2:
			exc.SetMessage("The Aaronia GPS receiver was not found.");
			throw(exc);
		case 3:
			exc.SetMessage("The Aaronia GPS receiver could not be opened.");
			throw(exc);
		default:
			std::ostringstream oss;
			oss << "The function FT_OpenEx() returned a FT_STATUS value of " << ftStatus;
			exc.SetMessage( oss.str() );
			throw(exc);
		}
	}

	ftStatus=FT_SetTimeouts(ftHandle, RD_TIMEOUT_MS, WR_TIMEOUT_MS);
	if(ftStatus!=FT_OK)
	{
		CustomException exc("The read and write timeouts could not be set up.");
		throw(exc);
	}

	ftStatus=FT_SetBaudRate(ftHandle, 625000);
	if(ftStatus!=FT_OK)
	{
		if(ftStatus==7)
		{
			CustomException exc("The given value of baud rate is not valid.");
			throw(exc);
		}
		else
		{
			CustomException exc("The baud rate could not be set up");
			throw(exc);
		}
	}

	ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
	if(ftStatus!=FT_OK)
	{
		CustomException exc("The data characteristics could not be set up.");
		throw(exc);
	}

	ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0);
	if(ftStatus!=FT_OK)
	{
		CustomException exc("Error: the flow control could not be set up.");
		throw(exc);
	}

	DIR * dir;
	struct dirent * ent;
	std::string filename;
	bool flagFound=false;
	unsigned int index, maxIndex=0;
	std::istringstream iss;
	size_t posIndex, posPoint;

	dir=opendir( SENSORS_FILES_PATH.c_str() );
	if( dir==NULL )
	{
		CustomException exc("The directory where files with sensors data are saved could not be opened.");
		throw(exc);
	}

	while( (ent=readdir(dir)) != NULL )
	{
		filename = ent->d_name;
		if( filename.find("sensorsdata_")!=std::string::npos && filename.find(".txt")!=std::string::npos )
		{
			flagFound=true;
			posIndex = filename.find('_') + 1;
			posPoint = filename.find('.');
			iss.clear();
			iss.str( filename.substr(posIndex, posPoint-posIndex) );
			iss >> index;
			if(index > maxIndex)
				maxIndex = index;
		}
	}

	std::string pathAndName;
	if(flagFound)
	{
		std::ostringstream oss;
		oss << "sensorsdata_" << (maxIndex + 1) << ".txt" << endl;
		pathAndName = SENSORS_FILES_PATH + oss.str();
	}
	else
	{
		pathAndName = SENSORS_FILES_PATH + "sensorsdata_1.txt";
	}

	//Enabling exceptions for logical errors (failbit) and read/writing errors (badbit) on i/o operations
	sensorFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	//Associating the output file stream with the corresponding file where the sensors data will be stored
	sensorFile.open(pathAndName);
	sensorFile << "Timestamp,Gyroscope.x,Gyroscope.y,Gyroscope.z,Compass.x,Compass.y,Compass.z,Accelerometer.x,Accelerometer.y,Accelerometer.z,yaw,pitch,roll\n";

	Purge();
	usleep(300000);
}


/*! The destructor has to make sure that the data streaming and the data logging into the microSD card are stopped,
 * and then it closes the communication with the Aaronia GPS receiver.
 */
GPSInterface::~GPSInterface()
{
	cout << "Closing the communication with the GPS interface." << endl;

	//Disabling the data streaming
	try
	{
		DisableStreaming();
	}
	catch(std::exception& exc)
	{
		cerr << "Warning: The GPSInterface's destructor could not make sure the data streaming is disabled." << endl;
	}

	//Disabling the data logging into the microSD card
	std::string command("PAAG,FILE,STOP,\r\n");
	try
	{
		Write(command);
	}
	catch(std::exception& exc)
	{
		cerr << "Warning: The GPSInterface's destructor could not make sure the data logging into file is disabled." << endl;
	}

	//Purging the input and output buffers
	try
	{
		Purge();
	}
	catch(std::exception& exc)
	{
		cerr << exc.what();
	}

	//Closing the communication with the Aaronia GPS receiver
	ftStatus=FT_Close(ftHandle);
	if(ftStatus!=FT_OK)
	{
		cerr << "Error: the communication with the Aaronia GPS receiver could not be closed." << endl;
	}

	//Closing the file where the sensors data are stored
	sensorFile.close();
}

/*! This method takes a command and sends it to the Aaronia GPS receiver, using the D2XX library.
 * After the writing was performed, the method checks if there were errors and if all bytes were written.
 * \param [in] command A `std::string` object which contains the command to be sent.
 */
inline void GPSInterface::Write(const std::string& command)
{
	DWORD writtenBytes;
	DWORD numOfBytes = command.size();
	char txBuffer[numOfBytes];

	strcpy(txBuffer, command.c_str());

	ftStatus=FT_Write(ftHandle, txBuffer, numOfBytes, &writtenBytes);
	if(ftStatus!=FT_OK)
	{
		CustomException exc("The GPS interface tried to send a command but the function FT_Write() returned an error value.");
		throw(exc);
	}
	else if(writtenBytes!=numOfBytes)
	{
		CustomException exc("The GPS interface tried to send a command but not all bytes were written.");
		throw(exc);
	}
}

/*! This method reads a reply from the Aaronia GPS receiver, using the D2XX library, and stores it in a `std::string`
 * object which is passed as a non-const reference.
 *
 * This method has two parameters: the first one is a `std::string` object where the reply will be stored and the second
 * one is the minimum number of bytes the reply would have. The second parameter is compared with the number of available
 * bytes in an internal loop to ensure the reading does not fail. That query of available bytes is performed every 10 ms.
 * The loop ends when the needed bytes are available, and then a small interval time of 30 ms is waited as a precaution,
 * or it ends when a determined number of iterations is reached, what it is interpreted as an error. When the number of
 * bytes is not given, i.e. the second parameter takes the value zero, the method will just wait a fixed interval time
 * of 150 ms to ensure the needed bytes are available. After of that, the reading operation is performed inside a loop
 * where it is read one byte at a time until the '\n' character is read.
 * \param [in] reply A `std::string` object where the will store the reply of the GPS receiver.
 * \param [in] numOfBytes The minimum number of bytes the reply would have.
 */
inline void GPSInterface::Read(std::string& reply, const unsigned int numOfBytes)
{
	DWORD receivedBytes;
	char rxBuffer;

	if(numOfBytes!=0)
	{
		//Loop to wait for the bytes
		unsigned int i=0;
		while(Available()<numOfBytes && i++ < 20)
			usleep(10000); //10ms

		if(i>=20)
		{
			CustomException exc("The data set (GPRMC, GPGGA and PAAG replies) was waited too much time.");
			throw(exc);
		}
		usleep(30000); //An additional delay to wait for some additional bytes
	}
	else
	{
		//A fixed time interval to wait for the bytes
		usleep(150000);
	}

	//Loop where the characters are read until a '\n' character is read
	do
	{
		ftStatus=FT_Read(ftHandle, &rxBuffer, 1, &receivedBytes);
		if(ftStatus!=FT_OK)
		{
			CustomException exc("The GPS interface tried to read a reply, but the function FT_Read() returned an error value.");
			throw(exc);
		}
		else if(receivedBytes!=1)
		{
			CustomException exc("The GPS interface tried to read a reply, but one character could not be read.");
			throw(exc);
		}
		reply += rxBuffer;
	}while(rxBuffer!='\n');
}

/*! The checksum is at the end of each reply, after an asterisk (*), and it is a 2 character hexadecimal
 *	representation of the XOR combination of all characters between the $ (all replies and commands start
 *	with this symbol) and the * (as defined by the	NMEA protocol).
 * 	\param [in] reply The reply to be checked.
 */
inline bool GPSInterface::ControlChecksum(const std::string& reply)
{
	unsigned int receivedChecksum, calculChecksum = 0;
	unsigned int i=1; //the operation starts with the second element

	while(reply[i]!='*')
		calculChecksum ^= reply[i++];

	size_t checksumPos = reply.find('*') + 1;
	std::string checksumString = reply.substr(checksumPos, 2);
	sscanf(checksumString.c_str(), "%x", &receivedChecksum);

	if(receivedChecksum==calculChecksum)
		return true;

	return false;
}

/*! This method simplifies the operation of set an internal variable of the GPS variable. The possible
 * 	variables are the following:
 * 	- ACCRANGE: accelerometer range, 2g, 4g or 8g.
 * 	- FILTERFREQ: a number ranging from 0-7 to select the frequency of the average filter of the gyroscope sensor.
 * 	- FILTERDIV: the divisor of the average filter for the gyroscope sensor, ranging from 0-255.
 * 	- DATARATE: the data rate of the GPS sensors data, measured in Hz.
 *
 *	The method builds the corresponding command, performs the writing of this one, then it reads the reply,
 *	controls the checksum (calling ControlChecksum()) and, finally, it checks if the current value of the
 *	variable is the desired value.
 *
 *	To get more info about the commands to set variables and their replies, see the "Aaronia GPS Logger
 *	Programming Guide" document.
 *	\param [in] variable The name of the variable (upper case) to be set.
 *	\param [in] value The value which must be used to set the given variable.
 */
void GPSInterface::ConfigureVariable(const std::string& variable, const unsigned int value)
{
	//Setting up a GPS receiver's variable
	std::stringstream ss;
	ss << "$PAAG,VAR," << variable << ',' << value << "\r\n";
	std::string command( ss.str() );
	std::string reply;
	try
	{
		Write(command);
		Read(reply);
	}
	catch(CustomException& exc)
	{
		exc.Append("\nThe setting of the variable " + variable + " failed.");
		throw;
	}

	if( !ControlChecksum(reply) )
	{
		CustomException exc( "The reply of the command to set up the variable " + variable + " was wrong." );
		throw(exc);
	}
	size_t valuePos = reply.find(variable) + variable.size() + 1; //The position of the current value of the accelerometer range
	size_t delPos = reply.find('*'); //The position of the delimiter *
	std::string valueString = reply.substr(valuePos, delPos-valuePos);
	ss.clear();
	ss.str(valueString);
	unsigned int currentValue;
	ss >> currentValue;
	if(currentValue!=value)
	{
		std::ostringstream oss;
		oss << "The reply of the command to set up the variable " << variable;
		oss << " stated that the range was not configured with the desired value, " << value;
		CustomException exc( oss.str() );
		throw(exc);
	}
}


inline void GPSInterface::CalculateCardanAngles()
{
	yaw = atan2(compassData.y, compassData.x) * 180.0/M_PI;

	if( (yaw+=180.0) >= 360.0 )
		yaw -= 360.0;

	pitch = -atan2( accelData.y, sqrt(pow(accelData.x,2) + pow(accelData.z,2)) ) * 180.0/M_PI;
	roll = atan2( -accelData.x, (accelData.z<0 ? -1 : 1) * sqrt(pow(accelData.y,2) + pow(accelData.z,2)) ) * 180.0/M_PI;
}

/*! Also, this method controls the checksum of the desired reply. The method throws exception when the checksum is wrong
 * and when the desired reply was not found.
 */
unsigned int GPSInterface::FindAndCheckDataReply(const std::vector<std::string>& replies, const std::string& replyType, const char sensor)
{
	std::string header;
	if(replyType=="PAAG")
	{
		header = "$PAAG,DATA,";
		header += sensor;
	}
	else
	{
		header = '$';
		header += replyType;
	}

	unsigned int i=0;
	while( replies[i].find(header)==std::string::npos && i<replies.size() )
		i++;

	if( i>=replies.size() )
	{
		CustomException exc;
		if(sensor=='\0')
		{
			exc.SetMessage("The reply " + replyType + " was not found between the data replies.");
		}
		else
		{
			exc.SetMessage("The reply " + replyType + " with the data of sensor " + sensor + " was not found between the data replies.");
		}
		throw(exc);
	}

	if( !ControlChecksum(replies[i]) )
	{
		CustomException exc("The PAAG reply with the gyroscope data was wrong because the checksum was wrong.");
		throw(exc);
	}

	return i;
}

/*! This method is intended for debugging purposes. */
void GPSInterface::SaveSensorsData()
{
	//Timestamp,Gyroscope.x,Gyroscope.y,Gyroscope.z,Compass.x,Compass.y,Compass.z,Accelerometer.x,Accelerometer.y,Accelerometer.z,yaw,pitch,roll\n

	sensorFile << timeData.GetTimestamp() << ',' << gyroData.x << ',' << gyroData.y << ',' << gyroData.z << ',';
	sensorFile << compassData.x << ',' << compassData.y << ',' << compassData.z << ',';
	sensorFile << accelData.x << ',' << accelData.y << ',' << accelData.z << ',';
	sensorFile << yaw << ',' << pitch << ',' << roll << '\n';
}

void GPSInterface::ProcessDataReplies(const std::vector<std::string>& replies)
{
	nmea_s * data;
	std::stringstream ss;
	unsigned int i;
	char * auxString;

	//Searching GPRMC reply
	i = FindAndCheckDataReply(replies, "GPRMC");

	auxString = new char[replies[i].size()+10];
	strcpy(auxString, replies[i].c_str());

	//Parsing GPRMC reply
	data = nmea_parse(auxString, replies[i].size(), 0);
	if(data==NULL)
	{
		CustomException exc("It was not possible to parse the GPRMC reply.");
		throw(exc);
	}

	nmea_gprmc_s * gprmc = (nmea_gprmc_s*) data;

	timeData.day = gprmc->time.tm_mday; timeData.month = gprmc->time.tm_mon+1; timeData.year = gprmc->time.tm_year+1900;
	timeData.hour = gprmc->time.tm_hour-3; timeData.minute = gprmc->time.tm_min; timeData.second = gprmc->time.tm_sec;

	coordinates.latitude = double(gprmc->latitude.degrees + gprmc->latitude.minutes/60.0) * (gprmc->latitude.cardinal=='S' ? -1.0 : 1.0);
	coordinates.longitude = double(gprmc->longitude.degrees + gprmc->longitude.minutes/60.0) * (gprmc->longitude.cardinal=='W' ? -1.0 : 1.0);

	//Searching GPGGA reply
	i = FindAndCheckDataReply(replies, "GPGGA");

	//Parsing GPGGA reply
	data = nmea_parse((char*)replies[i].c_str(), replies[i].size(), 0);
	if(data==NULL)
	{
		CustomException exc("It was not possible to parse the GPGGA reply.");
		throw(exc);
	}
	nmea_gpgga_s * gpgga = (nmea_gpgga_s*) data;
	gpsElevation = gpgga->altitude;
	numOfSatellites = gpgga->n_satellites;

	delete[] auxString;

	//Searching PAAG reply with Gyroscope data
	i = FindAndCheckDataReply(replies, "PAAG", 'G');

	//Parsing PAAG reply with Gyroscope data
	size_t statusPos = replies[i].find('*') - 1;
	if( replies[i].at(statusPos)!='A' )
	{
		CustomException exc("The PAAG reply with the gyroscope data stated an invalid status.");
		throw(exc);
	}
	size_t xPos = replies[i].find(',', 13) + 1;
	char delimiter[2];
	ss.clear();
	std::string aux = replies[i].substr(xPos, statusPos-xPos-1);
	ss.str( aux );
	ss >> gyroData.x >> delimiter[0] >> gyroData.y >> delimiter[1] >> gyroData.z;
	//Scaling the gyroscope data so they are in degrees/s
	gyroData.x /= 14.375;
	gyroData.y /= 14.375;
	gyroData.z /= 14.375;

	//Searching PAAG reply with compass data
	i = FindAndCheckDataReply(replies, "PAAG", 'C');

	//Parsing PAAG reply with compass data
	statusPos = replies[i].find('*') - 1;
	if( replies[i].at(statusPos)!='A' )
	{
		CustomException exc("The PAAG reply with the compass data stated an invalid status.");
		throw(exc);
	}
	xPos = replies[i].find(',', 13) + 1;
	ss.clear();
	aux.clear();
	aux = replies[i].substr(xPos, statusPos-xPos-1);
	ss.str( aux );
	ss >> compassData.x >> delimiter[0] >> compassData.y >> delimiter[1] >> compassData.z;
	//Scaling the compass data so they are in Gauss
	compassData.x /= 1090.0;
	compassData.y /= 1090.0;
	compassData.z /= 1090.0;

	//Searching PAAG reply with accelerometer data
	i = FindAndCheckDataReply( replies, "PAAG", 'T');

	//Parsing the PAAG reply with the accelerometer data
	statusPos = replies[i].find('*') - 1;
	if( replies[i].at(statusPos)!='A' )
	{
		CustomException exc("The PAAG reply with the accelerometer data stated an invalid status.");
		throw(exc);
	}
	xPos = replies[i].find(',', 13) + 1;
	ss.clear();
	aux.clear();
	aux = replies[i].substr(xPos, statusPos-xPos-1);
	ss.str( aux );
	ss >> accelData.x >> delimiter[0] >> accelData.y >> delimiter[1] >> accelData.z;
	//Scaling the accelerometer data
	accelData.x /= 8192.0;
	accelData.y /= 8192.0;
	accelData.z /= 8192.0;

	//Searching PAAG reply with the barometer data
	i = FindAndCheckDataReply(replies, "PAAG", 'B');

	//Parsing PAAG reply with the barometer data
	statusPos = replies[i].find('*') - 1;
	if( replies[i].at(statusPos)!='A' )
	{
		CustomException exc("The PAAG reply with the barometer data stated an invalid status.");
		throw(exc);
	}
	size_t valuePos = replies[i].find(',', 13) + 1;
	size_t nextDelimPos = replies[i].find(',', valuePos);
	ss.clear();
	aux.clear();
	aux = replies[i].substr(valuePos, nextDelimPos-valuePos);
	ss.str( aux );
	ss >> pressure;
	//Applying the formula from the Portland State Aerospace Society (PSAS) to calculate the elevation from atmosphere pressure.
	presElevation = ( pow( pressure/1013.25, -(-6.5e-3 * 287.053)/9.8 ) - 1.0 ) * 295.0 / -6.5e-3;

	//Calculating the Cardan angles: yaw, pitch and roll
	CalculateCardanAngles();

	SaveSensorsData();

	nmea_free(data);
}

void GPSInterface::Purge()
{
	//The input and output buffers are purged
	ftStatus=FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
	if (ftStatus!=FT_OK){
		CustomException except("The GPS interface failed when it tried to purge the input and output buffers.");
		throw(except);
	}
}

/*! First, this method tries the communication with an ID command, and if the reply is right it shows the hardware
 * version, firmware version and protocol version and then it disables the data streaming and the data logging
 * into the microSD card. After, the GPS interface set up the following variables: data rate of streaming,
 * accelerometer range and bandwidth of the average filter of the gyroscope sensor.
 */
void GPSInterface::Initialize()
{
	//Disabling the data streaming
	try
	{
		DisableStreaming();
	}
	catch(CustomException& exc)
	{
		exc.Append("\nThe GPS interface could not disable the streaming at initialization.");
		throw;
	}

	//Trying the communication with an ID command
	std::string command("$PAAG,ID\r\n");
	std::string reply;
	bool flagSuccess=false;
	unsigned int i=0;
	while(flagSuccess==false)
	{
		try
		{
			Write(command);
			Read(reply);

			if( !ControlChecksum(reply) )
			{
				CustomException exc("The reply of the ID command was wrong.");
				throw(exc);
			}

			//Showing the reply of the ID command
			std::string hardVersion, firmVersion, protocolVersion;
			size_t pos[4]; //an array with the positions of the elements after the delimiters (comma and asterisk)
			pos[0] = reply.find(',', 6) + 1;
			pos[1] = reply.find(',', pos[0]) + 1;
			pos[2] = reply.find(',', pos[1]) + 1;
			pos[3] = reply.find('*', pos[2]) + 1;
			hardVersion = reply.substr(pos[0], pos[1]-pos[0]-1);
			firmVersion = reply.substr(pos[1], pos[2]-pos[1]-1);
			protocolVersion = reply.substr(pos[2], pos[3]-pos[2]-1);
			cout << "The GPS receiver replied correctly. Its features are:" << endl;
			cout << "\tHardware version:" << hardVersion;
			cout << "\tFirmware version:" << firmVersion;
			cout << "\tProtocol version:" << protocolVersion << endl;

			flagSuccess=true;
		}
		catch(CustomException& exc)
		{
			cerr << "Warning: one of the ID commands failed: " << exc.what() << endl;
			if(++i >= 3)
			{
				CustomException exc2("\nThe ID commands failed: ");
				exc2.Append( exc.what() );
				throw(exc2);
			}
		}
	}

	//Disabling the data logging into the microSD card
	command.clear();
	command = "PAAG,FILE,STOP,\r\n";
	try
	{
		Write(command);
	}
	catch(CustomException& exc)
	{
		exc.Append("\nThe GPS interface could not disable the data logging into the microSD card.");
		throw;
	}

	//Setting up the GPS receiver's variables
	std::string variable;
	//Accelerometer range
	variable = "ACCRANGE";
	ConfigureVariable(variable, ACCELER_RANGE);

	//Gyroscope's average filter frequency
	variable.clear();
	variable = "FILTERFREQ";
	ConfigureVariable(variable, GYRO_FILTERFREQ);

	//Gyroscope's filter divisor
	variable.clear();
	variable = "FILTERDIV";
	ConfigureVariable(variable, GYRO_FILTERDIV);

	//Data rate for streaming
	variable.clear();
	variable = "DATARATE";
	ConfigureVariable(variable, DATARATE);

	//The loop where is waited the GPS receiver has established communication with a minimum number of satellites
	cout << "Waiting for the GPS receiver to get connected with at least " << MIN_NUM_OF_SATELLITES << " satellites..." << endl;
	do
	{
		sleep(1); //1s
		std::string command("$PAAG,MODE,READONE\r\n");
		std::vector<std::string> dataReplies(7);
		try
		{
			Write(command);
			for(unsigned int i=0; i<7; i++)
				Read(dataReplies[i]);
		}
		catch(CustomException& exc)
		{
			exc.Append("\nThe GPS interface could not determine if the GPS receiver had established communication with a minimum number of satellites.");
			throw;
		}
		unsigned int i = FindAndCheckDataReply(dataReplies, "GPGGA");
		//Parsing GPGGA reply
		nmea_s * data = nmea_parse((char*)dataReplies[i].c_str(), dataReplies[i].size(), 0);
		if(data==NULL)
		{
			CustomException exc("It was not possible to parse the GPGGA reply when the GPS interface was determining the number of satellites at initialization.");
			throw(exc);
		}
		nmea_gpgga_s * gpgga = (nmea_gpgga_s*) data;
		numOfSatellites = gpgga->n_satellites;
	}while( numOfSatellites < MIN_NUM_OF_SATELLITES );

	Purge();
	usleep(300000);

	flagConnected=true;
}

/*! It uses the function FT_GetQueueStatus() of the D2XX library. */
unsigned int GPSInterface::Available()
{
	DWORD numOfInputBytes, numOfOutputBytes, numOfEvents;
	//ftStatus=FT_GetQueueStatus(ftHandle, &numOfInputBytes);
	ftStatus=FT_GetStatus(ftHandle, &numOfInputBytes, &numOfOutputBytes, &numOfEvents);
	if(ftStatus!=FT_OK)
	{
		CustomException exc("The GPS interface failed when it tried to determine the number of bytes in the input buffer.");
		throw(exc);
	}
	return numOfInputBytes;
}

void GPSInterface::ReadOneDataSet()
{
	std::string command("$PAAG,MODE,READONE\r\n");
	try
	{
		Write(command);
	}
	catch(CustomException& exc)
	{
		exc.Append("\nThe GPS interface failed when it tried to send a command to get just one data set.");
		throw;
	}

	std::vector<std::string> dataReplies(7);
	try
	{
		for(unsigned int i=0; i<7; i++)
		{
			Read(dataReplies[i]);
		}
		ProcessDataReplies(dataReplies);
	}
	catch(CustomException& exc)
	{
		exc.Append("\nThe GPS interface failed when it tried to read the data replies or when it tried to process those replies.");
		throw;
	}
}

////! This method is intended to enable the streaming of data from the GPS receiver.
///*! After the streaming has been enable the method CaptureStreamData() must be used to get the stream data
// * and update the class attributes from them.
// */
//void GPSInterface::EnableStreaming()
//{
//	Purge();
//
//	std::string command("$PAAG,MODE,START\r\n");
//	try
//	{
//		Write(command);
//	}
//	catch(CustomException& exc)
//	{
//		exc.Append("\nThe command to enable the data streaming failed.");
//		throw;
//	}
//}
//
////! This method is intended to capture the continuous flow of data (stream) which sent from the Aaronia GPS receiver (when the streming is enabled).
///*! When this method is called the GPS interface reads one set of data from the USB interface and calls the method
// * ProcessDataReplies(), which extract the data from the replies and update the class attributes from them.
// */
//void GPSInterface::CaptureStreamData()
//{
//	//Waiting for the next data set
//	usleep( (__useconds_t)1/DATARATE );
//
//	std::vector<std::string> dataReplies(7);
//	try
//	{
//		ReadDataReplies(dataReplies);
//		ProcessDataReplies(dataReplies);
//	}
//	catch(CustomException& exc)
//	{
//		exc.Append("\nThe GPS interface failed when it tried to capture the stream data or when it tried to process them.");
//		throw;
//	}
//}

void GPSInterface::DisableStreaming()
{
	std::string command("$PAAG,MODE,STOP\r\n");
	try
	{
		Write(command);
	}
	catch(CustomException& exc)
	{
		exc.Append("\nThe command to disable the data streaming failed.");
		throw;
	}

	Purge();
	usleep(30000);
}
