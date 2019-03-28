/*
 * AntennaPositioning.h
 *
 *  Created on: 21/03/2019
 *      Author: new-mauro
 */

/*! \file AntennaPositioning.h
 * \brief This file contains the declarations of classes AntennaPositioner and GPSInterface.
 *
 * This header files includes the following libraries:
 * - iostream: cin, cout.
 * - libnmea: parsers for the GPRMC and GPGGA messages
 * - string: the USB protocol of GPS receiver is text-based.
 * - ftd2xx: communication with the FTDI IC which is in the GPS receiver.
 *
 * The namespace *std* is used to simplify the uses of objects like cout and cin.
 */

#ifndef ANTENNAPOSITIONING_H_
#define ANTENNAPOSITIONING_H_

#include <iostream>
#include <nmea.h> //Library libnmea
#include <nmea/gprmc.h> //parser of GPRMC messages
#include <nmea/gpgga.h> //parser of GPGGA messages
#include <string>
#include <ftd2xx.h> //FTDI library
#include <sstream> //stringstream
#include <fstream> //filestream
#include <unistd.h> //usleep
#include <vector>
#include <cmath> //atan2, M_PI
#include <dirent.h> //To get filenames

using namespace std;

//! A structure intended to save the the tri-axial values of the 3D sensors which are integrated in the GPS receiver.
typedef struct
{
	enum SensorType : char { GYROSCOPE, COMPASS, BAROMETER, ACCELEROMETER, TEMPERATURE, UNINITIALIZED };
	SensorType sensor;
	string time;
	double x;
	double y;
	double z;
} Data3D;

//! A structure which saves the GPS coordinates.
typedef struct
{
	//!Both quantities are in decimal degrees and they are negatives in western and southern hemispheres
	double latitude;
	double longitude;
} GPSCoordinates;

///////////////////////Classes//////////////////////////
//!Class CustomException derived from standard class exception
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

//!It is intended to establish the communication with the Aaronia GPS receiver, to request and capture messages from this and extract useful data from messages
class GPSInterface
{
	///////////Attributes///////////
	//Constants
	const DWORD VID = 0x0403, PID = 0xE8DB;
	const string DEVICE_DESCRIPTION = "Aaronia GPS Logger A";
	const DWORD RD_TIMEOUT_US = 50000, WR_TIMEOUT_US = 10000;
	const unsigned int READ_DELAY_US = 200000;
	const unsigned int ACCRANGE = 2; //+-2g
	const unsigned int GYRO_FILTERFREQ = 4;
	const unsigned int GYRO_FILTERDIV = 1;
	const unsigned int DATARATE = 20; //T = 50 mS
	const string SENSORS_FILES_PATH = "/home/new-mauro/RFIMS-CART/gps/";
	const unsigned int MIN_NUM_OF_SATELLITES = 1;
	//Variables
	FT_HANDLE ftHandle;
	FT_STATUS ftStatus;
	ofstream sensorFile;
	bool flagConnected;
	string timestamp; //date & time
	GPSCoordinates coordinates;
	unsigned int numOfSatellites;
	unsigned int gpsElevation; //over sea level, measured in meters (m) and based on GPS data
	Data3D gyroData;
	Data3D compassData;
	Data3D accelData;
	double yaw;
	double pitch;
	double roll;
	float pressure; //measured in hPa
	float presElevation; //over sea level, measured in meters (m) and based on pressure.
	///////Private Methods//////////
	inline void Write(const string& command);
	inline void Read(string& reply, unsigned int numOfBytes=0);
	inline bool ControlChecksum(const string& reply);
	void ConfigureVariable(const string& variable, unsigned int value);
	inline void CalculateCardanAngles();
	unsigned int FindAndCheckDataReply(const vector<string>& replies, const string& replyType, char sensor='\0');
	inline void SaveSensorsData();
	void ProcessDataReplies(vector<string>& replies);
public:
	///////Class Interface//////////
	GPSInterface();
	~GPSInterface();
	void Initialize();
	unsigned int Available();
	void ReadOneDataSet();
	//void EnableStreaming();
	//void CaptureStreamData();
	void DisableStreaming();
	void Purge();
	string GetTimestamp() const {	return timestamp;	}
	GPSCoordinates GetCoordinates() const {	return coordinates;	}
	unsigned int GetNumOfSatellites() const {	return numOfSatellites;	}
	float GetGPSElevation() const {	return gpsElevation;	}
	Data3D GetGyroData() const {	return gyroData;	}
	Data3D GetCompassData() const {	return compassData;	}
	Data3D GetAccelerData() const {	return accelData;	}
	float GetPressure() const {	return pressure;	}
	float GetPressureElevation() const {	return presElevation;	}
	double GetYaw() const {		return yaw;		}
	double GetRoll() const {	return roll;	}
	double GetPitch() const {	return pitch;	}
	bool IsConnected() const {	return flagConnected;	}
};


#endif /* ANTENNAPOSITIONING_H_ */
