/*
 * FreqValues.cpp
 *
 *  Created on: 08/04/2019
 *      Author: new-mauro
 */

#include "RFIMS_CART.h"

/////////////////////////Definitions of FreqValues struct's methods////////////////////

void FreqValues::Clear()
{
	values.clear();
	frequencies.clear();
	timeData.year = timeData.month = timeData.day = timeData.hour = timeData.minute = timeData.second = 0;
}

bool FreqValues::PushBack(const FreqValues& freqValues)
{
	bool flagPopBack=false;
	if( !frequencies.empty() && approximatelyEqual(frequencies.back(), freqValues.frequencies.front()) )
	{
		frequencies.pop_back();
		values.pop_back();
		flagPopBack=true;
	}
	values.insert(values.end(), freqValues.values.begin(), freqValues.values.end());
	frequencies.insert(frequencies.end(), freqValues.frequencies.begin(), freqValues.frequencies.end());
	return flagPopBack;
}

//! The overloading of the assignment operator.
const FreqValues& FreqValues::operator=(const FreqValues & freqValues)
{
	type = freqValues.type;
	values = freqValues.values;
	frequencies = freqValues.frequencies;
	timeData = freqValues.timeData;

	return *this;
}

//! The overloading of the += operator.
const FreqValues& FreqValues::operator+=(const FreqValues& rhs)
{
//	if( !approximatelyEqual(rhs.frequencies, frequencies) )
//	{
//		CustomException exc("A sum could not be performed because the frequencies do not match");
//		throw(exc);
//	}
	if( rhs.frequencies.size() != frequencies.size() )
	{
		CustomException exc("A sum could not be performed because the frequencies do not match");
		throw(exc);
	}

	if( frequencies.size()!=values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("A sum could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	auto it1=values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=values.end(); it1++, it2++)
		*it1 += *it2;

	return *this;
}


/////////////////////////Friends functions/////////////////////////////////////

FreqValues operator-(const FreqValues & argument)
{
	FreqValues result;
	result.values.reserve( argument.values.size() );

	for(auto v : argument.values)
		result.values.push_back( -v );

	result.type = argument.type;
	result.timeData = argument.timeData;
	result.frequencies = argument.frequencies;

	return result;
}

//! The overloading of the operation "sum", which is a friend function of the struct FreqValueSet
/*! This function takes two arguments of type FreqValueSet and returns a different object of the same type
 * whose "values" vector will have the sum of the corresponding elements of the "values" vectors of the arguments.
 * Before performing the sum, the function checks if the frequencies match and if the "values" vectors have the same
 * sizes as the "frequencies" vectors. The other attributes of the returned object (type, index and timestamp) will
 * take the contents of the corresponding attributes of the left-hand argument.
 */
FreqValues operator+(const FreqValues & lhs, const FreqValues & rhs)
{
//	if( !approximatelyEqual(lhs.frequencies, rhs.frequencies) )
//	{
//		CustomException exc("A sum could not be performed because the frequencies do not match");
//		throw(exc);
//	}
	if( lhs.frequencies.size() != rhs.frequencies.size() )
	{
		CustomException exc("A sum (or subtraction) could not be performed because the frequencies do not match");
		throw(exc);
	}

	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("A sum (or subtraction) could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	FreqValues result;
	result.values.reserve( lhs.values.size() );

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=lhs.values.end(); it1++, it2++)
		result.values.push_back( *it1 + *it2 );

	result.type = lhs.type;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;

	return result;
}


FreqValues operator+(const FreqValues & lhs, const float rhs)
{
	FreqValues result;
	result.values.reserve( lhs.values.size() );

	for(auto& value : lhs.values)
		result.values.push_back( value + rhs );

	result.type = lhs.type;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;

	return result;
}

FreqValues operator+(const float lhs, const FreqValues & rhs) {	return (rhs + lhs);		}

FreqValues operator-(const FreqValues & lhs, const FreqValues & rhs) { return( lhs + (-rhs) );	}

FreqValues operator-(const FreqValues & lhs, const float rhs) {	return( lhs + (-rhs) );	}

FreqValues operator-(const float lhs, const FreqValues & rhs) {	return( lhs + (-rhs) );		}

FreqValues operator*(const FreqValues & lhs, const FreqValues & rhs)
{
//	if( !approximatelyEqual(lhs.frequencies, rhs.frequencies) )
//	{
//		CustomException exc("A multiplication could not be performed because the frequencies do not match.");
//		throw(exc);
//	}
	if( lhs.frequencies.size() != rhs.frequencies.size() )
	{
		CustomException exc("A multiplication (or division) could not be performed because the frequencies do not match.");
		throw(exc);
	}

	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("A multiplication (or division) could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	FreqValues result;
	result.values.reserve( lhs.values.size() );

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=lhs.values.end(); it1++, it2++)
		result.values.push_back( *it1 * *it2 );

	result.type = lhs.type;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;

	return result;
}


FreqValues operator*(const float lhs, const FreqValues & rhs)
{
	FreqValues result;
	result.values.reserve( rhs.values.size() );

	for(auto& value : rhs.values)
		result.values.push_back( lhs*value );

	result.type = rhs.type;
	result.timeData = rhs.timeData;
	result.frequencies = rhs.frequencies;

	return result;
}

FreqValues operator*(const FreqValues & lhs, const float rhs){	return (rhs * lhs);		}

FreqValues operator/(const FreqValues & lhs, const FreqValues & rhs)
{
//	if( !approximatelyEqual(lhs.frequencies, rhs.frequencies) )
//	{
//		CustomException exc("A division could not be performed because the frequencies do not match");
//		throw(exc);
//	}
	if( lhs.frequencies.size() != rhs.frequencies.size() )
	{
		CustomException exc("A division could not be performed because the frequencies do not match");
		throw(exc);
	}

	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("A division could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	FreqValues result;
	result.values.reserve( lhs.values.size() );

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=lhs.values.end(); it1++, it2++)
		result.values.push_back( *it1 / *it2 );

	result.type = lhs.type;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;

	return result;
}

FreqValues operator/(const float lhs, const FreqValues & rhs)
{
	FreqValues result;
	result.values.reserve( rhs.values.size() );

	for(auto& value : rhs.values)
		result.values.push_back( lhs/value );

	result.type = rhs.type;
	result.timeData = rhs.timeData;
	result.frequencies = rhs.frequencies;

	return result;
}

FreqValues operator/(const FreqValues & lhs, const float rhs) { 	return( lhs * (1/rhs) );	}

FreqValues log10(const FreqValues & argument) //decimal logarithm
{
	FreqValues result;
	result.values.reserve( argument.values.size() );

	for(auto& value : argument.values)
		result.values.push_back( log10(value) );

	result.type = argument.type;
	result.timeData = argument.timeData;
	result.frequencies = argument.frequencies;

	return result;
}

FreqValues pow(const FreqValues & base, const float exponent) //exponentiation
{
	FreqValues result;
	result.values.reserve( base.values.size() );

	for(auto& value : base.values)
		result.values.push_back( pow(value, exponent) );

	result.frequencies = base.frequencies;
	result.type = base.type;
	result.timeData = base.timeData;

	return result;
}

FreqValues pow(const float base, const FreqValues & exponent)
{
	FreqValues result;
	result.values.reserve( exponent.values.size() );

	for(auto& value : exponent.values)
		result.values.push_back( pow(base, value) );

	result.frequencies = exponent.frequencies;
	result.type = exponent.type;
	result.timeData = exponent.timeData;

	return result;
}

//////////////Sweep structure's friend methods////////////////

Sweep operator+(const Sweep & lhs, const Sweep & rhs)
{
//	if( !approximatelyEqual(lhs.frequencies, rhs.frequencies) )
//	{
//		CustomException exc("A sum could not be performed because the frequencies do not match");
//		throw(exc);
//	}
	if( lhs.frequencies.size() != rhs.frequencies.size() )
	{
		CustomException exc("A sum (or subtraction) could not be performed because the frequencies do not match");
		throw(exc);
	}

	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("A sum (or subtraction) could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	Sweep result;
	result.values.reserve( lhs.values.size() );

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=lhs.values.end(); it1++, it2++)
		result.values.push_back( *it1 + *it2 );

	result.type = lhs.type;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;
	result.azimuthAngle = lhs.azimuthAngle;
	result.polarization = lhs.polarization;

	return result;
}

Sweep operator+(const Sweep & lhs, const std::vector<float> & rhs)
{
	if( lhs.values.size() != rhs.size() )
	{
		CustomException exc("A sum (or subtraction) could not be performed because the values vectors do not match");
		throw(exc);
	}

	Sweep rhs_aux;
	rhs_aux.values = rhs;

	Sweep result = lhs + rhs_aux;
	return result;
}

Sweep operator+(const std::vector<float> & lhs, const Sweep & rhs) {	return( rhs + lhs );	}
