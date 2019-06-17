#include "SweepProcessing.h"

DataLogger::DataLogger()
{
	sweepIndex=10000; //To make sure this variable will be set to zero the first time the method SaveData() is called
	flagNewBandsParam=false;
	flagNewFrontEndParam=false;

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
		CustomException customExc("The creation of a directory failed: ");
		customExc.Append( exc.what() );
		throw(customExc);
	}

	ofs.exceptions( std::ofstream::failbit | std::ofstream::badbit );
	ofs.setf(std::ios::fixed, std::ios::floatfield);

	//It is controlled if the shell is available
	if( system(nullptr)==0 )
		cerr << "\nWarning: the shell is not available so the files will not be able to be compressed." << endl;
}

DataLogger::~DataLogger()
{
	ofs.exceptions( std::ofstream::goodbit );
	ofs.close();
}

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
			CustomException customExc("The file " + filePath.string() + " could not be opened: ");
			customExc.Append( exc.what() );
			throw( customExc );
		}

		//Saving the header row
		ofs << "Band Index,Enabling,Fstart(MHz),Fstop(MHz),RBW(Hz),VBW(Hz),Sweep Time,Sample Points,Detector\r\n";

		//Saving the data of each frequency band
		for(const BandParameters & oneBandParam : bandsParamVector)
		{
			ofs << oneBandParam.bandNumber << ',';
			ofs << oneBandParam.flagEnable << ',';
			ofs << std::setprecision(1) << (oneBandParam.startFreq)/1e6 << ','; //It is saved in MHz
			ofs << std::setprecision(1) << (oneBandParam.stopFreq)/1e6 << ','; //It is saved in MHz
			ofs << oneBandParam.rbw << ','; //It is saved in Hz
			ofs << oneBandParam.vbw << ','; //It is saved in Hz
			ofs << oneBandParam.sweepTime << ','; //It is saved in ms
			ofs << oneBandParam.samplePoints << ',';
			if( oneBandParam.detector==0 )
				ofs << "RMS";
			else
				ofs << "Min/Max";
			ofs << "\r\n";
		}

		ofs.close();

		flagNewBandsParam=true;
	}
	else
		throw( CustomException("The data logger was asked to save an empty bands parameters vector.") );
}

void DataLogger::SaveFrontEndParam(const FreqValues & gain, const FreqValues & noiseFigure)
{
	if( !gain.Empty() && !noiseFigure.Empty() )
	{
		currMeasCycleDate = gain.timeData.GetDate();
		std::string timestamp = gain.timeData.GetTimestamp();

		boost::filesystem::path filePath(FRONT_END_PARAM_PATH);

		std::string filename("noisefigure_");
		filename += currMeasCycleDate + ".csv";
		filePath /= filename;
		try
		{
			ofs.open( filePath.string() );
		}
		catch(std::ofstream::failure& exc)
		{
			CustomException customExc("The file " + filePath.string() + " could not be opened: ");
			customExc.Append( exc.what() );
			throw( customExc );
		}

		//Saving the noise figure data
		ofs << "Timestamp";
		for(const auto & freq : noiseFigure.frequencies)
			//ofs << ',' << std::setprecision(4) << (freq/1e6);
			ofs << ',' << std::setprecision(4) << double(freq)/1e6;
		ofs << "\r\n";
		ofs << timestamp;
		for(const auto& nf : noiseFigure.values)
			ofs << ',' << std::setprecision(2) << nf;
		ofs << "\r\n";

		ofs.close();

		filename = "gain_" + currMeasCycleDate + ".csv";
		filePath.remove_filename();
		filePath /= filename;
		try
		{
			ofs.open( filePath.string() );
		}
		catch(std::ofstream::failure& exc)
		{
			CustomException customExc("The file " + filePath.string() + " could not be opened: ");
			customExc.Append( exc.what() );
			throw( customExc );
		}

		//Saving the gain data
		ofs << "Timestamp";
		for(const auto& freq : gain.frequencies)
			//ofs << ',' << std::setprecision(4) << (freq/1e6);
			ofs << ',' << std::setprecision(4) << double(freq)/1e6;
		ofs << "\r\n";
		ofs << timestamp;
		for(const auto& g : gain.values)
			ofs << ',' << std::setprecision(1) << g;
		ofs << "\r\n";

		ofs.close();

		flagNewFrontEndParam=true;
	}
	else
		throw( CustomException("The data logger was asked to save empty front end parameters curves.") );
}

void DataLogger::SaveSweep(const Sweep & sweep)
{
	//The data are saved only if a sweep has been loaded
	if( !sweep.Empty() )
	{
		if(++sweepIndex >= 2*NUM_OF_POSITIONS) //2 polarizations multiplied by NUM_OF_POSITIONS azimuthal positions
			sweepIndex=0;

		if(sweepIndex==0)
		{
			/////////New measurement cycle//////////

			//Updating the first sweep date if the method SaveFrontEndParam() has not been called before
			if(!flagNewFrontEndParam)
				currMeasCycleDate=sweep.timeData.GetDate();

			//Creating the new sweeps file
			boost::filesystem::path filePath(MEASUREMENTS_PATH);
			filePath /= ( "sweeps_" + currMeasCycleDate + ".csv" );
			try
			{
				ofs.open( filePath.string() );
			}
			catch(std::ofstream::failure& exc)
			{
				CustomException customExc("The file " + filePath.string() + " could not be created: ");
				customExc.Append( exc.what() );
				throw(customExc);
			}
			
			//Writing header with frequency values
			ofs << "Timestamp,Azimuthal Angle,Polarization";
			for(const auto& f : sweep.frequencies)
				//ofs << std::setprecision(4) << ',' << (f/1e6);
				ofs << std::setprecision(4) << ',' << double(f)/1e6; //The frequency values are saved in MHz
			ofs << "\r\n";
		}
		else
		{
			//Opening an existing sweeps file
			boost::filesystem::path filePath(MEASUREMENTS_PATH);
			filePath /= ( "sweeps_" + currMeasCycleDate + ".csv" );
			try
			{
				ofs.open( filePath.string(), std::ofstream::out | std::ofstream::app );
			}
			catch(std::ofstream::failure& exc)
			{
				CustomException customExc("The file " + filePath.string() + " could not be opened: ");
				customExc.Append( exc.what() );
				throw( customExc );
			}
		}
		
		//Writing sweep's power values
		ofs << sweep.timeData.GetTimestamp() << ',' << std::setprecision(4) << sweep.azimuthAngle;
		ofs << ',' << sweep.polarization;
		for(const auto& power : sweep.values)
			ofs << std::setprecision(1) << ',' << power; //The power values are saved in dBm with just one decimal digit
		ofs << "\r\n";
		
		ofs.close();
	}
	else
		throw( CustomException("The data logger was asked to save an empty sweep.") );
}

void DataLogger::CompressLastFiles()
{
	///////////Copying data files to the uploads folder////////////
	boost::filesystem::path destPath(UPLOADS_PATH);

	boost::filesystem::path sweepsFilePath(MEASUREMENTS_PATH);
	std::string sweepFilename = "sweeps_" + currMeasCycleDate + ".csv";
	sweepsFilePath /= sweepFilename;
	if( boost::filesystem::exists(sweepsFilePath) )
		boost::filesystem::copy_file(sweepsFilePath, (destPath / sweepFilename), boost::filesystem::copy_option::overwrite_if_exists);
	else
		throw( CustomException("The sweep file does not exist.") );

	boost::filesystem::path gainFilePath(FRONT_END_PARAM_PATH);
	boost::filesystem::path noiseFigFilePath(FRONT_END_PARAM_PATH);
	std::string gainFilename;
	std::string noiseFigFilename;
	if(flagNewFrontEndParam)
	{
		gainFilename = "gain_" + currMeasCycleDate + ".csv";
		noiseFigFilename = "noisefigure_" + currMeasCycleDate + ".csv";
	}
	else
	{
		gainFilename = "gain_default.csv";
		noiseFigFilename = "noisefigure_default.csv";
	}

	gainFilePath /= gainFilename;
	if( boost::filesystem::exists(gainFilePath) )
		boost::filesystem::copy_file(gainFilePath, (destPath / gainFilename), boost::filesystem::copy_option::overwrite_if_exists);
	else
		throw( CustomException("The gain file does not exist.") );

	noiseFigFilePath /= noiseFigFilename;
	if( boost::filesystem::exists(noiseFigFilePath) )
		boost::filesystem::copy_file(noiseFigFilePath, (destPath / noiseFigFilename), boost::filesystem::copy_option::overwrite_if_exists);
	else
		throw( CustomException("The noise figure file does not exist.") );

	if(flagNewBandsParam)
	{
		boost::filesystem::path bandsParamFilePath(BANDS_PARAM_CSV_PATH);
		bandsParamFilePath /= "freqbands.csv";
		if( boost::filesystem::exists(bandsParamFilePath) )
			boost::filesystem::copy_file(bandsParamFilePath, (destPath / "freqbands.csv"), boost::filesystem::copy_option::overwrite_if_exists);
		else
			throw( CustomException("The bands parameters file (CSV) does not exist.") );
	}
	/////////////////////////////////////////////////////////////////////////////

	///////////////////Archiving and compressing//////////////////////

	//Calling the utility 'tar' to archive the data files
	std::string archiveName = "rfims_data_" + currMeasCycleDate + ".tar";

	std::string command("tar --create --file ");
	command += UPLOADS_PATH + '/' + archiveName + ' ';
	command += "--directory " + UPLOADS_PATH + "/ ";
	command += sweepFilename;
	if( system( command.c_str() ) < 0 )
		throw( CustomException("The calling to the utility 'tar', using system(), to archive the data files failed.") );

	command.clear();
	command += "tar --append --file ";
	command += UPLOADS_PATH + '/' + archiveName + ' ';
	command += "--directory " + UPLOADS_PATH + "/ ";
	command += gainFilename;
	if( system( command.c_str() ) < 0 )
		throw( CustomException("The calling to the utility 'tar', using system(), to archive the data files failed.") );

	command.clear();
	command += "tar --append --file ";
	command += UPLOADS_PATH + '/' + archiveName + ' ';
	command += "--directory " + UPLOADS_PATH + "/ ";
	command += noiseFigFilename;
	if( system( command.c_str() ) < 0 )
		throw( CustomException("The calling to the utility 'tar', using system(), to archive the data files failed.") );

	if(flagNewBandsParam)
	{
		command.clear();
		command += "tar --append --file ";
		command += UPLOADS_PATH + '/' + archiveName + ' ';
		command += "--directory " + UPLOADS_PATH + "/ ";
		command += "freqbands.csv";
		if( system( command.c_str() ) < 0 )
			throw( CustomException("The calling to the utility 'tar', using system(), to archive the data files failed.") );
	}

	//Compressing with lzma
	command.clear();
	command += "lzma --compress -9 --threads=0 ";
	command += UPLOADS_PATH + '/' + archiveName;
	if( system( command.c_str() ) < 0 )
		throw( CustomException("The calling to the utility 'lzma', using system(), to compress the archive file failed.") );
	archiveName += ".lzma";
	////////////////////////////////////////////////////////////////////////////

	////////Deleting the files which were copied to the uploads folder//////////
	boost::filesystem::remove(UPLOADS_PATH + '/' + sweepFilename);
	boost::filesystem::remove(UPLOADS_PATH + '/' + gainFilename);
	boost::filesystem::remove(UPLOADS_PATH + '/' + noiseFigFilename);
	if(flagNewBandsParam)
		boost::filesystem::remove(UPLOADS_PATH + '/' + "freqbands.csv");
	//////////////////////////////////////////////////////////////////

	filesToUpload.push(archiveName);

	flagNewFrontEndParam=false;
	flagNewBandsParam=false;
}

void DataLogger::DeleteOldFiles() const
{
	//Getting the date 30 days back
	TimeData oneMonthBackDate;
	oneMonthBackDate.SetDate(currMeasCycleDate);
	oneMonthBackDate.TurnBackDays(30);

	//Deleting old sweeps files
	for( const auto & entry : boost::filesystem::directory_iterator(MEASUREMENTS_PATH) )
	{
		std::string filename = entry.path().filename().string();
		size_t datePos = filename.find('_');
		if( datePos == std::string::npos )
			cerr << "\nWarning: A file with a unexpected name was found." << endl;
		else
		{
			datePos++;
			TimeData fileDate;
			fileDate.SetDate( filename.substr(datePos, 10) );
			if( fileDate < oneMonthBackDate )
				boost::filesystem::remove( entry.path() );
		}
	}

	//Deleting old front end parameters files
	for( const auto & entry : boost::filesystem::directory_iterator(FRONT_END_PARAM_PATH) )
	{
		std::string filename = entry.path().filename().string();
		size_t datePos = filename.find('_');
		if( datePos == std::string::npos )
			cerr << "\nWarning: A file with a unexpected name was found." << endl;
		else
		{
			datePos++;
			TimeData fileDate;
			fileDate.SetDate( filename.substr(datePos, 10) );
			if( fileDate < oneMonthBackDate )
				boost::filesystem::remove( entry.path() );
		}
	}
}

void DataLogger::UploadData()
{
	int ret=0;
	while( !filesToUpload.empty() && ret==0 )
	{
		std::string command("python3 /usr/local/client.py ");
		command += UPLOADS_PATH + '/' + filesToUpload.front();
		if( ( ret = system( command.c_str() ) ) < 0 )
		{
			std::ostringstream oss;
			oss << "The calling to the utility client.py, using system(), to upload the data failed. ";
			oss << filesToUpload.size() << " archive files remain to be uploaded.";
			throw( CustomException( oss.str() ) );
		}

		if(ret==0)
		{
			filesToUpload.pop();
			if( filesToUpload.empty() )
				cout << "All archive files were uploaded" << endl;
		}
		else
		{
			std::ostringstream oss;
			oss << "The utility client.py returned the following error message:  " << ret;
			oss << ". " << filesToUpload.size() << " files remain to be uploaded.";
			throw( CustomException( oss.str() ) );
		}
	}
}
