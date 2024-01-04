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
    _datetime  = Datetime_Table{_connection};
}

Employees_Table::Employees_Table(PGconn* connection, States* states)
    : Table{connection},
      _states{states}
{
    // before this execute in terminal command : "CREATE DATABASE Parker WITH ENCODING 'UTF8'
    // LC_COLLATE='ru_RU.UTF-8' LC_CTYPE='ru_RU.UTF-8' TEMPLATE=template0;"
    auto res{PQexec(_connection,
                    " CREATE TABLE IF NOT EXISTS employees ("
                    " tg_id bigint PRIMARY KEY NOT NULL, "
                    " firstname text, "
                    " middlename text, "
                    " lastname text, "
                    " car_model text, "
                    " license text );")};

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        std::cerr << std::string{"FAILED TO CREATE employees TABLE: "}
                         + PQerrorMessage(_connection);
        PQclear(res);
    }
    PQclear(res);
}

bool
Employees_Table::add(Query_Args args) const
{
    if(!args.tg_id.empty() && !exists({.tg_id = args.tg_id}))
    {
        std::string query{};
        query += args.tg_id;
        query += (args.firstname.empty()) ? ", NULL " : ", '" + args.firstname + "' ";
        query += (args.middlename.empty()) ? ", NULL " : ", '" + args.middlename + "' ";
        query += (args.lastname.empty()) ? ", NULL " : ", '" + args.lastname + "' ";
        query += (args.car_model.empty()) ? ", NULL " : ", '" + args.car_model + "' ";
        query += (args.license.empty()) ? ", NULL " : ", '" + args.license + "' ";
        query  = "INSERT INTO employees VALUES ( " + query + " ); ";

        // std::cerr << query << std::endl;

        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            std::cerr << std::string{"INSERT in employees failed: "}
                             + PQerrorMessage(_connection);
            PQclear(res);
            return false;
        }
        _states->add(std::stoll(args.tg_id));

        PQclear(res);

        return true;
    }
    else { return false; }
}

bool
Employees_Table::remove(Query_Args args) const
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
        std::cerr << std::string{"DELETE in employees failed: "} + PQerrorMessage(_connection);
        PQclear(res);
        return false;
    }

    for(int i{0}; i < PQntuples(res); ++i)
        _states->remove(std::stoi(PQgetvalue(res, i, PQfnumber(res, "tg_id"))));
    PQclear(res);

    return true;
}

bool
Employees_Table::update(Query_Args args) const
{
    if(exists({.tg_id = args.tg_id}))
    {
        std::string query{};

        query += "tg_id = '" + args.tg_id + "'";
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
            std::cerr << std::string{"UPDATE in employees failed: "}
                             + PQerrorMessage(_connection);
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
        std::cerr << std::string{"EXISTS in employees failed: "} + PQerrorMessage(_connection);
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

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << std::string{"COUNT in employees failed: "} + PQerrorMessage(_connection);
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
    query = "SELECT * FROM employees WHERE " + query + " LIMIT 20; ";

    std::cerr << query << std::endl;

    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << std::string{"SELECT WHERE in employees failed: "}
                         + PQerrorMessage(_connection);
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
    _states.insert({id, FSM::USER_AUTHORIZED});
}

Datetime_Table::Datetime_Table(PGconn* connection)
    : Table{connection}
{
    auto res{PQexec(_connection,
                    " CREATE TABLE IF NOT EXISTS datetime ("
                    " tg_id bigint PRIMARY KEY REFERENCES employees (tg_id) , "
                    " parking_date date);")};

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        std::cerr << std::string{"FAILED TO CREATE datetime TABLE: "}
                         + PQerrorMessage(_connection);
        PQclear(res);
    }
    else
    {
        PQclear(res);
        occup_places = count({.datetime = toDatetimeStr(std::chrono::system_clock::now()
                                                        + std::chrono::days{1})});
    }
}

void
Datetime_Table::operator=(const Datetime_Table& other)
{
    max_places.store(other.max_places.load());
    occup_places.store(other.occup_places.load());
}

bool
Datetime_Table::add(Query_Args args) const
{
    if(!args.tg_id.empty() && !args.datetime.empty() && occup_places < max_places)
    {
        ++occup_places;
        std::string query{};
        query += args.tg_id;
        query += "' " + args.datetime + " '";
        query  = "INSERT INTO datetime VALUES ( " + query + " ); ";
        // std::cerr << query << std::endl;
        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            std::cerr << std::string{"INSERT in datetime failed: "}
                             + PQerrorMessage(_connection);
            PQclear(res);
            return false;
        }
        PQclear(res);

        return true;
    }
    else
        return false;
}

bool
Datetime_Table::remove(Query_Args args) const
{
    occup_places.store((occup_places > 0) ? --occup_places : 0);

    std::string query{};

    if(!args.tg_id.empty()) query += "tg_id = " + args.tg_id;

    if(!args.datetime.empty())
    {
        if(!query.empty()) query += " AND ";
        query += "parking_date = " + args.datetime;
    }
    query = "DELETE FROM datetime WHERE " + query + ";";
    // std::cerr << query << std::endl;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << std::string{"DELETE in datetime failed: "} + PQerrorMessage(_connection);
        PQclear(res);
        return false;
    }
    PQclear(res);

    return true;
}

bool
Datetime_Table::update(Query_Args args) const
{
    if(exists({.tg_id = args.tg_id}))
    {
        std::string query{};

        query += "tg_id = '" + args.tg_id + "'";

        if(!args.datetime.empty())
        {
            if(!query.empty()) query += " , ";
            query += "parking_date = '" + args.datetime + "'";
        }
        query = "UPDATE datetime SET " + query + " WHERE tg_id = " + args.tg_id + "; ";
        // std::cerr << query << std::endl;
        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            std::cerr << std::string{"UPDATE in datetime failed: "}
                             + PQerrorMessage(_connection);
            PQclear(res);
            return false;
        }
        PQclear(res);
        return true;
    }
    else { return false; }
}

bool
Datetime_Table::exists(Query_Args args) const
{
    std::string query{};

    if(!args.tg_id.empty()) query += "tg_id = " + args.tg_id;

    if(!args.datetime.empty())
    {
        if(!query.empty()) query += " , ";
        query += "parking_date = '" + args.datetime + "'";
    }
    query = "SELECT EXISTS(SELECT 1 FROM datetime WHERE " + query + " ) AS exists ; ";
    // std::cerr << query;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << std::string{"EXISTS in datetime failed: "} + PQerrorMessage(_connection);
        PQclear(res);
        return false;
    }
    auto res_str{PQgetvalue(res, 0, 0)};
    PQclear(res);

    return (res_str[0] == 't') ? true : false;
}

int
Datetime_Table::count(Query_Args args) const
{
    std::string query{};

    if(!args.tg_id.empty()) query += "tg_id = '" + args.tg_id + "'";

    if(!args.datetime.empty())
    {
        if(!query.empty()) query += " , ";
        query += "parking_date = '" + args.datetime + "'";
    }
    query = "SELECT COUNT(*) FROM datetime WHERE " + query + " ; ";
    // std::cerr << query << std::endl;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << std::string{"COUNT in datetime failed: "} + PQerrorMessage(_connection);
        PQclear(res);
        return 0;
    }
    auto count_str{PQgetvalue(res, 0, 0)};
    PQclear(res);
    return std::stoi(count_str);
}

std::optional<std::vector<Query_Args>>
Datetime_Table::select_where(Query_Args args) const
{
    std::string query{};

    if(!args.tg_id.empty()) query += "tg_id = " + args.tg_id;

    if(!args.datetime.empty())
    {
        if(!query.empty()) query += " , ";
        query += "parking_date = '" + args.datetime + "'";
    }
    query = "SELECT * FROM datetime WHERE " + query + " LIMIT 20; ";
    // std::cerr << query << std::endl;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << std::string{"SELECT WHERE in datetime failed: "}
                         + PQerrorMessage(_connection);
        PQclear(res);
        return {};
    }
    std::vector<Query_Args> datetimes;
    for(int i{0}; i < PQntuples(res); ++i)
    {
        datetimes.push_back({.tg_id    = PQgetvalue(res, i, PQfnumber(res, "tg_id")),
                             .datetime = PQgetvalue(res, i, PQfnumber(res, "parking_date"))});
    }
    PQclear(res);

    return datetimes;
}

std::optional<int>
Datetime_Table::maxPlaces(std::optional<int> num)
{
    if(num)
    {
        if(*num >= occup_places)
        {
            max_places = *num;
            return max_places;
        }
        else
            return {};
    }
    else
        return max_places;
}

int
Datetime_Table::curPlaces()
{
    return max_places - occup_places;
}

Query_Args
parseBioStr(std::string& str)
{
    auto firstname_index{str.find("➡️ Фамилия :")};
    auto middlename_index{str.find("➡️ Имя :")};
    auto lastname_index{str.find("➡️ Отчество :")};
    auto car_model_index{str.find("➡️ Марка машины :")};
    auto license_index{str.find("➡️ Номерной знак :")};

    if(firstname_index != std::string::npos && middlename_index != std::string::npos
       && lastname_index != std::string::npos && car_model_index != std::string::npos
       && license_index != std::string::npos)
    {
        std::locale prev_locale{std::locale::global(std::locale("ru_RU.UTF-8"))};
        std::smatch firstname_res{};
        std::smatch middlename_res{};
        std::smatch lastname_res{};
        std::smatch car_model_res{};
        std::smatch license_res{};

        std::regex  name_regex{"([А-ЯЁа-яё]{1,30})"};
        std::regex  car_model_regex{"([А-ЯЁа-яё0-9 ]{1,30})"};
        std::regex license_regex{"([АВЕКМНОРСТУХавекмнорстух]{1}[0-9]{3}(?!000)["
                                 "АВЕКМНОРСТУХавекмнорстух]{2}[0-9]{2,3})"};
        //! dont use flags -- not working

        std::regex_search({str.begin() + firstname_index + strlen("➡️ Фамилия :")},
                          {str.begin() + middlename_index},
                          firstname_res,
                          name_regex);
        std::regex_search({str.begin() + middlename_index + strlen("➡️ Имя :")},
                          {str.begin() + lastname_index},
                          middlename_res,
                          name_regex);
        std::regex_search({str.begin() + lastname_index + strlen("➡️ Отчество :")},
                          {str.begin() + car_model_index},
                          lastname_res,
                          name_regex);
        std::regex_search({str.begin() + car_model_index + strlen("➡️ Марка машины :")},
                          {str.begin() + license_index},
                          car_model_res,
                          car_model_regex);
        std::regex_search({str.begin() + license_index + strlen("➡️ Номерной знак :")},
                          {str.end()},
                          license_res,
                          license_regex);
        std::locale::global(std::locale(prev_locale));

        return {{},
                firstname_res.str(),
                middlename_res.str(),
                lastname_res.str(),
                car_model_res.str(),
                license_res.str()};
    }
    else
        return {};
}

std::string
toDatetimeStr(std::chrono::time_point<std::chrono::system_clock> time_p)
{
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(time_p)};
    return std::string{std::to_string(static_cast<int>(ymd.year())) + "-"
                       + std::to_string(static_cast<unsigned>(ymd.month())) + "-"
                       + std::to_string(static_cast<unsigned>(ymd.day()))};
}
