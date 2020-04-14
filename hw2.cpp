//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux 
#include <stdio.h> 
#include <string.h> //strlen 
#include <iostream>
#include <string>
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>  
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <sqlite3.h>  // Database library

	
#define TRUE 1 
#define FALSE 0 
#define PORT 54088 
	
using namespace std;

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
	//cout<<"=================================raaaaaaaw data is:========================"<<register_info<<"haha";
	//cout<<"check size of: "<<sizeof(register_info)<<endl;
	cut_pos = register_info.find(" ");
	newline_pos  =register_info.find("\n"); 
	//cout<<"1st time cut on ================="<<cut_pos<<endl;
	//cout<<"new line position is: "<<newline_pos<<endl;
	if(newline_pos!=1 && newline_pos!=0 && cut_pos!= -1 && cut_pos!= 0) // if something left behind and exist a space to cut and the position is not first
	{
		reg_uname = register_info.substr(0,cut_pos);
		//cout<<"username: "<<reg_uname<<endl;
		temp_str = register_info.substr(cut_pos+1);
		//cout<<"temp1: "<<temp_str;
	}
	else{ //something like % register
		send(clientfd, "Usage: register <username> <email> <password>\n% ", sizeof ("Usage: register <username> <email> <password>\n% "), 0);
		if(newline_pos <= 1)
		{
			//send(clientfd, "no username.\n", sizeof("no username\n"),0);
		}
		else if(newline_pos > 1)
		{
			//send(clientfd, "no email.\n", sizeof("no email\n"),0);
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
		//cout<<"email: "<<reg_email<<endl;
		temp_str = temp_str.substr(cut_pos+1);
		//cout<<"temp2: "<<temp_str;	
	}
	else{
		send(clientfd, "Usage: register <username> <email> <password>\n% ", sizeof ("Usage: register <username> <email> <password>\n% "), 0);
		if(newline_pos <= 1)
		{
			//send(clientfd, "no email.\n", sizeof("no email\n"),0);
		}
		else if(newline_pos > 1)
		{
			//send(clientfd, "no password.\n", sizeof("no password\n"),0);
		}
		return;
	}
	cut_pos = temp_str.find(" ");
	newline_pos = temp_str.find("\n");
	//cout<<"3rd time cut on ==============="<<cut_pos<<endl;
	//cout<<"new line position is: "<<newline_pos<<endl;
	if(newline_pos!=1 && newline_pos!=0 && cut_pos == -1) //if something left behind and no space exist
	{
		reg_pwd = temp_str.substr(0,newline_pos);////////////////////////
		//cout<<"password: "<<reg_pwd<<endl; //it shoud be pswd.	
		//cout<<"last cut on "<<cut_pos<<endl;
		
		user_request = true;				
	}
	else{
		//cout<<"last cut on "<<cut_pos<<endl;
		temp_str = temp_str.substr(cut_pos+1);
		//cout<<"temp3: "<<temp_str;//should be something left
		send(clientfd, "Usage: register <username> <email> <password>\n% ", sizeof ("Usage: register <username> <email> <password>\n% "), 0);
		if(temp_str.empty() || (cut_pos== -1 || cut_pos== 0))
		{
			//send(clientfd, "no password.\n", sizeof("no password\n"),0);
		}
		else if(cut_pos > 0)
		{
			//send(clientfd, "too much(#`Д´)ﾉ\n", sizeof("too much(#`Д´)ﾉ\n"),0);
		}
		
		return;
	}
	////todo do the database
	//have such account, notify the user that the username has existed
	if(user_request)
	{
		string S2 = "SELECT * FROM BBSUSER WHERE(Username = '";
		S2.append(reg_uname);
		S2.append("')"); 
	    char **result;
		int rows,cols;
		sqlite3_get_table(Database , S2.c_str(), &result , &rows, &cols, &messaggeError);
		if(rows == 0 ){
			user_add = true;
			send(clientfd, "Register successfully.\n% ", sizeof("Register successfully.\n% "),0);
		}
		else 
		{
			send(clientfd, "Username is already used.\n% ", sizeof("Username is already used\n% "),0);
		}
    	sqlite3_free_table(result);
	}
	

	if(user_add) //no such account, register
	{
		string S1 = "INSERT INTO BBSUSER (Username,Email,Password) " "VALUES ('";
		S1.append(reg_uname);
		S1.append("', '");
		S1.append(reg_email);
		S1.append("', '");
		S1.append(reg_pwd);
		S1.append("');");
		sql_cmd = S1;
        adding = sqlite3_exec(Database, sql_cmd.c_str(), callback, 0, &messaggeError);
        cout<<"Database update!"<<endl;
	}
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
	//cout<<"raaaaaaaw data is:========================"<<login_info;
	cut_pos = login_info.find(" ");
	newline_pos = login_info.find("\n");
	if(newline_pos!=1 && newline_pos!=0 && cut_pos!= -1 && cut_pos!= 0) // if something left behind and exist a space to cut and the position is not first
	{
		login_uname = login_info.substr(0,cut_pos);
		///cout<<"username: "<<login_uname<<endl;
		temp_str = login_info.substr(cut_pos+1);
		//cout<<"temp1: "<<temp_str;
	}
	else{ //something like % login
		send(clientfd, "Usage: login <username> <password>\n% ", sizeof("Usage: login <username> <password>\n% "), 0);
		if(newline_pos <= 1)
		{
			//send(clientfd, "no username.\n", sizeof("no username\n"),0);
		}
		else if(newline_pos > 1)
		{
			//send(clientfd, "no password.\n", sizeof("no password\n"),0);
		}
		return false;
	}
	cut_pos = temp_str.find(" ");
	newline_pos = temp_str.find("\n");
	//cout<<"2nd time cut on ========================="<<cut_pos<<endl;
	//cout<<"new line position is: "<<newline_pos<<endl;
	if(newline_pos!=1 && newline_pos!=0 && cut_pos == -1)//if something left behind and no space exist
	{
		login_password = temp_str.substr(0,newline_pos);
		//cout<<"password: "<<login_password; //it shoud be pswd.	
		//cout<<"last cut on "<<cut_pos<<endl;
		//send(clientfd, "Login format accept.\n", sizeof("Login format accept.\n"),0);
		login_request = true;
	}
	else{
		//cout<<"last cut on "<<cut_pos<<endl;
		temp_str = temp_str.substr(cut_pos+1);
		//cout<<"temp3: "<<temp_str;//should be something left
		send(clientfd, "Usage: login <username> <password>\n% ", sizeof("Usage: login <username> <password>\n% "), 0);
		if(temp_str.empty() || (cut_pos== -1 || cut_pos== 0))
		{
			//send(clientfd, "no password.\n", sizeof("no password\n"),0);
		}
		else if(cut_pos > 0)
		{
			//send(clientfd, "too much.\n", sizeof("too much\n"),0);
		}
		return false;
	}
	if(login_request)
	{
		sql_login = "SELECT * FROM BBSUSER WHERE(Username = '";
		sql_login.append(login_uname);
		sql_login.append("' AND Password = '");
		sql_login.append(login_password);
		sql_login.append("')");

		//cout<<"my sql_login is: "<<sql_login<<endl;
		sqlite3_get_table(Database , sql_login.c_str(), &result , &rows, &cols, &messaggeError);
		//cout<<"cols?:"<<cols<<endl;
		sqlite3_free_table(result);
		if(cols!=0)
		{
			
			//send(clientfd, "login successfully\n", sizeof("login successfully\n"), 0);
			string welcome_note = "Welcome, ";
			welcome_note.append(login_uname);
			welcome_note.append("\n% ");
			/*static_cast<void*>(&welcome_note)*/
			//cout<<"my length is "<<welcome_note.length()<<endl;
			send(clientfd, welcome_note.c_str() , welcome_note.length(), 0);
			login_suc = true;
		}
		else {
			send(clientfd, "Login failed.\n% ", sizeof("Login failed.\n% "), 0);
		}
	}
	else 
	{
		return false;
	}
	return login_suc;
}

std::string get_name(string login_info)
{
	int newline_pos;
	int cut_pos;
	string login_uname="";
	//cout<<"raaaaaaaw data is:========================"<<login_info<<"haha";
	//cout<<"check size of: "<<sizeof(login_info)<<endl;
	cut_pos = login_info.find(" ");
	newline_pos = login_info.find("\n");
	if(newline_pos!=1 && newline_pos!=0 && cut_pos!= -1 && cut_pos!= 0) // if something left behind and exist a space to cut and the position is not first
	{
		login_uname = login_info.substr(0,cut_pos);
		//cout<<"username: "<<login_uname<<endl;
	}
	
	return login_uname;	
}


int main(int argc , char *argv[]) 
{ 

	int opt = TRUE; 
	int master_socket , addrlen , new_socket , client_socket[30] , 
		max_clients = 30 , activity, i , p , valread , clientfd; 
	int max_sd; 
	bool login_state[30] = {false}; //every user initiated as "logout" state
	struct sockaddr_in address; 
	string register_info;
	string login_info;
	string account_name[30];
	bool login_accepted;

		
	char buffer[1025]; //data buffer of 1K 
		
	//set of socket descriptors 
	fd_set readfds; 
		
	//a message 
	string message = "********************************\n** Welcome to the BBS server. **\n********************************\n% "; 
	
	//create a sqlite database
	sqlite3* Database;
	int working = 0; 
	sqlite3_stmt *statement;
	const char* data = "Callback function called";
    working = sqlite3_open("BBSserver.db", &Database); 
    string sql = "CREATE TABLE BBSUSER("  \
         "Username TEXT PRIMARY KEY     NOT NULL," \
         "Email           TEXT    NOT NULL," \
         "Password        TEXT  NOT NULL );";
  	char* messaggeError; 
  	working = sqlite3_exec(Database, sql.c_str(), NULL, 0, &messaggeError); 
    if (working != SQLITE_OK) { 
        cerr << "Error Creating Table: it might already existed" << endl; 
        sqlite3_free(messaggeError); 
    } 
    else
        cout << "Database created Successfully (๑•̀ㅂ•́)و✧" << endl; 
	//initialise all client_socket[] to 0 so not checked 
	for (p = 0; p < max_clients; p++) 
	{ 
		client_socket[p] = 0; 
	} 
		
	//create a master socket 
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	
	//type of socket created 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 	
	//bind the socket to localhost port 54088
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	printf("Listening on port %d \n", PORT); 
		
	//try to specify maximum of 20 pending connections for the master socket 
	if (listen(master_socket, 20) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	

	//accept the incoming connection 
	addrlen = sizeof(address); 

	while(TRUE) 
	{ 
		//clear the socket set 
		FD_ZERO(&readfds); 
		//add master socket to set 
		FD_SET(master_socket, &readfds); 
		max_sd = master_socket; 
			
		//add child sockets to set 
		for ( i = 0 ; i < max_clients ; i++) 
		{ 
			//socket descriptor 
			clientfd = client_socket[i]; 	
			//if valid socket descriptor then add to read list 
			if(clientfd > 0) 
				FD_SET( clientfd , &readfds); 
			//highest file descriptor number, need it for the select function 
			if(clientfd > max_sd) 
				max_sd = clientfd; 
		} 
	
 
		// wait forever
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL); 
		if ((activity < 0) && (errno != EINTR)){   
            printf("select error");   
        }		
		//If something happened on the master socket , then its an incoming connection 
		if (FD_ISSET(master_socket, &readfds)) 
		{ 
			if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
			{ 
				perror("accept"); 
				exit(EXIT_FAILURE); 
			} 
			
		
			cout<<"New connection."<<endl;
			if( send(new_socket, message.c_str(), message.length(), 0) != message.length() ) 
			{ 
				perror("send"); 
			}			
			//add new socket to array of sockets 
			for (i = 0; i < max_clients; i++) 
			{ 
				//if position is empty 
				if( client_socket[i] == 0 ) 
				{ 
					client_socket[i] = new_socket;
					break; 
				} 
			} 
		} 
		//else its some IO operation on some other socket 
		for (i = 0; i < max_clients; i++) 
		{ 
			clientfd = client_socket[i]; 
				
			if (FD_ISSET( clientfd , &readfds)) 
			{ 
				//Check if it was for closing , and also read the 
				//incoming message 
				if ((valread = read( clientfd , buffer, 1024)) == 0) 
				{ 
					close(clientfd);
					client_socket[i] = 0; 

				} 
					
				//recieve the message from client and do the string management
				else
				{ 
					char cmd[1025]={};
					buffer[valread] = '\0'; 
					for(int k=0; k<1025;k++)///just assign buffer to cmd = = 
					{
						cmd[k]=buffer[k];
					}
					if(strncmp(cmd,"exit",4)==0)
					{
						login_state[i] = false;
						close(clientfd);
						client_socket[i] = 0; 

					}
					if(strncmp(cmd, "register", 8)==0)
					{
						register_info = string(cmd).substr(9);
						//cout<<"new user want to join!"<<endl;
						Register(clientfd, register_info, Database);
					}
					if(strncmp(cmd, "login", 5)==0)
					{
						if(login_state[i])
						{
							send(clientfd, "Please logout first\n% ", sizeof("Please logout first\n% ") , 0);
						}
						else
						{
						login_accepted = false;
						login_info = string(cmd).substr(6);
						login_accepted= log_in_process(clientfd ,login_info, Database);
						if(login_accepted)/// login accepted, main function need to access user'name
						{
							login_state[i] = true;
							account_name[i] = get_name(login_info);
						}
						}
					}
					if(strncmp(cmd, "logout", 6)==0)
					{
						if(login_state[i])
						{
							string bye_message = "Bye, ";
							bye_message.append(account_name[i]);
							bye_message.append("\n% ");
							send(clientfd, bye_message.c_str(), bye_message.length() , 0);
							// todo: get the user name
							//cout<<"your name is: "<<account_name<<endl;
							login_state[i] = false;
						}
						else 
						{send(clientfd, "Please login first.\n% ", sizeof("Please login first.\n% ") , 0);}
					}
					if(strncmp(cmd, "whoami", 6)==0)
					{
						if(login_state[i])
						{
							string whoami_message ;
							whoami_message.append(account_name[i]);
							whoami_message.append("\n% ");
							send(clientfd, whoami_message.c_str(), whoami_message.length(), 0);
							//todo get the result from sqlite
						}
						else
						{send(clientfd, "Please login first.\n% ", sizeof("Please login first.\n% ") , 0);}
					}
				} 
			} 
		} 
	} 
		
	return 0; 
} 
