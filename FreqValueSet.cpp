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

FreqValueSet operator+(const FreqValueSet & lhs, const FreqValueSet & rhs)
{
	if(lhs.frequencies != rhs.frequencies)
	{
		CustomException exc("The sum could not be performed because the frequencies do not match");
		throw(exc);
	}

	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("The sum could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	FreqValueSet result;

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	auto it3=lhs.frequencies.begin();
	for( ; it1!=lhs.values.end(); it1++, it2++, it3++)
	{
		result.values.push_back( *it1 + *it2 );
		result.frequencies.push_back( *it3 );
	}

	result.type = lhs.type;
	result.index = lhs.index;
	result.timeData = lhs.timeData;

	return result;
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
		CustomException exc("The sum could not be performed because the frequencies do not match");
		throw(exc);
	}

	if( frequencies.size()!=values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("The sum could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	auto it1=values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=values.end(); it1++, it2++)
		*it1 += *it2;

	return *this;
}

void FreqValueSet::PushBack(const FreqValueSet& freqValueSet)
{
	if( frequencies.back()==freqValueSet.frequencies.front() )
	{
		frequencies.pop_back();
		values.pop_back();
	}
	frequencies.insert(frequencies.end(), freqValueSet.frequencies.begin(), freqValueSet.frequencies.end());
	values.insert(values.end(), freqValueSet.values.begin(), freqValueSet.values.end());
}
