/*! \file FreqValues.cpp
 * 	\brief This file contains the definitions of several methods of the structure _FreqValues_ and its derived structures, and the functions related to theses ones.
 * 	\author Mauro Diamantino
 */

#include "Basics.h"

/////////////////////////Definitions of FreqValues struct's methods////////////////////

void FreqValues::Clear()
{
	values.clear();
	frequencies.clear();
	timeData.Clear();
}

/*!	\param [in] freqValues Another _FreqValues_ structure whose values must be inserted at the end of this structure.	*/
bool FreqValues::PushBack(const FreqValues& freqValues)
{
	bool flagPopBack=false;
	if( !frequencies.empty() && frequencies.back()==freqValues.frequencies.front() )
	{
		frequencies.pop_back();
		values.pop_back();
		flagPopBack=true;
	}
	values.insert(values.end(), freqValues.values.begin(), freqValues.values.end());
	frequencies.insert(frequencies.end(), freqValues.frequencies.begin(), freqValues.frequencies.end());
	return flagPopBack;
}

/*! The method returns a `const` reference to the base structure.
 * 	\param [in] freqValues Another _FreqValues_ structure which is given to copy its attributes.
 */
const FreqValues& FreqValues::operator=(const FreqValues & freqValues)
{
//	type = freqValues.type;
	values = freqValues.values;
	frequencies = freqValues.frequencies;
	timeData = freqValues.timeData;

	return *this;
}

/*! This method is not intended to concatenate two _FreqValues_ structure, what is performed by the
 * 	method `PushBack()`, but its aim is to sum each	point of base structure with the corresponding point
 * 	of the structure given as argument, and to save the results in the base structure.
 *
 * 	The method returns a `const` reference to the base structure.
 * 	\param [in] rhs A _FreqValues_ structure which is given to perform the sum.
 */
const FreqValues& FreqValues::operator+=(const FreqValues& rhs)
{
//	if( rhs.frequencies!=frequencies )
//	{
//		rfims_exception exc("a sum could not be performed because the frequencies do not match");
//		throw(exc);
//	}
//
//	if( frequencies.size()!=values.size() && rhs.frequencies.size()!=rhs.values.size() )
//	{
//		rfims_exception exc("a sum could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
//		throw(exc);
//	}

	auto it1=values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=values.end(), it2!=rhs.values.end(); it1++, it2++)
		*it1 += *it2;

	return *this;
}

FreqValues::FreqValues::value_type FreqValues::MeanValue() const
{
	FreqValues::value_type sum=0.0;
	for(const auto & value : values)
		sum += value;

	return( sum/values.size() );
}

/////////////////////////FreqValues struct's friends functions/////////////////////////////////////

/*! This function takes each point of the given structure, negates it (the stored value, not the frequency)
 * 	and stores the result in a different _FreqValues_ structure, which is then returned. The rest of
 * 	attributes (frequency, type, timestamp, etc.) are copied as-is to the object to be returned.
 *	\param [in] argument The _FreqValues_ structure to be negated.
 */
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

/*! Before performing the operation, the function checks if the frequencies of each structure match
 * 	and if the "values"	vectors	have the same sizes as the "frequencies" vectors. The values of the
 * 	of object to be returned are determined by the operation, while the rest of attributes (frequency,
 * 	type, timestamp, etc.) are copied from the left-hand side argument.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator+(const FreqValues & lhs, const FreqValues & rhs)
{
//	if( lhs.frequencies!=rhs.frequencies )
//	{
//		rfims_exception exc("a sum could not be performed because the frequencies do not match");
//		throw(exc);
//	}
//
//	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
//	{
//		rfims_exception exc("a sum (or subtraction) could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
//		throw(exc);
//	}

	FreqValues result;
	result.values.reserve( lhs.values.size() );

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=lhs.values.end(), it2!=rhs.values.end(); it1++, it2++)
		result.values.push_back( *it1 + *it2 );

	result.type = lhs.type;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;

	return result;
}

/*! The values of the of object to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the left-hand side argument, which is the only argument
 * 	that is of type _FreqValues_.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator+(const FreqValues & lhs, const double rhs)
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

FreqValues operator+(const FreqValues & lhs, const float rhs) {	return( lhs + double(rhs) );	}

/*! This function calls the function `operator+(FreqValues,float)` with the order of its arguments inverted, taking into
 *  account the commutative property of the sum. The values of the of object to be returned are determined by the
 *  operation, while the rest of attributes (frequency, type, timestamp, etc.) are copied from the right-hand side
 *  argument, which is the only argument that is of type _FreqValues_.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator+(const double lhs, const FreqValues & rhs) {		return (rhs + lhs);		}

FreqValues operator+(const float lhs, const FreqValues & rhs) {		return (rhs + lhs);		}

/*! This function calls the function `operator+(FreqValues,FreqValues)` with the right-hand side
 * 	operand negated.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator-(const FreqValues & lhs, const FreqValues & rhs) { return( lhs + (-rhs) );	}

/*! This function calls the function `operator+(FreqValues,float)` with the right-hand side
 * 	operand negated.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator-(const FreqValues & lhs, const double rhs) {	return( lhs + (-rhs) );		}

FreqValues operator-(const FreqValues & lhs, const float rhs) {	return( lhs + (-rhs) );		}

/*! This function calls the function `operator+(float,FreqValues)` with the right-hand side
 * 	operand negated.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator-(const double lhs, const FreqValues & rhs) {	return( lhs + (-rhs) );		}

FreqValues operator-(const float lhs, const FreqValues & rhs) {	return( lhs + (-rhs) );		}

/*! Before performing the operation, the function checks if the frequencies of each structure match
 * 	and if the "values"	vectors	have the same sizes as the "frequencies" vectors. The values of the
 * 	of object to be returned are determined by the operation, while the rest of attributes (frequency,
 * 	type, timestamp, etc.) are copied from the left-hand side argument.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator*(const FreqValues & lhs, const FreqValues & rhs)
{
//	if( lhs.frequencies!=rhs.frequencies )
//	{
//		rfims_exception exc("a multiplication could not be performed because the frequencies do not match.");
//		throw(exc);
//	}
//
//	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
//	{
//		rfims_exception exc("a multiplication (or division) could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
//		throw(exc);
//	}

	FreqValues result;
	result.values.reserve( lhs.values.size() );

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=lhs.values.end(), it2!=rhs.values.end(); it1++, it2++)
		result.values.push_back( *it1 * *it2 );

	result.type = lhs.type;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;

	return result;
}

/*! The values of the object to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the right-hand argument, which is the only argument
 * 	that is of type _FreqValues_.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator*(const double lhs, const FreqValues & rhs)
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

FreqValues operator*(const float lhs, const FreqValues & rhs) 	{	return( double(lhs) * rhs );	}

/*! This function calls the function `operator*(float,FreqValues)` with the order of its argument
 * 	inverted, taking into account the commutative property of the multiplication.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator*(const FreqValues & lhs, const double rhs){	return (rhs * lhs);		}

FreqValues operator*(const FreqValues & lhs, const float rhs){	return (rhs * lhs);		}

/*! Before performing the operation, the function checks if the frequencies of each structure match
 * 	and if the "values"	vectors	have the same sizes as the "frequencies" vectors. The values of the
 * 	of object to be returned are determined by the operation, while the rest of attributes (frequency,
 * 	type, timestamp, etc.) are copied from the left-hand side argument.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator/(const FreqValues & lhs, const FreqValues & rhs)
{
//	if( lhs.frequencies!=rhs.frequencies )
//	{
//		rfims_exception exc("a division could not be performed because the frequencies do not match");
//		throw(exc);
//	}
//
//	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
//	{
//		rfims_exception exc("a division could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
//		throw(exc);
//	}

	FreqValues result;
	result.values.reserve( lhs.values.size() );

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=lhs.values.end(), it2!=rhs.values.end(); it1++, it2++)
		result.values.push_back( *it1 / *it2 );

	result.type = lhs.type;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;

	return result;
}

/*! The values of the object to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the right-hand argument, which is the only argument
 * 	that is of type _FreqValues_.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator/(const double lhs, const FreqValues & rhs)
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

FreqValues operator/(const float lhs, const FreqValues & rhs) {	return( double(lhs) / rhs );	}

/*! This function calls the function `operator*(FreqValues,float)` wit the right-hand argument inverted.
 *
 * 	The function returns a _FreqValues_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
FreqValues operator/(const FreqValues & lhs, const double rhs) { 	return( lhs * (1/rhs) );	}

FreqValues operator/(const FreqValues & lhs, const float rhs) { 	return( lhs * (1/rhs) );	}

/*! This function takes each point of the given structure, apply the decimal logarithm whit its value
 * 	and stores the result in a different _FreqValues_ structure, which is then returned. The rest of
 * 	attributes (frequency, type, timestamp, etc.) are copied as-is to the object to be returned.
 *	\param [in] argument The _FreqValues_ structure to be used as argument to the decimal logarithm operation.
 */
FreqValues log10(const FreqValues & argument)
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

/*! This function takes each point of the structure, raises its value to the exponent and stores
 * 	the result in a different _FreqValues_ structure, which is then returned. The rest of
 * 	attributes (frequency, type, timestamp, etc.) are copied from the left-hand side argument, which
 * 	the only argument that is of type _FreqValues_.
 *	\param [in] base The _FreqValues_ structure to be used as the base of the power function.
 *	\param [in] exponent The float value which will be used as the exponent.
 */
FreqValues pow(const FreqValues & base, const double exponent)
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

FreqValues pow(const FreqValues & base, const float exponent) {		return( pow( base, double(exponent) ) );	}

/*! This function takes the `float` value, given as the base, and raises it to each of the
 * 	values of the _FreqValues_ structure given as the exponent. Each result is stored in a
 * 	different _FreqValues_ structure which is then returned. The rest of attributes
 * 	(frequency, type, timestamp, etc.) of this structure are copied from the right-hand
 * 	side argument, which is the only argument that is of type _FreqValues_.
 * 	\param [in] base The `float` value given as the base of the exponentiation operation.
 * 	\param [in] exponent The _FreqValues_ structure whose values will be used as the exponents of the exponentiation operation.
 */
FreqValues pow(const double base, const FreqValues & exponent)
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

FreqValues pow(const float base, const FreqValues & exponent) {	return( pow( double(base), exponent) );		}

/////////////////////////Definitions of some Sweep structure's methods////////////////////

/*!	\param [in] sweep Another _Sweep_ structure which is given to copy its attributes.	*/
const Sweep & Sweep::operator=(const Sweep & sweep)
{
//	type=sweep.type;
	azimuthAngle=sweep.azimuthAngle;
	polarization=sweep.polarization;
	timeData=sweep.timeData;
	frequencies=sweep.frequencies;
	values=sweep.values;

	return *this;
}

////////////////////////Sweep structure's friend methods///////////////////////////

/*! This function takes each point of the given structure, negates it (the stored value, not the frequency)
 * 	and stores the result in a different _Sweep_ structure, which is then returned. The rest of
 * 	attributes (frequency, type, timestamp, etc.) are copied as-is to the object to be returned.
 *	\param [in] argument The _Sweep_ structure to be negated.
 */
Sweep operator-(const Sweep & argument)
{
	Sweep result = (Sweep) operator-((FreqValues&) argument);
	result.azimuthAngle = argument.azimuthAngle;
	result.polarization = argument.polarization;
	return result;
}

/*! Before performing the operation, the function checks if the frequencies of each structure match
 * 	and if the "values"	vectors	have the same sizes as the "frequencies" vectors. The values of the
 * 	of object to be returned are determined by the operation, while the rest of attributes (frequency,
 * 	type, timestamp, etc.) are copied from the left-hand side argument.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator+(const Sweep & lhs, const Sweep & rhs)
{
//	if( lhs.frequencies!=rhs.frequencies )
//	{
//		rfims_exception exc("a sum could not be performed because the frequencies do not match");
//		throw(exc);
//	}
//
//	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
//	{
//		rfims_exception exc("a sum (or subtraction) could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
//		throw(exc);
//	}

	Sweep result;
	result.values.reserve( lhs.values.size() );

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=lhs.values.end(), it2!=rhs.values.end(); it1++, it2++)
		result.values.push_back( *it1 + *it2 );

	result.type = lhs.type;
	result.timeData = lhs.timeData;
	result.frequencies = lhs.frequencies;
	result.azimuthAngle = lhs.azimuthAngle;
	result.polarization = lhs.polarization;

	return result;
}

/*! Before performing the operation, the function checks if the "values" vectors have the same sizes. The
 * 	values of the of object to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the left-hand side argument.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator+(const Sweep & lhs, const std::vector<FreqValues::value_type> & rhs)
{
	if( lhs.values.size() != rhs.size() )
	{
		rfims_exception exc("a sum (or subtraction) could not be performed because the values vectors do not match");
		throw(exc);
	}

	Sweep rhs_aux;
	rhs_aux.values = rhs;
	rhs_aux.frequencies = lhs.frequencies;

	Sweep result = lhs + rhs_aux;
	return result;
}

/*! Before performing the operation, the function checks if the "values" vectors have the same sizes. The
 * 	values of the of object to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the left-hand side argument.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator+(const std::vector<FreqValues::value_type> & lhs, const Sweep & rhs) {	return( rhs + lhs );	}

/*! To perform this operation the _FreqValues_ argument is casted to _Sweep_. The values of the of object
 * 	to be returned are determined by the operation, while the rest of attributes (frequency, type,
 * 	timestamp, etc.) are copied from the left-hand side argument, because the other argument has less
 * 	attributes as it is an object of the base class _FreqValues_ from which the class _Sweep_ derives.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator+(const Sweep & lhs, const FreqValues & rhs) {	return( lhs + (Sweep)rhs );		}

/*! The values of the of object	to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the left-hand side argument, the only one argument
 * 	of type _Sweep_.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator+(const Sweep & lhs, const double rhs)
{
	Sweep result = (Sweep) ( (FreqValues) lhs + rhs);
	result.azimuthAngle = lhs.azimuthAngle;
	result.polarization = lhs.polarization;
	return result;
}

Sweep operator+(const Sweep & lhs, const float rhs) {	return( lhs + double(rhs) );	}

/*! The values of the of object	to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the right-hand side argument, the only one argument
 * 	of type _Sweep_.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator+(const double lhs, const Sweep & rhs) {	return( rhs + lhs );	}

Sweep operator+(const float lhs, const Sweep & rhs) {	return( rhs + lhs );	}

/*! Before performing the operation, the function checks if the frequencies of each structure match
 * 	and if the "values"	vectors	have the same sizes as the "frequencies" vectors. The values of the
 * 	of object to be returned are determined by the operation, while the rest of attributes (frequency,
 * 	type, timestamp, etc.) are copied from the left-hand side argument.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator-(const Sweep & lhs, const Sweep & rhs) {		return( lhs + (-rhs) );		}

/*! Before performing the operation, the function checks if the "values" vectors have the same sizes. The
 * 	values of the of object to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the left-hand side argument.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator-(const Sweep & lhs, const std::vector<FreqValues::value_type> & rhs) {	return( lhs + (-rhs) );		}

/*! Before performing the operation, the function checks if the "values" vectors have the same sizes. The
 * 	values of the of object to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the right-hand side argument.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator-(const std::vector<FreqValues::value_type> & lhs, const Sweep & rhs) {	return( lhs + (-rhs) );		}

/*! To perform this operation the _FreqValues_ argument is casted to _Sweep_. The values of the of object
 * 	to be returned are determined by the operation, while the rest of attributes (frequency, type,
 * 	timestamp, etc.) are copied from the left-hand side argument, because the other argument has less
 * 	attributes as it is an object of the base class _FreqValues_ from which the class _Sweep_ derives.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator-(const Sweep & lhs, const FreqValues & rhs) {	return( lhs - (Sweep)rhs );		}

/*! Before performing the operation, the function checks if the frequencies of each structure match
 * 	and if the "values"	vectors	have the same sizes as the "frequencies" vectors. The values of the
 * 	of object to be returned are determined by the operation, while the rest of attributes (frequency,
 * 	type, timestamp, etc.) are copied from the left-hand side argument.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator*(const Sweep & lhs, const Sweep & rhs)
{
	Sweep result = (Sweep) ( (FreqValues&)lhs * (FreqValues&)rhs );
	result.azimuthAngle=lhs.azimuthAngle;
	result.polarization=lhs.polarization;
	return result;
}

/*! The values of the of object	to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the right-hand side argument, the only one argument
 * 	of type _Sweep_.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator*(const double lhs, const Sweep & rhs)
{
	Sweep result = (Sweep) ( lhs * (FreqValues)rhs );
	result.azimuthAngle=rhs.azimuthAngle;
	result.polarization=rhs.polarization;
	return result;
}

Sweep operator*(const float lhs, const Sweep & rhs) {	return( double(lhs) * rhs );	}

/*! The values of the of object	to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the left-hand side argument, the only one argument
 * 	of type _Sweep_.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator*(const Sweep & lhs, const double rhs) {	return( rhs * lhs );	}

Sweep operator*(const Sweep & lhs, const float rhs) {	return( rhs * lhs );	}

/*! Before performing the operation, the function checks if the frequencies of each structure match
 * 	and if the "values"	vectors	have the same sizes as the "frequencies" vectors. The values of the
 * 	of object to be returned are determined by the operation, while the rest of attributes (frequency,
 * 	type, timestamp, etc.) are copied from the left-hand side argument.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator/(const Sweep & lhs, const Sweep & rhs)
{
	Sweep result = (Sweep) ( (FreqValues)lhs / (FreqValues)rhs );
	result.azimuthAngle=lhs.azimuthAngle;
	result.polarization=lhs.polarization;
	return result;
}

/*! The values of the of object	to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the right-hand side argument, the only one argument
 * 	of type _Sweep_.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator/(const double lhs, const Sweep & rhs)
{
	Sweep result = (Sweep) ( lhs / (FreqValues)rhs );
	result.azimuthAngle=rhs.azimuthAngle;
	result.polarization=rhs.polarization;
	return result;
}

Sweep operator/(const float lhs, const Sweep & rhs) {	return( double(lhs) / rhs );	}

/*! The values of the of object	to be returned are determined by the operation, while the rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the left-hand side argument, the only one argument
 * 	of type _Sweep_.
 *
 * 	The function returns a _Sweep_ object with the results of the operation.
 * 	\param [in] lhs The left-hand side operand.
 * 	\param [in] rhs The right-hand side operand.
 */
Sweep operator/(const Sweep & lhs, const double rhs) {	return( lhs * (1/rhs) );	}

Sweep operator/(const Sweep & lhs, const float rhs) {	return( lhs * (1/rhs) );	}

/*! This function takes each point of the given structure, apply the decimal logarithm whit its value
 * 	and stores the result in a different _Sweep_ structure, which is then returned. The rest of
 * 	attributes (frequency, type, timestamp, etc.) are copied as-is to the object to be returned.
 *	\param [in] argument The _Sweep_ structure to be used as argument to the decimal logarithm operation.
 */
Sweep log10(const Sweep & argument)
{
	Sweep result = (Sweep) log10( (FreqValues)argument );
	result.azimuthAngle=argument.azimuthAngle;
	result.polarization=argument.polarization;
	return result;
}

/*! This function takes each point of the structure, raises its value to the exponent and stores
 * 	the result in a different _Sweep_ structure, which is then returned. The rest of attributes
 * 	(frequency, type, timestamp, etc.) are copied from the base argument, which the only argument
 * 	that is of type _Sweep_.
 *	\param [in] base The _Sweep_ structure to be used as the base of the power function.
 *	\param [in] exponent The float value which will be used as the exponent.
 */
Sweep pow(const Sweep & base, const double exponent)
{
	Sweep result = (Sweep) pow((FreqValues&)base, exponent);
	result.azimuthAngle = base.azimuthAngle;
	result.polarization = base.polarization;
	return result;
}

Sweep pow(const Sweep & base, const float exponent) {	return( pow( base, double(exponent) ) );	}

/*! This function takes the `float` value, given as the base, and raises it to each of the
 * 	values of the _Sweep_ structure given as the exponent. Each result is stored in a
 * 	different _Sweep_ structure which is then returned. The rest of attributes
 * 	(frequency, type, timestamp, etc.) of this structure are copied from the right-hand
 * 	side argument, which is the only argument that is of type _Sweep_.
 * 	\param [in] base The `float` value given as the base of the exponentiation operation.
 * 	\param [in] exponent The _Sweep_ structure whose values will be used as the exponents of the exponentiation operation.
 */
Sweep pow(const double base, const Sweep & exponent)
{
	Sweep result = (Sweep) pow(base, (FreqValues&)exponent);
	result.azimuthAngle = exponent.azimuthAngle;
	result.polarization = exponent.polarization;
	return result;
}

Sweep pow(const float base, const Sweep & exponent) {	return( pow( double(base), exponent) );		}
