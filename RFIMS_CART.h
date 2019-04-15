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

using std::cout;
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
	virtual const char * what() const throw()
	{
		return message.c_str();
	}
};

struct TimeData
{
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;
	TimeData();
	TimeData(const TimeData& timeData);
	std::string date();
	std::string time();
	std::string timestamp();
	const TimeData& operator=(const TimeData& anotherTimeData);
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
