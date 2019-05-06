/*
 * RFIMS_CART.h
 *
 *  Created on: 02/04/2019
 *      Author: new-mauro
 */

#ifndef RFIMS_CART_H_
#define RFIMS_CART_H_

//#define RASPBERRY_PI

#include <iostream> //cout, cin
#include <exception>
#include <vector>
#include <string>
#include <sstream> //ostringstream, istringstream
#include <iomanip>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <cmath>
#include <fstream>
#include <unistd.h>
#include <cstdint>
#ifdef RASPBERRY_PI
#include <wiringPi.h>
#endif

using std::cout;
using std::cerr;
using std::cin;
using std::endl;

//!Class CustomException derived from standard class exception.
class CustomException : public std::exception
{
	std::string message;
public:
	CustomException(const std::string& msg="Error") : message(msg) {}
	void SetMessage(const std::string& msg) {	message=msg;	}
	void Append(const std::string& msg){		message+=msg;	}
	virtual const char * what() const throw(){	return message.c_str();	}
};

struct TimeData
{
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;
	TimeData() {	year=month=day=hour=minute=second=0;	}
	TimeData(const TimeData& timeData) {	*this=timeData;		}
	std::string date() const
	{
		std::ostringstream oss;
		oss.fill('0'); oss.setf(std::ios::right, std::ios::adjustfield);
		oss << std::setw(2) << day << '-' << std::setw(2) << month << '-' << year;
		return oss.str();
	}
	std::string time() const
	{
		std::ostringstream oss;
		oss.fill('0'); oss.setf(std::ios::right, std::ios::adjustfield);
		oss << std::setw(2) << hour << ':' << std::setw(2) << minute << ':' << std::setw(2) << second;
		return oss.str();
	}
	std::string timestamp() const {		return ( date() + 'T' + time() );	}
	const TimeData& operator=(const TimeData& anotherTimeData)
	{
		year=anotherTimeData.year; month=anotherTimeData.month; day=anotherTimeData.day;
		hour=anotherTimeData.hour; minute=anotherTimeData.minute; second=anotherTimeData.second;
		return *this;
	}
};

struct FreqValues
{
	std::string type; //!< ”sweep”, “frequency response”, “calibration curve”, “threshold curve” or “rfi x”, where 'x' is a positive integer number
	std::vector<float> values; //!< RF power (dBm) or gain (dB or dBi)
	std::vector<float> frequencies; //!< Frequency values in Hz.
	TimeData timeData; //!< Timestamp with the following format: DD-MM-YYYYTHH:MM:SS (where the word T separates the date and time)
	FreqValues(const std::string& typ="sweep") : type(typ) {}
	FreqValues(const FreqValues& freqValues) {	*this=freqValues;	}
	virtual ~FreqValues() {}
	bool PushBack(const FreqValues& FreqValues);
	virtual void Clear();
	bool Empty() const {	return values.empty();		}
	const FreqValues& operator=(const FreqValues & freqValues);
	const FreqValues& operator+=(const FreqValues& rhs);
	friend FreqValues operator-(const FreqValues& argument);
	friend FreqValues operator+(const FreqValues & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator+(const FreqValues & lhs, const std::vector<float> & rhs);
	friend FreqValues operator+(const std::vector<float> & lhs, const FreqValues & rhs);
	friend FreqValues operator+(const FreqValues & lhs, const float rhs); //defined in FreqValues.cpp
	friend FreqValues operator+(const float lhs, const FreqValues & rhs);
	friend FreqValues operator-(const FreqValues & lhs, const FreqValues & rhs);
	friend FreqValues operator-(const FreqValues & lhs, const float rhs);
	friend FreqValues operator-(const float lhs, const FreqValues & rhs);
	friend FreqValues operator*(const FreqValues & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator*(const float lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator*(const FreqValues & lhs, const float rhs);
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
	Sweep() : FreqValues("sweep") {	azimuthAngle=0; }
	void Clear() {	FreqValues::Clear(); azimuthAngle=0; polarization.clear();	}
	const Sweep & operator=(const Sweep & freqValues)
	{
		FreqValues::operator=(freqValues);
		azimuthAngle = freqValues.azimuthAngle;
		polarization = freqValues.polarization;
		return *this;
	}
	friend Sweep operator+(const Sweep & lhs, const Sweep & rhs);
	friend Sweep operator+(const Sweep & lhs, const std::vector<float> & rhs);
	friend Sweep operator+(const std::vector<float> & lhs, const Sweep & rhs);
};

struct BandParameters
{
	bool flagEnable; //It determines if the band is used or not
	float startFreq;
	float stopFreq;
	float rbw; //resolution bandwidth
	float vbw; //video bandwidth
	unsigned long int sweepTime;
	bool flagDefaultSamplePoints; //It determines if the sample points number must be configured with next parameter or
								//if it is left with its default value which is determined by the Spectran device.
	unsigned int samplePoints; //Number of samples points. The value determined by the Spectran device (default value) or the forced value.
	unsigned int detector; //”rms”(0) or “min/max”(1)
};

//Functions intended to compare floating-point numbers
bool approximatelyEqual(float a, float b);
bool approximatelyEqual(std::vector<float> vectorA, std::vector<float> vectorB);

#endif /* RFIMS_CART_H_ */
