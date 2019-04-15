/*
 * TimeData.cpp
 *
 *  Created on: 15/04/2019
 *      Author: new-mauro
 */

#include "RFIMS_CART.h"

TimeData::TimeData()
{
	year=month=day=hour=minute=second=0;
}

TimeData::TimeData(const TimeData& timeData)
{
	*this=timeData;
}

std::string TimeData::date()
{
	std::ostringstream oss;
	oss.fill('0'); oss.setf(std::ios::right, std::ios::adjustfield);
	oss << std::setw(2) << day << '-' << std::setw(2) << month << '-' << year;
	return oss.str();
}

std::string TimeData::time()
{
	std::ostringstream oss;
	oss.fill('0'); oss.setf(std::ios::right, std::ios::adjustfield);
	oss << std::setw(2) << hour << ':' << std::setw(2) << minute << ':' << std::setw(2) << second;
	return oss.str();
}

std::string TimeData::timestamp()
{
	std::string date = this->date();
	std::string time = this->time();
	std::string timestamp = date + 'T' + time;
	return timestamp;
}

const TimeData& TimeData::operator=(const TimeData& anotherTimeData)
{
	year=anotherTimeData.year;
	month=anotherTimeData.month;
	day=anotherTimeData.day;
	hour=anotherTimeData.hour;
	minute=anotherTimeData.minute;
	second=anotherTimeData.second;
	return *this;
}
