//Libraries
#include <iostream>
#include <vector>
#include <string>
#include <boost/algorithm/string/case_conv.hpp>

using namespace std;
using namespace ios;

class Command{
private:
	vector<string> fields;
	string variable;
	string argument;
public:
	Command();
	~Command();
	void SetSpectranVariable(string, string);
	void ToAuthenticate(void);
	void ToServer(string);
	void GetInfoSpectran(string);
	void SetUpSpectranFcn(string);
	string GetString(void);
	string GetGroup(){	return fields[0];	}
	string GetField(unsigned int);
	string GetVariable(){	return variable;	}
	string GetArgument(){	return argument;	}
	unsigned int GetNumOfFields(){	return fields.size();	}
};

class Reply{
private:
	vector<string> fields;
	vector<string> subfields; //contiene los subcampos de uno solo de los campos de la respuesta
	char subfieldSeparator;
public:
	Reply();
	Reply(string);
	~Reply();
	void SetUpReply(string);
	string GetString();
	string GetType(){	return fields[0];	}
	string GetField(unsigned int);
	string GetSubfield(unsigned int);
	unsigned int GetNumOfFields(){	return fields.size();	}
	unsigned int GetNumOfSubFields(){	return subfields.size();	}
};

class ASWEEPReply : public Reply{
private:
	vector<string> powerValues; //corresponde a un solo subcampo
	vector<string> freqValues;
	char valueSeparator;
public:
	ASWEEPReply();
	ASWEEPReply(string);
	~ASWEEPReply();
	virtual void SetUpReply(string);
	vector<string> GetPowerValues(){	return powerValues;	}
	vector<string> GetFreqValues(){	return freqValues;	}
	unsigned int GetNumOfSweepValues(){	return powerValues.size();	}
};
