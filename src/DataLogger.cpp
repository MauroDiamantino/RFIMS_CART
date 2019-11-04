/*! \file DataLogger.cpp
 * 	\brief This file contains the definitions of several methods of the class _DataLogger_.
 * 	\author Mauro Diamantino
 */

#include "SweepProcessing.h"

////////////////////Friends functions////////////////////////

//! The function which is executed by the thread which is responsible for the concurrent uploading of the data files.
void *UploadThreadFunc(void *arg)
{
	auto * dataLoggerPtr = (DataLogger*) arg;

	try
	{
		dataLoggerPtr->ArchiveAndCompress();
		dataLoggerPtr->UploadData();
	}
	catch(std::exception & exc)
	{
		strcpy( dataLoggerPtr->threadMsg, exc.what() );
		pthread_exit( (void*)dataLoggerPtr->threadMsg );
	}

	return NULL;
}

//////////////////Class' methods////////////////////

/*! The constructor initializes all the internal attributes, checks if the corresponding folders exist and if
 * any folder does not exist then it is created. Also, it checks if there is a shell available to be able to
 * execute the external python script "client.py" to upload the data.
 */
DataLogger::DataLogger()
{
	sweepIndex=10000; //To make sure this variable will be set to zero the first time the method SaveData() is called
	flagNewBandsParam=false;
	flagNewFrontEndParam=false;
	flagStoredRFI=false;
	uploadThread=0;

	try
	{
		if( !boost::filesystem::exists(MEASUREMENTS_PATH) )
			boost::filesystem::create_directory(MEASUREMENTS_PATH);

		if( !boost::filesystem::exists(BANDS_PARAM_CSV_PATH) )
			boost::filesystem::create_directory(BANDS_PARAM_CSV_PATH);

		if( !boost::filesystem::exists(FRONT_END_PARAM_PATH) )
			boost::filesystem::create_directory(FRONT_END_PARAM_PATH);
	}
	catch(boost::filesystem::filesystem_error & exc)
	{
		rfims_exception rfimsExc("the creation of a directory failed");
		rfimsExc.Append( exc.what() );
		throw(rfimsExc);
	}

	ofs.exceptions( std::ofstream::failbit | std::ofstream::badbit );
	ofs.setf(std::ios::fixed, std::ios::floatfield);
	ofs.setf( std::ios::dec | std::ios::left | std::ios::showpoint);

	//It is controlled if the shell is available
	if( system(nullptr)==0 )
		cerr << "\nWarning: the shell is not available so the files will not be able to be compressed." << endl;
}

DataLogger::~DataLogger()
{
	ofs.exceptions( std::ofstream::goodbit );
	ofs.flush();
	ofs.close();

	void **retval = (void**) &sweepIndex; //The pointer to the return value of the thread is initialized with the direction
											//of any variable to avoid this pointer to be equal to NULL

	// if the thread finished
	int retValueJoin = pthread_join(uploadThread, retval);
	//Checking if the last operation finished wrongly or if the thread does not exist
	if(retValueJoin!=0 && retValueJoin!=ESRCH)
		cerr << "\nWarning: The checking of finishing of the thread to upload data failed." << endl;
	//Checking the value returned by the thread if that existed
	if(retValueJoin==0 && retval!=NULL)
		cerr << "\nWarning: " << (char*) (*retval) << endl;
}

/*! This method should be called each time the bands' parameters are reloaded (or loaded by first time),
 * because of the file [BASE_PATH](\ref BASE_PATH)/parameters/freqbands.txt was modified, to update the
 * file [BASE_PATH](\ref BASE_PATH)/parameters/csv/freqbands.csv to that has the same parameters, just
 * in a different format. The CSV file is generated because it is easier the bands' parameters to be
 * loaded from a file with CSV format than from a file with a more human-readable format like
 * freqbands.txt. Each time this method is called the file freqbands.csv is regenerated. This file is then
 * incorporated in the archive file, in the method ArchiveAndCompress().
 * \param [in] bandsParamVector A vector with the parameters of all frequency bands.
 */
void DataLogger::SaveBandsParamAsCSV(const std::vector<BandParameters> & bandsParamVector)
{
	if( !bandsParamVector.empty() )
	{
		boost::filesystem::path filePath(BANDS_PARAM_CSV_PATH);
		filePath /= "freqbands.csv";
		if( boost::filesystem::exists(filePath) )
			boost::filesystem::remove(filePath);

		try
		{
			ofs.open( filePath.string() );
		}
		catch(std::ofstream::failure& exc)
		{
			rfims_exception rfimsExc("the file " + filePath.string() + " could not be opened");
			rfimsExc.Append( exc.what() );
			throw( rfimsExc );
		}

		//Saving the header row
		ofs << "Band Index,Enabling,Fstart(MHz),Fstop(MHz),RBW(MHz),VBW(MHz),Sweep Time(ms),Sample Points,Detector\r\n";

		//Saving the data of each frequency band
		for(const BandParameters & oneBandParam : bandsParamVector)
		{
			ofs << oneBandParam.bandNumber << ',';
			ofs << oneBandParam.flagEnable << ',';
			ofs << std::setprecision(2) << (oneBandParam.startFreq)/1e6 << ','; //It is saved in MHz
			ofs << std::setprecision(2) << (oneBandParam.stopFreq)/1e6 << ','; //It is saved in MHz
			ofs << std::setprecision(4) << (oneBandParam.rbw)/1e6 << ','; //It is saved in MHz
			ofs << std::setprecision(4) << (oneBandParam.vbw)/1e6 << ','; //It is saved in MHz
			ofs << oneBandParam.sweepTime << ','; //It is saved in ms
			ofs << oneBandParam.samplePoints << ',';
			if( oneBandParam.detector==0 )
				ofs << "RMS";
			else
				ofs << "Min/Max";
			ofs << "\r\n";
		}

		ofs.flush();
		ofs.close();

		flagNewBandsParam=true;
	}
	else
		throw( rfims_exception("the data logger was asked to save an empty bands parameters vector.") );
}

/*! The given front end parameters are saved in two different files:
 * - [BASE_PATH](\ref BASE_PATH)/calibration/frontendparam/gain_DD-MM-YYYYTHH:MM:SS.csv
 * - [BASE_PATH](\ref BASE_PATH)/calibration/frontendparam/noisefigure_DD-MM-YYYYTHH:MM:SS.csv
 * where DD-MM-YYYYTHH:MM:SS is the timestamp of the current measurement cycle.
 *
 * Those files are then incorporated into the archive file which will be uploaded at the end
 * of the measurement cycle. If the front end parameters were not estimated in a measurement
 * cycle, then the default front end parameters are used, which are curves that were estimated
 * in the laboratory and which are saved in the following files:
 * - [BASE_PATH](\ref BASE_PATH)/calibration/frontendparam/default/gain_default.csv
 * - [BASE_PATH](\ref BASE_PATH)/calibration/frontendparam/default/noisefigure_default.csv
 * In that case, these files are incorporated into the archive file to be uploaded.
 * \param [in] gain A structure with the estimated values of the total front end gain versus the frequency.
 * \param [in] noiseFigure A structure with the estimated values of the total front end noise figure versus the frequency.
 */
void DataLogger::SaveFrontEndParam(const FreqValues & gain, const FreqValues & noiseFigure)
{
	if( !gain.Empty() && !noiseFigure.Empty() )
	{
		std::string timestamp = currMeasCycleTimestamp = gain.timeData.GetTimestamp();

		boost::filesystem::path filePath(FRONT_END_PARAM_PATH);

		std::string filename("noisefigure_");
		filename += currMeasCycleTimestamp + ".csv";
		filePath /= filename;
		try
		{
			ofs.open( filePath.string() );
		}
		catch(std::ofstream::failure& exc)
		{
			rfims_exception rfimsExc("the file " + filePath.string() + " could not be opened");
			rfimsExc.Append( exc.what() );
			throw( rfimsExc );
		}

		//Saving the noise figure data
		ofs << "Timestamp";
		for(const auto & freq : noiseFigure.frequencies)
			ofs << ',' << std::setprecision(4) << double(freq)/1e6;
		ofs << "\r\n";
		ofs << timestamp;
		for(const auto& nf : noiseFigure.values)
			ofs << ',' << std::setprecision(2) << nf;
		ofs << "\r\n";

		ofs.flush();
		ofs.close();

		filename = "gain_" + currMeasCycleTimestamp + ".csv";
		filePath.remove_filename();
		filePath /= filename;
		try
		{
			ofs.open( filePath.string() );
		}
		catch(std::ofstream::failure& exc)
		{
			rfims_exception rfimsExc("the file " + filePath.string() + " could not be opened");
			rfimsExc.Append( exc.what() );
			throw( rfimsExc );
		}

		//Saving the gain data
		ofs << "Timestamp";
		for(const auto& freq : gain.frequencies)
			ofs << ',' << std::setprecision(4) << double(freq)/1e6;
		ofs << "\r\n";
		ofs << timestamp;
		for(const auto& g : gain.values)
			ofs << ',' << std::setprecision(1) << g;
		ofs << "\r\n";

		ofs.flush();
		ofs.close();

		flagNewFrontEndParam=true;
	}
	else
		throw( rfims_exception("the data logger was asked to save empty front end parameters curves.") );
}

/*!	The given sweep is saved in [BASE_PATH](\ref BASE_PATH)/measurements/ with the filename format
 * "sweep_DD-MM-YYYYTHH:MM:SS.csv" where the last part is the timestamp of the measurement cycle,
 * which correspond to the beginning of this one.
 *
 * All sweeps which corresponds to the same measurement cycle are saved in the same file and the method
 * automatically create a new file or reopen the corresponding file each time a new sweep must be saved.
 * \param [in] sweep A structure with the sweep to be saved.
 */
void DataLogger::SaveSweep(const Sweep & sweep)
{
	//The data are saved only if a sweep has been loaded
	if( !sweep.Empty() )
	{
		if(++sweepIndex >= numOfSweeps) //numOfSweeps should be the double of the number of azimuth positions
			sweepIndex=0;

		if(sweepIndex==0)
		{
			//New measurement cycle//

			//Updating the first sweep date if the method SaveFrontEndParam() has not been called before
			if(!flagNewFrontEndParam)
				currMeasCycleTimestamp=sweep.timeData.GetTimestamp();

			//Creating the new sweeps file
			boost::filesystem::path filePath(MEASUREMENTS_PATH);
			filePath /= ( "sweeps_" + currMeasCycleTimestamp + ".csv" );
			try
			{
				ofs.open( filePath.string() );
			}
			catch(std::ofstream::failure& exc)
			{
				rfims_exception rfimsExc("the file " + filePath.string() + " could not be created");
				rfimsExc.Append( exc.what() );
				throw(rfimsExc);
			}
			
			//Writing header with frequency values
			ofs << "Timestamp,Azimuthal Angle,Polarization";
			for(const auto& freq : sweep.frequencies)
				ofs << ',' << std::setprecision(4) << double(freq)/1e6; //The frequency values are saved in MHz
			ofs << "\r\n";
		}
		else
		{
			//Opening an existing sweeps file
			boost::filesystem::path filePath(MEASUREMENTS_PATH);
			filePath /= ( "sweeps_" + currMeasCycleTimestamp + ".csv" );
			try
			{
				ofs.open( filePath.string(), std::ofstream::out | std::ofstream::app );
			}
			catch(std::ofstream::failure& exc)
			{
				rfims_exception rfimsExc("the file " + filePath.string() + " could not be opened");
				rfimsExc.Append( exc.what() );
				throw( rfimsExc );
			}
		}
		
		//Writing the extra data
		ofs << sweep.timeData.GetTimestamp();
		ofs << ',' << std::setprecision(1) << sweep.azimuthAngle;
		ofs << ',' << sweep.polarization;

		//Writing sweep's power values
		for(const auto& power : sweep.values)
			ofs << ',' << std::setprecision(1) << power; //The power values are saved in dBm with just one decimal digit
		ofs << "\r\n";
		
		ofs.flush();
		ofs.close();
	}
	else
		throw( rfims_exception("the data logger was asked to save an empty sweep.") );
}

/*! Each given structure with the RFI detected in the last sweep is saved in a different file with the filename format
 * RFI_x.csv, where 'x' is an integer number between 1 and 2*(number of azimuth positions). So each file corresponds to
 * a sweep of the measurement cycle and all the files of determined measurement cycle are in the same folder,
 * [BASE_PATH](\ref BASE_PATH)/measurement/RFI_DD-MM-YYYYTHH:MM:SS/ where the last part of the folder's name is the
 * timestamp of the measurement cycle.
 * \param [in] rfi A structure with RFI to be saved.
 */
void DataLogger::SaveRFI(const RFI& rfi)
{
	if( !rfi.Empty() )
	{
		boost::filesystem::path filePath(MEASUREMENTS_PATH);

		//Adding the folder's name to the path
		filePath /= ("RFI_" + currMeasCycleTimestamp); //This variable is controlled in SaveSweep() or SaveFrontEndParam()

		if(sweepIndex==0) //This variables is controlled in SaveSweep()
			//Creating a new folder to save there the RFI files corresponding to a new measurement cycle
			boost::filesystem::create_directory(filePath);

		//Adding the filename to the path
		std::ostringstream oss;
		oss << "RFI_" << (sweepIndex+1) << ".csv";
		filePath /= oss.str();

		ofs.open( filePath.string() );

		//Writing the header with frequency values where the RFI was detected
		ofs << "RFI Index,Timestamp,Azimuthal Angle,Polarization";
		for(const auto& freq : rfi.frequencies)
			ofs << ',' << std::setprecision(4) << double(freq)/1e6; //The frequency values are saved in MHz
		ofs << "\r\n";

		//Writing the extra data
		ofs << (sweepIndex+1);
		ofs << ',' << rfi.timeData.GetTimestamp();
		ofs << ',' << std::setprecision(1) << rfi.azimuthAngle;
		ofs << ',' << rfi.polarization;

		//Writing the power values, in dBm
		for(const auto& power : rfi.values)
			ofs << ',' << std::setprecision(1) << power;
		ofs << "\r\n";

		ofs.flush();
		ofs.close();

		flagStoredRFI=true;
	}
	else
		throw( rfims_exception("the data logger was asked to save an empty RFI structure.") );
}

/*!	To archive the data files the utility 'tar' is used and thr utility 'lzma' is used to compress to resulting archive
 * file. The resulting compressed archive file is name as "rfims_data_DD-MM-YYYYTHH:MM:SS.tar.lzma", where the last
 * part, before the extension is the timestamp of the corresponding measurement cycle.
 */
void DataLogger::ArchiveAndCompress()
{
	///////////Copying data files to the uploads folder////////////
	boost::filesystem::path destPath(UPLOADS_PATH);

	boost::filesystem::path sweepsFilePath(MEASUREMENTS_PATH);
	std::string sweepFilename = "sweeps_" + currMeasCycleTimestamp + ".csv";
	sweepsFilePath /= sweepFilename;
	if( boost::filesystem::exists(sweepsFilePath) )
		boost::filesystem::copy_file(sweepsFilePath, (destPath / sweepFilename), boost::filesystem::copy_option::overwrite_if_exists);
	else
		throw( rfims_exception("the sweep file does not exist.") );

	boost::filesystem::path gainFilePath(FRONT_END_PARAM_PATH);
	boost::filesystem::path noiseFigFilePath(FRONT_END_PARAM_PATH);
	std::string gainFilename;
	std::string noiseFigFilename;
	if(flagNewFrontEndParam)
	{
		gainFilename = "gain_" + currMeasCycleTimestamp + ".csv";
		gainFilePath /= gainFilename;
		noiseFigFilename = "noisefigure_" + currMeasCycleTimestamp + ".csv";
		noiseFigFilePath /= noiseFigFilename;
	}
	else
	{
		gainFilename = "gain_default.csv";
		gainFilePath /= "default";
		gainFilePath /= gainFilename;
		noiseFigFilename = "noisefigure_default.csv";
		noiseFigFilePath /= "default";
		noiseFigFilePath /= noiseFigFilename;
	}

	if( boost::filesystem::exists(gainFilePath) )
		boost::filesystem::copy_file(gainFilePath, (destPath / gainFilename), boost::filesystem::copy_option::overwrite_if_exists);
	else
		throw( rfims_exception("the gain file does not exist.") );

	if( boost::filesystem::exists(noiseFigFilePath) )
		boost::filesystem::copy_file(noiseFigFilePath, (destPath / noiseFigFilename), boost::filesystem::copy_option::overwrite_if_exists);
	else
		throw( rfims_exception("the noise figure file does not exist.") );

	if(flagNewBandsParam)
	{
		boost::filesystem::path bandsParamFilePath(BANDS_PARAM_CSV_PATH);
		bandsParamFilePath /= "freqbands.csv";
		if( boost::filesystem::exists(bandsParamFilePath) )
			boost::filesystem::copy_file(bandsParamFilePath, (destPath / "freqbands.csv"), boost::filesystem::copy_option::overwrite_if_exists);
		else
			throw( rfims_exception("the bands parameters file (CSV) does not exist.") );
	}

	std::string rfiFolderName = "RFI_" + currMeasCycleTimestamp;
	if(flagStoredRFI)
	{
		boost::filesystem::path rfiPath(MEASUREMENTS_PATH);
		rfiPath /= rfiFolderName;
		if( boost::filesystem::exists(rfiPath) && boost::filesystem::is_directory(rfiPath) )
		{
			auto destRFIFolderPath = destPath / rfiFolderName;
			boost::filesystem::create_directory(destRFIFolderPath);
			for(const boost::filesystem::directory_entry& rfiFile : boost::filesystem::directory_iterator(rfiPath))
				boost::filesystem::copy_file(rfiFile.path(), (destRFIFolderPath / rfiFile.path().filename()), boost::filesystem::copy_option::overwrite_if_exists);
		}
		else
			throw( rfims_exception("the folder with RFI files does not exist or there is an error in the path.") );
	}

	/////////////////////////////////////////////////////////////////////////////

	///////////////////Archiving the files//////////////////////
	std::string archiveName = "rfims_data_" + currMeasCycleTimestamp + ".tar";

	//If there is an archive file with the same name is removed
	boost::filesystem::path archivePath(UPLOADS_PATH);
	archivePath /= archiveName;
	if( boost::filesystem::exists(archivePath) )
		boost::filesystem::remove(archivePath);

	//Building the command to call 'tar'
	std::string command("tar --create");
	command += " --file " + archivePath.string();
	command += " --directory " + UPLOADS_PATH + '/';
	command += ' ' + sweepFilename;
	command += ' ' + gainFilename;
	command += ' ' + noiseFigFilename;
	if(flagNewBandsParam)
		command += " freqbands.csv";
	if(flagStoredRFI)
		command += ' ' + rfiFolderName;

	//Calling the utility 'tar'
	if( system( command.c_str() ) < 0 )
		throw( rfims_exception("the calling to the utility 'tar', using system(), to archive the data files failed.") );
	//////////////////////////////////////////////////////////////

	/////////////Compressing the archive with the utility 'lzma'/////////////////
	std::string compArchiveName = archiveName + ".lzma";

	//If there is a compressed archive file with the same name is removed
	boost::filesystem::path compArchivePath(UPLOADS_PATH);
	compArchivePath /= compArchiveName;
	if( boost::filesystem::exists(compArchivePath) )
		boost::filesystem::remove(compArchivePath);

	//Building the command
	command.clear();
	command += "lzma --compress -9 --threads=0 ";
	command += archivePath.string();

	//Calling the utility 'lzma'
	if( system( command.c_str() ) < 0 )
		throw( rfims_exception("the calling to the utility 'lzma', using system(), to compress the archive file failed.") );
	///////////////////////////////////////////////////////////////

	////////Deleting the files which were copied to the uploads folder//////////
	boost::filesystem::remove(UPLOADS_PATH + '/' + sweepFilename);
	boost::filesystem::remove(UPLOADS_PATH + '/' + gainFilename);
	boost::filesystem::remove(UPLOADS_PATH + '/' + noiseFigFilename);
	if(flagNewBandsParam)
		boost::filesystem::remove(UPLOADS_PATH + '/' + "freqbands.csv");
	if(flagStoredRFI)
		boost::filesystem::remove_all(UPLOADS_PATH + '/' + rfiFolderName);
	//////////////////////////////////////////////////////////////////

	filesToUpload.push(compArchiveName);

	flagNewFrontEndParam=false;
	flagNewBandsParam=false;
	flagStoredRFI=false;
}

void DataLogger::DeleteOldFiles() const
{
	//Getting the date 30 days back
	TimeData oneMonthBackDate;
	oneMonthBackDate.SetTimestamp(currMeasCycleTimestamp);
	oneMonthBackDate.TurnBackDays(30);

	//Deleting old sweeps files and directories with old rfi files
	for( const auto & dirEntry : boost::filesystem::directory_iterator(MEASUREMENTS_PATH) )
	{
		std::string dirEntryName = dirEntry.path().filename().string();
		std::size_t datePos = dirEntryName.find('_');
		if( datePos == std::string::npos )
			cerr << "\nWarning: A file or directory with a unexpected name format was found in " << MEASUREMENTS_PATH << '.' << endl;
		else
		{
			datePos++;
			TimeData fileDate;
			fileDate.SetDate( dirEntryName.substr(datePos, 10) );
			if( fileDate < oneMonthBackDate )
				boost::filesystem::remove_all( dirEntry.path() );
		}
	}

	//Deleting old front end parameters files
	for( const auto & dirEntry : boost::filesystem::directory_iterator(FRONT_END_PARAM_PATH) )
	{
		std::string dirEntryName = dirEntry.path().filename().string();
		if(dirEntryName != "default")
		{
			size_t datePos = dirEntryName.find('_');
			if( datePos == std::string::npos )
				cerr << "\nWarning: A file with a unexpected name format was found in " << FRONT_END_PARAM_PATH << '.' << endl;
			else
			{
				datePos++;
				TimeData fileDate;
				fileDate.SetDate( dirEntryName.substr(datePos, 10) );
				if( fileDate < oneMonthBackDate )
					boost::filesystem::remove( dirEntry.path() );
			}
		}
	}
}

/*! To upload the files the script /usr/local/client.py is called. This script try to send the archive
 * file many times through one hour, taking into account the possibility that there is no Internet
 * connection in the first try. If the script achieves the sending, then it wakes up the remote server
 * to that one to read the files, and finally the script removes the local archive file. If the script
 * ends with errors, the file is not deleted and remains in a queue waiting to be send. The idea is the
 * uploading to perform at the end of each measurement cycle.
 */
void DataLogger::UploadData()
{
	int retValue=0, procRetValue=0;
	while( !filesToUpload.empty() && retValue==0 )
	{
		std::string command("python3 /usr/local/client.py ");
		command += UPLOADS_PATH + '/' + filesToUpload.front();
		if( ( retValue = system( command.c_str() ) ) < 0 )
		{
			std::ostringstream oss;
			oss << "the calling to the utility client.py to upload the data failed.";
			throw( rfims_exception( oss.str() ) );
		}

		if(retValue==0)
			filesToUpload.pop();
		else
		{
			procRetValue = retValue >> 8;
			std::string str = "the utility client.py was executed but this failed to upload data: ";
			switch(procRetValue)
			{
				case 3:
					str += "there is no Internet connection.";
					break;
				case 4:
					str += "the remote server did not wake up.";
					break;
				case 5:
					str += "the archive file could not be transmitted to the remote server.";
					break;
				default:
					str += "unknown error";
			}
			throw( rfims_exception( str ) );
		}
	}
}

/*! This method creates a thread where the methods `ArchiveAndCompress()` and `UploadData()` are called.
 * After the creation of the thread, the method ends and the main thread can continues with the next
 * operations, like the moving of the antenna, the capture of a new sweep, etc. The next time this method
 * is called, it will control if the last thread has finished, if not, the method will wait to the thread
 * to finish, and then it will create a new thread for the uploading of the new data.
 */
void DataLogger::PrepareAndUploadData()
{
	void **retval = (void**) &sweepIndex; //The pointer to the return value of the thread is initialized with the direction
										//of any variable to avoid this pointer to be equal to NULL

	//Checking if the previous thread finished
	int retValueJoin = pthread_join(uploadThread, retval);
	//Checking if the last operation finished wrongly or if the thread does not exist
	if(retValueJoin!=0 && retValueJoin!=ESRCH)
		throw rfims_exception("the checking of the finishing of the last thread to upload data failed.");

	//Checking the value returned by the thread if that existed
	if(retValueJoin==0)
	{
		//Checking the value returned by the last thread
		if(retval==PTHREAD_CANCELED)
			cerr << "\nWarning: the last thread to upload data was cancelled." << endl;
		else if(retval!=NULL)
			throw rfims_exception( (char*) (*retval) );
	}

	//Creating a new thread to prepare data (archive and compress) and to upload the data
	int retValueCreate = pthread_create(&uploadThread, NULL, UploadThreadFunc, (void*)this);
	if(retValueCreate!=0)
	{
		rfims_exception exc("the creation of the thread to prepare and upload data failed");
		if(retValueCreate==EAGAIN)
			exc.Append("insufficient resources to create a thread.");
		else
			exc.Append("unknown error.");
		throw(exc);
	}
}
