#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h> // MySQL C API头文件

// 查询user表中所有用户信息并打印
void query_all_users(MYSQL *conn)
{
    // 定义SQL查询语句，查询user表的主要字段
    const char *query = "SELECT `user_id`,`username`,`password`,`mobile`,`email` FROM user";
    // 执行SQL查询
    if (mysql_query(conn, query))
    {
        // 查询失败时输出错误信息
        fprintf(stderr, "查询失败: %s\n", mysql_error(conn));
        return;
    }

    // 获取查询结果集
    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL)
    {
        // 获取结果集失败时输出错误信息
        fprintf(stderr, "获取结果集失败: %s\n", mysql_error(conn));
        return;
    }

    // 打印表头
    printf("%-10s %-20s %-20s %-15s %-30s\n", "user_id", "username", "password", "mobile", "email");
    printf("------------------------------------------------------------------------------------------\n");

    // 定义行记录，代表某行数据
    MYSQL_ROW row;

    // 遍历结果集中的每一行
    while (row = mysql_fetch_row(result)) // 每次循环，row 都指向结果集中的下一行
    {
        printf("%-10s %-20s %-20s %-15s %-30s\n",
               row[0] ? row[0] : "NULL",
               row[1] ? row[1] : "NULL",
               row[2] ? row[2] : "NULL",
               row[3] ? row[3] : "NULL",
               row[4] ? row[4] : "NULL");
    }

    // 释放结果集资源
    mysql_free_result(result);
}

int main()
{
    // 声明一个数据库连接指针
    MYSQL *conn;

    // 初始化MySQL连接对象
    conn = mysql_init(NULL);
    if (conn == NULL) // 检查初始化是否成功
    {
        // 如果初始化失败，打印错误信息
        fprintf(stderr, "mysql_init()失败: %s\n", mysql_error(conn));
        return 1; // 返回错误码
    }

    // 设置数据库连接参数
    char *host = "192.168.20.130"; // MySQL服务器IP地址
    char *user = "tom";            // 数据库用户名
    char *password = "123456";     // 数据库密码
    char *db = "music";            // 要连接的数据库名
    int port = 3306;               // MySQL服务端口号(默认3306)

    // 连接数据库
    // 参数：连接对象、主机地址、用户名、密码、数据库名、端口号、Unix套接字、客户端标志
    if (!mysql_real_connect(conn, host, user, password, db, port, NULL, 0))
    {
        // 如果连接失败，打印错误信息
        fprintf(stderr, "mysql_real_connect()失败:%s\n", mysql_error(conn));
        mysql_close(conn); // 关闭连接对象
        return 1;          // 返回错误码
    }

    // 连接成功提示
    printf("连接数据库成功\n");

    query_all_users(conn);

    // 结束程序前关闭数据库连接，释放资源
    mysql_close(conn);

    return 0; // 程序正常结束
}