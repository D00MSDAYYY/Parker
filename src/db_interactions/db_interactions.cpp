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
    _employees.reset(new Employees_Table{_connection});
}

Employees_Table::Employees_Table(PGconn* connection)
    : Table{connection}
{
    auto res{PQexec(_connection,
                    " CREATE TABLE IF NOT EXISTS employees ("
                    " tg_id integer, "
                    " firstname text, "
                    " middlename text, "
                    " lastname text, "
                    " car_model text, "
                    " license_plate text );")};

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "FAILED TO CREATE employees TABLE: %s", PQerrorMessage(_connection));
        PQclear(res);
    }
    PQclear(res);
}

bool
Employees_Table::add(std::vector<std::string> args) const
{
    const int param_num{6};

    if(args.size() == param_num && !exists({args[0]}))
    {
        std::string query{"INSERT INTO employees VALUES ( "};
        query += (args[0].empty()) ? " NULL " : args[0];
        query += (args[1].empty()) ? " NULL " : ", ' " + args[1] + " ' ";
        query += (args[2].empty()) ? " NULL " : ", ' " + args[2] + " ' ";
        query += (args[3].empty()) ? " NULL " : ", ' " + args[3] + " ' ";
        query += (args[4].empty()) ? " NULL " : ", ' " + args[4] + " ' ";
        query += (args[5].empty()) ? " NULL " : ", ' " + args[5] + " ' ";
        query += " ); ";

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
Employees_Table::remove(std::vector<std::string> args) const
{
    const int param_num{6};
    std::cerr
        << "**************************************************************************\n";
    if(args.size() == param_num && exists({args[0]}))
    {
        std::string      query{"DELETE FROM employees WHERE "};

        std::vector<int> indexes{};
        int              i{0};
        std::for_each(args.cbegin(),
                      args.cend(),
                      [&indexes, &i](const auto& elem)
                      {
                          if(!elem.empty()) indexes.push_back(i);
                          ++i;
                      });

        if(std::find(indexes.cbegin(), indexes.cend(), 0) < indexes.cend())
            query += std::string("tg_id = ") + " " + args[0].c_str() + " ";
        if(std::find(indexes.cbegin(), indexes.cend(), 0) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 1) < indexes.cend())
            query += std::string("firstname = ") + " '" + args[1].c_str() + "' ";
        if(std::find(indexes.cbegin(), indexes.cend(), 1) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 2) < indexes.cend())
            query += std::string("middlename = ") + " '" + args[2].c_str()+ "' ";
        if(std::find(indexes.cbegin(), indexes.cend(), 2) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 3) < indexes.cend())
            query += std::string("lastname = ") + " '" + args[3].c_str()+ "' ";
        if(std::find(indexes.cbegin(), indexes.cend(), 3) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 4) < indexes.cend())
            query += std::string("car_model = ") + " '" + args[4].c_str() + "' ";
        if(std::find(indexes.cbegin(), indexes.cend(), 4) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 5) < indexes.cend())
            query += std::string("license_plate = ") + " '" + args[5].c_str() + "' ";

        query += " ); ";

        std::cerr << query << std::endl;

        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "DELETE in employees failed: %s", PQerrorMessage(_connection));
            PQclear(res);
            return false;
        }
        PQclear(res);

        return true;
    }
    else { return false; }
}

bool
Employees_Table::update(std::vector<std::string> args) const
{
    const int param_num{6};

    if(args.size() == param_num && exists({args[0]}))
    {
        std::string query{"UPDATE employees SET "};

        query += (args[0].empty()) ? " " : " " + args[0];
        query += (args[1].empty()) ? " " : ", ' " + args[1] + " ' ";
        query += (args[2].empty()) ? " " : ", ' " + args[2] + " ' ";
        query += (args[3].empty()) ? " " : ", ' " + args[3] + " ' ";
        query += (args[4].empty()) ? " " : ", ' " + args[4] + " ' ";
        query += (args[5].empty()) ? " " : ", ' " + args[5] + " ' ";
        query += " ); ";

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
Employees_Table::exists(std::vector<std::string> args) const
{
    const int param_num{6};

    if(args.size() <= param_num)
    {
        std::string      query{"SELECT EXISTS(SELECT 1 FROM employees WHERE "};

        std::vector<int> indexes{};
        int              i{0};
        std::for_each(args.cbegin(),
                      args.cend(),
                      [&indexes, &i](const auto& elem)
                      {
                          if(!elem.empty()) indexes.push_back(i);
                          ++i;
                      });

        if(std::find(indexes.cbegin(), indexes.cend(), 0) < indexes.cend())
            query += std::string(" tg_id = ") + args[0].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 0) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 1) < indexes.cend())
            query += std::string(" firstname = ") + args[1].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 1) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 2) < indexes.cend())
            query += std::string(" middlename = ") + args[2].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 2) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 3) < indexes.cend())
            query += std::string(" lastname = ") + args[3].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 3) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 4) < indexes.cend())
            query += std::string(" car_model = ") + args[4].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 4) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 5) < indexes.cend())
            query += std::string(" license_plate = ") + args[5].c_str();
        query += " ) AS exists ; ";

        

        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            fprintf(stderr, "EXISTS in employees failed: %s", PQerrorMessage(_connection));
            PQclear(res);
            return false;
        }
        
        auto exists_str{PQgetvalue(res, 0, 0)};
        PQclear(res);

        std::cerr << query << exists_str[0]  << std::endl;

        return (exists_str[0] = 't') ? true : false;
    }
    else { return false; }
}

int
Employees_Table::count(std::vector<std::string> args) const
{
    const int param_num{6};

    if(args.size() <= param_num)
    {
        std::string      query{"SELECT COUNT(*) FROM employees WHERE "};

        std::vector<int> indexes{};
        int              i{0};
        std::for_each(args.cbegin(),
                      args.cend(),
                      [&indexes, &i](const auto& elem)
                      {
                          ++i;
                          if(elem.empty()) indexes.push_back(i);
                      });

        if(std::find(indexes.cbegin(), indexes.cend(), 0) < indexes.cend())
            query += std::string(" tg_id = ") + args[0].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 0) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 1) < indexes.cend())
            query += std::string(" firstname = ") + args[1].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 1) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 2) < indexes.cend())
            query += std::string(" middlename = ") + args[2].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 2) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 3) < indexes.cend())
            query += std::string(" lastname = ") + args[3].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 3) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 4) < indexes.cend())
            query += std::string(" car_model = ") + args[4].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 4) + 1 < indexes.cend())
            query += " AND ";

        if(std::find(indexes.cbegin(), indexes.cend(), 5) < indexes.cend())
            query += std::string(" license_plate = ") + args[5].c_str();
        query += " ); ";

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
    else { return 0; }
}

bool
Table::update(std::vector<std::string> args) const
{
    const int param_num{6};

    if(args.size() <= param_num)
    {
        std::string      query{"UPDATE employees SET "};

        std::vector<int> indexes{};
        int              i{0};
        std::for_each(args.cbegin(),
                      args.cend(),
                      [&indexes, &i](const auto& elem)
                      {
                          ++i;
                          if(elem.empty()) indexes.push_back(i);
                      });

        if(std::find(indexes.cbegin(), indexes.cend(), 0) < indexes.cend())
            query += std::string(" tg_id = ") + args[0].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 0) + 1 < indexes.cend()) query += " , ";

        if(std::find(indexes.cbegin(), indexes.cend(), 1) < indexes.cend())
            query += std::string(" firstname = ") + args[1].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 1) + 1 < indexes.cend()) query += " , ";

        if(std::find(indexes.cbegin(), indexes.cend(), 2) < indexes.cend())
            query += std::string(" middlename = ") + args[2].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 2) + 1 < indexes.cend()) query += " , ";

        if(std::find(indexes.cbegin(), indexes.cend(), 3) < indexes.cend())
            query += std::string(" lastname = ") + args[3].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 3) + 1 < indexes.cend()) query += " , ";

        if(std::find(indexes.cbegin(), indexes.cend(), 4) < indexes.cend())
            query += std::string(" car_model = ") + args[4].c_str();
        if(std::find(indexes.cbegin(), indexes.cend(), 4) + 1 < indexes.cend()) query += " , ";

        if(std::find(indexes.cbegin(), indexes.cend(), 5) < indexes.cend())
            query += std::string(" license_plate = ") + args[5].c_str();

        query += " WHERE tg_id = " + args[0];
        query += " ); ";

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
    else { return 0; }
}
