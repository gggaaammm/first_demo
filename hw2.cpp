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
#include <ctime>
	
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
void read_post(int clientfd, string read_post_info, sqlite3* Database)
{
	//ok, first we need to het the post-id
	sqlite3_stmt *stmt;
	string whole_date = "2020-";
	string text, num;
	int newline_pos = read_post_info.find("\n");
	string post_id;
	post_id = read_post_info.substr(0, newline_pos-1);
	///sql query reamain thing
	int i;
	
	cout<<"My damn post id"<<post_id<<"!"<<endl;
	string sql_read = "SELECT Content FROM ARTICLES WHERE (Post_id = '";
	sql_read.append(post_id);
	sql_read.append("')");
	sqlite3_prepare_v2(Database , sql_read.c_str(), -1,&stmt,NULL);
	
	bool OOLA = FALSE;
	while (sqlite3_step(stmt) != SQLITE_DONE)
	{
		int num_cols = sqlite3_column_count(stmt);	
		for (i = 0; i < num_cols; i++)
		{
			OOLA = TRUE;
			switch (sqlite3_column_type(stmt, i))
			{
			case (SQLITE3_TEXT):
				text = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
				//send(clientfd, text.c_str(),text.length(),0);
				break;
			case (SQLITE_INTEGER):			
				num =to_string(sqlite3_column_int(stmt, i));
				//send(clientfd, num.c_str(),num.length(),0); //todo: need to comment
				break;
			default:
				break;
			}
			//send(clientfd, "\t", sizeof("\t"),0);
		}
		//send(clientfd, "\n", sizeof("\n"),0);
	}
	if(!OOLA){send(clientfd, "Post does not exist.\n% ", sizeof("Post does not exist.\n% "),0);}
	else //formating the content of the CONTENT
	{//key point: replace!
		//my opinion: cut off-> replace ->append back
		string final_text = "--\n";
		string temp_str = text;
		cout<<"My text"<<text<<endl;
		while(text.find("<br>") != -1) //if newline appear
		{
			//cut off from first to >
			final_text.append(text.substr(0,text.find("<br>")));
			final_text.append("\n");
			text = text.substr(text.find("<br>")+4);
			cout<<"temp final"<<final_text<<endl;
			cout<<"temp origin"<<text<<endl;
		}
		final_text.append(text);
		final_text.append("--\n");

		string show_info = "SELECT Author, Title, Date FROM ARTICLES WHERE (Post_id ='";
		show_info.append(post_id);
		show_info.append("')");
		sqlite3_prepare_v2(Database , show_info.c_str(), -1,&stmt,NULL);
		while (sqlite3_step(stmt) != SQLITE_DONE)
		{
			int num_cols = sqlite3_column_count(stmt);	
			for (i = 0; i < num_cols; i++)
			{
				switch(i)
				{
					case 0: send(clientfd, "Author :",sizeof("Author :"),0); break;
					case 1: send(clientfd, "Title :",sizeof("Title :"),0); break;
					case 2: send(clientfd, "Date :",sizeof("Date :"),0); break;
					default: break;
				}
				text = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
				if(i!=2)
				{
					send(clientfd, text.c_str(),text.length(),0);
				}
				else
				{// do Date transform
					text = text.replace(text.find("/"),1,"-");
					whole_date.append(text);
					cout<<"my whole date"<<whole_date<<endl;
					send(clientfd, whole_date.c_str(),whole_date.length(),0);
				}
				send(clientfd, "\n",sizeof("\n"),0);
			}
			
		}
		send(clientfd, final_text.c_str(), final_text.length(), 0);
	}

	///still, comment are required!!
	return;
}

void delete_post(int clientfd, string delete_post_info, string author,sqlite3* Database)
{
	///ok, first we need to get the post-id 
	sqlite3_stmt *stmt;
	string text, num;
	string post_id;
	char* messaggeError; 
	int working;
	int newline_pos = delete_post_info.find("\n");
	post_id = delete_post_info.substr(0, newline_pos-1);
	string sql_delete = "SELECT Author FROM ARTICLES WHERE (Post_id = '";
	sql_delete.append(post_id);
	sql_delete.append("')");
	sqlite3_prepare_v2(Database , sql_delete.c_str(), -1,&stmt,NULL);
	int i;
	bool OOLA = FALSE;
	while (sqlite3_step(stmt) != SQLITE_DONE)
	{
		int num_cols = sqlite3_column_count(stmt);	
		for (i = 0; i < num_cols; i++)
		{
			OOLA = TRUE;
			switch (sqlite3_column_type(stmt, i))
			{
			case (SQLITE3_TEXT):
				text = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
				//send(clientfd, text.c_str(),text.length(),0);
				break;
			case (SQLITE_INTEGER):			
				num =to_string(sqlite3_column_int(stmt, i));
				//send(clientfd, num.c_str(),num.length(),0); //todo: need to comment
				break;
			default:
				break;
			}
			//send(clientfd, "\t", sizeof("\t"),0);
		}
		//send(clientfd, "\n", sizeof("\n"),0);
	}
	if(!OOLA){send(clientfd, "Post does not exist.\n% ", sizeof("Post does not exist.\n% "),0);}
	else //second part: check author name
	{
		cout<<"author is: "<<text<<endl;
		if(author == text) //if author name are the same
		{
			string sql_delete_final = "DELETE FROM ARTICLES WHERE (Post_id = ' ";
			sql_delete_final.append(post_id);
			sql_delete_final.append("')");
			cout<<"my damn delete"<<sql_delete_final<<endl;
			working  = sqlite3_exec(Database, sql_delete_final.c_str(), NULL, 0, &messaggeError); 
    		if (working != SQLITE_OK) { 
        		cerr << "Error DELETE" << endl; 
        		sqlite3_free(messaggeError); 
    			} 
    		else cout << "ARTICLES Database DELETE Successfully (๑•̀ㅂ•́)و✧" << endl;
			send(clientfd, "Delete successfully.\n% ", sizeof("Delete successfully.\n% "),0);
		}
		else
		{
			send(clientfd, "Not the post owner.\n% ", sizeof("Not the post owner.\n% "),0);
		}
	}
}

void list_post(int clientfd, string list_post_info, sqlite3* Database)
{
	//so, like list board's way?
	//Let's format :D
	//get the board name first :D
	sqlite3_stmt *stmt;
	string text, num;
	//fuck, we go AGANE
	int cut_pos, newline_pos;
	cut_pos = list_post_info.find("##");
	newline_pos = list_post_info.find("\n");
	string board_name;
	string keyword;
	if(cut_pos==-1)// just search for board name
	{

		board_name = list_post_info.substr(0, newline_pos-1);
		cout<<"list post check:"<<board_name<<"!"<<endl;
		
		string sql_post = "SELECT Post_id, Title, Author, Date FROM ARTICLES WHERE (B_name = '";
		sql_post.append(board_name);
		sql_post.append("')");
		cout<<"my damn inst are:"<<sql_post<<endl;
		sqlite3_prepare_v2(Database , sql_post.c_str(), -1,&stmt,NULL);
		int i;
		bool OOLA= FALSE;
		while (sqlite3_step(stmt) != SQLITE_DONE)
		{
			if(!OOLA) send(clientfd,"ID\tTitle\tAuthor\tDate\n",sizeof("ID\tTitle\tAuthor\tDate\n"),0);
			OOLA = TRUE;
			int num_cols = sqlite3_column_count(stmt);	
			for (i = 0; i < num_cols; i++)
			{
				switch (sqlite3_column_type(stmt, i))
				{
				case (SQLITE3_TEXT):
					//printf("%s\t ", sqlite3_column_text(stmt, i));
					text = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
					send(clientfd, text.c_str(),text.length(),0);
					break;
				case (SQLITE_INTEGER):
					//printf("%d\t ", sqlite3_column_int(stmt, i));
					num =to_string(sqlite3_column_int(stmt, i));
					send(clientfd, num.c_str(),num.length(),0);
					break;
				default:
					break;
				}
				send(clientfd, "\t", sizeof("\t"),0);
			}
			send(clientfd, "\n", sizeof("\n"),0);
		}
		if(!OOLA){send(clientfd, "Board does not exist.\n% ", sizeof("Board does not exist.\n% "),0);}
		else{send(clientfd, "% ", sizeof("% "),0);}
	}
	else //we need another search!!
	{
		board_name = list_post_info.substr(0, cut_pos-1);
		cout<<"my damn board name is:"<<board_name<<"!"<<endl;
		keyword = list_post_info.substr(cut_pos+3, newline_pos-cut_pos-4);
		cout<<cut_pos<<newline_pos<<endl;
		cout<<"my damn kw is:"<<keyword<<"!"<<endl;
		///Let's have some query
		string sql_search = "SELECT Post_id, Title, Author, Date FROM ARTICLES WHERE (B_name = '";
		sql_search.append(board_name);
		sql_search.append("' AND Title LIKE '%");
		sql_search.append(keyword);
		sql_search.append("%')");
		cout<<"my damn sql:"<<sql_search<<endl;
		sqlite3_prepare_v2(Database , sql_search.c_str(), -1,&stmt,NULL);
		int i;
		bool OOLA= FALSE;
		while (sqlite3_step(stmt) != SQLITE_DONE)
		{
			if(!OOLA) send(clientfd,"ID\tTitle\tAuthor\tDate\n",sizeof("ID\tTitle\tAuthor\tDate\n"),0);
			OOLA = TRUE;
			int num_cols = sqlite3_column_count(stmt);	
			for (i = 0; i < num_cols; i++)
			{
				switch (sqlite3_column_type(stmt, i))
				{
				case (SQLITE3_TEXT):
					//printf("%s\t ", sqlite3_column_text(stmt, i));
					text = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
					send(clientfd, text.c_str(),text.length(),0);
					break;
				case (SQLITE_INTEGER):
					//printf("%d\t ", sqlite3_column_int(stmt, i));
					num =to_string(sqlite3_column_int(stmt, i));
					send(clientfd, num.c_str(),num.length(),0);
					break;
				default:
					break;
				}
				send(clientfd, "\t", sizeof("\t"),0);
			}
			send(clientfd, "\n", sizeof("\n"),0);
		}
		if(!OOLA){send(clientfd, "Board does not exist.\n% ", sizeof("Board does not exist.\n% "),0);}
		else{send(clientfd, "% ", sizeof("% "),0);}
	}
	return; //done ^^
}

void list_board(int clientfd, string list_board_info ,sqlite3* Database)
{
	//first, we need to divide that if we need keyword searching
	cout<<"so length is "<<list_board_info.length()<<endl;
	cout<<"inside: "<<list_board_info<<endl;
	sqlite3_stmt *stmt;
	string  num, fnum;
	string text;
	if(list_board_info.length()==1) //no keyword searching
	{
		send(clientfd,"Index\tName\tModerator\n",sizeof("Index\tName\tModerator\n"),0);
		string sql_list_all = "SELECT Indexu, B_name, Mod FROM BOARDS";
		sqlite3_prepare_v2(Database , sql_list_all.c_str(), -1,&stmt,NULL);
		while (sqlite3_step(stmt) != SQLITE_DONE) 
		{
			int i;
			int num_cols = sqlite3_column_count(stmt);	
			for (i = 0; i < num_cols; i++)
			{
				switch (sqlite3_column_type(stmt, i))
				{
				case (SQLITE3_TEXT):
					//printf("%s\t ", sqlite3_column_text(stmt, i));
					text = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
					send(clientfd, text.c_str(),text.length(),0);
					break;
				case (SQLITE_INTEGER):
					//printf("%d\t ", sqlite3_column_int(stmt, i));
					num =to_string(sqlite3_column_int(stmt, i));
					send(clientfd, num.c_str(),num.length(),0);
					break;
				default:
					break;
				}
				send(clientfd, "\t", sizeof("\t"),0);
			}
			send(clientfd, "\n", sizeof("\n"),0);
		
		}
	}
	else if(list_board_info.find("##")!=-1)//need keyword searching
	{
		int cut_pos = list_board_info.find("##");
		int newline_pos = list_board_info.find("\n");
		list_board_info = list_board_info.substr(2, newline_pos-3);
		string sql_list_key="SELECT Indexu, B_name, Mod FROM BOARDS WHERE (B_name LIKE '%";
		sql_list_key.append(list_board_info);
		sql_list_key.append("%')");
		
		send(clientfd,"Index\tName\tModerator\n",sizeof("Index\tName\tModerator\n"),0);
		sqlite3_prepare_v2(Database , sql_list_key.c_str(), -1,&stmt,NULL);
		while (sqlite3_step(stmt) != SQLITE_DONE) 
		{
			int i;
			int num_cols = sqlite3_column_count(stmt);	
			for (i = 0; i < num_cols; i++)
			{
				switch (sqlite3_column_type(stmt, i))
				{
				case (SQLITE3_TEXT):
					//printf("%s\t ", sqlite3_column_text(stmt, i));
					text = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
					send(clientfd, text.c_str(),text.length(),0);
					break;
				case (SQLITE_INTEGER):
					//printf("%d\t ", sqlite3_column_int(stmt, i));
					num =to_string(sqlite3_column_int(stmt, i));
					send(clientfd, num.c_str(),num.length(),0);
					break;
				default:
					break;
				}
				send(clientfd, "\t", sizeof("\t"),0);
			}
			send(clientfd, "\n", sizeof("\n"),0);	
		}
	}

	send(clientfd, "\n% ", sizeof("\n% "),0);
	return;
}

void create_board(int clientfd, string create_board_info, string MOD ,sqlite3* Database)
{
	string board_name;
	string temp_str;
	int cut_pos;
	int newline_pos;
	string sql_cmd;
	int adding;
	char* messaggeError; 
	bool board_add = FALSE;
	int index = 0;

	cut_pos = create_board_info.find(" ");
	newline_pos = create_board_info.find("\n"); 
	///the format: create-board <name>, so be sure that no space left behind
	//lets ignore if format is incorrect :P
	if(cut_pos == -1 && newline_pos > 0) // if something ok
	{
		board_name = create_board_info.substr(0,newline_pos-1);
		cout<<"boardname: "<<board_name<<endl;
		
		string S_search = "SELECT * FROM BOARDS WHERE(B_name = '";
		S_search.append(board_name);
		S_search.append("')"); 
		int test;
		test = sqlite3_exec(Database, S_search.c_str(), callback, 0, &messaggeError);
	    char **result;
		int rows,cols;
		sqlite3_get_table(Database , S_search.c_str(), &result , &rows, &cols, &messaggeError);
		if(rows == 0 ){
			board_add = true;
			send(clientfd, "create-board successfully.\n% ", sizeof("create-board successfully.\n% "),0);
		}
		else 
		{
			send(clientfd, "Board name is already used.\n% ", sizeof("Board name is already used\n% "),0);
		}
    	sqlite3_free_table(result);
	}
	else send(clientfd, "% ", sizeof("% "),0);
	string sql_board_create;
	if(board_add)
	{
		string S_CB = "INSERT INTO BOARDS (Indexu,B_name,Mod) " "VALUES ('";
		S_CB.append(to_string(index));
		S_CB.append("', '");
		S_CB.append(board_name);
		S_CB.append("', '");
		S_CB.append(MOD);
		S_CB.append("');");
		sql_board_create = S_CB;
        adding = sqlite3_exec(Database, S_CB.c_str(), callback, 0, &messaggeError);
        cout<<"Board Database update!"<<endl;
	}

	return;
}

void create_post(int clientfd, string create_post_info, string author ,sqlite3* Database)
{
	//prepare the time!
	time_t now = time(0);
	tm *ltm = localtime(&now);
	cout<<"Month"<<1+ltm->tm_mon<<endl;
	cout<<"Day"<<ltm->tm_mday<<endl;
	int Month = 1+ltm->tm_mon;
	int Day = ltm->tm_mday;
	string Date = to_string(Month);
	Date.append("/");
	Date.append(to_string(Day));
	int counting;
	sqlite3_stmt *stmt;
	int rows,cols;
	int post_id;
	bool post_add= false;
	string temp_str;
	string title;
	string content;
	//from here, we will use Database "Article"
	string S_count = "SELECT MAX(Post_id) FROM ARTICLES";
	sqlite3_prepare_v2(Database , S_count.c_str(), -1,&stmt,NULL);
	/*while (sqlite3_step(stmt) != SQLITE_DONE) 
	{
		post_id = sqlite3_column_int(stmt,0);
		int i;
		int num_cols = sqlite3_column_count(stmt);	
		for (i = 0; i < num_cols; i++)
		{
			switch (sqlite3_column_type(stmt, i))
			{
			case (SQLITE3_TEXT):
				printf("%s, ", sqlite3_column_text(stmt, i));
				break;
			case (SQLITE_INTEGER):
				printf("%d, ", sqlite3_column_int(stmt, i));
				break;
			case (SQLITE_FLOAT):
				printf("%g, ", sqlite3_column_double(stmt, i));
				break;
			default:
				break;
			}
		}
		printf("\n");
	}*/
	while (sqlite3_step(stmt) != SQLITE_DONE) 
	{
		post_id = sqlite3_column_int(stmt,0);
		cout<<"last post_id? "<<post_id<<endl;
		post_id = post_id+1;
	}
	//Lets do the formating!
	//first, we load the board name from create_post_info
	int cut_pos;
	string board_name;
	cut_pos = create_post_info.find(" ");
	if(cut_pos > 0)
	{
		board_name = create_post_info.substr(0,cut_pos);
		cout<<"i want to post on: "<<board_name<<endl;
	}
	//now, we get the board name, lets check if this board exist in table"BOARD"
	string S_search = "SELECT COUNT(*) FROM BOARDS WHERE( B_name = '";
	S_search.append(board_name);
	S_search.append("')"); 
	int approved;
	sqlite3_prepare_v2(Database , S_search.c_str(), -1,&stmt,NULL);
	while (sqlite3_step(stmt) != SQLITE_DONE) 
	{
		approved = sqlite3_column_int(stmt,0);
		cout<<"approved? "<<approved<<endl;
	}
	if(approved>0) //have such board to post
	{
		post_add = true;
	}
	else
	{
		send(clientfd, "Board does not exist.\n% ", sizeof("Board does not exist.\n% "),0);
	}
	//part 2: we need to check keyword"--title"
	temp_str = create_post_info.substr(cut_pos+1);
	cut_pos = temp_str.find("--title ");
	if(cut_pos == -1)
	{
		send(clientfd, "% ", sizeof("% "),0);
		return;
	}
	else
	{
		temp_str = temp_str.substr(8);
		cout<<"lets check the remain string is "<<temp_str<<endl;
	}
	//then, we need to record the title :D
	cut_pos = temp_str.find(" --content ");
	if(cut_pos == -1)
	{
		send(clientfd, "% ", sizeof("% "),0);
		return;
	}
	else
	{
		//cout<<"so where to cut? "<<cut_pos<<endl;
		title = temp_str.substr(0,cut_pos);
		//cout<<"test title? "<<title<<"!"<<endl;
	}
	///part3 now we need to record the content!
	temp_str = temp_str.substr(cut_pos+1);
	temp_str = temp_str.substr(10);
	cout<<"assume this is content: "<<temp_str<<endl;
	content = temp_str;
	////now, we need to insert to table "article" :D

	cout<<"show me the date: "<<Date<<endl;
	string sql_add;
	char* messaggeError;
	if(post_add)
	{
		sql_add = "INSERT INTO ARTICLES (Post_id,B_name,Title, Author, Date, Content) " "VALUES ('";
		sql_add.append(to_string(post_id));
		sql_add.append("', '");
		sql_add.append(board_name);
		sql_add.append("', '");
		sql_add.append(title);
		sql_add.append("', '");
		sql_add.append(author);
		sql_add.append("', '");
		sql_add.append(Date);
		sql_add.append("', '");
		sql_add.append(content);
		sql_add.append("');");
		int adding = sqlite3_exec(Database, sql_add.c_str(), callback, 0, &messaggeError);
		send(clientfd, "Create post successfully.\n% ", sizeof("Create post successfully.\n% "),0);
    	cout<<"Table ARTICLE update!"<<endl;
	}


	return;
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
	string create_board_info;
	string create_post_info;
	string list_board_info;
	string list_post_info;
	string delete_post_info;
	string read_post_info;
	string account_name[30];
	bool login_accepted;
	int board_index=1;

		
	char buffer[1025]; //data buffer of 1K 
		
	//set of socket descriptors 
	fd_set readfds; 
		
	//a message 
	string message = "********************************\n** Welcome to the BBS server. **\n********************************\n% "; 
	string PLZ_LOGIN = "Please login first\n% ";
	//create a sqlite database
	sqlite3* Database;
	int working = 0, working_2 = 0, working_3 =0;
	sqlite3_stmt *statement;
	const char* data = "Callback function called";
	char* messaggeError; 
    
    working = sqlite3_open("BBSserver.db", &Database); 
    string sql_BBSUSER = "CREATE TABLE BBSUSER("  \
         "Username TEXT PRIMARY KEY     NOT NULL," \
         "Email           TEXT    NOT NULL," \
         "Password        TEXT  NOT NULL );";
  	
  	working  = sqlite3_exec(Database, sql_BBSUSER.c_str(), NULL, 0, &messaggeError); 
    if (working != SQLITE_OK) { 
        cerr << "Error Creating BBSUSER Table" << endl; 
        sqlite3_free(messaggeError); 
    } 
    else
        cout << "BBSUSER Database created Successfully (๑•̀ㅂ•́)و✧" << endl;
	
    working_2 = sqlite3_open("BBSserver.db", &Database);
    string sql_BOARDS = "CREATE TABLE BOARDS("  \
         "Indexu TEXT     NOT NULL," \
         "B_name           TEXT    NOT NULL," \
         "Mod        TEXT  NOT NULL );";
    working_2 = sqlite3_exec(Database, sql_BOARDS.c_str(), NULL, 0, &messaggeError);
	if (working_2 != SQLITE_OK) { 
        cerr << "Error Creating BOARDS Table" << endl; 
        sqlite3_free(messaggeError); 
    } 
    else
        cout << "BOARDS Database created Successfully (๑•̀ㅂ•́)و✧" << endl; 
	
	working_3 = sqlite3_open("BBSserver.db", &Database);
    string sql_Articles = "CREATE TABLE ARTICLES("  \
         "Post_id INT NOT NULL," \
         "B_name  TEXT NOT NULL," \
         "Title   TEXT  NOT NULL,"\
         "Author  TEXT  NOT NULL,"\
         "Date    TEXT  NOT NULL,"\
         "Content TEXT  NOT NULL);";
    working_3 = sqlite3_exec(Database, sql_Articles.c_str(), NULL, 0, &messaggeError);
	if (working_3 != SQLITE_OK) { 
        cerr << "Error Creating ARTICLES Table" << endl; 
        sqlite3_free(messaggeError); 
    } 
    else
        cout << "ARTICLES Database created Successfully (๑•̀ㅂ•́)و✧" << endl; 

    		
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
					if(strncmp(cmd, "create-board", 12)==0)
					{
						if(login_state[i])
						{
							create_board_info = string(cmd).substr(13);
							create_board(clientfd, create_board_info, account_name[i],Database);
							board_index = board_index+1;

						}
						else
						{
							send(clientfd, PLZ_LOGIN.c_str(), PLZ_LOGIN.length(), 0);
						}
					}
					if(strncmp(cmd, "create-post",11)==0)
					{
						if(login_state[i])
						{
							create_post_info = string(cmd).substr(12);
							create_post(clientfd , create_post_info, account_name[i],Database);
						}
						else
						{
							send(clientfd, PLZ_LOGIN.c_str(), PLZ_LOGIN.length(), 0);	
						}
					}
					if(strncmp(cmd, "list-board",10)==0)
					{
						list_board_info = string(cmd).substr(11);
						list_board(clientfd, list_board_info ,Database);
					}
					if(strncmp(cmd, "list-post", 9)==0)
					{
						list_post_info = string(cmd).substr(10);
						list_post(clientfd, list_post_info, Database);
					}
					if(strncmp(cmd, "delete-post", 11)==0)
					{
						if(login_state[i])
						{
							delete_post_info = string(cmd).substr(12);
							delete_post(clientfd , delete_post_info, account_name[i],Database);
						}
						else
						{
							send(clientfd, PLZ_LOGIN.c_str(), PLZ_LOGIN.length(), 0);	
						}
					}
					if(strncmp(cmd, "read", 4)==0)
					{
						read_post_info = string(cmd).substr(5);
						read_post(clientfd, read_post_info, Database);
					}

				} 
			} 
		} 
	} 
		
	return 0; 
} 
