/*! \file Basics.h
 * 	\brief This header file contains the declarations of the most basic and global entities which are used by many others entities.
 *
 * 	This header file contains the declarations and definitions of the following:
 * 	- Preprocessor definitions (`define`): these elements define the blocks of code will be taking into
 * 	account by the compiler, so the way the software will behave after the compilation and linking.
 * 	- Global libraries, i.e. the libraries which are used by several functions and classes from different files.
 * 	- Global constants and variables.
 * 	- Global structures which are used to interchange data between different objects and functions.
 * 	- Global structures which contain common data to all software's entities.
 * 	- A global class to handle exceptions.
 * 	- Global functions which are used in several different places.
 * 	\author	Mauro Diamantino
 */

#ifndef BASICS_H_
#define BASICS_H_

//#//////////////////////DEFINE'S//////////////////////////////

#define RASPBERRY_PI // This preprocessor definition enables the use of the code which can only be used in the Raspberry Pi board (for example WiringPi functions).
//#define DEBUG // This definition enables some code blocks which are aimed to test the software performance.
#ifdef RASPBERRY_PI
	//#define BUTTON // This definition determines the use of Enter key to respond to some software messages or the use of a button connected to a GPIO pin.
#endif

//#////////////////////////////////////////////////////////////


//#/////////////////////GLOBAL LIBRARIES AND HEADERS//////////////////////////////

// The must-have library which allows to use the objects `std::cout` and `std::cin` among a lot of other things.
#include <iostream>
// It is included to use base class `std::exception`.
#include <exception>
// It is included to use the famous container `std::vector`.
#include <vector>
// This library is included to make use of the famous container `std::string`.
#include <string>
// It is included to state the classes `std::ostringstream` and `std::istringstream` are being used, but it is unnecessary if the *iostream* library has already bee included.
#include <sstream>
// *iomanip* is a library which allows to use parametric manipulators to modify the way things (like numbers) are plotted with `std::cout`, `std::cerr`, etc.
#include <iomanip>
// This library is included to make use the function `boost::algorithm::to_lower()` to convert upper-case characters to lower-case ones.
#include <boost/algorithm/string.hpp>
// This library allows to use several functions and classes to perform tasks related to the filesystem such as creating a new file, see a directory's content, remove a file, etc.
#include <boost/filesystem.hpp>
// A library which allows to perform task related with dates, such as getting the date of 30 days ago from the current date. Classes `boost::gregorian::date()` and `boost::gregorian::date_duration()` are used.
#include <boost/date_time/gregorian/gregorian.hpp>
// This library is used to perform common mathematical operations like arc tangent, logarithm, power, and so on.
#include <cmath>
// This library allows to use to classes `std::ifstream` and `std::ofstream` to read and write data from files.
#include <fstream>
// This is a header file that provides access to the POSIX operating system API, for instance it allows to use the command `sleep()`.
#include <unistd.h>
// This header file defines a set of integral type aliases with specific width requirements.
#include <cstdint>
// This header defines several general purpose functions and it is included here to use `exit()` function.
/* This header file includes dynamic memory management, integer arithmetics, searching, sorting and converting. */
#include <cstdlib>
// This library specifies a set of interfaces for threaded programming commonly known as POSIX threads, or Pthread.
#include <pthread.h>
// This library has been included to use the function std::max_element().
#include <algorithm>

#ifdef RASPBERRY_PI
// WiringPi is a PIN based GPIO access library for the SoC devices used in all Raspberry Pi versions.
/* It’s designed to be familiar to people who have used the Arduino “wiring” system. It's developed directly
 * on a Raspberry Pi running 32-bit Raspbian. It supports the BCM2835, BCM2836 and BCM2837 SoC devices.
 */
#include <wiringPi.h>
#endif

//#////////////////////////////////////////////////////////////////////////////////


using std::cout;
using std::cerr;
using std::cin;
using std::endl;


//#//////////////////GLOBAL CONSTANTS/////////////////////////

#ifdef RASPBERRY_PI
//! This constant define the base path where there are the files and folders used by the software.
const std::string BASE_PATH = "/home/pi/RFIMS-CART";
#else
//! This constant define the base path where there are the files and folders used by the software.
const std::string BASE_PATH = "/home/new-mauro/RFIMS-CART";
#endif

//#//////////////////////////////////////////////////////////


//#//////////////////////GLOBAL CLASSES AND STRUCTURES/////////////////////////

//! A class derived from standard class `std::exception`.
/*! This class is customized to managed the exceptions in a desired way. This class has been
 * defined to ease the appending of data to the message carried by an exception object.
 */
class rfims_exception : public std::exception
{
	std::string message; //!< The internal message which contains the info of the exception.
public:
	//! The default constructor, which can set the exception message.
	/*! \param [in] msg The exception message, which can be of type `char*` or `std::string`.	*/
	rfims_exception(const std::string& msg="Error") : message(msg) {}
	//! The aim of this function is to modify the exception message.
	/*! \param [in] msg The exception message, which can be of type `char*` or `std::string`.	*/
	void SetMessage(const std::string& msg) {	message=msg;	}
	//! This function is intended to add some text at the beginning of the exception message.
	/*! \param [in] msg The sentence which must be prepended to the exception message and which can be of type `char*` or `std::string`.	*/
	void Prepend(const std::string& msg) {	message.insert(0, ": "); message.insert(0, msg);	}
	//! This function is intended to add some text at the end of the exception message.
	/*! \param [in] msg The sentence which must be appended to the exception message and which can be of type `char*` or `std::string`.	*/
	void Append(const std::string& msg) {	message.append(": "); message.append(msg);	}
	//! This is a standard function for classes which manage exceptions and is intended to return the exception message as a C string (`char*`).
	const char * what() const throw() {	return message.c_str();	}
};

//! This structure is intended to store data related to _date_ and _time_ and to perform some operations with that data.
/*! This structure can store time data (hour, minute and second) and date data (year, month and date) which are obtained
 * from a GPS receiver. It can offer just the date as a string, or just the time as a string or even can build a
 * _timestamp_ with a specific format: DD-MM-YYYYTHH:MM:SS. Also it can take a date and time and turn that back a
 * specific number of days taking into account the _gregorian_ calendar. An object of this class can even be compared as
 * equal to, less than or bigger than another object of the same class.
 */
struct TimeData
{
	unsigned int year; //!< This variable stores the year, taking into account the estimated birth of Jesus.
	unsigned int month; //!< This variable stores the month as a number that can be between 1 and 12.
	unsigned int day; //!< This variable stores the day as a number that can be between 1 and 31, taking into account the corresponding month.
	unsigned int hour; //!< This variable stores the hours as a number that can be between 0 and 23.
	unsigned int minute; //!< This variable stores the minutes as a number that can be between 0 and 59.
	unsigned int second; //!< This variable stores the seconds as a number that can be between 0 and 59.

	//! The class' constructor which clear all attributes, i.e. set date and time as 00-00-0000T00:00:00.
	TimeData() {	Clear();	}
	//! The class' copy constructor.
	/*!	\param [in]	timeData Another _TimeData_ object which is given to copy its attributes.	*/
	TimeData(const TimeData& timeData) {	operator=(timeData);	}
	//! A method to get the date as a `std::string`
	std::string GetDate() const;
	//! A method to get the time as a `std::string`
	std::string GetTime() const;
	//! A method to get a timestamp as a `std::string` with the format DD-MM-YYYYTHH:MM:SS.
	std::string GetTimestamp() const {		return ( GetDate() + 'T' + GetTime() );		}
	//! This method is intended to set just the date.
	void SetDate(const std::string & date);
	//! This method is intended to set just the time.
	void SetTime(const std::string & time);
	//! This method is intended to modify all attributes, giving these as a timestamp.
	void SetTimestamp(const std::string & timestamp);
	//! This method allows to turn a time point back a specified number of days.
	void TurnBackDays(const unsigned int days);
	//! An overloading of the assignment operator.
	const TimeData& operator=(const TimeData& anotherTimeData);
	//! A method to clear all attributes, i.e. it set the object as 00-00-00T00:00:00.
	void Clear() {	year=month=day=hour=minute=second=0;	}

	//Friends functions//
	//! An overloading of the operator < to compare two TimeData objects as the first one lesser than the second one.
	friend bool operator<(const TimeData & lhs, const TimeData & rhs); //defined in TimeData.cpp
	//! An overloading of the operator > to compare two TimeData objects as the first one bigger than the second one.
	friend bool operator>(const TimeData & lhs, const TimeData & rhs); //defined in TimeData.cpp
	//! An overloading of the operator == to compare two TimeData objects as equal.
	friend bool operator==(const TimeData & lhs, const TimeData & rhs); //defined in TimeData.cpp
};


//! The aim of this structure is to store the curve of a determined parameter or variable versus the frequency, which is named a frequency curve here.
struct FreqValues
{
	typedef float value_type;
	std::string type; //!< Type of frequency values: ”sweep”, “frequency response”, “calibration curve”, “threshold curve”, “rfi", etc.
	std::vector<value_type> values; //!< RF power values (dBm), gain values (dB or dBi), noise figure values (dB), etc.
	std::vector<std::uint_least64_t> frequencies; //!< Frequency values in Hz.
	TimeData timeData; //!< A TimeData object which contains information about the time when the values were captured, defined, etc.
	//! The default constructor which can receive the curve type.
	/*! \param [in] typ The type of parameter whose curve of values versus frequency is stored in the structure.	*/
	FreqValues(const std::string& typ="unknown") : type(typ) {}
	//! The copy constructor
	/*! \param [in] freqValues Another _FreqValues_ structure which is given to copy its attributes.	*/
	FreqValues(const FreqValues& freqValues) {	operator=(freqValues);	}
	//! This is the structure's destructor which is virtual because there are structures derived from this structure.
	virtual ~FreqValues() {}
	//! This method is intended to insert one data point (frequency,value) or a set of data points in the structure, at the end.
	bool PushBack(const FreqValues& freqValues);
	//! This method is intended to clear the structure, i.e. to delete all its data points.
	virtual void Clear();
	//! This method allows to know if the structure is empty, i.e. it has no data points.
	bool Empty() const {	return values.empty();	}
	//! An overloading of the assignment operator adapted for this structure.
	const FreqValues& operator=(const FreqValues & freqValues);
	//! An overloading of the operator += adapted for this structure.
	const FreqValues& operator+=(const FreqValues& rhs);
	//! The aim of this method is to offer the mean value of all data points, i.e. it calculates the average.
	value_type MeanValue() const;

	//Friends functions//
	//! An overloading of the unary operator - which negates a _FreqValues_ object.
	friend FreqValues operator-(const FreqValues& argument); //defined in FreqValues.cpp
	//! An overloading of operator + which calculates the sum of two objects of structure _FreqValues_.
	friend FreqValues operator+(const FreqValues & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	//! An overloading of operator + which calculates the sum of a _FreqValues_ object and a _float_ value, in that order.
	friend FreqValues operator+(const FreqValues & lhs, const double rhs); //defined in FreqValues.cpp
	friend FreqValues operator+(const FreqValues & lhs, const float rhs); //defined in FreqValues.cpp
	//! An overloading of operator + which calculates the sum of a _float_ value and a _FreqValues_ object, in that order.
	friend FreqValues operator+(const double lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator+(const float lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	//! An overloading of operator - which calculates the subtraction of two objects of structure _FreqValues_.
	friend FreqValues operator-(const FreqValues & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	//! An overloading of operator - which calculates the subtraction of a _FreqValues_ object and a _float_ value, in that order.
	friend FreqValues operator-(const FreqValues & lhs, const double rhs); //defined in FreqValues.cpp
	friend FreqValues operator-(const FreqValues & lhs, const float rhs); //defined in FreqValues.cpp
	//! An overloading of operator - which calculates the subtraction of a _float_ value and a _FreqValues_ object, in that order.
	friend FreqValues operator-(const double lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator-(const float lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	//! An overloading of operator * which multiplies two objects of structure _FreqValues_.
	friend FreqValues operator*(const FreqValues & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	//! An overloading of operator * which multiplies a _FreqValues_ object and a _float_ value, in that order.
	friend FreqValues operator*(const FreqValues & lhs, const double rhs); //defined in FreqValues.cpp
	friend FreqValues operator*(const FreqValues & lhs, const float rhs); //defined in FreqValues.cpp
	//! An overloading of operator * which multiplies a _float_ value and a _FreqValues_ object, in that order.
	friend FreqValues operator*(const double lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator*(const float lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	//! An overloading of operator / which calculates the division between two objects of structure _FreqValues_.
	friend FreqValues operator/(const FreqValues & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	//! An overloading of operator / which calculates the division between a _FreqValues_ object and a _float_ value, in that order.
	friend FreqValues operator/(const FreqValues & lhs, const double rhs); //defined in FreqValues.cpp
	friend FreqValues operator/(const FreqValues & lhs, const float rhs); //defined in FreqValues.cpp
	//! An overloading of operator / which calculates the division between a _float_ value and a _FreqValues_ object, in that order.
	friend FreqValues operator/(const double lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	friend FreqValues operator/(const float lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	//! An overloading of function `log10()`, decimal logarithm, adapted to receive an argument of type _FreqValues_.
	friend FreqValues log10(const FreqValues & argument); //decimal logarithm, defined in FreqValues.cpp
	//! An overloading of function `pow()`, power function, adapted to receive an argument of type _FreqValues_ as base and an argument of type _float_ as exponent.
	friend FreqValues pow(const FreqValues & base, const double exponent); //exponentiation, defined in FreqValues.cpp
	friend FreqValues pow(const FreqValues & base, const float exponent); //exponentiation, defined in FreqValues.cpp
	//! An overloading of function `pow()`, exponentiation, adapted to receive an argument of type _float_ as base and an argument of type _FreqValues_ as exponent.
	friend FreqValues pow(const double base, const FreqValues & exponent); //exponentiation, defined in FreqValues.cpp
	friend FreqValues pow(const float base, const FreqValues & exponent); //exponentiation, defined in FreqValues.cpp
};


//! The aim of this structure is to store the data points of a sweep obtained with the spectrum analyzer in a determined azimuth position, with a specific polarization.
struct Sweep : public FreqValues
{
	float azimuthAngle; //!< The azimuth position (or angle) of the antenna when the sweep was captured.
	std::string polarization; //!< The antenna polarization when the sweep was captured.
	//! The default constructor which calls the default constructor of _FreqValues_ structure and set type to "sweep" and azimuth angle to zero.
	Sweep() : FreqValues("sweep") {	azimuthAngle=0.0; }
	//! A copy constructor which receives a _FreqValues_ object.
	/*!	Just the compatible attributes are copied.
	 * 	\param [in] freqValues A _FreqValues_ structure which is given to copy its attributes.
	 */
	Sweep(const FreqValues & freqValues) : FreqValues(freqValues) { azimuthAngle=0.0; }
	//! A copy constructor which receives a _Sweep_ object.
	/*!	\param [in] sweep Another _Sweep_ structure which is given to copy its attributes.	*/
	Sweep(const Sweep & sweep) {	operator=(sweep);		}
	//! The aim of this method is to clean the structure, i.e. to delete all data points, set azimuth angle to zero and clean polarization.
	void Clear() {	FreqValues::Clear(); azimuthAngle=0.0; polarization.clear();	}
	//! An overloading of the assignment operator adapted to this structure.
	const Sweep & operator=(const Sweep & sweep);

	//Friends functions//
	//! An overloading of the unary operator - which negates a _Sweep_ object.
	friend Sweep operator-(const Sweep & argument); //defined in FreqValues.cpp
	//! An overloading of operator + which calculates the sum of two objects of structure _Sweep_.
	friend Sweep operator+(const Sweep & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	//! An overloading of operator + which calculates the sum of a _Sweep_ object and a `std::vector<float>` container, in that order.
	friend Sweep operator+(const Sweep & lhs, const std::vector<value_type> & rhs); //defined in FreqValues.cpp
	//! An overloading of operator + which calculates the sum of a `std::vector<float>` container and a _Sweep_ object, in that order.
	friend Sweep operator+(const std::vector<value_type> & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	//! An overloading of operator + which calculates the sum of a _Sweep_ object and a _FreqValues_ object, in that order.
	friend Sweep operator+(const Sweep & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	//! An overloading of operator + which calculates the sum of a _Sweep_ object and a _float_ value, in that order.
	friend Sweep operator+(const Sweep & lhs, const double rhs); //defined in FreqValues.cpp
	friend Sweep operator+(const Sweep & lhs, const float rhs); //defined in FreqValues.cpp
	//! An overloading of operator + which calculates the sum of a _float_ value and a _Sweep_ object, in that order.
	friend Sweep operator+(const double lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator+(const float lhs, const Sweep & rhs); //defined in FreqValues.cpp
	//! An overloading of operator - which calculates the subtraction of two objects of structure _Sweep_.
	friend Sweep operator-(const Sweep & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	//! An overloading of operator - which calculates the subtraction of a _Sweep_ object and a `std::vector<float>` container, in that order.
	friend Sweep operator-(const Sweep & lhs, const std::vector<value_type> & rhs); //defined in FreqValues.cpp
	//! An overloading of operator - which calculates the subtraction of a `std::vector<float>` container and a _Sweep_ object, in that order.
	friend Sweep operator-(const std::vector<value_type> & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	//! An overloading of operator - which calculates the subtraction of a _Sweep_ object and a _FreqValues_ object, in that order.
	friend Sweep operator-(const Sweep & lhs, const FreqValues & rhs); //defined in FreqValues.cpp
	//! An overloading of operator * which multiplies two objects of structure _Sweep_.
	friend Sweep operator*(const Sweep & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	//! An overloading of operator * which multiplies a _Sweep_ object and a _float_ value, in that order.
	friend Sweep operator*(const Sweep & lhs, const double rhs); //defined in FreqValues.cpp
	friend Sweep operator*(const Sweep & lhs, const float rhs); //defined in FreqValues.cpp
	//! An overloading of operator * which multiplies a _float_ value and a _Sweep_ object, in that order.
	friend Sweep operator*(const double lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator*(const float lhs, const Sweep & rhs); //defined in FreqValues.cpp
	//! An overloading of operator / which calculates the division between two objects of structure _Sweep_.
	friend Sweep operator/(const Sweep & lhs, const Sweep & rhs); //defined in FreqValues.cpp
	//! An overloading of operator / which calculates the division between a _Sweep_ object and a _float_ value, in that order.
	friend Sweep operator/(const Sweep & lhs, const double rhs); //defined in FreqValues.cpp
	friend Sweep operator/(const Sweep & lhs, const float rhs); //defined in FreqValues.cpp
	//! An overloading of operator / which calculates the division between a _float_ value and a _Sweep_ object, in that order.
	friend Sweep operator/(const double lhs, const Sweep & rhs); //defined in FreqValues.cpp
	friend Sweep operator/(const float lhs, const Sweep & rhs); //defined in FreqValues.cpp
	//! An overloading of function `log10()` adapted to receive an argument of type _Sweep_.
	friend Sweep log10(const Sweep & argument); //decimal logarithm, defined in FreqValues.cpp
	//! An overloading of function `pow()` adapted to receive an argument of type _Sweep_ as base and an argument of type _float_ as exponent.
	friend Sweep pow(const Sweep & base, const double exponent); //exponentiation, defined in FreqValues.cpp
	friend Sweep pow(const Sweep & base, const float exponent); //exponentiation, defined in FreqValues.cpp
	//! An overloading of function `pow()` adapted to receive an argument of type _float_ as base and an argument of type _Sweep_ as exponent.
	friend Sweep pow(const double base, const Sweep & exponent); //exponentiation, defined in FreqValues.cpp
	friend Sweep pow(const float base, const Sweep & exponent); //exponentiation, defined in FreqValues.cpp
};


//! The aim of this structure is to store the data related with the detected RF interference (RFI): frequency, power, azimuth angle, polarization, time, reference norm, etc.
struct RFI : public FreqValues
{
	//! Enumeration which contains the reference documents (recommendations, protocols, etc.) of harmful RFI levels (a.k.a. thresholds): the ITU recommendation RA.769-2, SKA protocol Mode 1, SKA protocol Mode 2.
	enum ThresholdsNorm {ITU_RA769_2_VLBI, SKA_MODE1, SKA_MODE2};
	float azimuthAngle; //!< The azimuth angle where this RFI was detected.
	std::string polarization; //!< The antenna polarization of the sweep where the RFI was detected.
	unsigned int numOfRFIBands; //!< The number of RFI bands defined as intervals of continuous data points where it was detected RFI.
	ThresholdsNorm threshNorm; //!< The norm (recommendation, protocol, etc.) which was used to define the harmful interference levels.
	//! The default constructor which calls the default constructor of structure _FreqValues_ and set type to "rfi", azimuth angle to zero, number of bands to zero and set, by default, threshold norm to SKA_MODE1.
	RFI() : FreqValues("rfi") { azimuthAngle=0.0; numOfRFIBands=0; threshNorm=ThresholdsNorm::SKA_MODE1;	}
	//! The copy constructor which receives a _RFI_ object.
	/*! \param [in] rfi A _RFI_ structure given to copy its attributes.	*/
	RFI(const RFI & rfi) {	operator=(rfi);		}
	//! The aim of this method is to clean the attributes of this structure.
	void Clear() { 	FreqValues::Clear(); azimuthAngle=0.0; numOfRFIBands=0; polarization.clear();	}
	//! An overloading of the assignment operator adapted to receive a _RFI_ object.
	/*! \param [in] anotherRFI Another _RFI_ structure given to copy its attributes.	*/
	const RFI & operator=(const RFI & anotherRFI)
	{
		azimuthAngle=anotherRFI.azimuthAngle; polarization=anotherRFI.polarization;
		numOfRFIBands=anotherRFI.numOfRFIBands; threshNorm=anotherRFI.threshNorm;
		frequencies=anotherRFI.frequencies; timeData=anotherRFI.timeData; values=anotherRFI.values;
		return *this;
	}
};

//! This structure is intended to store the parameters which are used to configure the spectrum analyzer in each frequency band.
struct BandParameters
{
	unsigned int bandNumber; //!< This is an integer number which identifies the frequency band (like an index).
	bool flagEnable; //!< This parameter determines if the band is used or not.
	float startFreq; //!< Initial frequency (Fstart) in Hz.
	float stopFreq; //!< Final frequency (Fstop) in Hz.
	float rbw; //!< Resolution Bandwidth (RBW) in Hz.
	float vbw; //!< Video Bandwidth (VBW) in Hz.
	unsigned int sweepTime; //!< Time to sweep the given span, expressed in ms.
	bool flagDefaultSamplePoints; //!< This parameter determines if the sample points number must be configured with user-defined number or if it is left with its default value which is determined by the Spectran device.
	unsigned int samplePoints; //!< Number of samples points. This value can be determined by the Spectran device (default value) or it can be a forced value.
	unsigned int detector; //!< Display detector: ”RMS” takes the sample as the root mean square of the values present in the bucket, or “Min/Max” takes two samples as the minimum and maximum peaks in the bucket.
};

#ifdef RASPBERRY_PI
//! This unnamed structure is intended to store the constants with the assignment of GPIO pins to input and output external signals.
struct
{
	const unsigned int SWITCH = 8; //!< This pin is initialized as an output in LOW state, so the RF switch output will start connected to the noise source.
	const unsigned int NOISE_SOURCE = 13; //!< This pin is initialized as an output in LOW state, so the noise source will start turned off.
	const unsigned int LNAS = 12;  //!< This pin is initialized as an output in LOW state, so the LNAs will start turned off.
	const unsigned int SPECTRAN = 14; //!< This pin is initialized as an input with the pull-down resistor enabled, so the Spectran device will start turned off.
	const unsigned int LED_SWEEP_CAPTURE = 9; //!< This pin is initialized as an output in LOW state, so the led will start turned off.
	const unsigned int LED_SWEEP_PROCESS = 10; //!< This pin is initialized as an output in LOW state, so the led will start turned off.
	const unsigned int LED_INIT_POS = 1; //!< This pin is initialized as an output in LOW state, so the led will start turned off.
	const unsigned int LED_NEXT_POS = 3; //!< This pin is initialized as an output in LOW state, so the led will start turned off.
	const unsigned int LED_POLARIZ = 5; //!< This pin is initialized as an output in LOW state, so the led will start turned off.
	const unsigned int BUTTON_ENTER = 2; //!< This pin is initialized as in input wit the pull-up resistor enabled, so the button must connect the pin to GND.
	//Pines de la clase AntennaPositioner
	//Se debe indicar los pines de PUL, DIR, EN, SENSOR_NORTE en donde van a ir conectados
	const unsigned int PUL=7; //pin 7
	const unsigned int DIRECCION=9; //pin 5
	const unsigned int EN=8; //pin 3
	const unsigned int SENSOR_NORTE=2; //pin 13
	const unsigned int POL=0; //pin 11
	const unsigned int FASE_A=1; //pin 12
	const unsigned int FASE_B=4; //pin 16
} piPins;

struct
{
	const int SWITCH_TO_NS = LOW; //!< This constant define the value the switch's pin must take to connect noise source to input, provided that the noise source is connected to switch's J2 connector.
	const int SWITCH_TO_ANT = !SWITCH_TO_NS; //!< This constant define the value the switch's pin must take to connect antenna to input, provided that the antenna is connected to switch's J1 connector.

	const int NS_ON = HIGH;
	const int NS_OFF = !NS_ON;

	const int LNAS_ON = HIGH;
	const int LNAS_OFF = !LNAS_ON;

	const int SPECTRAN_ON = HIGH;
	const int SPECTRAN_OFF = !SPECTRAN_ON;

	const int PUL_ON = HIGH;
	const int PUL_OFF = !PUL_ON;

	const int DIR_ON = HIGH;
	const int DIR_OFF = !DIR_ON;

	const int SENS_NOR_ON = HIGH;
	const int SENS_NOR_OFF = !SENS_NOR_ON;

	const int POL_HOR = HIGH;
	const int POL_VERT = !POL_HOR;

	const int FASE_A_ON = HIGH;
	const int FASE_A_OFF = !FASE_A_ON;

	const int FASE_B_ON = HIGH;
	const int FASE_B_OFF = !FASE_B_ON;
} pinsValues;
#endif

//#////////////////////////////////////////////////////////////////////


//#////////////////////GLOBAL FUNCTIONS//////////////////////

//! Function intended to compare floating-point numbers as approximately equal, taking into account the floating-point rounding errors.
bool approximatelyEqual(float a, float b);
//! Function intended to compare `std::vector<float>` containers as approximately equal, taking into account the floating-point rounding errors.
bool approximatelyEqual(std::vector<float> vectorA, std::vector<float> vectorB);

//! An overloading of unary operator - which negates the elements of a `std::vector<float>` container.
std::vector<FreqValues::value_type> operator-(const std::vector<FreqValues::value_type> & vect); //defined in Basics.cpp

//! This function stop the execution until any key is pressed by the user and it was used for debugging purpose.
void WaitForKey();

//! This function initializes all GPIO pins which are used for the input and output signals, in the way it is described in structure _piPins_.
void InitializeGPIO();

//! The aim of this function is to turn all LEDs off.
void TurnOffLeds();

//! The aim of this function is to turn on the RF front-end elements in a sequential manner, from the spectrum analyzer to the antenna..
void TurnOnFrontEnd();

//! The aim of this function is to turn off the RF front-end elements in a sequential manner, from the antenna to the spectrum analyzer.
void TurnOffFrontEnd();

//#/////////////////////////////////////////////////////////////



#endif /* BASICS_H_ */
