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
    Person person;
    person.first_name = "Steve";
    person.last_name = "Jobs";
    try
    {
        sql << "insert into Person(first_name, last_name) values(:first_name, :last_name)", use(person);
    }
    catch (const soci::soci_error& e)
    {
        cout << "err:" << e.what() << endl;
        return -1;
    }
    long id;
    if (!sql.get_last_insert_id("Person", id))
    {
        return -1;
    }
    person.id = id;
    cout << "insert into Person success" << endl
        << "  id:" << person.id << endl
        << "  first_name:" << person.first_name << endl
        << "  last_name:" << person.last_name << endl;
    return id;
}

void select(int id)
{
    session sql(g_pool);
    Person person;
    try
    {
        sql << "select * from Person where id=:id", use(id), into(person);
        if (!sql.got_data())
        {
            cout << "select success, no record of id:" << id << endl; 
            return ;
        }
    }
    catch (const soci::soci_error& e)
    {
        cout << "err:" << e.what() << endl;
        return;
    }
    cout << "select success" << endl
        << "  id:" << person.id << endl
        << "  first_name:" << person.first_name << endl
        << "  last_name:" << person.last_name << endl; 
}

void update(int id, const string& first_name, const string& last_name)
{
    session sql(g_pool);
    Person person;
    person.id = id;
    person.first_name = first_name;
    person.last_name = last_name;
    try
    {
        sql << "update Person set first_name=:first_name, last_name=:last_name where id=:id", use(person);
    }
    catch (const soci::soci_error& e)
    {
        cout << "err:" << e.what() << endl;
        return;
    }
    cout << "update success." << endl
        << "  id:" << person.id << endl
        << "  first_name:" << person.first_name << endl
        << "  last_name:" << person.last_name << endl;
}

void remove(int id)
{
    session sql(g_pool);
    Person person;
    person.id = id;
    statement st = (sql.prepare << "delete from Person where id=:id", use(person));
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
    rowset<Person> rs = (sql.prepare << "select * from Person");
    cout << "select_all success" << endl;
    for (rowset<Person>::iterator it = rs.begin(); it != rs.end(); ++it)
    {
        cout << "----------------" << endl;
        const Person& person = *it;
        cout << "  id:" << person.id << endl
            << "  first_name:" << person.first_name << endl
            << "  last_name:" << person.last_name << endl;
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
