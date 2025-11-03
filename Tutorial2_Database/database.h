#include <iostream>
#include <map>
#include <vector>
#include <string>

using namespace std;

class database{

	private:
		map<string,vector<string>> dataStorage;
		string name;
	public:
		database(string name);
		database() :database("") {};
		
		bool addRecord(string key,string data);
		bool addRecordSet(string key,vector<string> dataSet);
		string getRecord(string key,int index);
		
		~database();
};

