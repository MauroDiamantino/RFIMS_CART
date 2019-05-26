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

#include <nmea.h> //Library libnmea
#include <nmea/gprmc.h> //parser of GPRMC messages
#include <nmea/gpgga.h> //parser of GPGGA messages
#include <ftd2xx.h> //FTDI library
#include <dirent.h> //To get filenames
#include "Basics.h"

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
	const DWORD RD_TIMEOUT_US = 50000, WR_TIMEOUT_US = 10000;
	const unsigned int READ_DELAY_US = 200000;
	const unsigned int ACCRANGE = 2; //+-2g
	const unsigned int GYRO_FILTERFREQ = 4;
	const unsigned int GYRO_FILTERDIV = 1;
	const unsigned int DATARATE = 20; //T = 50 mS
#ifdef RASPBERRY_PI
	const std::string SENSORS_FILES_PATH = "/home/pi/RFIMS-CART/gps/";
#else
	const std::string SENSORS_FILES_PATH = "/home/new-mauro/RFIMS-CART/gps/";
#endif
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
	inline void Write(const std::string& command);
	inline void Read(std::string& reply, unsigned int numOfBytes=0);
	inline bool ControlChecksum(const std::string& reply);
	void ConfigureVariable(const std::string& variable, unsigned int value);
	inline void CalculateCardanAngles();
	unsigned int FindAndCheckDataReply(const std::vector<std::string>& replies, const std::string& replyType, char sensor='\0');
	inline void SaveSensorsData();
	void ProcessDataReplies(std::vector<std::string>& replies);
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
	TimeData GetTimeData() const {	return timeData;	}
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

enum Polarization : char { HORIZONTAL, VERTICAL, UNKNOWN };

//!The aim of this class is to drive the antenna positioning
class AntennaPositioner
{
	//enum Polarization : char { HORIZONTAL, VERTICAL, UNKNOWN };
	/////////Attributes//////////
	//Constants
	struct
	{
		const int LED_INIT_POS = 1;
		const int BUTTON_INIT_POS = 2;
		const int LED_NEXT_POS = 3;
		const int BUTTON_NEXT_POS = 4;
		const int LED_POLARIZ = 5;
		const int BUTTON_POLARIZ = 6;
	} piPins;
	const std::uint8_t NUM_OF_POSITIONS = 8;
	const std::uint8_t ROTATION_ANGLE = 360/NUM_OF_POSITIONS;
	//Variables
	GPSInterface & gpsInterface;
	float azimuthAngle;
	Polarization polarization;
	std::uint8_t positionIndex;
public:
	AntennaPositioner(GPSInterface & gpsInterf);
	bool Initialize();
	bool NextAzimPosition();
	bool ChangePolarization();
	float GetAzimPosition() {	return azimuthAngle;	}
	std::string GetPolarizationString();
	Polarization GetPolarization() {	return polarization;	}
	unsigned int GetPositionIndex()	{	return positionIndex;	}
	unsigned int GetNumOfPositions() {	return NUM_OF_POSITIONS;	}
	bool IsLastPosition() {	return ( positionIndex >= (NUM_OF_POSITIONS-1) );	}
};

#endif /* ANTENNAPOSITIONING_H_ */
