#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h> // MySQL C API头文件

// 用户注册功能
void user_sign(MYSQL *conn, char *n, char *pwd, char *ph, char *em)
{
    // 关闭自动提交，开启事务
    mysql_autocommit(conn, 0);

    char query[256];
    snprintf(query, sizeof(query),
             "insert into user(`username`,`password`,`mobile`,`email`) "
             "value('%s','%s','%s','%s')",
             n, pwd, ph, em);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "用户创建失败:%s\n", mysql_error(conn));
        mysql_rollback(conn);      // 回滚事务
        mysql_autocommit(conn, 1); // 恢复自动提交
        return;
    }

    // 获取新用户的id
    int newuser_id = mysql_insert_id(conn);

    char friends_query[256];
    snprintf(friends_query, sizeof(friends_query),
             "insert into friends(`user_id`,`follower_count`,`following_count`,`blocked_count`) "
             "values(%d, 0, 0, 0)",
             newuser_id);

    if (mysql_query(conn, friends_query))
    {
        fprintf(stderr, "新用户friends关系插入失败:%s\n", mysql_error(conn));
        mysql_rollback(conn);      // 回滚事务
        mysql_autocommit(conn, 1); // 恢复自动提交
        return;
    }

    // 提交事务
    mysql_commit(conn);
    mysql_autocommit(conn, 1); // 恢复自动提交
    printf("用户创建成功,且成功插入friends表\n");
}

// 添加好友功能
void friends_add(MYSQL *conn, int op_id, int oped_id, char op)
{
    mysql_autocommit(conn, 0);

    if (!(op == 'f' || op == 'u' || op == 'b' || op == 'r'))
    {
        printf("操作非法,请检查操作是否为f,u,b,r\n");
        mysql_autocommit(conn, 1);
        return;
    }

    int update_ok = 1;
    char query[256];

    if (op == 'f') // 关注
    {
        // 检查是否已关注
        snprintf(query, sizeof(query),
                 "SELECT 1 FROM follow_relation WHERE user_id = %d AND follow_id = %d", op_id, oped_id);
        if (mysql_query(conn, query) == 0)
        {
            MYSQL_RES *res = mysql_store_result(conn);
            if (mysql_fetch_row(res))
            {
                printf("已关注该用户，不能重复关注！\n");
                mysql_free_result(res);
                mysql_rollback(conn);
                mysql_autocommit(conn, 1);
                return;
            }
            mysql_free_result(res);
        }
        // 插入关注关系
        snprintf(query, sizeof(query),
                 "INSERT INTO follow_relation(user_id, follow_id) VALUES(%d, %d)", op_id, oped_id);
        if (mysql_query(conn, query))
            update_ok = 0;
    }
    else if (op == 'u') // 取关
    {
        // 检查是否已关注
        snprintf(query, sizeof(query),
                 "SELECT 1 FROM follow_relation WHERE user_id = %d AND follow_id = %d", op_id, oped_id);
        if (mysql_query(conn, query) == 0)
        {
            MYSQL_RES *res = mysql_store_result(conn);
            if (!mysql_fetch_row(res))
            {
                printf("未关注该用户，无法取关！\n");
                mysql_free_result(res);
                mysql_rollback(conn);
                mysql_autocommit(conn, 1);
                return;
            }
            mysql_free_result(res);
        }
        // 删除关注关系
        snprintf(query, sizeof(query),
                 "DELETE FROM follow_relation WHERE user_id = %d AND follow_id = %d", op_id, oped_id);
        if (mysql_query(conn, query))
            update_ok = 0;
    }
    else if (op == 'b') // 拉黑
    {
        // 检查是否已拉黑
        snprintf(query, sizeof(query),
                 "SELECT 1 FROM block_relation WHERE user_id = %d AND block_id = %d", op_id, oped_id);
        if (mysql_query(conn, query) == 0)
        {
            MYSQL_RES *res = mysql_store_result(conn);
            if (mysql_fetch_row(res))
            {
                printf("已拉黑该用户，不能重复拉黑！\n");
                mysql_free_result(res);
                mysql_rollback(conn);
                mysql_autocommit(conn, 1);
                return;
            }
            mysql_free_result(res);
        }
        // 插入拉黑关系
        snprintf(query, sizeof(query),
                 "INSERT INTO block_relation(user_id, block_id) VALUES(%d, %d)", op_id, oped_id);
        if (mysql_query(conn, query))
            update_ok = 0;
    }
    else if (op == 'r') // 解除拉黑
    {
        // 检查是否已拉黑
        snprintf(query, sizeof(query),
                 "SELECT 1 FROM block_relation WHERE user_id = %d AND block_id = %d", op_id, oped_id);
        if (mysql_query(conn, query) == 0)
        {
            MYSQL_RES *res = mysql_store_result(conn);
            if (!mysql_fetch_row(res))
            {
                printf("未拉黑该用户，无法解除拉黑！\n");
                mysql_free_result(res);
                mysql_rollback(conn);
                mysql_autocommit(conn, 1);
                return;
            }
            mysql_free_result(res);
        }
        // 删除拉黑关系
        snprintf(query, sizeof(query),
                 "DELETE FROM block_relation WHERE user_id = %d AND block_id = %d", op_id, oped_id);
        if (mysql_query(conn, query))
            update_ok = 0;
    }

    if (!update_ok)
    {
        fprintf(stderr, "操作失败: %s\n", mysql_error(conn));
        mysql_rollback(conn);
        mysql_autocommit(conn, 1);
        return;
    }

    mysql_commit(conn);
    mysql_autocommit(conn, 1);
    printf("好友操作成功，已记录到关系表。\n");
}

// 查询user表中所有用户信息并打印
void query_all_users(MYSQL *conn)
{
    // 定义SQL查询语句，查询user表的主要字段
    const char *query = "SELECT `user_id`,`username`,`password`,`mobile`,`email` FROM user";
    // 执行SQL查询
    if (mysql_query(conn, query)) // 返回值为0成功
    {
        // 查询失败时输出错误信息
        fprintf(stderr, "查询用户失败: %s\n", mysql_error(conn));
        return;
    }

    // 获取查询结果集
    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL)
    {
        // 获取结果集失败时输出错误信息
        fprintf(stderr, "获取用户结果集失败: %s\n", mysql_error(conn));
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

// 查询top3音乐
void query_top3_music(MYSQL *conn)
{
    // 定义语句
    const char *query =
        "SELECT music_name, COUNT(*) as play_count "
        "FROM user_music_history "
        "WHERE is_finished = 1 "
        "GROUP BY music_name "
        "ORDER BY play_count DESC "
        "LIMIT 3";
    // 执行SQL查询
    if (mysql_query(conn, query))
    {
        fprintf(stderr, "查询top3失败:%s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL)
    {
        fprintf(stderr, "查询top3结果集失败%s\n", mysql_error(conn));
        return;
    }

    printf("排行前三的歌曲为:\n");

    MYSQL_ROW row;
    int i = 0;
    while (row = mysql_fetch_row(result))
    {
        printf("NO%d:%-20s\n", i + 1, row[0] ? row[0] : "NULL");
        i++;
    }

    mysql_free_result(result);
}

// 根据用户听过的某首歌的类型，推荐同类型最热门的两首歌曲
void recommend_by_user_and_music(MYSQL *conn, int user_id, int music_id)
{
    char music_type[64] = {0}; // 用于存储指定歌曲的类型

    // 1. 查询该歌曲的类型
    char query_type[256];
    snprintf(query_type, sizeof(query_type),
             "SELECT music_type FROM music WHERE music_id = %d", music_id);

    // 执行查询，获取music_id对应的歌曲类型
    if (mysql_query(conn, query_type))
    {
        fprintf(stderr, "查询歌曲类型失败: %s\n", mysql_error(conn));
        return;
    }

    // 获取查询结果集
    MYSQL_RES *type_result = mysql_store_result(conn);
    if (type_result == NULL)
    {
        fprintf(stderr, "获取歌曲类型结果集失败: %s\n", mysql_error(conn));
        return;
    }

    // 取出查询到的类型
    MYSQL_ROW type_row = mysql_fetch_row(type_result);
    if (type_row && type_row[0])
    {
        // 将类型字符串拷贝到music_type变量
        strncpy(music_type, type_row[0], sizeof(music_type) - 1);
    }
    else
    {
        // 没查到类型，无法推荐
        printf("未找到该歌曲类型，无法推荐。\n");
        mysql_free_result(type_result);
        return;
    }
    mysql_free_result(type_result); // 释放类型查询结果集

    // 2. 推荐同类型最热门的两首歌曲（不包含当前music_id）
    char query_recommend[512];
    snprintf(query_recommend, sizeof(query_recommend),
             "SELECT b.music_name, COUNT(*) as play_count "
             "FROM user_music_history a "
             "LEFT JOIN music b ON a.music_id = b.music_id "
             "WHERE a.is_finished = 1 AND b.music_type = '%s' AND b.music_id != %d "
             "GROUP BY b.music_name "
             "ORDER BY play_count DESC "
             "LIMIT 2",
             music_type, music_id);

    // 执行推荐查询
    if (mysql_query(conn, query_recommend))
    {
        fprintf(stderr, "推荐歌曲查询失败: %s\n", mysql_error(conn));
        return;
    }

    // 获取推荐结果集
    MYSQL_RES *rec_result = mysql_store_result(conn);
    if (rec_result == NULL)
    {
        fprintf(stderr, "获取推荐结果集失败: %s\n", mysql_error(conn));
        return;
    }

    // 打印推荐结果
    printf("为用户%d推荐类型(%s)最热门的两首歌曲:\n", user_id, music_type);
    MYSQL_ROW row;
    while (row = mysql_fetch_row(rec_result))
    {
        // 输出歌曲名（row[0]），若为空则输出"NULL"
        printf("%-20s\n", row[0] ? row[0] : "NULL");
    }

    mysql_free_result(rec_result); // 释放推荐结果集
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

    // user_sign(conn, "liujeilun", "chaoqiqwq", "17899992341", "2403247788@qq.com");
    // query_all_users(conn);
    // query_top3_music(conn);
    // recommend_by_user_and_music(conn, 3, 5);
    friends_add(conn, 1, 7, 'f');

    // 结束程序前关闭数据库连接，释放资源
    mysql_close(conn);

    return 0; // 程序正常结束
}