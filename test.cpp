#include <iostream>
#include <string>
#include <stdint.h>
#include "soci/soci.h"
//#include "soci/type-conversion.h"

using namespace soci;
using namespace std;

int g_pool_size = 3;
connection_pool g_pool(g_pool_size);

struct Person
{
    uint32_t id;
    string first_name;
    string last_name;
};

namespace soci {
template<>
struct type_conversion<Person>
{
    typedef values base_type;
    static void from_base(const values& v, indicator ind, Person& person)
    {
        person.id = v.get<long long>("id");
        person.first_name = v.get<string>("first_name");
        person.last_name = v.get<string>("last_name");

    }
    static void to_base(const Person& person, values& v, indicator& ind)
    {
        v.set("id", (long long)person.id);
        v.set("first_name", person.first_name);
        v.set("last_name", person.last_name);
    }
};
}

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
    cout << "insert into Person success, id:" << id << " first_name:" << first_name << " last_name:" << last_name << endl;
    return id;
}

void select(int id)
{
    session sql(g_pool);
    string first_name;
    string last_name;
    try
    {
        //sql << "select first_name, last_name from Person where id=:id", use(id), into(first_name), into(last_name);
        sql << "select first_name, last_name from Person where id=:id", use(id);
    }
    catch (const soci::soci_error& e)
    {
        cout << "err:" << e.what();
        return;
    }
    cout << "select success, id:" << id << " first_name:" << first_name << " last_name:" << last_name << endl; 
}

void update(int id, const string& first_name, const string& last_name)
{
    session sql(g_pool);
    sql << "update Person set first_name=:first_name, last_name=:last_name where id=:id", use(first_name), use(last_name), use(id);
}

void remove(int id)
{
    session sql(g_pool);
    //string first_name = "hello";
    //string last_name = "world";
    statement st = (sql.prepare << "delete from Person where id=:id", use(id));
    st.execute(true);
    int affected_rows = st.get_affected_rows();
    cout << "delte id:" << id << " affected_rows:" << affected_rows;
    //sql << "delete from Person where first_name=:first_name and last_name=:last_name", use(first_name), use(last_name);
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
    for (rowset<row>::iterator it = rs.begin(); it != rs.end(); ++it)
    {
        const row& row = *it;
        cout << "id:" << row.get<unsigned long long>(0)
            << " first_name:" << row.get<string>(1)
            << " last_name:" << row.get<string>(2) << endl;
    }
    //rowset<Person> rs = (sql.prepare << "select * from Person");
    //for (rowset<Person>::iterator it = rs.begin(); it != rs.end(); ++it)
    //{
    //    Person person = *it;
    //    cout << "id:" << person.id
    //        << " first_name:" << person.first_name
    //        << " last_name:" << person.last_name << endl;
    //}
}

int main()
{
    init_pool();
    //int id = insert();
    //select(3);
    //update(id, "hello", "world");
    //remove(2);
    select_all();
    return 0;
}
