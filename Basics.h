/*
 * Basics.h
 *
 *  Created on: 02/04/2019
 *      Author: new-mauro
 */

#ifndef BASICS_H_
#define BASICS_H_

//#define RASPBERRY_PI
//#define DEBUG
#ifdef RASPBERRY_PI
	//#define BUTTON
#endif

//! The must-have library which allows to use the objects `std::cout` and `std::cin` among a lot of other things.
#include <iostream>
//! It is included to use base class `std::exception`. A specific class, called CustomException, is derived from that base class to manage the exceptions.
#include <exception>
//! It is included to use the famous container `std::vector`.
#include <vector>
//! This library is included to make use of the famous container `std::string`.
#include <string>
//! It is included to state the classes `std::ostringstream` and `std::istringstream` are being used, but it is unnecessary if the *iostream* library has already bee included.
#include <sstream>
//! *iomanip* is a library which allows to use parametric manipulators to modify the way things (like numbers) are plotted with `std::cout`, `std::cerr`, etc.
#include <iomanip>
//! This library is included to make use the function `boost::algorithm::to_lower()` to convert upper-case characters to lower-case ones.
#include <boost/algorithm/string.hpp>
//! This library allows to use several functions and classes to perform tasks related to the filesystem such as creating a new file, see a directory's content, remove a file, etc.
#include <boost/filesystem.hpp>
//! A library which allows to perform task related with dates, such as getting the date of 30 days ago from the current date. Classes `boost::gregorian::date()` and `boost::gregorian::date_duration()` are used.
#include <boost/date_time/gregorian/gregorian.hpp>

#include <cmath>
#include <fstream>
#include <unistd.h>
#include <cstdint>
#include <cstdlib>
#ifdef RASPBERRY_PI
#include <wiringPi.h>
#endif

using std::cout;
using std::cerr;
using std::cin;
using std::endl;

#ifdef RASPBERRY_PI
const std::string BASE_PATH = "/home/pi/RFIMS-CART";
#else
const std::string BASE_PATH = "/home/new-mauro/RFIMS-CART";
#endif


////////////////////////////GLOBAL CLASSES AND STRUCTURES//////////////////////

//!Class CustomException derived from standard class exception.
class CustomException : public std::exception
{
	std::string message;
public:
	CustomException(const std::string& msg="Error") : message(msg) {}
	void SetMessage(const std::string& msg) {	message=msg;	}
	void Append(const std::string& msg){		message+=msg;	}
	const char * what() const throw(){	return message.c_str();	}
};

struct TimeData
{
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;
	TimeData() {	Clear();	}
	TimeData(const TimeData& timeData) {	operator=(timeData);	}
	std::string GetDate() const;
	std::string GetTime() const;
	std::string GetTimestamp() const {		return ( GetDate() + 'T' + GetTime() );		}
	void SetDate(const std::string & date);
	void SetTime(const std::string & time);
	void SetTimestamp(const std::string & timestamp);
	void TurnBackDays(const unsigned int days);
	const TimeData& operator=(const TimeData& anotherTimeData);
	void Clear() {	year=month=day=hour=minute=second=0;	}
	////////Friends functions//////////
	friend bool operator<(const TimeData & lhs, const TimeData & rhs); //defined in TimeData.cpp
	friend bool operator>(const TimeData & lhs, const TimeData & rhs); //defined in TimeData.cpp
	friend bool operator==(const TimeData & lhs, const TimeData & rhs); //defined in TimeData.cpp
};

struct FreqValues
{
	std::string type; //!< ”sweep”, “frequency response”, “calibration curve”, “threshold curve”, “rfi", etc
	std::vector<float> values; //!< RF power values (dBm), gain values (dB or dBi), noise figure values (dB), etc
	//std::vector<float> frequencies; //!< Frequency values in Hz.
	std::vector<std::uint_least64_t> frequencies; //!< Frequency values in Hz.
	TimeData timeData; //!< A TimeData object which contains all the information about the time. This object builds the timestamp in the format DD-MM-YYTHH:MM:SS
	FreqValues(const std::string& typ="unknown") : type(typ) {}
	FreqValues(const FreqValues& freqValues) {	operator=(freqValues);	}
	virtual ~FreqValues() {}
	bool PushBack(const FreqValues& freqValues);
	virtual void Clear();
	bool Empty() const {	return values.empty();	}
	const FreqValues& operator=(const FreqValues & freqValues);
	const FreqValues& operator+=(const FreqValues& rhs);
	float MeanValue() const;
	////////Friends functions///////
	friend FreqValues operator-(const FreqValues& argument); //defined in FreqValues.cpp
	friend FreqValues operator+(const FreqValues & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator+(const FreqValues & lhs, const float rhs); //defined in FreqValues.cpp
	friend FreqValues operator+(const float lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator-(const FreqValues & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator-(const FreqValues & lhs, const float rhs); //defined in FreqValues.cpp
	friend FreqValues operator-(const float lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator*(const FreqValues & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator*(const float lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator*(const FreqValues & lhs, const float rhs); //defined in FreqValues.cpp
	friend FreqValues operator/(const FreqValues & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator/(const float lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator/(const FreqValues & lhs, const float rhs); //defined in FreqValues.cpp
	friend FreqValues log10(const FreqValues & argument); //decimal logarithm, defined in FreqValues.cpp
	friend FreqValues pow(const FreqValues & base, const float exponent); //exponentiation, defined in FreqValues.cpp
	friend FreqValues pow(const float base, const FreqValues & exponent); //exponentiation, defined in FreqValues.cpp
};

struct Sweep : public FreqValues
{
	float azimuthAngle;
	std::string polarization;
	Sweep() : FreqValues("sweep") {	azimuthAngle=0.0; }
	Sweep(const FreqValues & freqValues) : FreqValues(freqValues) { azimuthAngle=0.0; }
	Sweep(const Sweep & sweep) {	operator=(sweep);		}
	void Clear() {	FreqValues::Clear(); azimuthAngle=0.0; polarization.clear();	}
	const Sweep & operator=(const Sweep & sweep);
	friend Sweep operator-(const Sweep & argument); //defined in FreqValues.cpp
	friend Sweep operator+(const Sweep & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator+(const Sweep & lhs, const std::vector<float> & rhs); //defined in FreqValues.cpp
	friend Sweep operator+(const std::vector<float> & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator+(const Sweep & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend Sweep operator+(const Sweep & lhs, const float rhs); //defined in FreqValues.cpp
	friend Sweep operator+(const float lhs, const Sweep & rhs); //defined in FreqValues.cpp
	//friend Sweep operator+(const FreqValues & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator-(const Sweep & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator-(const Sweep & lhs, const std::vector<float> & rhs); //defined in FreqValues.cpp
	friend Sweep operator-(const std::vector<float> & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator-(const Sweep & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	//friend Sweep operator-(const FreqValues & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator*(const Sweep & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator*(const float lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator*(const Sweep & lhs, const float rhs); //defined in FreqValues.cpp
	friend Sweep operator/(const Sweep & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator/(const float lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator/(const Sweep & lhs, const float rhs); //defined in FreqValues.cpp
	friend Sweep log10(const Sweep & argument); //defined in FreqValues.cpp
	friend Sweep pow(const Sweep & base, const float exponent); //exponentiation, defined in FreqValues.cpp
	friend Sweep pow(const float base, const Sweep & exponent); //exponentiation, defined in FreqValues.cpp
};

struct RFI : public FreqValues
{
	//! Enumeration which contains the sources of harmful RF interference levels (aka thresholds): the ITU recommendation RA.769-2, SKA protocol Mode 1, SKA protocol Mode 2.
	enum ThresholdsNorm {ITU_RA769, SKA_MODE1, SKA_MODE2};
	float azimuthAngle;
	std::string polarization;
	unsigned int numOfRFIBands;
	ThresholdsNorm threshNorm;
	RFI() : FreqValues("rfi") { azimuthAngle=0.0; numOfRFIBands=0; threshNorm=ThresholdsNorm::SKA_MODE1;	}
	RFI(const RFI & rfi) {	operator=(rfi);		}
	void Clear() { 	FreqValues::Clear(); azimuthAngle=0.0; numOfRFIBands=0; polarization.clear();	}
	const RFI & operator=(const RFI & anotherRFI)
	{
		azimuthAngle=anotherRFI.azimuthAngle; polarization=anotherRFI.polarization;
		numOfRFIBands=anotherRFI.numOfRFIBands; threshNorm=anotherRFI.threshNorm;
		frequencies=anotherRFI.frequencies; timeData=anotherRFI.timeData; values=anotherRFI.values;
		return *this;
	}
};

struct BandParameters
{
	unsigned int bandNumber;
	bool flagEnable; //!< This parameter determines if the band is used or not.
	float startFreq; //!< Fstart in Hz.
	float stopFreq; //!< Fstop in Hz.
	float rbw; //!< Resolution BandWidth in Hz.
	float vbw; //!< Video BandWidth in Hz.
	unsigned int sweepTime; //!< Time to sweep the given span, expresed in ms.
	bool flagDefaultSamplePoints; //!< This parameter determines if the sample points number must be configured with next parameter or if it is left with its default value which is determined by the Spectran device.
	unsigned int samplePoints; //!< Number of samples points. The value determined by the Spectran device (default value) or the forced value.
	unsigned int detector; //!< Display detector: ”RMS” takes the sample as the root mean square of the values present in the bucket, or “Min/Max” takes two samples as the minimum and maximum peaks.
};

#ifdef RASPBERRY_PI
struct
{
	const unsigned int SWITCH = 8; //!< This pin is initialized as an input with the pull-up resistor (2K) enabled, so the RF switch output will start connected to the noise source.
	const unsigned int NOISE_SOURCE = 13; //!< This pin is initialized as an input with the pull-down resistor enabled, so the noise source will start turned off.
	const unsigned int LNAS = 12;  //!< This pin is initialized as an input with the pull-down resistor enabled, so the LNAs will start turned off.
	const unsigned int SPECTRAN = 14; //!< This pin is initialized as an input with the pull-down resistor enabled, so the Spectran device will start turned off.
	const unsigned int LED_SWEEP_CAPTURE = 9;
	const unsigned int LED_SWEEP_PROCESS = 10;
	const unsigned int LED_INIT_POS = 1;
	const unsigned int LED_NEXT_POS = 3;
	const unsigned int LED_POLARIZ = 5;
	const unsigned int BUTTON_ENTER = 2;
}piPins;
const int SWITCH_TO_NS = HIGH; //!< The noise source output must be connected to switch's J2 connector
const int SWITCH_TO_ANTENNA = LOW; //!< The antenna must be connected to switch's J1 connector
#endif

///////////////////////END OF GLOBAL CLASSES AND STRUCTURES//////////////////


///////////////////////GLOBAL FUNCTIONS//////////////////////

//Functions intended to compare floating-point numbers
bool approximatelyEqual(float a, float b);
bool approximatelyEqual(std::vector<float> vectorA, std::vector<float> vectorB);

std::vector<float> operator-(const std::vector<float> & vect); //defined in Basics.cpp

void WaitForKey();

void InitializeGPIO();

void TurnOnFrontEnd();
void TurnOffFrontEnd();

void PrintHelp();
bool ProcessMainArguments(int argc, char * argv[]);

//////////////////////END OF GLOBAL FUNCTIONS//////////////////


/////////////////////////GLOBAL VARIABLES///////////////////////

//! Flags which are defined by the software arguments and which indicates the way the software must behave
extern bool flagCalEnabled, flagPlot, flagInfiniteLoop, flagRFI;
//! A variable which saves the number of measurements cycles which left to be done. It is used when the user wishes a finite number of measurements cycles.
extern unsigned int numOfMeasCycles;
//! A variable which saves the norm which defines the harmful RF interference levels: ska-mode1, ska-mode2, itu-ra769
extern RFI::ThresholdsNorm rfiNorm;

//////////////////////////END OF GLOBAL VARIABLES///////////////

#endif /* BASICS_H_ */
