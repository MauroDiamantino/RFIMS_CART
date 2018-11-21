//Libraries
#include <vector>
#include <string>

class Command{
private:
	string gruop;
	vector<string> fields;
	string variable;
	int argument;
public:
	Command();
	~Command();
	void SetSpectranVariable(string, unsigned int);
	void ToAuthenticate(void);
	void ToServer(string);
	void GetInfoSpectran(string);
	void SetUpSpectranFcn(string);
	string GetString(void);
	string GetGroup(){	return group;	}
	string GetField(unsigned int);
	string GetVariable(){	return variable;	}
	int GetArgument(){	return argument;	}
}

class Reply{
private:
	string type;
	vector<string> fields;
	vector<string> subfields; //contien los subcampos de uno solo de los campos de la respuesta
	char subfieldSeparator;
public:
	Reply();
	Reply(string);
	~Reply();
	void SetUpReply(string);
	string GetString();
	string GetType(){	retunr type;	}
	string GetField(unsigned int);
	string GetSubfield(unsigned int);
}

class ASWEEPReply : public Reply{
private:
	vector<string> sweepValues; //corresponde a un solo subcampo
	char valueSeparator;
public:
	ASWEEPReply();
	ASWEEPReply(string);
	~ASWEEPReply();
	void SetUpReply(string);
	vector<string> GetSweepValues();
};
