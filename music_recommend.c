#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>

int main()
{
    // 声明一个数据库连接
    MYSQL *conn;
    conn = mysql_init(NULL);
    if (conn == NULL)
    {
        fprintf(stderr, "mysql_init()失败: %s\n", mysql_error(conn));
        return 1;
    }

    // 设置连接参数
    char *host = "192.168.20.130";
    char *user = "tom";
    char *password = "123456";
    char *db = "music";
    int port = 3306;
    // 连接数据库,主机地址、用户名、密码、数据库名、端口号
    if (!mysql_real_connect(conn, host, user, password, db, port, NULL, 0))
    {
        fprintf(stderr, "mysql_real_connect()失败:%s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }
    printf("连接数据库成功\n");

    // 结束，释放资源
    mysql_close(conn);

    return 0;
}