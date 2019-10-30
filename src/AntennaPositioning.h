/*
 * AntennaPositioning.h
 *
 *  Created on: 21/03/2019
 *      Author: new-mauro
 */

/*! \file AntennaPositioning.h
 * 	\brief This file contains the declarations of classes AntennaPositioner and GPSInterface.
 *
 *	These classes are responsible for the positioning of the antenna, which can rotate its azimuth
 *	angle and its polarization. To model these rotations it were used the Cardan angles (or nautical angles):
 *	the "yaw" angle which represents the heading, the "pitch" angle which represents the elevation and the
 *	"roll" angle which models the bank angle. The yaw angle is used to represent the antenna's azimuth angle,
 *	the roll angle is used to represent the antenna polarization and the pitch angle is not used.
 *	\image html cardan_angles.png "The Cardan or nautical angles"
 *	\image latex cardan_angles.eps "The Cardan or nautical angles" width=10cm
 *	Moreover, the class GPSInterface allows to get data related with time and date, elevation (based on
 *	pressure), ambient pressure, among other things.
 *
 *	Also, several structures are defined here which store data related with the mentioned classes.
 *	\author Mauro Diamantino
 */

#ifndef ANTENNAPOSITIONING_H_
#define ANTENNAPOSITIONING_H_

#include "Basics.h"

#include <nmea.h> //Library "libnmea", which simplify the parsing of the GPS data which follows the NMEA 0180 syntax.
#include <nmea/gprmc.h> //Parser of GPRMC sentences.
#include <nmea/gpgga.h> //Parser of GPGGA sentences.
#include <ftd2xx.h> //FTDI library, for the communication with the GPS receiver
#include <dirent.h> //To get filenames

//! A structure intended to save the the values of the 3d sensors which are integrated in the GPS receiver.
typedef struct
{
	//! An enumeration with the names of the sensors integrated in the GPS receiver.
	enum SensorType : char { GYROSCOPE, COMPASS, ACCELEROMETER, UNINITIALIZED };
	SensorType sensor; //!< The specific sensor whose values are represented in the structure.
	std::string time; //!< The time when the measurement were made, in the format HHMMSS.NNN, where NNN is a GPS's internal counter that is reset when the time is updated by a new GPS data.
	double x; //!< The x-axis value.
	double y; //!< The y-axis value.
	double z; //!< The z-axis value.
} Data3D;

//! A structure which saves the GPS coordinates.
typedef struct
{
	double latitude; //!< The latitude angle represented in decimal degrees. This angle becomes negative in the southern hemisphere.
	double longitude; //!< The longitude angle represented in decimal degrees. This angle becomes negative in the western hemisphere.
} GPSCoordinates;


//! The class _GPSInterace_ is intended to establish the communication with the Aaronia GPS receiver, to request and capture messages from this one and extract useful data from the messages.
class GPSInterface
{
	//Attributes//
	//Constants
	const DWORD VID = 0x0403; //!< The Vendor ID of the Aaronia GPS receiver.
	const DWORD PID = 0xE8DB; //!< The Product ID of the Aaronia GPS receiver.
	const std::string DEVICE_DESCRIPTION = "Aaronia GPS Logger A"; //!< The "USB Device Description" of the Aaronia GPS receiver.
	const DWORD RD_TIMEOUT_MS = 500; //!< The timeout of the reading operations, in milliseconds (ms).
	const DWORD WR_TIMEOUT_MS = 1000; //!< The timeout of the writing operations, in milliseconds (ms).
	const unsigned int ACCELER_RANGE = 2; //!< The maximum range of the accelerometer measured in g (gravitational acceleration). It can be 2g, 4g or 8g.
	const unsigned int GYRO_FILTERFREQ = 4; //!< It is a number ranging from 0-7 to select the frequency of the average filter of the gyroscope sensor.
	const unsigned int GYRO_FILTERDIV = 1; //!< This number represents the divisor of the average filter for the gyroscope sensor, ranging from 0-255.
	const unsigned int DATARATE = 2; //!< The data rate of the GPS sensors data, measured in Hz. f=2 Hz, T=0.5s.
	const unsigned int MIN_NUM_OF_SATELLITES = 3; //!< The minimum number of satellites the GPS receiver must connect with, at initialization.
	//Variables
	FT_HANDLE ftHandle; //!< The handler of the communication with the Aaronia GPS receiver.
	FT_STATUS ftStatus; //!< A variable which is used to store the returned values of the D2XX functions.
	bool flagConnected; //!< A flag which states if the communication with the GPS receiver has been initialized.
	TimeData timeData; //!< The time data (date and time) which were received from the GPS satellites.
	GPSCoordinates coordinates; //!< The GPS coordinates which were received from the GPS satellites.
	unsigned int numOfSatellites; //!< The current number of satellites which the GPS receiver is connected with.
	unsigned int gpsElevation; //!< The elevation of the GPS receiver over the sea level, measured in meters (m) and based on GPS data.
	Data3D gyroData; //!< The processed 3D values of the gyroscope, measured in degrees/s.
	Data3D compassData; //!< The processed 3D values of the compass, measured in Gauss.
	Data3D accelData; //!< The processed 3D values of the accelerometer.
	//! The yaw angle, measured in degrees and whose range is 0 to 359. North corresponds to 0°, east to 90°, south to 180° and west to 270°.
	double yaw;
	//! The pitch angle, measured in degrees and whose range is -180 to 180. This angle is zero if the device is put on horizontal surface, positive if the elevation is over the surface and negative if the elevation is below the surface.
	double pitch;
	//! The roll angle, measured in degrees and whose range is -180 to 180. This angle is zero if the device is put on horizontal surface, positive if it turns right and negative if it turns left.
	double roll;
	float pressure; //!< The ambient pressure, measured in hectopascal (hPa).
	float presElevation; //!< The elevation of the GPS receiver over the sea level, measured in meters (m) and based on the ambient pressure.
	bool flagNewGPRMCData, flagNewGPGGAData;
	bool flagNewGyroData, flagNewCompassData, flagNewAccelerData,flagNewBaromData;
	bool flagNewTimeData, flagNewCoordinates, flagNewNumOfSatellites, flagNewGPSElevation;
	bool flagNewYaw, flagNewRoll, flagNewPitch;
	pthread_t threadID;
	bool flagStreamingEnabled;
	int streamingRetVal;

	//Private Methods//
	//! This method performs the writing operations.
	void Write(const std::string& command);
	//! This method performs the reading operations.
	void Read(std::string& reply, const unsigned int numOfBytes=0);
	//! This method returns the number of bytes in the input buffer.
	unsigned int Available();
	//! This method performs the checking of each reply, taking into account the checksum.
	bool ControlChecksum(const std::string& reply);
	//! This method checks if the status of a PAAG,DATA reply or a GPRMC reply is ok.
	bool ControlStatus(const std::string & reply);
	//! This method is intended to set an internal variable of the GPS receiver.
	void ConfigureVariable(const std::string& variable, const unsigned int value);
	//! This method sends a command to get just one set of data replies (GPS and sensors data) which are put in the vector received as an argument.
	void ReadOneDataSet(std::vector<std::string> & dataReplies);
	//! This method extracts the corresponding GPS data from a GPRMC reply.
	void ExtractGPRMCData(const std::string & reply);
	//! This method extracts the corresponding GPS data from a GPGAA reply.
	void ExtractGPGGAData(const std::string & reply);
	//! This method extracts the gyroscope data from the corresponding GPS Logger reply.
	void ExtractGyroData(const std::string & reply);
	//! This method extracts the 3D compass data from the corresponding GPS Logger reply.
	void ExtractCompassData(const std::string & reply);
	//! This method extracts the 3D accelerometer data from the corresponding GPS Logger reply.
	void ExtractAccelerData(const std::string & reply);
	//! This method extracts the barometer data from the corresponding GPS Logger reply.
	void ExtractBarometerData(const std::string & reply);
	//! The aim of this method is to calculate the yaw angle (one of the Cardan angles) from the 3D compass data.
	void CalculateYaw();
	//! The aim of this method is to calculate the pitch angle (one of the Cardan angles) from the 3D accelerometer data.
	void CalculatePitch();
	//! The aim of this method is to calculate the roll angle (one of the Cardan angles) from the 3D accelerometer data.
	void CalculateRoll();
public:
	//Class Interface//
	//! The default constructor of class GPSInterface.
	GPSInterface();
	//!The GPSInterface class' destructor.
	~GPSInterface();
	//! This method is intended to try the communication with the Aaronia GPS receiver and configure the device.
	void Initialize();
	//! This method is intended to enable the streaming of data from the GPS receiver.
	void EnableStreaming();
	//! This method is intended to disable the data streaming from the Aaronia GPS receiver.
	void DisableStreaming();
	//! A method intended to purge the input and output buffers of the USB interface
	void Purge();
	//! A method which reads the corresponding data reply to update the time data and returns it.
	TimeData UpdateTimeData();
	//! A method which reads the corresponding data reply to update the gyroscope data and returns them.
	Data3D UpdateGyroData();
	//! A method which reads the corresponding data reply to update the compass data and returns them.
	Data3D UpdateCompassData();
	//! A method which reads the corresponding data reply to update the accelerometer data and returns them.
	Data3D UpdateAccelerData();
	//! A method which reads the corresponding data replies to update the yaw angle and returns it.
	double UpdateYaw();
	//! A method which reads the corresponding data replies to update the roll angle and returns it.
	double UpdateRoll();
	//! A method which reads the corresponding data replies to update the pitch angle and returns it.
	double UpdatePitch();
	//! A method which reads the corresponding data reply to update the pressure and the pressure-based elevation.
	void UpdatePressAndElevat();
	//! A method which reads the corresponding data reply (GPGGA reply) to update the number of satellites and returns it.
	unsigned int UpdateNumOfSatellites();
	//! A method which reads all data replies to update all attributes.
	void UpdateAll();
	//! A method which allows to know if there are new time data.
	/*!	This method is mainly intended to be used with the streaming option. When a GPRMC reply is received and the
	 * 	time data are extracted from it, it is considered the time data are new data. Once the time data are got with
	 * 	the method GetTimeData(), then they are considered old data. */
	bool NewTimeData() const {	return flagNewTimeData;	}
	//! A method which allows to know if there are new GPS coordinates.
	/*!	This method is mainly intended to be used with the streaming option. When a GPRMC reply is received and the
	 * 	coordinates are extracted from it, it is considered the coordinates are new data. Once the GPS coordinates are
	 * 	got with the method GetCoordinates(), then they are considered old data. */
	bool NewCoordinates() const { return flagNewCoordinates;	}
	//! A method which allows to know if there is a new value of the number of satellites.
	/*!	This method is mainly intended to be used with the streaming option. When a GPGGA reply is received and the
	 * 	number of satellites is extracted from it, it is considered this number as a new data. Once the number of
	 * 	satellites is got with the method GetNumOfSatellites(), then it is considered old data.	 */
	bool NewNumOfSatellites() const {	return flagNewNumOfSatellites;	}
	//! A method which allows to know if there is a new value of the GPS-based elevation.
	/*!	This method is mainly intended to be used with the streaming option. When a GPGGA reply is received and the
	 * 	elevation is extracted from it, it is considered this number as a new data. Once the GPS-based elevation
	 * 	is got with the method GetGPSElevation(), then it is considered old data. */
	bool NewGPSElevation() const {	return flagNewGPSElevation;	}
	//! A method which allows to know if there are new gyroscope data.
	/*!	This method is mainly intended to be used with the streaming option. When a PAAG,DATA,G reply is received
	 * 	and the	gyroscope data are extracted from it, it is considered these data are new. Once the gyroscope
	 * 	data are got with the method GetGyroData(), then they are considered old data. */
	bool NewGyroData() const {	return flagNewGyroData;	}
	//! A method which allows to know if there are new compass data.
	/*!	This method is mainly intended to be used with the streaming option. When a PAAG,DATA,C reply is received
	 * 	and the compass data are extracted from it, it is considered these data are new. Once the compass data
	 * 	are got with the method GetCompassData(), then they are considered old data. */
	bool NewCompassData() const {	return flagNewCompassData;	}
	//! A method which allows to know if there are new accelerometer data.
	/*!	This method is mainly intended to be used with the streaming option. When a PAAG,DATA,T reply is received
	 * 	and the	accelerometer data are extracted from it, it is considered these data are new. Once the accelerometer
	 * 	data are got with the method GetAccelerData(), then they are considered old data. */
	bool NewAccelerData() const {	return flagNewAccelerData;	}
	//! A method which allows to know if there is a new value of pressure.
	/*!	This method is mainly intended to be used with the streaming option. When a PAAG,DATA,B reply is received
	 * 	and the	pressure is extracted from it, it is considered this data is new. Once the pressure value (or the
	 * 	pressure-based elevation) is got with the method GetPressure(), then it is considered old data. */
	bool NewPressure() const {	return flagNewBaromData;	}
	//! A method which allows to know if there is a new value of pressure.
	/*!	This method is mainly intended to be used with the streaming option. When a PAAG,DATA,B reply is received
	 * 	and the	pressure is extracted from it, it is considered this data is new. Once the pressure value (or the
	 * 	pressure-based elevation) is got with the method GetPressure(), then it is considered old data. */
	bool NewYaw() const {	return flagNewYaw;	}
	//! A method which allows to know if there is a new value of the roll angle.
	/*!	This method is mainly intended to be used with the streaming option. When the corresponding replies are
	 * 	received and the roll angle is calculated from their data, it is considered this value is new. Once this value
	 * 	is got with the method GetRoll(), then it is considered old data. */
	bool NewRoll() const {	return flagNewRoll;	}
	//! A method which allows to know if there is a new value of the pitch angle.
	/*!	This method is mainly intended to be used with the streaming option. When the corresponding replies are
	 * 	received and the pitch angle is calculated from their data, it is considered this value is new. Once this value
	 * 	is got with the method GetPitch(), then it is considered old data. */
	bool NewPitch() const {	return flagNewPitch;	}
	//! A method which allows to know if there are new GPS data.
	/*!	This method is mainly intended to be used with the streaming option. When the corresponding replies, GPRMC and GPGGA, are
	 * 	received and the GPS data (time, coordinates, etc.) are extracted from them, it is considered these data are new. Once these
	 * 	data are got with the corresponding methods, then they are considered old data. */
	bool NewGPSData() const {	return(flagNewGPRMCData && flagNewGPGGAData);	}
	//! This method returns the time data (date and time) which was received from the GPS satellites.
	const TimeData & GetTimeData() {	flagNewTimeData=false; if(!flagNewCoordinates) flagNewGPRMCData=false; return timeData;	}
	//! This method returns the GPS coordinates.
	const GPSCoordinates & GetCoordinates() {	flagNewCoordinates=false; if(!flagNewTimeData) flagNewGPRMCData=false; return coordinates;	}
	//! This method returns the current number of satellites which the GPS receiver is connected with.
	unsigned int GetNumOfSatellites() {	flagNewNumOfSatellites=false; if(!flagNewGPSElevation) flagNewGPGGAData=false;	return numOfSatellites;	}
	//! This method returns the elevation of the GPS receiver over the sea level, measured in meters (m) and based on GPS data.
	float GetGPSElevation() {	flagNewGPSElevation=false; if(!flagNewNumOfSatellites) flagNewGPGGAData=false; return gpsElevation;	}
	//! This method returns the 3D gyroscope data.
	const Data3D & GetGyroData() {	flagNewGyroData=false; return gyroData;	}
	//! This method returns the 3D compass data.
	const Data3D & GetCompassData() {	flagNewCompassData=false; return compassData;	}
	//! This method returns the 3D accelerometer data.
	const Data3D & GetAccelerData() {	flagNewAccelerData=false; return accelData; }
	//! This method returns the ambient temperature, measured in hectopascal (hPa).
	float GetPressure() {	flagNewBaromData=false; return pressure;	}
	//! This method returns the elevation of the GPS receiver over the sea level, measured in meters (m) and based on the ambient pressure.
	float GetPressElevation() {	flagNewBaromData=false; return presElevation; }
	//! This method returns the yaw angle, measured in degrees and whose range is 0 to 359. North corresponds to 0°, east to 90°, south to 180° and west to 270°.
	double GetYaw() {	flagNewYaw=false; return yaw;	}
	//! This method returns the roll angle, measured in degrees and whose range is -180 to 180. This angle is zero if the device is put on horizontal surface, positive if it turns right and negative if it turns left.
	double GetRoll() {	flagNewRoll=false; return roll;	}
	//! This method returns the pitch angle, measured in degrees and whose range is -180 to 180. This angle is zero if the device is put on horizontal surface, positive if the elevation is over the surface and negative if the elevation is below the surface.
	double GetPitch() {	flagNewPitch=false; return pitch;	}
	//! This method returns the data rate when the streaming is enabled.
	unsigned int GetDataRate() const { return DATARATE;	}
	//! This method states if the communication with the Aaronia GPS receiver has been initialized.
	bool IsConnected() const {	return flagConnected;	}
	//! This method states if the data streaming is enabled.
	bool IsStreamingEnabled() const {	return flagStreamingEnabled;	}

	//Friend functions//
	//! The function which is executed by the thread which reads each reply of the GPS Logger and extract the data from them.
	friend void *StreamingThread(void * arg);
};


//! An enumeration which contains the possible states of the antenna polarization: HORIZONTAL, VERTICAL or UNKNOWN.
enum Polarization : char { HORIZONTAL, VERTICAL, UNKNOWN };


//! The aim of the class _AntennaPositioner_ is to handle the antenna positioning system.
class AntennaPositioner
{
	//Attributes//
	//Constants
	//const std::uint8_t NUM_OF_POSITIONS = 6; //! The number of azimuth positions.
	//const double ROTATION_ANGLE = 360.0/NUM_OF_POSITIONS; //! The angle which the antenna must rotate to move to the next position.
	//Variables
	double rotationAngle;
	GPSInterface & gpsInterface; //! A reference to the object which is responsible for the communication with the Aaronia GPS receiver.
	float azimuthAngle; //! The current azimuth angle of the antenna.
	Polarization polarization; //! The current antenna polarization.
	std::uint8_t positionIndex; //! An index which allows to know how many positions have been swept and how many remains to reach the last position.
public:
	//! The unique constructor of the class _AntennaPositioner_.
	AntennaPositioner(GPSInterface & gpsInterf);
	//! The class destructor.
	/*!	Its implementation is empty because the attributes are implicitly destroyed. However, the
	 * destructor is defined here to allow this one to be called explicitly in any part of the code,
	 * what is used by the signals handler to destroy the objects when a signal to finish the execution
	 * of the software is received.
	 */
	~AntennaPositioner() {}
	//! This method performs the initialization of the antenna positioning system.
	bool Initialize();
	//! This method moves the antenna to the next azimuth position.
	bool NextAzimPosition();
	//! This method change the antenna polarization.
	bool ChangePolarization();
	//! This method returns the current antenna azimuth angle.
	float GetAzimPosition() const {		return azimuthAngle;	}
	//! This method returns the current antenna polarization, as a `std::string` object.
	std::string GetPolarizationString() const;
	//! This method returns the current antenna polarization, as a value of the enumeration [Polarization](\ref Polarization).
	Polarization GetPolarization() const {	return polarization;	}
	//! This method returns the current azimuth position index.
	unsigned int GetPositionIndex() const	{	return positionIndex;	}
	//! This method return the total number of azimuth positions.
	//unsigned int GetNumOfPositions() const {	return NUM_OF_POSITIONS;	}
	//! This method states if the current position is the last one.
	bool IsLastPosition() const {	return ( positionIndex >= (numOfAzimPos-1) );	}
};

#endif /* ANTENNAPOSITIONING_H_ */
