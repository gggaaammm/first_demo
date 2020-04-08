#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <sqlite3.h> 

using namespace std;

#define MY_PORT 9453



static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}


void Register(int clientfd, string register_info, sqlite3* Database )// todo: link to sql DB save
{
	string reg_uname;
	string reg_email;
	string reg_pwd;
	string temp_str;
	string sql_cmd;
	int cut_pos;
	int newline_pos;
	int adding;
	char* messaggeError; 
	bool user_request = false;
	bool user_add = false;

	//todo cut 3 pieces: username, e-mail, password
	cout<<"=================================raaaaaaaw data is:========================"<<register_info<<"haha";
	cout<<"check size of: "<<sizeof(register_info)<<endl;
	cut_pos = register_info.find(" ");
	newline_pos  =register_info.find("\n"); 
	//cout<<"1st time cut on ================="<<cut_pos<<endl;
	//cout<<"new line position is: "<<newline_pos<<endl;
	if(newline_pos!=1 && newline_pos!=0 && cut_pos!= -1 && cut_pos!= 0) // if something left behind and exist a space to cut and the position is not first
	{
		reg_uname = register_info.substr(0,cut_pos);
		cout<<"username: "<<reg_uname<<endl;
		temp_str = register_info.substr(cut_pos+1);
		cout<<"temp1: "<<temp_str;
	}
	else{ //something like % register
		if(newline_pos <= 1)
		{
			send(clientfd, "no username.\n", sizeof("no username\n"),0);
		}
		else if(newline_pos > 1)
		{
			send(clientfd, "no email.\n", sizeof("no email\n"),0);
		}
		return;
	}
	cut_pos = temp_str.find(" ");
	newline_pos = temp_str.find("\n");
	//cout<<"2nd time cut on ========================="<<cut_pos<<endl;
	//cout<<"new line position is: "<<newline_pos<<endl;
	if(newline_pos!=1 && newline_pos!=0 && cut_pos!= -1 && cut_pos!= 0)
	{
		reg_email = temp_str.substr(0,cut_pos);
		cout<<"email: "<<reg_email<<endl;
		temp_str = temp_str.substr(cut_pos+1);
		cout<<"temp2: "<<temp_str;	
	}
	else{
		if(newline_pos <= 1)
		{
			send(clientfd, "no email.\n", sizeof("no email\n"),0);
		}
		else if(newline_pos > 1)
		{
			send(clientfd, "no password.\n", sizeof("no password\n"),0);
		}
		return;
	}
	cut_pos = temp_str.find(" ");
	newline_pos = temp_str.find("\n");
	//cout<<"3rd time cut on ==============="<<cut_pos<<endl;
	cout<<"new line position is: "<<newline_pos<<endl;
	if(newline_pos!=1 && newline_pos!=0 && cut_pos == -1) //if something left behind and no space exist
	{
		reg_pwd = temp_str.substr(0,newline_pos);////////////////////////
		cout<<"password: "<<reg_pwd<<endl; //it shoud be pswd.	
		cout<<"last cut on "<<cut_pos<<endl;
		send(clientfd, "Register successfully.\n", 23,0);
		user_request = true;				
	}
	else{
		cout<<"last cut on "<<cut_pos<<endl;
		temp_str = temp_str.substr(cut_pos+1);
		cout<<"temp3: "<<temp_str;//should be something left
		if(temp_str.empty() || (cut_pos== -1 || cut_pos== 0))
		{
			send(clientfd, "no password.\n", sizeof("no password\n"),0);
		}
		else if(cut_pos > 0)
		{
			send(clientfd, "too much.\n", sizeof("too much\n"),0);
		}
		
		return;
	}
	////todo do the database
	//have such account, notify the user that the username has existed
	if(user_request)
	{
		string S2 = "SELECT * FROM COMPANY WHERE(Username = '";
		S2.append(reg_uname);
		S2.append("')"); 
	    char **result;
		int rows,cols;
		sqlite3_get_table(Database , S2.c_str(), &result , &rows, &cols, &messaggeError);
		cout<<"rows"<<rows<<"cols"<<cols<<endl;
		if(rows == 0 ){user_add = true;}
		else 
		{
			send(clientfd, "Username is already used.\n", sizeof("Username is already used\n"),0);
		}
    	sqlite3_free_table(result);
	}
	
	//string S2 = "SELECT * FROM departments WHERE EXISTS (SELECT * FROM employees WHERE departments.department_id = employees.department_id);"
	//adding = sqlite3_exec(Database, S2.c_str(), callback, 0, &messaggeError);

	if(user_add) //no such account, register
	{
		//sql_cmd = "INSERT INTO COMPANY (Username,Email,Password) " "VALUES ('reg_uname', 'reg_email', 'reg_pwd'); " ;
		string S1 = "INSERT INTO COMPANY (Username,Email,Password) " "VALUES ('";
		S1.append(reg_uname);
		S1.append("', '");
		S1.append(reg_email);
		S1.append("', '");
		S1.append(reg_pwd);
		S1.append("');");
		sql_cmd = S1;
		cout<<"my register cmd"<<sql_cmd<<endl;
        adding = sqlite3_exec(Database, sql_cmd.c_str(), callback, 0, &messaggeError);
        cout<<"Database update!"<<endl;
	}
}

std::string get_name(string login_info)
{
	int newline_pos;
	int cut_pos;
	string login_uname="";
	cout<<"raaaaaaaw data is:========================"<<login_info<<"haha";
	cout<<"check size of: "<<sizeof(login_info)<<endl;
	cut_pos = login_info.find(" ");
	newline_pos = login_info.find("\n");
	if(newline_pos!=1 && newline_pos!=0 && cut_pos!= -1 && cut_pos!= 0) // if something left behind and exist a space to cut and the position is not first
	{
		login_uname = login_info.substr(0,cut_pos);
		cout<<"username: "<<login_uname<<endl;
	}
	
	return login_uname;	
}


bool log_in_process(int clientfd, string login_info, sqlite3* Database )
{
	int cut_pos;
	int newline_pos;
	bool login_request;
	bool login_suc = false;
	string temp_str;
	string login_uname;
	string login_email;
	string login_password;
	string sql_login;
	int cols, rows;
	char **result;
	char* messaggeError; 
	cout<<"raaaaaaaw data is:========================"<<login_info;
	cut_pos = login_info.find(" ");
	newline_pos = login_info.find("\n");
	if(newline_pos!=1 && newline_pos!=0 && cut_pos!= -1 && cut_pos!= 0) // if something left behind and exist a space to cut and the position is not first
	{
		login_uname = login_info.substr(0,cut_pos);
		cout<<"username: "<<login_uname<<endl;
		temp_str = login_info.substr(cut_pos+1);
		cout<<"temp1: "<<temp_str;
	}
	else{ //something like % login
		if(newline_pos <= 1)
		{
			send(clientfd, "no username.\n", sizeof("no username\n"),0);
		}
		else if(newline_pos > 1)
		{
			send(clientfd, "no password.\n", sizeof("no password\n"),0);
		}
		return false;
	}
	cut_pos = temp_str.find(" ");
	newline_pos = temp_str.find("\n");
	//cout<<"2nd time cut on ========================="<<cut_pos<<endl;
	cout<<"new line position is: "<<newline_pos<<endl;
	if(newline_pos!=1 && newline_pos!=0 && cut_pos == -1)//if something left behind and no space exist
	{
		login_password = temp_str.substr(0,newline_pos);
		cout<<"password: "<<login_password; //it shoud be pswd.	
		cout<<"last cut on "<<cut_pos<<endl;
		send(clientfd, "Login format accept.\n", sizeof("Login format accept.\n"),0);
		login_request = true;
	}
	else{
		cout<<"last cut on "<<cut_pos<<endl;
		temp_str = temp_str.substr(cut_pos+1);
		cout<<"temp3: "<<temp_str;//should be something left
		if(temp_str.empty() || (cut_pos== -1 || cut_pos== 0))
		{
			send(clientfd, "no password.\n", sizeof("no password\n"),0);
		}
		else if(cut_pos > 0)
		{
			send(clientfd, "too much.\n", sizeof("too much\n"),0);
		}
		return false;
	}
	if(login_request)
	{
		sql_login = "SELECT * FROM COMPANY WHERE(Username = '";
		sql_login.append(login_uname);
		sql_login.append("' AND Password = '");
		sql_login.append(login_password);
		sql_login.append("')");

		cout<<"my sql_login is: "<<sql_login<<endl;
		sqlite3_get_table(Database , sql_login.c_str(), &result , &rows, &cols, &messaggeError);
		cout<<"cols?:"<<cols<<endl;
		sqlite3_free_table(result);
		if(cols!=0)
		{
			
			send(clientfd, "login successfully\n", sizeof("login successfully\n"), 0);
			string welcome_note = "Welcome, ";
			welcome_note.append(login_uname);
			welcome_note.append("  σ`∀´)σ\n");
			/*static_cast<void*>(&welcome_note)*/
			cout<<"my length is "<<welcome_note.length()<<endl;
			send(clientfd, welcome_note.c_str() , welcome_note.length(), 0);
			login_suc = true;
		}
		else {
			send(clientfd, "Login failed\n", sizeof("Login failed\n"), 0);
		}
	}
	else 
	{
		return false;
	}
	return login_suc;
}
void log_out()
{

}
int main(int argc, char *argv[])
{
	sqlite3* Database;
	int working = 0; 
	sqlite3_stmt *statement;
	const char* data = "Callback function called";
    working = sqlite3_open("demo.db", &Database); 
    string sql = "CREATE TABLE COMPANY("  \
         "Username TEXT PRIMARY KEY     NOT NULL," \
         "Email           TEXT    NOT NULL," \
         "Password        TEXT  NOT NULL );";
  	char* messaggeError; 
  	working = sqlite3_exec(Database, sql.c_str(), NULL, 0, &messaggeError); 
    if (working != SQLITE_OK) { 
        cerr << "Error Create Table: it might already existed" << std::endl; 
        sqlite3_free(messaggeError); 
    } 
    else
        cout << "Table created Successfully (๑•̀ㅂ•́)و✧" << std::endl; 
    //string S2 = "SELECT * FROM COMPANY WHERE(Username = 'bmw')";
	//working = sqlite3_exec(Database, S2.c_str(), callback, (void*)data, &messaggeError); 
    //char **result;
	//int rows,cols;
	//sqlite3_get_table(Database , S2.c_str(), &result , &rows, &cols, &messaggeError);
	//cout<<"rows"<<rows<<"cols"<<cols<<endl;
	/*for (int i=1;i<=rows;i++) {
       for (int j=0;j<cols;j++) {
           cout<<result[i*cols+j]<<"\t";
       }
       cout<<endl;
   }
    sqlite3_free_table(result);*/
	int sockfd;
	struct sockaddr_in self;
	bool login_state = false;
	bool login_accepted = false;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("cant open socket\n");
		exit(errno);
	}

	bzero(&self, sizeof(self));
	self.sin_family = AF_INET;
	self.sin_port = htons(MY_PORT);
	self.sin_addr.s_addr = INADDR_ANY;

	if( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )
	{
		perror("cant bind");
		exit(errno);
	}

	if( listen(sockfd, 20) != 0 )
	{
		perror("cant listen");
		exit(errno);
	}

	while(1)
	{
		int clientfd;
		struct sockaddr_in client_addr;
		int addrlen = sizeof(client_addr);
		char cmd[256]={};
		string register_info;
		string login_info;
		string message;
		string account_name;
		string account_addr;
		string account_pswd;
		

		clientfd = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
		printf("Connected client: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		
		send(clientfd, "********************************\n", 34,0);
		send(clientfd, "Welcome to the BBS serverヽ( ^ω^ ゞ )\n", 42 , 0);
		send(clientfd, "********************************\n",34 ,0);
		
		while(recv(clientfd, cmd, sizeof(cmd), 0))
		{
			if(strncmp(cmd,"% exit",6)==0)
			{
				send(clientfd, "Connection close (´;ω;`)\n", 26 , 0);
				close(clientfd);
				break;
			}
			if(strncmp(cmd, "% register", 10)==0)
			{
				register_info = string(cmd).substr(11);
				cout<<"new user add!"<<endl;
				Register(clientfd, register_info, Database);
			}
			if(strncmp(cmd, "% login", 7)==0)
			{
				if(login_state)
				{
					send(clientfd, "Please logout first\n", sizeof("Please logout first\n") , 0);
				}
				else
				{
					login_accepted = false;
					login_info = string(cmd).substr(8);
					cout<<"Login state"<<endl;
					login_accepted= log_in_process(clientfd ,login_info, Database);
					if(login_accepted)
					{
						login_state = true;
						account_name = get_name(login_info);
					}
				}
			}
			if(strncmp(cmd, "% logout", 8)==0)
			{
				if(login_state)
					{
						string bye_message = "Bye, ";
						bye_message.append(account_name);
						bye_message.append(" ┐(´д`)┌\n");
						send(clientfd, bye_message.c_str(), bye_message.length() , 0);
						// todo: get the user name
						//cout<<"your name is: "<<account_name<<endl;
						login_state = false;
					}
				else 
					{send(clientfd, "Please login first.\n", sizeof("Please login first.\n") , 0);}

			}
			if(strncmp(cmd, "% whoami", 8)==0)
			{
				if(login_state)
				{
					string whoami_message = "(´_ゝ`) C'mon bro, You are: ";
					whoami_message.append(account_name);
					
					whoami_message.append("\n");
					send(clientfd, whoami_message.c_str(), whoami_message.length(), 0);
					//todo get the result from sqlite
				}
				else
					{send(clientfd, "Please login first.\n", sizeof("Please login first.\n") , 0);}
			}
			memset(cmd, 0 , sizeof cmd);
		}
		
	}

	close(sockfd);
	sqlite3_close(Database);
	return 0;
}
