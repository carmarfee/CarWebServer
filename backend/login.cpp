#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include "fcgi_stdio.h"
#include "../inc/sqlconn.h"
#include "../inc/util.h"

using namespace std;

int main(int argc, char *argv[])
{
    SqlConnPool::GetInstance()->init("localhost",
                                     3306,
                                     "root",
                                     "password",
                                     "mydb",
                                     8);

    while (FCGI_Accept() >= 0)
    {
        string content = "<title>Hello World!</title><h1>FastCGI Hello!</h1>";

        char *querystring = getenv("QUERY_STRING");
        if (!querystring)
        {
            cout << "QUERY_STRING is null" << endl;
            return 1;
        }
        string Querystring(querystring);
        MYSQL *sql;
        SqlConn(&sql, SqlConnPool::GetInstance());

        unordered_map<string, string> param_kv;
        Utils::ParseUrlencoded(Querystring, param_kv);
        const char *username = param_kv["username"].c_str();
        const char *password = param_kv["password"].c_str();

        char query[256];

        snprintf(query, sizeof(query), "SELECT username, password FROM user WHERE username='%s' AND password='%s'", username, password);

        if (mysql_query(sql, query))
        {
            cout << "mysql_query error" << endl;
            return 1;
        }

        MYSQL_RES *result = mysql_store_result(sql);
        if (result == nullptr)
        {
            cout << "mysql_store_result error" << endl;
            return 1;
        }

        int num_rows = mysql_num_rows(result);
        if (num_rows == 0)
        {
            cout << "login failed" << endl;
            content.append("<h2>login failed</h2>");
            content.append("<h3>username: ");
            content.append(username);
            content.append("</h3>");
            content.append("<h3>password: ");
            content.append(password);
            content.append("</h3>");
        }
        else
        {
            cout << "login success" << endl;
            content.append("<h2>login success</h2>");
            content.append("<h3>username: ");
            content.append(username);
            content.append("</h3>");
            content.append("<h3>password: ");
            content.append(password);
            content.append("</h3>");
        }

        mysql_free_result(result);

        SqlConnPool::GetInstance()->DisConn(sql);
        printf("Content-Length: %d\r\n\r\n", content.size());
        printf("%s", content.c_str());
    }
    return 0;
}