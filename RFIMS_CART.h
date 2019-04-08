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

using namespace std;

//!Class CustomException derived from standard class exception.
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

struct Timestamp
{
	string date;
	string time;
	string Whole() { return (date + time);	}
};

struct FreqValueSet
{
	string type; //!< ”sweep”, “frequency response”, “calibration curve”, “threshold curve” or “rfi x”, where 'x' is a positive integer number
	unsigned int index; //!< A positive integer number associated with a detected RFI
	vector<float> values; //!< RF power (dBm) or gain (dB or dBi)
	vector<float> frequencies; //!< Frequency values in Hz.
	Timestamp timestamp; //!< Timestamp with the following format: DD-MM-YYYYTHH:MM:SS (where the word T separates the date and time)
	void PushBack(const FreqValueSet& freqValueSet);
	void Clear() { values.clear(); frequencies.clear();	}
	bool Empty() const {	return values.empty();		}
	const FreqValueSet& operator=(const FreqValueSet & freqValueSet); //defined in SweepBuilder.cpp
	const FreqValueSet& operator+=(const FreqValueSet& rhs); //defined in SweepBuilder.cpp
	friend FreqValueSet operator+(const FreqValueSet & lhs, const FreqValueSet & rhs); //defined in SweepBuilder.cpp
};

#endif /* RFIMS_CART_H_ */
