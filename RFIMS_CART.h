/*
 * RFIMS_CART.h
 *
 *  Created on: 02/04/2019
 *      Author: new-mauro
 */

#ifndef RFIMS_CART_H_
#define RFIMS_CART_H_

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
	TimeData()
	{
		year=month=day=hour=minute=second=0;
	}
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
	std::string timestamp() const {	return ( date() + 'T' + time() );	}
	const TimeData& operator=(const TimeData& anotherTimeData)
	{
		year=anotherTimeData.year; month=anotherTimeData.month; day=anotherTimeData.day;
		hour=anotherTimeData.hour; minute=anotherTimeData.minute; second=anotherTimeData.second;
		return *this;
	}
};

struct FreqValueSet
{
	std::string type; //!< ”sweep”, “frequency response”, “calibration curve”, “threshold curve” or “rfi x”, where 'x' is a positive integer number
	unsigned int index; //!< A positive integer number associated with a detected RFI
	std::vector<float> values; //!< RF power (dBm) or gain (dB or dBi)
	std::vector<float> frequencies; //!< Frequency values in Hz.
	TimeData timeData; //!< Timestamp with the following format: DD-MM-YYYYTHH:MM:SS (where the word T separates the date and time)
	FreqValueSet(const std::string& typ="sweep", unsigned int ind=0) : type(typ), index(ind) {}
	FreqValueSet(const FreqValueSet& freqValueSet);
	void PushBack(const FreqValueSet& freqValueSet);
	void Clear() { values.clear(); frequencies.clear();	}
	bool Empty() const {	return values.empty();		}
	const FreqValueSet& operator=(const FreqValueSet & freqValueSet);
	const FreqValueSet& operator+=(const FreqValueSet& rhs);
	friend FreqValueSet operator-(const FreqValueSet& argument);
	friend FreqValueSet operator+(const FreqValueSet & lhs, const FreqValueSet & rhs); //defined in FreqValueSet.cpp
	friend FreqValueSet operator+(const FreqValueSet & lhs, const float rhs); //defined in FreqValueSet.cpp
	friend FreqValueSet operator+(const float lhs, const FreqValueSet & rhs);
	friend FreqValueSet operator-(const FreqValueSet & lhs, const FreqValueSet & rhs);
	friend FreqValueSet operator-(const FreqValueSet & lhs, const float rhs);
	friend FreqValueSet operator-(const float lhs, const FreqValueSet & rhs);
	friend FreqValueSet operator*(const FreqValueSet & lhs, const FreqValueSet & rhs); //defined in FreqValueSet.cpp
	friend FreqValueSet operator*(const float lhs, const FreqValueSet & rhs); //defined in FreqValueSet.cpp
	friend FreqValueSet operator*(const FreqValueSet & lhs, const float rhs);
	friend FreqValueSet operator/(const FreqValueSet & lhs, const FreqValueSet & rhs); //defined in FreqValueSet.cpp
	friend FreqValueSet operator/(const float lhs, const FreqValueSet & rhs); //defined in FreqValueSet.cpp
	friend FreqValueSet operator/(const FreqValueSet & lhs, const float rhs); //defined in FreqValueSet.cpp
	friend FreqValueSet log10(const FreqValueSet & argument); //decimal logarithm, defined in FreqValueSet.cpp
	friend FreqValueSet pow(const FreqValueSet & base, const float exponent); //exponentiation, defined in FreqValueSet.cpp
	friend FreqValueSet pow(const float base, const FreqValueSet & exponent); //exponentiation, defined in FreqValueSet.cpp
};

struct BandParameters
{
	bool flagEnable; //It determines if the band is used or not
	float startFreq;
	float stopFreq;
	float rbw; //resolution bandwidth
	float vbw; //video bandwidth
	unsigned long int sweepTime;
	bool flagDefaultSamplePoints; //It determines if the sample points number must be configured with next
								//parameter or if it is left with its default value which is determined
								//by the Spectran device.
	unsigned int samplePoints; //Number of samples points. The value determined by the Spectran device (default value) or the forced value.
	unsigned int detector; //”rms”(0) or “min/max”(1)
};

//Functions intended to compare floating-point numbers
bool approximatelyEqual(float a, float b);
bool approximatelyEqual(std::vector<float> vectorA, std::vector<float> vectorB);


#endif /* RFIMS_CART_H_ */
