/*
 * SpectranInterface.h
 *
 *  Created on: 26/02/2019
 *      Author: new-mauro
 */

#ifndef SPECTRANINTERFACE_H_
#define SPECTRANINTERFACE_H_

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

class Command{
	////Attributes////
	//Constants
	const unordered_map<string,uint8_t>& VARIABLE_ID;
	const unordered_map<float,float>& RBW_INDEX;
	//Variables
	vector<uint8_t> bytes;
	string commandType; //Verify, LogOut, GetSTPVar or SetSTPVar
	string variableName;
	float value;
	////Private methods////
	void FillBytesVector();
public:
	////Class interface////
	Command(const unordered_map<string,unsigned int>& var_id,
			const unordered_map<float,float>& rbw_ind);
	Command(const unordered_map<string,unsigned int>& var_id,
			const unordered_map<float,float>& rbw_ind,
			string& commType);
	~Command();
	void SetAs(string commType){	commandType=commType;	}
	void SetAs(string commType, string varName, float val=0.0);
	void SetParameters(string varName, float val=0.0);
	string GetCommandType(){	return commandType;	}
	string GetVariableName(){	return variableName;	}
	float GetValue(){	return value;	}
	vector<uint8_t> GetBytesVector(){	return bytes;	}
	const uint8_t* GetBytesPointer(){	return bytes.data();	}
	unsigned int GetNumOfBytes(){	return bytes.size();	}
	void Clear();
	Command& operator=(Command& command);
};



#endif /* SPECTRANINTERFACE_H_ */
