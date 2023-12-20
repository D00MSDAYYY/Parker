#include "db_interactions.hpp"

bool
Employees_Table::add()
{
    if(args.empty())
        return false;
    else
    {
        for(auto& arg: _args)
            if(arg.empty()) return false;  //! check for empty string
        auto res = PQexecParams(_connection, "INSERT INTO users VALUES ('$1','$2','$3');", );
    }
    return false;
}
