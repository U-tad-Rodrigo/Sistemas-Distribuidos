#ifndef DATABASE_H
#define DATABASE_H

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
		explicit database(string name);
		database() :database("") {};

		bool addRecord(const string& key, const string& data);
		bool addRecordSet(const string& key, const vector<string>& dataSet);
		string getRecord(const string& key, int index);

		~database();
};

#endif // DATABASE_H

