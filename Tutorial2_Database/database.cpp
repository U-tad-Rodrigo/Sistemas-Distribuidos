#include "database.h"


database::database(string name){
	this->name=name;	
}

bool database::addRecord(string key,string data){
	
	//get the vector of data by key
	auto dataVec=this->dataStorage[key];
		//search if data is duplicated
	bool duplicated=false;
	for(auto d : dataVec) 
	{
		duplicated|= (d==data);
	}
	
	if(duplicated) 
		return false;
	else
	{
		this->dataStorage[key].push_back(data);
		return true;
	}
}

bool database::addRecordSet(string key,vector<string> dataSet){
	
	bool duplicated=false;
	for(auto d: dataSet)
		duplicated!=addRecord(key,d);
	return duplicated;
	
}
string database::getRecord(string key,int position)
{
	string res="";
	
	if(this->dataStorage[key].size() > position)
		res=this->dataStorage[key][position];
	
	return res;
}

database::~database(){
	
	cout<<"Destroying "<<name<<"\n";
	
}



