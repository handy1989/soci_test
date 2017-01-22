#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include "soci/soci.h"
#include "soci/error.h"
#include "soci/mysql/soci-mysql.h"
#include "soci/connection-pool.h"

using namespace soci;
using namespace std;


const size_t poolSize = 1;
connection_pool pool(poolSize);

struct Person
{
    uint64_t id;
    std::string firstName;
    std::string lastName;
};

namespace soci
{
    template<>
    struct type_conversion<Person>
    {
        typedef values base_type;

        static void from_base(values const & v, indicator /* ind */, Person & p)
        {
            p.id = v.get<unsigned long>("ID");
            p.firstName = v.get<std::string>("FIRST_NAME");
            p.lastName = v.get<std::string>("LAST_NAME");

            p.gender = v.get<std::string>("GENDER");
            // p.gender will be set to the default value "unknown"
            // when the column is null:
            //p.gender = v.get<std::string>("GENDER", "unknown");

            // alternatively, the indicator can be tested directly:
            //if (v.get_indicator("GENDER") == i_null)
            //{
            //    p.gender = "unknown";
            //}
            //else
            //{
            //    p.gender = v.get<std::string>("GENDER");
            //}
        }
    
        static void to_base(const Person & p, values & v, indicator & ind)
        {
            v.set("ID", p.id);
            v.set("FIRST_NAME", p.firstName);
            v.set("LAST_NAME", p.lastName);
            v.set("GENDER", p.gender, p.gender.empty() ? i_null : i_ok);
            ind = i_ok;
        }
    };
}

struct ApiSecret
{
    ApiSecret() {}
    ApiSecret(const std::string& api_key, const std::string& secret) :
        api_key_(api_key), secret_(secret) {}

    //unsigned long long id_;
    uint64_t id_;
    int32_t num_;
    std::string api_key_;
    std::string secret_;
};

namespace soci
{

template<>
struct type_conversion<ApiSecret>
{
    typedef values base_type;
    static void from_base(const values& v, indicator ind, ApiSecret& api_secret)
    {
        //api_secret.id_ = v.get<unsigned long long>("id");
        api_secret.id_ = v.get<long long>("id");
        api_secret.num_ = v.get<int>("num");
        api_secret.api_key_ = v.get<string>("api_key");
        api_secret.secret_ = v.get<string>("secret");

    }
    static void to_base(const ApiSecret& api_secret, values& v, indicator& ind)
    {
        v.set("id", (long long)api_secret.id_);
        v.set("num", (int)api_secret.num_);
        v.set("api_key", api_secret.api_key_);
        v.set("secret", api_secret.secret_);
    }
};

}
void func(connection_pool& pool, int x)
{
    size_t pos;
    if (!pool.try_lease(pos, 1000))
    {
        return ;
    }
    printf("x:%d pos:%lu\n", x, pos);
}

void init_pool()
{
    for (int i = 0; i < poolSize; ++i)
    {
        session& sql = pool.at(i);
        sql.open("mysql", "host=localhost port=6379 dbname=test user=zhangmenghan password=123456");
    }
    cout << "init_pool success" << endl;
}

void test1()
{
    session sql(pool);
    int count;
    try
    {
        sql << "select count(*) from Message", into(count);
        cout << "count:" << count << endl;
    }
    catch (const soci::soci_error& e)
    {
        cout << "err1:" << e.what() << endl; 
        try
        {
            sql.reconnect();
            sql << "select count(*) from Message", into(count);
            cout << "count:" << count << endl;
        }
        catch (const soci::soci_error& e)
        {
            cout << "err2:" << e.what() << endl;
        }
    }
}

void test_reconnect()
{
    try
    {
        int line_size = 1024;
        char line[1024];
        cout << "continue?[Y/N]:";
        fgets(line, line_size, stdin);
        if (strcasecmp(line, "Y") != 0)
        {
            cout << "return" << endl;
            return ; 
        }
        test1();
        cout << "continue?[Y/N]:";
        fgets(line, line_size, stdin);
        if (strcasecmp(line, "Y") != 0)
        {
            cout << "return" << endl;
            return ; 
        }
        test1();
    }
    catch (soci::mysql_soci_error const & e)
    {
        cout << "mysql error:" << e.err_num_ << " " << e.what() << endl; 
    }
    catch (const soci::soci_error& e)
    {
        cout << "err:" << e.what() << endl;
    }

}


void test_object2()
{
    session sql(pool);
    Person p;
    //p.id = 5;
    p.lastName = "Smith";
    p.firstName = "Pat";
    p.gender = "M";
    sql << "insert into person(first_name, last_name, gender) "
           "values(:FIRST_NAME, :LAST_NAME, :GENDER)", use(p);

    long id;
    sql.get_last_insert_id("person", id);
    p.id = id;
    cout << "id:" << p.id;

    //Person p1;
    //sql << "select * from person where ID = " << id, into(p1);
    //cout << "id:" << p1.id << " name:" << p1.firstName << p1.lastName << " gender:" << p1.gender << endl;

    p.firstName = "Patricia";
    sql << "update person set first_name = :FIRST_NAME "
           "where id = :ID", use(p);
}

void test_object()
{
    ApiSecret api_secret;
    api_secret.api_key_ = "aaa";
    api_secret.secret_ = "bbb";
    string api_key = "aaa";
    string secret = "ccc";
    session sql(pool);
    sql << "insert into ApiSecret(api_key, secret) values(:api_key, :secret)", use(api_secret);
    //sql << "insert into ApiSecret(api_key, secret) values(:api_key, :secret)", use(api_secret.api_key_), use(api_secret.secret_);
    //sql << "insert into ApiSecret(api_key, secret) values(:api_key, :secret)", use(api_key), use(secret);
    //sql << "insert into ApiSecret(api_key, secret) values(:api_key, :secret)", use(api_secret);
    long id;
    sql.get_last_insert_id("ApiSecret", id);
    cout << "id:" << id << endl;
    api_secret.id_ = id;

    //ApiSecret api_secret2;
    ////sql << "select id, api_key, secret from ApiSecret where id = " << api_secret.id_, into(api_secret2.id_), into(api_secret2.api_key_), into(api_secret2.secret_);
    //sql << "select id, api_key, secret from ApiSecret where id = " << api_secret.id_, into(api_secret2);
    //cout << "id:" << api_secret2.id_ << " api_key:" << api_secret2.api_key_ << " secret:" << api_secret2.secret_ << endl;

}

void test_select()
{
    session sql(pool);
    ApiSecret api_secret2;
    //sql << "select * from ApiSecret where id=1", into(api_secret2.id_), into(api_secret2.api_key_), into(api_secret2.secret_);
    sql << "select * from ApiSecret where id=670", into(api_secret2);
    if (sql.got_data())
    {
        cout << "id:" << api_secret2.id_ << " api_key:" << api_secret2.api_key_ << " secret:" << api_secret2.secret_ << endl;
    }
    else
    {
        cout << "not exists" << endl;
    }

    //api_secret2.secret_ = "ccccccc";
    //sql << "update ApiSecret set secret = :secret where id = :id", use(api_secret2);
}
void test_delete()
{
    session sql(pool);
    int result;
    statement st = (sql.prepare << "delete from ApiSecret where id=91");
    st.execute(true);
    if (st.get_affected_rows())
    {
        cout << "delete got_data" << endl;
    }
    else
    {
        cout << "delete has not data" << endl;
    }
    //cout << "result:" << result << endl;
}
void test_exists()
{
    session sql(pool);
    int count;
    sql << "select count(*) from ApiSecret", into(count);
    cout << "ApiSecret count:" << count << endl;
}

int main()
{
    init_pool();
    //test_object();
    //test_select();
    //test_object2();
    //test_exists();
    test_delete();
    return 0;
}

