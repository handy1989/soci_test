#include <iostream>
#include <string>
#include <stdint.h>
#include "soci/soci.h"

using namespace soci;
using namespace std;

int g_pool_size = 3;
connection_pool g_pool(g_pool_size);

int insert()
{
    session sql(g_pool);
    string first_name = "Steve";
    string last_name = "Jobs";
    try
    {
        sql << "insert into Person(first_name, last_name) values(:first_nam, :last_name)", use(first_name), use(last_name);
    }
    catch (const soci::soci_error& e)
    {
        cout << "err:" << e.what();
        return -1;
    }
    long id;
    if (!sql.get_last_insert_id("Person", id))
    {
        return -1;
    }
    cout << "insert into Person success" << endl
        << "  id:" << id << endl
        << "  first_name:" << first_name << endl
        << "  last_name:" << last_name << endl;
    return id;
}

void select(int id)
{
    session sql(g_pool);
    string first_name;
    string last_name;
    try
    {
        sql << "select first_name, last_name from Person where id=:id", use(id), into(first_name), into(last_name);
        if (!sql.got_data())
        {
            cout << "select success, no record of id:" << id << endl; 
            return ;
        }
    }
    catch (const soci::soci_error& e)
    {
        cout << "err:" << e.what();
        return;
    }
    cout << "select success" << endl
        << "  id:" << id << endl
        << "  first_name:" << first_name << endl
        << "  last_name:" << last_name << endl; 
}

void update(int id, const string& first_name, const string& last_name)
{
    session sql(g_pool);
    try
    {
        sql << "update Person set first_name=:first_name, last_name=:last_name where id=:id", use(first_name), use(last_name), use(id);
    }
    catch (const soci::soci_error& e)
    {
        cout << "err:" << e.what() << endl;
        return;
    }
    cout << "update success." << endl
        << "  id:" << id << endl
        << "  first_name:" << first_name << endl
        << "  last_name:" << last_name << endl;
}

void remove(int id)
{
    session sql(g_pool);
    statement st = (sql.prepare << "delete from Person where id=:id", use(id));
    st.execute(true);
    int affected_rows = st.get_affected_rows();
    cout << "delte success" << endl
        << "  id:" << id  << endl
        << "  affected_rows:" << affected_rows << endl;
}

void init_pool()
{
    for (int i = 0; i < g_pool_size; ++i)
    {
        session& sql = g_pool.at(i);
        sql.open("mysql", "dbname=test user=zhangmenghan password=123456");
    }
}

void select_all()
{
    session sql(g_pool);
    rowset<row> rs = (sql.prepare << "select * from Person");
    cout << "select_all success" << endl;
    for (rowset<row>::iterator it = rs.begin(); it != rs.end(); ++it)
    {
        cout << "----------------" << endl;
        const row& row = *it;
        cout << "  id:" << row.get<long long>(0) << endl
            << "  first_name:" << row.get<string>(1) << endl
            << "  last_name:" << row.get<string>(2) << endl;
    }
}

int main()
{
    init_pool();
    int id = insert();
    select(id);
    select_all();
    update(id, "hello", "world");
    select(id);
    remove(id);
    select(id);
    return 0;
}
