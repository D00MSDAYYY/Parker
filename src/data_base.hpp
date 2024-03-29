﻿#pragma once
#include "FSM_states.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <libpq-fe.h>
#include <optional>
#include <regex>
#include <string>
#include <tgbot/tgbot.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>

struct Query_Args
{
    std::optional<std::string> tg_id{std::nullopt};
    std::optional<std::string> fname{std::nullopt};
    std::optional<std::string> midname{std::nullopt};
    std::optional<std::string> lname{std::nullopt};
    std::optional<std::string> car{std::nullopt};
    std::optional<std::string> license{std::nullopt};
    std::optional<std::string> date{std::nullopt};
};

class Table  // interface
{
protected:
    PGconn* _connection{nullptr};

public:
    Table(PGconn* connection)
        : _connection{connection} {};

    virtual ~Table() {}
};

class States;

std::optional<Query_Args>
parseBioStr(std::string& str);

class Employees_Table : public Table
{
private:
    States* _states{nullptr};

public:
    Employees_Table(PGconn* connection, States* states);

    bool
    add(const Query_Args& args);
    bool
    remove(const Query_Args& args);
    virtual bool
    update(const Query_Args& args);
    bool
    exists(const Query_Args& args);
    int
    count(const Query_Args& args);
    std::optional<std::vector<Query_Args>>
    selectWhere(const Query_Args& args);
};

std::string
toDatetimeStr(std::chrono::time_point<std::chrono::system_clock> time_p);

std::chrono::year_month_day
yearMonthDayFromStr(std::string str);

class Weekdays_Hasher
{
public:
    size_t
    operator()(const std::chrono::weekday& w) const
    {
        return std::hash<unsigned int>{}(w.iso_encoding());
    }
};

class Weekdays_Equal
{
public:
    bool
    operator()(const std::chrono::weekday& w1, const std::chrono::weekday& w2) const
    {
        return w1.iso_encoding() == w2.iso_encoding();
    }
};

typedef std::unordered_set<std::chrono::weekday, Weekdays_Hasher, Weekdays_Equal> Weekdays;

class Datetime_Table : public Table
{
private:
    std::atomic<int> _max_places{0};
    Weekdays         _allowed_weekdays{};
    bool
    checkWeekday(const std::chrono::year_month_day& date);

public:
    Datetime_Table(PGconn* connection);

    void
    operator=(const Datetime_Table& other);
    bool
    add(const Query_Args& args);
    bool
    remove(const Query_Args& args);
    virtual bool
    update(const Query_Args& args);
    bool
    exists(const Query_Args& args);
    int
    count(const Query_Args& args);
    std::optional<std::vector<Query_Args>>
    selectWhere(const Query_Args& args);

    std::optional<int>
    maxPlaces(std::optional<int> num = std::nullopt);
    Weekdays&
    allowedWeekdays();
};

// D00MSDAYY
// 8888888b.   .d8888b.   .d8888b.  888b     d888  .d8888b.  8888888b.        d8888 Y88b   d88P Y88b   d88P Y88b   d88P 
// 888  "Y88b d88P  Y88b d88P  Y88b 8888b   d8888 d88P  Y88b 888  "Y88b      d88888  Y88b d88P   Y88b d88P   Y88b d88P  
// 888    888 888    888 888    888 88888b.d88888 Y88b.      888    888     d88P888   Y88o88P     Y88o88P     Y88o88P   
// 888    888 888    888 888    888 888Y88888P888  "Y888b.   888    888    d88P 888    Y888P       Y888P       Y888P    
// 888    888 888    888 888    888 888 Y888P 888     "Y88b. 888    888   d88P  888     888         888         888     
// 888    888 888    888 888    888 888  Y8P  888       "888 888    888  d88P   888     888         888         888     
// 888  .d88P Y88b  d88P Y88b  d88P 888   "   888 Y88b  d88P 888  .d88P d8888888888     888         888         888     
// 8888888P"   "Y8888P"   "Y8888P"  888       888  "Y8888P"  8888888P" d88P     888     888         888         888     
 

class Data_Base;

class States
{
private:
    Data_Base*                       _db;
    std::unordered_map<int64_t, FSM> _states;
    FSM                              _def_state{FSM::USER_NOT_AUTHORIZED};

public:
    States(Data_Base* db)
        : _db{db} {};

    FSM&
    at(int64_t id);
    void
    remove(int64_t id);
    void
    add(int64_t id);
};

class Data_Base
{
private:
    PGconn*         _connection{nullptr};

    States          _states{nullptr};
    Employees_Table _employees{nullptr, nullptr};
    Datetime_Table  _datetime{nullptr};

public:
    Data_Base(std::string name = "");
    // clang-format off
    ~Data_Base() { PQfinish(_connection);}

    Employees_Table&
    employees(){ return _employees; }
    Datetime_Table&
    datetime(){ return _datetime; }
    States&
    states(){ return _states; }
    // clang-format on
};
