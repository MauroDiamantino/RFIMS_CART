/*
 * TimeData.cpp
 *
 *  Created on: 28/05/2019
 *      Author: new-mauro
 */

#include "Basics.h"

std::string TimeData::GetDate() const
{
	std::ostringstream oss;
	oss.fill('0'); oss.setf(std::ios::right, std::ios::adjustfield);
	oss << std::setw(2) << day << '-' << std::setw(2) << month << '-' << year;
	return oss.str();
}

std::string TimeData::GetTime() const
{
	std::ostringstream oss;
	oss.fill('0'); oss.setf(std::ios::right, std::ios::adjustfield);
	oss << std::setw(2) << hour << ':' << std::setw(2) << minute << ':' << std::setw(2) << second;
	return oss.str();
}

/*!	\param [in] date A date given as a `std::string` object, or even it can be inserted as `char` pointer.	*/
void TimeData::SetDate(const std::string & date)
{
	std::istringstream iss(date);
	char hyphen;
	iss >> day >> hyphen >> month >> hyphen >> year;
}

/*!	\param [in] time A time given as a `std::string` object, or even it can be inserted as `char` pointer.	*/
void TimeData::SetTime(const std::string & time)
{
	std::istringstream iss(time);
	char colon;
	iss >> hour >> colon >> minute >> colon >> second;
}

/*!	\param [in] timestamp A timestamp given as a `std::string` object, or even it can be inserted as `char` pointer.	*/
void TimeData::SetTimestamp(const std::string & timestamp)
{
	std::size_t delimiterPos = timestamp.find('T');
	if( delimiterPos==std::string::npos )
	{
		CustomException exc("The timestamp has an unexpected format.");
		throw(exc);
	}
	else
	{
		SetDate( timestamp.substr(0, delimiterPos) );
		SetTime( timestamp.substr(delimiterPos+1) );
	}
}

/*!	\param [in] days The number of days which must be used to turn back the current date.	*/
void TimeData::TurnBackDays(const unsigned int daysToTurnBack)
{
	boost::gregorian::date currDate(year, month, day);
	boost::gregorian::date_duration dd(daysToTurnBack);
	boost::gregorian::date backDate = currDate - dd;

	year=backDate.year();
	month=backDate.month();
	day=backDate.day();
}

/*!	\param [in] anotherTimeData Another _TimeData_ object which is given to copy its attributes.	*/
const TimeData& TimeData::operator=(const TimeData& anotherTimeData)
{
	year=anotherTimeData.year; month=anotherTimeData.month; day=anotherTimeData.day;
	hour=anotherTimeData.hour; minute=anotherTimeData.minute; second=anotherTimeData.second;
	return *this;
}

/////////////////////////Friends functions///////////////////////

/*!	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
bool operator<(const TimeData & lhs, const TimeData & rhs)
{
	if( lhs.year<rhs.year )
		return true;
	else if( lhs.year>rhs.year )
		return false;
	else
		//years are equals
		if( lhs.month<rhs.month )
			return true;
		else if( lhs.month>rhs.month )
			return false;
		else
			//months are equals
			if( lhs.day<rhs.day )
				return true;
			else if( lhs.day>rhs.day )
				return false;
			else
				//days are equals
				if( lhs.hour<rhs.hour )
					return true;
				else if( lhs.hour>rhs.hour )
					return false;
				else
					//hours are equals
					if( lhs.minute<rhs.minute )
						return true;
					else if( lhs.minute>rhs.minute )
						return false;
					else
						//minutes are equals
						if( lhs.second<rhs.second )
							return true;
						else
							return false;
}

/*!	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
bool operator>(const TimeData & lhs, const TimeData & rhs)
{
	if( lhs.year>rhs.year )
		return true;
	else if( lhs.year<rhs.year )
		return false;
	else
		//years are equals
		if( lhs.month>rhs.month )
			return true;
		else if( lhs.month<rhs.month )
			return false;
		else
			//months are equals
			if( lhs.day>rhs.day )
				return true;
			else if( lhs.day<rhs.day )
				return false;
			else
				//days are equals
				if( lhs.hour>rhs.hour )
					return true;
				else if( lhs.hour<rhs.hour )
					return false;
				else
					//hours are equals
					if( lhs.minute>rhs.minute )
						return true;
					else if( lhs.minute<rhs.minute )
						return false;
					else
						//minutes are equals
						if( lhs.second>rhs.second )
							return true;
						else
							return false;
}

/*!	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
bool operator==(const TimeData & lhs, const TimeData & rhs)
{
	if( lhs.year==rhs.year )
		if( lhs.month==rhs.month )
			if( lhs.day==rhs.day )
				if( lhs.hour==rhs.hour )
					if( lhs.minute==rhs.minute )
						if( lhs.second==rhs.second )
							return true;

	return false;
}
