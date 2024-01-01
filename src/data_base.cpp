#include "data_base.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>

Data_Base::Data_Base(std::string name)
{
    if(name.empty())
        name = "dbname=parker";
    else
        name = "dbname=" + name;
    _connection = PQconnectdb(name.c_str());

    if(PQstatus(_connection) != CONNECTION_OK) { std::cerr << PQerrorMessage(_connection); }
    _states    = States{this};
    _employees = Employees_Table{_connection, &_states};
}

Employees_Table::Employees_Table(PGconn* connection, States* states)
    : Table{connection},
      _states{states}
{
    // before this execute in terminal command : "CREATE DATABASE Parker WITH ENCODING 'UTF8'
    // LC_COLLATE='ru_RU.UTF-8' LC_CTYPE='ru_RU.UTF-8' TEMPLATE=template0;"
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
        query  = "INSERT INTO employees VALUES ( " + query + " ) RETURNING tg_id; ";

        // std::cerr << query << std::endl;

        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            fprintf(stderr, "INSERT in employees failed: %s", PQerrorMessage(_connection));
            PQclear(res);
            return false;
        }

        for(int i{0}; i < PQntuples(res); ++i)
            _states->add(std::stoi(PQgetvalue(res, i, PQfnumber(res, "tg_id"))));

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
        query = "DELETE FROM employees WHERE " + query + " RETURNING tg_id;";

        // std::cerr << query << std::endl;

        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            fprintf(stderr, "DELETE in employees failed: %s", PQerrorMessage(_connection));
            PQclear(res);
            return false;
        }

        for(int i{0}; i < PQntuples(res); ++i)
            _states->remove(std::stoi(PQgetvalue(res, i, PQfnumber(res, "tg_id"))));
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
        query = "UPDATE employees SET " + query + " WHERE tg_id = " + args.tg_id + "; ";

        // std::cerr << query << std::endl;

        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "UPDATE in employees failed: %s", PQerrorMessage(_connection));
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

    if(!args.tg_id.empty()) query += "tg_id = " + args.tg_id;

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

    // std::cerr << query;

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

    // std::cerr << query << std::endl;

    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "COUNT in employees failed: %s", PQerrorMessage(_connection));
        PQclear(res);
        return 0;
    }
    auto count_str{PQgetvalue(res, 0, 0)};

    PQclear(res);

    return std::stoi(count_str);
}

std::optional<std::vector<Query_Args>>
Employees_Table::select_where(Query_Args args) const
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
    query = "SELECT * FROM employees WHERE " + query + " LIMIT 20; ";

    std::cerr << query << std::endl;

    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "SELECT WHERE in employees failed: %s", PQerrorMessage(_connection));
        PQclear(res);
        return {};
    }
    std::vector<Query_Args> users;
    for(int i{0}; i < PQntuples(res); ++i)
    {
        users.push_back({PQgetvalue(res, i, PQfnumber(res, "tg_id")),
                        PQgetvalue(res, i, PQfnumber(res, "firstname")),
                        PQgetvalue(res, i, PQfnumber(res, "middlename")),
                        PQgetvalue(res, i, PQfnumber(res, "lastname")),
                        PQgetvalue(res, i, PQfnumber(res, "car_model")),
                        PQgetvalue(res, i, PQfnumber(res, "license"))});
    }
    PQclear(res);

    return users;
}

FSM&
States::at(decltype(TgBot::User::id) id)
{
    if(_states.contains(id))
        return _states.at(id);
    else
    {
        if(_db->employees().exists({.tg_id = std::to_string(id)}))
        {
            _states.insert({id, FSM::USER_AUTHORIZED});
            return _states.at(id);
        }
        else
        {
            _def_state = FSM::USER_NOT_AUTHORIZED;
            return _def_state;
        }
    }
}

void
States::remove(decltype(TgBot::User::id) id)
{
    _states.erase(id);
}

void
States::add(decltype(TgBot::User::id) id)
{
    _states.erase(id);
}
