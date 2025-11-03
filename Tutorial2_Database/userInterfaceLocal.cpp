 #include "database.h"
 
 
 int main()
 {
	 //greetings
	 cout<<"Welcome to Tutorial 2 RPC\n";
	 //give database name
	 cout<<"Give a name for your new database\n";
	 
	 string databaseName="";
	 bool exit=false;
	 getline(cin,databaseName);
	 //while ! exit
	 database newDataBase(databaseName);
	 
	 while(!exit){
		//ask for option
		int option=0;
		cout<<"1-add data\n";
		cout<<"2-readback data\n";
		cout<<"3-exit\n";
		cin>>option;
		string line;
		getline(cin,line);
				
		switch(option)
		{
			case 1:
			{
				//ask for key
				cout<<"Give key for data\n";
				string key;
				vector<string> dataSet;
				getline(cin, key);
				//ask for data
				cout<<"Write lines to store, write \"done\" to end\n";
				while(line!="done"){
					getline(cin, line);
					if(line!="done")
						dataSet.push_back(line);
				}
				//store data
				if(newDataBase.addRecordSet(key,dataSet))
					cout<<"Some data is duplicated\n";
				
			}break;
			case 2:
			{
				//ask for key
				cout<<"Give key for data\n";
				string key;
				getline(cin, key);
				cout<<"Give index for data\n";
				int index=0;
				cin>>index;
				getline(cin,line);
				string resultDataSet=newDataBase.getRecord(key,index);
				//search key
				//show data
				cout<<"Data recovered: \""<<resultDataSet<<"\"\n";
			}break;
			case 3:
			{
				exit=true;
			}break;
		};
		
	 }
	 return 0;
 }
 
 
 
 
 