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
 * - libnmea: parsers for the GPRMC and GPGGA messages
 * - string: the USB protocol of GPS receiver is text-based.
 * - ftd2xx: communication with the FTDI IC which is in the GPS receiver.
 */

#ifndef ANTENNAPOSITIONING_H_
#define ANTENNAPOSITIONING_H_

#include "Basics.h"

#include <nmea.h> //Library libnmea
#include <nmea/gprmc.h> //parser of GPRMC messages
#include <nmea/gpgga.h> //parser of GPGGA messages
#include <ftd2xx.h> //FTDI library
#include <dirent.h> //To get filenames

//! A structure intended to save the the tri-axial values of the 3D sensors which are integrated in the GPS receiver.
typedef struct
{
	enum SensorType : char { GYROSCOPE, COMPASS, BAROMETER, ACCELEROMETER, TEMPERATURE, UNINITIALIZED };
	SensorType sensor;
	std::string time;
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

//!It is intended to establish the communication with the Aaronia GPS receiver, to request and capture messages from this and extract useful data from messages
class GPSInterface
{
	///////////Attributes///////////
	//Constants
	const DWORD VID = 0x0403, PID = 0xE8DB;
	const std::string DEVICE_DESCRIPTION = "Aaronia GPS Logger A";
	const DWORD RD_TIMEOUT_MS = 500, WR_TIMEOUT_MS = 1000;
	const unsigned int READ_DELAY_US = 200000;
	const unsigned int ACCRANGE = 2; //+-2g
	const unsigned int GYRO_FILTERFREQ = 4;
	const unsigned int GYRO_FILTERDIV = 1;
	const unsigned int DATARATE = 20; //T = 50 mS
	const std::string SENSORS_FILES_PATH = BASE_PATH + "/gps/";
	const unsigned int MIN_NUM_OF_SATELLITES = 3;
	//Variables
	FT_HANDLE ftHandle;
	FT_STATUS ftStatus;
	std::ofstream sensorFile;
	bool flagConnected;
	TimeData timeData; //date & time
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
	void Write(const std::string& command);
	void Read(std::string& reply, const unsigned int numOfBytes=0);
	bool ControlChecksum(const std::string& reply);
	void ConfigureVariable(const std::string& variable, const unsigned int value);
	void CalculateCardanAngles();
	unsigned int FindAndCheckDataReply(const std::vector<std::string>& replies, const std::string& replyType, const char sensor='\0');
	void SaveSensorsData();
	void ProcessDataReplies(const std::vector<std::string>& replies);
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
	const TimeData & GetTimeData() const {	return timeData;	}
	const GPSCoordinates & GetCoordinates() const {	return coordinates;	}
	unsigned int GetNumOfSatellites() const {	return numOfSatellites;	}
	float GetGPSElevation() const {	return gpsElevation;	}
	const Data3D & GetGyroData() const {	return gyroData;	}
	const Data3D & GetCompassData() const {	return compassData;	}
	const Data3D & GetAccelerData() const {	return accelData;	}
	float GetPressure() const {	return pressure;	}
	float GetPressureElevation() const {	return presElevation;	}
	double GetYaw() const {		return yaw;		}
	double GetRoll() const {	return roll;	}
	double GetPitch() const {	return pitch;	}
	bool IsConnected() const {	return flagConnected;	}
};


enum Polarization : char { HORIZONTAL, VERTICAL, UNKNOWN };

//!The aim of this class is to drive the antenna positioning
class AntennaPositioner
{
	/////////Attributes//////////
	//Constants
	const std::uint8_t NUM_OF_POSITIONS = 6;
	const double ROTATION_ANGLE = 360.0/NUM_OF_POSITIONS;
	//Variables
	GPSInterface & gpsInterface;
	float azimuthAngle;
	Polarization polarization;
	std::uint8_t positionIndex;
public:
	AntennaPositioner(GPSInterface & gpsInterf);
	~AntennaPositioner() {}
	bool Initialize();
	bool NextAzimPosition();
	bool ChangePolarization();
	float GetAzimPosition() const {		return azimuthAngle;	}
	std::string GetPolarizationString() const;
	Polarization GetPolarization() const {	return polarization;	}
	unsigned int GetPositionIndex() const	{	return positionIndex;	}
	unsigned int GetNumOfPositions() const {	return NUM_OF_POSITIONS;	}
	bool IsLastPosition() const {	return ( positionIndex >= (NUM_OF_POSITIONS-1) );	}
};

#endif /* ANTENNAPOSITIONING_H_ */
