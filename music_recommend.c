#include "music_function.h"

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

    // user_sign(conn, "liujeilun", "chaoqiqwq", "17899992341", "2403247788@qq.com");
    // query_all_users(conn);
    // query_top3_music(conn);
    // recommend_by_user_and_music(conn, 3, 5);
    // friends_add(conn, 1, 7, 'f');

    // 结束程序前关闭数据库连接，释放资源
    mysql_close(conn);

    return 0; // 程序正常结束
}