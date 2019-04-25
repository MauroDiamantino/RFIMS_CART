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
#include <cassert> //To use assert() function to debug the code
#include <ftd2xx.h> //The library which allows to communicate with the FTDI driver
#include <cstdlib> //exit, EXIT_SUCCESS, EXIT_FAILURE
#include <sstream> //stringstream
#include <unistd.h> //usleep
#include <fstream> //ifstream
#include <iomanip>
#include <boost/algorithm/string.hpp> //to_lower(string)
#include <boost/filesystem/operations.hpp> //last_write_time(path)

//#define RASPBERRY_PI

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
	virtual const char * what() const throw() {		return message.c_str();		}
};

struct TimeData
{
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;
	std::ostringstream oss;
	TimeData() {	year=month=day=hour=minute=second=0; oss.fill('0'); oss.setf(std::ios::right, std::ios::adjustfield);	}
	TimeData(const TimeData& timeData) {	*this=timeData;		}
	std::string date()
	{
		oss.str("");
		oss << std::setw(2) << day << '-' << std::setw(2) << month << '-' << year;
		return oss.str();
	}
	std::string time()
	{
		oss.str("");
		oss << std::setw(2) << hour << ':' << std::setw(2) << minute << ':' << std::setw(2) << second;
		return oss.str();
	}
	std::string timestamp(){	return ( date() + 'T' + time() );	}
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
	const FreqValueSet& operator=(const FreqValueSet & freqValueSet); //defined in SweepBuilder.cpp
	const FreqValueSet& operator+=(const FreqValueSet& rhs); //defined in SweepBuilder.cpp
	friend FreqValueSet operator+(const FreqValueSet & lhs, const FreqValueSet & rhs); //defined in SweepBuilder.cpp
};

#endif /* RFIMS_CART_H_ */
