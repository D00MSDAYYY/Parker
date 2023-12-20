#include "db_interactions.hpp"

#include <iostream>

// bool
// Employees_Table::add()
// {
//
//     else
//     {
//         for(auto& arg: _args)
//             if(arg.empty()) return false;       //! check for empty strings

//           //! insert parameters for $... placeholders
//

//

//         if(PQresultStatus(res) != PGRES_TUPLES_OK)
//         {
//             fprintf(stderr, "INSERT failed: %s", PQerrorMessage(_connection));
//             PQclear(res);
//             exit_nicely(_connection);
//         }
//         PQclear(res);
//         return true;
//     }
// }

static void
exit_nicely(PGconn* conn)
{
    PQfinish(conn);
    exit(1);
}

bool
Employees_Table::add(std::vector<std::optional<const std::string>> args) const
{
    const int param_num{6};

    if(args.size() == param_num)
    {
        for(const auto& arg: args)
            if(!arg.has_value() || (!arg.has_value() && (*arg).empty())) return false;

        const char* param_values[param_num];
        for(int i{0}; i < param_num; ++i)
            param_values[i] = (*args[i]).c_str();
            
        auto res{PQexecParams(_connection,
                              "INSERT INTO users VALUES ($1, $2, $3,$4, $5, $6);",  // query
                              param_num,     // number of params
                              NULL,          // let the backend deduce
                              param_values,  // parameters to insert
                              NULL,          // don't need param lengths since text
                              NULL,          // default to all text params
                              0)};           // ask for text results

        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "INSERT failed: %s", PQerrorMessage(_connection));
            PQclear(res);
            exit_nicely(_connection);
        }
        PQclear(res);

        return true;
    }
    else { return false; }
}

Data_Base::Data_Base(std::string name)
{
    if(name.empty())
        name = "dbname=testing";
    else
        name = "dbname=" + name;
    _connection = PQconnectdb(name.c_str());

    if(PQstatus(_connection) != CONNECTION_OK)
    {
        std::cerr << PQerrorMessage(_connection);
        exit_nicely(_connection);
    }
    _employees.setConnection(_connection);
    auto res{PQexec(_connection,
                    "CREATE TABLE IF NOT EXISTS users ( "
                    " tg_id text, "
                    " firstname text, "
                    " middlename text, "
                    " lastname text, "
                    " car_model text, "
                    " license_plate text );")};

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "FAILED TO CREATE TABLE!: %s", PQerrorMessage(_connection));
        PQclear(res);
        exit_nicely(_connection);
    }
    else { printf("SUCCES WITH TABLE CREATING\n"); }
    PQclear(res);
}

void
Table::setConnection(PGconn* connection)
{
    if(!_connection) _connection = connection;
}
