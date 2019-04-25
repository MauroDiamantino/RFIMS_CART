/*
 * FreqValueSet.cpp
 *
 *  Created on: 08/04/2019
 *      Author: new-mauro
 */

#include "RFIMS_CART.h"

/////////////////////////Definitions of FreqValueSet struct's methods////////////////////

//! The overloading of the operation "sum", which is a friend function of the struct FreqValueSet
/*! This function takes two arguments of type FreqValueSet and returns a different object of the same type
 * whose "values" vector will have the sum of the corresponding elements of the "values" vectors of the arguments.
 * Before performing the sum, the function checks if the frequencies match and if the "values" vectors have the same
 * sizes as the "frequencies" vectors. The other attributes of the returned object (type, index and timestamp) will
 * take the contents of the corresponding attributes of the left-hand argument.
 */
FreqValueSet::FreqValueSet(const FreqValueSet& freqValueSet)
{
	*this=freqValueSet;
}

void FreqValueSet::PushBack(const FreqValueSet& freqValueSet)
{
	if( !frequencies.empty() && frequencies.back()==freqValueSet.frequencies.front() )
	{
		frequencies.pop_back();
		values.pop_back();
	}
	values.insert(values.end(), freqValueSet.values.begin(), freqValueSet.values.end());
	frequencies.insert(frequencies.end(), freqValueSet.frequencies.begin(), freqValueSet.frequencies.end());
}

//! The overloading of the assignment operator.
const FreqValueSet& FreqValueSet::operator=(const FreqValueSet & freqSet)
{
	type = freqSet.type;
	index = freqSet.index;
	values = freqSet.values;
	frequencies = freqSet.frequencies;
	timeData = freqSet.timeData;

	return *this;
}

//! The overloading of the += operator.
const FreqValueSet& FreqValueSet::operator+=(const FreqValueSet& rhs)
{
	if(rhs.frequencies != frequencies)
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


/////////Friends functions/////////////

FreqValueSet operator-(const FreqValueSet & argument)
{
	FreqValueSet result;
	result.values.reserve( argument.values.size() );

	for(auto v : argument.values)
		result.values.push_back( -v );

	result.type = argument.type;
	result.index = argument.index;
	result.timeData = argument.timeData;
	result.frequencies = argument.frequencies;

	return result;
}

FreqValueSet operator+(const FreqValueSet & lhs, const FreqValueSet & rhs)
{
	if(lhs.frequencies != rhs.frequencies)
	{
		CustomException exc("A sum could not be performed because the frequencies do not match");
		throw(exc);
	}

	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("A sum could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	FreqValueSet result;
	result.values.reserve( lhs.values.size() );

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=lhs.values.end(); it1++, it2++)
		result.values.push_back( *it1 + *it2 );

	result.type = lhs.type;
	result.index = lhs.index;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;

	return result;
}

FreqValueSet operator+(const FreqValueSet & lhs, const float rhs)
{
	FreqValueSet result;
	result.values.reserve( lhs.values.size() );

	for(auto& value : lhs.values)
		result.values.push_back( value + rhs );

	result.type = lhs.type;
	result.index = lhs.index;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;

	return result;
}

FreqValueSet operator+(const float lhs, const FreqValueSet & rhs) {	return (rhs + lhs);		}

FreqValueSet operator-(const FreqValueSet & lhs, const FreqValueSet & rhs) { return( lhs + (-rhs) );	}

FreqValueSet operator-(const FreqValueSet & lhs, const float rhs) {	return( lhs + (-rhs) );	}

FreqValueSet operator-(const float lhs, const FreqValueSet & rhs) {	return( lhs + (-rhs) );		}

FreqValueSet operator*(const FreqValueSet & lhs, const FreqValueSet & rhs)
{
	if(lhs.frequencies != rhs.frequencies)
	{
		CustomException exc("A multiplication could not be performed because the frequencies do not match.");
		throw(exc);
	}

	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("A multiplication could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	FreqValueSet result;
	result.values.reserve( lhs.values.size() );

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=lhs.values.end(); it1++, it2++)
		result.values.push_back( *it1 * *it2 );

	result.type = lhs.type;
	result.index = lhs.index;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;

	return result;
}


FreqValueSet operator*(const float lhs, const FreqValueSet & rhs)
{
	FreqValueSet result;
	result.values.reserve( rhs.values.size() );

	for(auto& value : rhs.values)
		result.values.push_back( lhs*value );

	result.type = rhs.type;
	result.index = rhs.index;
	result.timeData = rhs.timeData;
	result.frequencies = rhs.frequencies;

	return result;
}

FreqValueSet operator*(const FreqValueSet & lhs, const float rhs){	return (rhs * lhs);		}

FreqValueSet operator/(const FreqValueSet & lhs, const FreqValueSet & rhs)
{
	if(lhs.frequencies != rhs.frequencies)
	{
		CustomException exc("A division could not be performed because the frequencies do not match");
		throw(exc);
	}

	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("A division could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	FreqValueSet result;
	result.values.reserve( lhs.values.size() );

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=lhs.values.end(); it1++, it2++)
		result.values.push_back( *it1 / *it2 );

	result.type = lhs.type;
	result.index = lhs.index;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;

	return result;
}

FreqValueSet log10(const FreqValueSet & argument) //decimal logarithm
{
	FreqValueSet result;
	result.values.reserve( argument.values.size() );

	for(auto& value : argument.values)
		result.values.push_back( log10(value) );

	result.type = argument.type;
	result.index = argument.index;
	result.timeData = argument.timeData;
	result.frequencies = argument.frequencies;

	return result;
}

FreqValueSet pow(const FreqValueSet & base, const float exponent) //exponentiation
{
	FreqValueSet result;
	result.values.reserve( base.values.size() );

	for(auto& value : base.values)
		result.values.push_back( pow(value, exponent) );

	result.frequencies = base.frequencies;
	result.type = base.type;
	result.index = base.index;
	result.timeData = base.timeData;

	return result;
}

FreqValueSet pow(const float base, const FreqValueSet & exponent)
{
	FreqValueSet result;
	result.values.reserve( exponent.values.size() );

	for(auto& value : exponent.values)
		result.values.push_back( pow(base, value) );

	result.frequencies = exponent.frequencies;
	result.type = exponent.type;
	result.index = exponent.index;
	result.timeData = exponent.timeData;

	return result;
}
