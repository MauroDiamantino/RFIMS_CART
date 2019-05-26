/*
 * Basics.h
 *
 *  Created on: 02/04/2019
 *      Author: new-mauro
 */

#ifndef BASICS_H_
#define BASICS_H_

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
	void Clear() {	year=month=day=hour=minute=second=0;	}
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

struct BandParameters
{
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

//Functions intended to compare floating-point numbers
bool approximatelyEqual(float a, float b);
bool approximatelyEqual(std::vector<float> vectorA, std::vector<float> vectorB);

std::vector<float> operator-(const std::vector<float> & vect); //defined in FreqValues.cpp

void WaitForKey();

#endif /* BASICS_H_ */
