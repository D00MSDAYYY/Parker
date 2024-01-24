#include "data_base.hpp"

#include "string_trimming.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>

Data_Base::Data_Base(std::string name)
{
    // before this execute in terminal command : "CREATE DATABASE Parker WITH ENCODING 'UTF8'
    // LC_COLLATE='ru_RU.UTF-8' LC_CTYPE='ru_RU.UTF-8' TEMPLATE=template0;"

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
    auto res{PQexec(_connection,
                    " CREATE TABLE IF NOT EXISTS employees ("
                    " tg_id bigint PRIMARY KEY NOT NULL, "
                    " fname text, "
                    " midname text, "
                    " lname text, "
                    " car text, "
                    " license text );")};

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        // std::cerr << std::string{"FAILED TO CREATE employees TABLE: "}
        //                  + PQerrorMessage(_connection);
        PQclear(res);
    }
    PQclear(res);
}

bool
Employees_Table::add(const Query_Args& args)
{
    if(args.tg_id && !exists({.tg_id = *(args.tg_id)}))
    {
        std::string query{};
        query += *(args.tg_id);
        query += (args.fname) ? ", '" + *(args.fname) + "' " : ", NULL ";
        query += (args.midname) ? ", '" + *(args.midname) + "' " : ", NULL ";
        query += (args.lname) ? ", '" + *(args.lname) + "' " : ", NULL ";
        query += (args.car) ? ", '" + *(args.car) + "' " : ", NULL ";
        query += (args.license) ? ", '" + *(args.license) + "' " : ", NULL ";
        query  = "INSERT INTO employees VALUES ( " + query + " ); ";
        // std::cerr << query << std::endl;
        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            // std::cerr << std::string{"INSERT in employees failed: "} +
            // PQerrorMessage(_connection);
            PQclear(res);
            return false;
        }
        _states->add(std::stoll(*(args.tg_id)));

        PQclear(res);
        return true;
    }
    else
        return false;
}

bool
Employees_Table::remove(const Query_Args& args)
{
    std::string query{};

    if(args.tg_id) query += "tg_id = " + *(args.tg_id);
    if(args.fname)
    {
        if(!query.empty()) query += " AND ";
        query += "fname = " + *(args.fname);
    }
    if(args.midname)
    {
        if(!query.empty()) query += " AND ";
        query += "midname = " + *(args.midname);
    }
    if(args.lname)
    {
        if(!query.empty()) query += " AND ";
        query += "lname = " + *(args.lname);
    }
    if(args.car)
    {
        if(!query.empty()) query += " AND ";
        query += "car = " + *(args.car);
    }
    if(args.license)
    {
        if(!query.empty()) query += " AND ";
        query += "license = " + *(args.license);
    }
    query = "DELETE FROM employees WHERE " + query + " RETURNING tg_id;";
    // std::cerr << query << std::endl;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        // std::cerr << std::string{"DELETE in employees failed: "} + PQerrorMessage(_connection);
        PQclear(res);
        return false;
    }
    for(int i{0}; i < PQntuples(res); ++i)
        _states->remove(std::stoi(PQgetvalue(res, i, 0)));
    PQclear(res);

    return true;
}

bool
Employees_Table::update(const Query_Args& args)
{
    if(exists({.tg_id = args.tg_id}))
    {
        std::string query{};

        query += "tg_id = '" + *(args.tg_id) + "'";
        if(args.fname)
        {
            if(!query.empty()) query += " , ";
            query += "fname = '" + *(args.fname) + "'";
        }
        if(args.midname)
        {
            if(!query.empty()) query += " , ";
            query += "midname = '" + *(args.midname) + "'";
        }
        if(args.lname)
        {
            if(!query.empty()) query += " , ";
            query += "lname = '" + *(args.lname) + "'";
        }
        if(args.car)
        {
            if(!query.empty()) query += " , ";
            query += "car = '" + *(args.car) + "'";
        }
        if(args.license)
        {
            if(!query.empty()) query += " , ";
            query += "license = '" + *(args.license) + "'";
        }
        query = "UPDATE employees SET " + query + " WHERE tg_id = " + *(args.tg_id) + "; ";
        // std::cerr << query << std::endl;
        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            // std::cerr << std::string{"UPDATE in employees failed: "} +
            // PQerrorMessage(_connection);
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
Employees_Table::exists(const Query_Args& args)
{
    std::string query{};

    if(args.tg_id) query += "tg_id = " + *(args.tg_id);
    if(args.fname)
    {
        if(!query.empty()) query += " AND ";
        query += "fname = '" + *(args.fname) + "'";
    }
    if(args.midname)
    {
        if(!query.empty()) query += " AND ";
        query += "midname = '" + *(args.midname) + "'";
    }
    if(args.lname)
    {
        if(!query.empty()) query += " AND ";
        query += "lname = '" + *(args.lname) + "'";
    }
    if(args.car)
    {
        if(!query.empty()) query += " AND ";
        query += "car = '" + *(args.car) + "'";
    }
    if(args.license)
    {
        if(!query.empty()) query += " AND ";
        query += "license = '" + *(args.license) + "'";
    }
    query = "SELECT EXISTS(SELECT 1 FROM employees WHERE " + query + " ) AS exists ; ";
    // std::cerr << query;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        // std::cerr << std::string{"EXISTS in employees failed: "} + PQerrorMessage(_connection);
        PQclear(res);
        return false;
    }
    auto res_str{PQgetvalue(res, 0, 0)};
    PQclear(res);
    return (res_str[0] == 't') ? true : false;
}

int
Employees_Table::count(const Query_Args& args)
{
    std::string query{};

    if(args.tg_id) query += "tg_id = '" + *(args.tg_id) + "'";
    if(args.fname)
    {
        if(!query.empty()) query += " , ";
        query += "fname = '" + *(args.fname) + "'";
    }
    if(args.midname)
    {
        if(!query.empty()) query += " , ";
        query += "midname = '" + *(args.midname) + "'";
    }
    if(args.lname)
    {
        if(!query.empty()) query += " , ";
        query += "lname = '" + *(args.lname) + "'";
    }
    if(args.car)
    {
        if(!query.empty()) query += " , ";
        query += "car = '" + *(args.car) + "'";
    }
    if(args.license)
    {
        if(!query.empty()) query += " , ";
        query += "license = '" + *(args.license) + "'";
    }
    query = "SELECT COUNT(*) FROM employees WHERE " + query + " ); ";
    // std::cerr << query << std::endl;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        // std::cerr << std::string{"COUNT in employees failed: "} + PQerrorMessage(_connection);
        PQclear(res);
        return 0;
    }
    auto count_str{PQgetvalue(res, 0, 0)};
    PQclear(res);
    return std::stoi(count_str);
}

std::optional<std::vector<Query_Args>>
Employees_Table::selectWhere(const Query_Args& args)
{
    std::string query{};

    if(args.tg_id) query += "tg_id = " + *(args.tg_id);
    if(args.fname)
    {
        if(!query.empty()) query += " , ";
        query += "fname = '" + *(args.fname) + "'";
    }
    if(args.midname)
    {
        if(!query.empty()) query += " , ";
        query += "midname = '" + *(args.midname) + "'";
    }
    if(args.lname)
    {
        if(!query.empty()) query += " , ";
        query += "lname = '" + *(args.lname) + "'";
    }
    if(args.car)
    {
        if(!query.empty()) query += " , ";
        query += "car = '" + *(args.car) + "'";
    }
    if(args.license)
    {
        if(!query.empty()) query += " , ";
        query += "license = '" + *(args.license) + "'";
    }
    query = "SELECT * FROM employees WHERE " + query + " LIMIT 20; ";
    // std::cerr << query << std::endl;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        // std::cerr << std::string{"SELECT WHERE in employees failed: "}
        //                  + PQerrorMessage(_connection);
        PQclear(res);
        return {};
    }
    std::vector<Query_Args> users;
    for(int i{0}; i < PQntuples(res); ++i)
    {
        std::string tg_id_str{PQgetvalue(res, i, PQfnumber(res, "tg_id"))};
        std::string fname_str{PQgetvalue(res, i, PQfnumber(res, "fname"))};
        std::string midname_str{PQgetvalue(res, i, PQfnumber(res, "midname"))};
        std::string lname_str{PQgetvalue(res, i, PQfnumber(res, "lname"))};
        std::string car_str{PQgetvalue(res, i, PQfnumber(res, "car"))};
        std::string license_str{PQgetvalue(res, i, PQfnumber(res, "license"))};

        Query_Args  res{};  // default initalization with std::nullopt
        if(!tg_id_str.empty()) res.tg_id = tg_id_str;
        if(!fname_str.empty()) res.fname = fname_str;
        if(!midname_str.empty()) res.midname = midname_str;
        if(!lname_str.empty()) res.lname = lname_str;
        if(!car_str.empty()) res.car = car_str;
        if(!license_str.empty()) res.license = license_str;

        users.push_back(std::move(res));
    }
    PQclear(res);

    if(users.empty())
        return {};
    else
        return users;
}

FSM&
States::at(int64_t id)
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
States::remove(int64_t id)
{
    _states.erase(id);
}

void
States::add(int64_t id)
{
    _states.insert({id, FSM::USER_AUTHORIZED});
}

std::chrono::year_month_day
yearMonthDayFromStr(std::string str)
{
    if(str.size() > 9)
    {
        return std::chrono::year_month_day{
            std::chrono::year{std::stoi(std::string{str.begin(), str.begin() + 4})},
            std::chrono::month{unsigned(std::stoi(std::string{str.begin() + 5, str.begin() + 7}))},
            std::chrono::day{unsigned(std::stoi(std::string{str.begin() + 8, str.begin() + 10}))}};
    }
    return {};
}

bool
Datetime_Table::checkWeekday(const std::chrono::year_month_day& date)
{
    std::chrono::year_month_weekday y_m_wd{date};
    if(_allowed_weekdays.contains(y_m_wd.weekday())
       && count({.date = std::format("{:%F}", date)}) < _max_places)
        return true;
    else
        return false;
}

Datetime_Table::Datetime_Table(PGconn* connection)
    : Table{connection}
{
    auto res{PQexec(_connection,
                    " CREATE TABLE IF NOT EXISTS datetime ("
                    " tg_id bigint  NOT NULL, "
                    " date date NOT NULL);")};

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        // std::cerr << std::string{"FAILED TO CREATE datetime TABLE: "} +
        // PQerrorMessage(_connection);
        PQclear(res);
    }
}

void
Datetime_Table::operator=(const Datetime_Table& other)
{
    _max_places.store(other._max_places.load());
    _allowed_weekdays = other._allowed_weekdays;
    _connection       = other._connection;
}

bool
Datetime_Table::add(const Query_Args& args)
{
    if(args.tg_id && args.date && checkWeekday(yearMonthDayFromStr(*(args.date)))
       && !exists({.tg_id = args.tg_id, .date = args.date}))
    {
        std::string query{};
        query += *(args.tg_id);
        query += ",' " + *(args.date) + " '";
        query  = "INSERT INTO datetime VALUES ( " + query + " ); ";
        // std::cerr << query << std::endl;
        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            std::cerr << std::string{"INSERT in datetime failed: "} + PQerrorMessage(_connection);
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
Datetime_Table::remove(const Query_Args& args)
{
    std::string query{};
    if(args.tg_id) query += "tg_id = " + *(args.tg_id);
    if(args.date)
    {
        if(!query.empty()) query += " AND ";
        query += "date = '" + *(args.date) + "'";
    }
    query = "DELETE FROM datetime WHERE " + query + ";";
    // std::cerr << query << std::endl;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        // std::cerr << std::string{"DELETE in datetime failed: "} + PQerrorMessage(_connection);
        PQclear(res);
        return false;
    }
    PQclear(res);
    return true;
}

bool
Datetime_Table::update(const Query_Args& args)
{
    if(exists({.tg_id = args.tg_id}))
    {
        std::string query{};
        query += "tg_id = '" + *(args.tg_id) + "'";
        if(args.date)
        {
            if(!query.empty()) query += " , ";
            query += "date = '" + *(args.date) + "'";
        }
        query = "UPDATE datetime SET " + query + " WHERE tg_id = " + *(args.tg_id) + "; ";
        // std::cerr << query << std::endl;
        auto res{PQexec(_connection, query.c_str())};

        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            // std::cerr << std::string{"UPDATE in datetime failed: "} +
            // PQerrorMessage(_connection);
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
Datetime_Table::exists(const Query_Args& args)
{
    std::string query{};
    if(args.tg_id) query += "tg_id = " + *(args.tg_id);
    if(args.date)
    {
        if(!query.empty()) query += " AND ";
        query += "date = '" + *(args.date) + "'";
    }
    query = "SELECT EXISTS(SELECT 1 FROM datetime WHERE " + query + " ) AS exists ; ";
    // std::cerr << query;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        // std::cerr << std::string{"EXISTS in datetime failed: "} + PQerrorMessage(_connection);
        PQclear(res);
        return false;
    }
    auto res_str{PQgetvalue(res, 0, 0)};
    PQclear(res);
    return (res_str[0] == 't') ? true : false;
}

int
Datetime_Table::count(const Query_Args& args)
{
    std::string query{};

    if(args.tg_id) query += "tg_id = '" + *(args.tg_id) + "'";

    if(args.date)
    {
        if(!query.empty()) query += " , ";

        query += "date = '" + *(args.date) + "'";
    }
    query = "SELECT COUNT(*) FROM datetime WHERE " + query + " ; ";
    // std::cerr << query << std::endl;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        // std::cerr << std::string{"COUNT in datetime failed: "} + PQerrorMessage(_connection);
        PQclear(res);
        return 0;
    }
    auto count_str{PQgetvalue(res, 0, 0)};
    PQclear(res);
    return std::stoi(count_str);
}

std::optional<std::vector<Query_Args>>
Datetime_Table::selectWhere(const Query_Args& args)
{
    std::string query{};
    if(args.tg_id) query += "tg_id = " + *(args.tg_id);
    if(args.date)
    {
        if(!query.empty()) query += " , ";
        query += "date = '" + *(args.date) + "'";
    }
    query = "SELECT * FROM datetime WHERE " + query + " LIMIT 20; ";
    // std::cerr << query << std::endl;
    auto res{PQexec(_connection, query.c_str())};

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        // std::cerr << std::string{"SELECT WHERE in datetime failed: "} +
        // PQerrorMessage(_connection);
        PQclear(res);
        return {};
    }
    std::vector<Query_Args> datetimes;
    for(int i{0}; i < PQntuples(res); ++i)
    {
        std::string tg_id_str{PQgetvalue(res, i, PQfnumber(res, "tg_id"))};
        std::string date_str{PQgetvalue(res, i, PQfnumber(res, "date"))};

        Query_Args  res{};  // default initalization with std::nullopt
        if(!tg_id_str.empty()) res.tg_id = tg_id_str;
        if(!date_str.empty()) res.date = date_str;

        datetimes.push_back(res);
    }
    PQclear(res);

    if(datetimes.empty())
        return {};
    else
        return datetimes;
}

std::optional<int>
Datetime_Table::maxPlaces(std::optional<int> num)
{
    if(num)
    {
        using namespace std::chrono;

        zoned_time         local_tp{current_zone(), system_clock::now()};
        year_month_weekday y_m_wd_local{floor<days>(local_tp.get_local_time())};
        weekday            weekday_local{y_m_wd_local.weekday()};
        bool               is_ok{true};
        for(const auto& allowed_day: _allowed_weekdays)
        {
            auto future_day{local_tp.get_local_time() + (allowed_day - weekday_local)};
            if(count({.date = std::format("{:%F}", year_month_day{floor<days>(future_day)})}) > num)
            {
                is_ok = false;
                break;
            }
        }
        if(is_ok)
        {
            _max_places = *num;
            return _max_places;
        }
        else
            return {};
    }
    return _max_places;
}

Weekdays&
Datetime_Table::allowedWeekdays()
{
    return _allowed_weekdays;
}

std::optional<Query_Args>
parseBioStr(std::string& str)
{
    auto       fname_index{str.find("➡️ Фамилия :")};
    auto       midname_index{str.find("➡️ Имя :")};
    auto       lname_index{str.find("➡️ Отчество :")};
    auto       car_index{str.find("➡️ Марка машины :")};
    auto       license_index{str.find("➡️ Номерной знак :")};

    std::regex name_regex{
        "([АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя-]{1,30})"};

    std::regex car_regex{"([АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя0-9 "
                         "]{1,32})"};

    // std::regex license_regex{"([АВЕКМНОРСТУХавекмнорстух]{2}[0123456789]{6}(?!000)["
    //                          "АВЕКМНОРСТУХавекмнорстух]{4}[0123456789]{4,6})"};

    Query_Args res{};  // default initalization with std::nullopt

    if(fname_index != std::string::npos && midname_index != std::string::npos)
    {
        std::smatch fname_res{};
        std::regex_search({str.begin() + fname_index + strlen("➡️ Фамилия :")},
                          {str.begin() + midname_index},
                          fname_res,
                          name_regex);
        auto fname_str{fname_res.str()};
        if(!fname_str.empty()) res.fname = trim(fname_str);
    }
    if(midname_index != std::string::npos && lname_index != std::string::npos)
    {
        std::smatch midname_res{};
        std::regex_search({str.begin() + midname_index + strlen("➡️ Имя :")},
                          {str.begin() + lname_index},
                          midname_res,
                          name_regex);
        auto midname_str{midname_res.str()};
        if(!midname_str.empty()) res.midname = trim(midname_str);
    }
    if(lname_index != std::string::npos && car_index != std::string::npos)
    {
        std::smatch lname_res{};
        std::regex_search({str.begin() + lname_index + strlen("➡️ Отчество :")},
                          {str.begin() + car_index},
                          lname_res,
                          name_regex);
        auto lname_str{lname_res.str()};
        if(!lname_str.empty()) res.lname = trim(lname_str);
    }
    if(car_index != std::string::npos && license_index != std::string::npos)
    {
        std::smatch car_res{};
        std::regex_search({str.begin() + car_index + strlen("➡️ Марка машины :")},
                          {str.begin() + license_index},
                          car_res,
                          car_regex);
        auto car_str{car_res.str()};
        if(!car_str.empty()) res.car = trim(car_str);
    }
    if(license_index != std::string::npos)
    {
        std::smatch license_res{};
        // std::regex_search({str.begin() + license_index + strlen("➡️ Номерной знак :")},
        //                   {str.end()},
        //                   license_res,
        //                   license_regex);
        std::string lic_str{str.begin() + license_index + strlen("➡️ Номерной знак :"), str.end()};
        if(!lic_str.empty()) res.license = trim(lic_str);
    }
    return res;
}

std::string
toDatetimeStr(std::chrono::time_point<std::chrono::system_clock> time_p)
{
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(time_p)};
    return std::string{std::to_string(static_cast<int>(ymd.year())) + "-"
                       + std::to_string(static_cast<unsigned>(ymd.month())) + "-"
                       + std::to_string(static_cast<unsigned>(ymd.day()))};
}

// D00MSDAYY
// 8888888b.   .d8888b.   .d8888b.  888b     d888  .d8888b.  8888888b.        d8888 Y88b   d88P Y88b   d88P Y88b   d88P 
// 888  "Y88b d88P  Y88b d88P  Y88b 8888b   d8888 d88P  Y88b 888  "Y88b      d88888  Y88b d88P   Y88b d88P   Y88b d88P  
// 888    888 888    888 888    888 88888b.d88888 Y88b.      888    888     d88P888   Y88o88P     Y88o88P     Y88o88P   
// 888    888 888    888 888    888 888Y88888P888  "Y888b.   888    888    d88P 888    Y888P       Y888P       Y888P    
// 888    888 888    888 888    888 888 Y888P 888     "Y88b. 888    888   d88P  888     888         888         888     
// 888    888 888    888 888    888 888  Y8P  888       "888 888    888  d88P   888     888         888         888     
// 888  .d88P Y88b  d88P Y88b  d88P 888   "   888 Y88b  d88P 888  .d88P d8888888888     888         888         888     
// 8888888P"   "Y8888P"   "Y8888P"  888       888  "Y8888P"  8888888P" d88P     888     888         888         888     
 