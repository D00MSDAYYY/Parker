#include "db_interactions.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>

Data_Base::Data_Base(std::string name)
{
    if(name.empty())
        name = "dbname=Parker";
    else
        name = "dbname=" + name;
    _connection = PQconnectdb(name.c_str());

    if(PQstatus(_connection) != CONNECTION_OK) { std::cerr << PQerrorMessage(_connection); }

    _employees = Employees_Table{_connection};
}

Employees_Table::Employees_Table(PGconn* connection)
    : Table{connection}
{
    auto res{PQexec(_connection,
                    " CREATE TABLE IF NOT EXISTS employees ("
                    " tg_id bigint, "
                    " firstname text, "
                    " middlename text, "
                    " lastname text, "
                    " car_model text, "
                    " license text );")};

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "FAILED TO CREATE employees TABLE: %s", PQerrorMessage(_connection));
        PQclear(res);
    }
    PQclear(res);
}

bool
Employees_Table::add(Query_Args args) const
{
    if(!exists({.tg_id = args.tg_id}))
    {
        std::string query{};
        query += (args.tg_id.empty()) ? " NULL " : args.tg_id;
        query += (args.firstname.empty()) ? ", NULL " : ", '" + args.firstname + "' ";
        query += (args.middlename.empty()) ? ", NULL " : ", '" + args.middlename + "' ";
        query += (args.lastname.empty()) ? ", NULL " : ", '" + args.lastname + "' ";
        query += (args.car_model.empty()) ? ", NULL " : ", '" + args.car_model + "' ";
        query += (args.license.empty()) ? ", NULL " : ", '" + args.license + "' ";
        query = "INSERT INTO employees VALUES ( " + query + " ); ";

        std::cerr << query << std::endl;

        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "INSERT in employees failed: %s", PQerrorMessage(_connection));
            PQclear(res);
            return false;
        }
        PQclear(res);

        return true;
    }
    else { return false; }
}

bool
Employees_Table::remove(Query_Args args) const
{
    if(exists({.tg_id = args.tg_id}))
    {
        std::string query{};

        if(!args.tg_id.empty()) query += "tg_id = " + args.tg_id;

        if(!args.firstname.empty())
        {
            if(!query.empty()) query += " AND ";
            query += "firstname = " + args.firstname;
        }

        if(!args.middlename.empty())
        {
            if(!query.empty()) query += " AND ";
            query += "middlename = " + args.middlename;
        }
        if(!args.lastname.empty())
        {
            if(!query.empty()) query += " AND ";
            query += "lastname = " + args.lastname;
        }
        if(!args.car_model.empty())
        {
            if(!query.empty()) query += " AND ";
            query += "car_model = " + args.car_model;
        }
        if(!args.license.empty())
        {
            if(!query.empty()) query += " AND ";
            query += "license = " + args.license;
        }
        query = "DELETE FROM employees WHERE " + query + ";";

        std::cerr << query << std::endl;

        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "DELETE in employees failed: %s", PQerrorMessage(_connection));
            PQclear(res);
            return false;
        }
        auto res_str{PQgetvalue(res, 0, 0)};
        std::cerr << "!!!!!!!!!! -> " << res_str << std::endl;
        PQclear(res);

        return true;
    }
    else { return false; }
}

bool
Employees_Table::update(Query_Args args) const
{
    if(exists({.tg_id = args.tg_id}))
    {
        std::string query{};

        if(!args.tg_id.empty()) query += "tg_id = '" + args.tg_id + "'";

        if(!args.firstname.empty())
        {
            if(!query.empty()) query += " , ";
            query += "firstname = '" + args.firstname + "'";
        }

        if(!args.middlename.empty())
        {
            if(!query.empty()) query += " , ";
            query += "middlename = '" + args.middlename + "'";
        }
        if(!args.lastname.empty())
        {
            if(!query.empty()) query += " , ";
            query += "lastname = '" + args.lastname + "'";
        }
        if(!args.car_model.empty())
        {
            if(!query.empty()) query += " , ";
            query += "car_model = '" + args.car_model + "'";
        }
        if(!args.license.empty())
        {
            if(!query.empty()) query += " , ";
            query += "license = '" + args.license + "'";
        }
        query = "UPDATE employees SET " + query + ";";

        std::cerr << query << std::endl;

        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "INSERT in employees failed: %s", PQerrorMessage(_connection));
            PQclear(res);
            return false;
        }
        PQclear(res);

        return true;
    }
    else { return false; }
}

bool
Employees_Table::exists(Query_Args args) const
{
    std::string query{};

    if(!args.tg_id.empty()) query += "tg_id = " + args.tg_id ;

    if(!args.firstname.empty())
    {
        if(!query.empty()) query += " , ";
        query += "firstname = '" + args.firstname + "'";
    }
    if(!args.middlename.empty())
    {
        if(!query.empty()) query += " , ";
        query += "middlename = '" + args.middlename + "'";
    }
    if(!args.lastname.empty())
    {
        if(!query.empty()) query += " , ";
        query += "lastname = '" + args.lastname + "'";
    }
    if(!args.car_model.empty())
    {
        if(!query.empty()) query += " , ";
        query += "car_model = '" + args.car_model + "'";
    }
    if(!args.license.empty())
    {
        if(!query.empty()) query += " , ";
        query += "license = '" + args.license + "'";
    }
    query = "SELECT EXISTS(SELECT 1 FROM employees WHERE " + query + " ) AS exists ; ";

    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "EXISTS in employees failed: %s", PQerrorMessage(_connection));
        PQclear(res);
        return false;
    }
    auto res_str{PQgetvalue(res, 0, 0)};
    PQclear(res);

    return (res_str[0] == 't') ? true : false;
}

int
Employees_Table::count(Query_Args args) const
{
    std::string query{};

    if(!args.tg_id.empty()) query += "tg_id = '" + args.tg_id + "'";

    if(!args.firstname.empty())
    {
        if(!query.empty()) query += " , ";
        query += "firstname = '" + args.firstname + "'";
    }
    if(!args.middlename.empty())
    {
        if(!query.empty()) query += " , ";
        query += "middlename = '" + args.middlename + "'";
    }
    if(!args.lastname.empty())
    {
        if(!query.empty()) query += " , ";
        query += "lastname = '" + args.lastname + "'";
    }
    if(!args.car_model.empty())
    {
        if(!query.empty()) query += " , ";
        query += "car_model = '" + args.car_model + "'";
    }
    if(!args.license.empty())
    {
        if(!query.empty()) query += " , ";
        query += "license = '" + args.license + "'";
    }

    query = "SELECT COUNT(*) FROM employees WHERE " + query + " ); ";

    std::cerr << query << std::endl;

    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "COUNT in employees failed: %s", PQerrorMessage(_connection));
        PQclear(res);
        return 0;
    }
    auto count_str{PQgetvalue(res, 0, 0)};

    PQclear(res);

    return std::atoi(count_str);
}

bool
Table::update(Query_Args args) const
{
    std::string query{};

    if(!args.firstname.empty()) { query += "firstname = '" + args.firstname + "'"; }

    if(!args.middlename.empty())
    {
        if(!query.empty()) query += " , ";
        query += "middlename = '" + args.middlename + "'";
    }
    if(!args.lastname.empty())
    {
        if(!query.empty()) query += " , ";
        query += "lastname = '" + args.lastname + "'";
    }
    if(!args.car_model.empty())
    {
        if(!query.empty()) query += " , ";
        query += "car_model = '" + args.car_model + "'";
    }
    if(!args.license.empty())
    {
        if(!query.empty()) query += " , ";
        query += "license = '" + args.license + "'";
    }

    query = "UPDATE employees SET " + query + " WHERE tg_id = " + args.tg_id + " ); ";

    std::cerr << query << std::endl;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "UPDATE in employees failed: %s", PQerrorMessage(_connection));
        PQclear(res);
        return 0;
    }
    auto count_str{PQgetvalue(res, 0, 0)};

    PQclear(res);

    return std::atoi(count_str);
}
