#include "FSM_states.hpp"
#include "libpq-fe.h"

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <tgbot/tgbot.h>
#include <unordered_map>
#include <utility>

struct Query_Args
{
    std::string tg_id{};
    std::string firstname{};
    std::string middlename{};
    std::string lastname{};
    std::string car_model{};
    std::string license{};
};

class Table  // interface
{
protected:
    PGconn* _connection{nullptr};

public:
    Table(PGconn* connection)
        : _connection{connection}
    {
    }

    virtual ~Table() {}

    virtual bool
    add(Query_Args args) const = 0;
    virtual bool
    remove(Query_Args args) const = 0;
    virtual bool
    update(Query_Args args) const = 0;
    virtual bool
    exists(Query_Args args) const = 0;
    virtual int
    count(Query_Args args) const = 0;
    virtual std::optional<std::vector<Query_Args>>
    select_where(Query_Args args) const = 0;
};

class States;

class Employees_Table : public Table
{
private:
    States* _states{nullptr};

public:
    Employees_Table(PGconn* connection, States* states);

    bool
    add(Query_Args args) const override;
    bool
    remove(Query_Args args) const override;
    virtual bool
    update(Query_Args args) const override;
    bool
    exists(Query_Args args) const override;
    int
    count(Query_Args args) const override;
    std::optional<std::vector<Query_Args>>
    select_where(Query_Args args) const override;
};

class Data_Base;

class States
{
private:
    Data_Base*                                         _db;
    std::unordered_map<decltype(TgBot::User::id), FSM> _states;
    FSM                                                _def_state{FSM::USER_NOT_AUTHORIZED};

public:
    States(Data_Base* db)
        : _db{db}
    {
    }

    FSM&
    at(decltype(TgBot::User::id) id);
    void
    remove(decltype(TgBot::User::id) id);
    void
    add(decltype(TgBot::User::id) id);
};

class Data_Base
{
private:
    PGconn*         _connection{nullptr};

    States          _states{nullptr};
    Employees_Table _employees{nullptr, nullptr};

public:
    Data_Base(std::string name = "");

    Table&
    employees()
    {
        return _employees;
    };

    States&
    states()
    {
        return _states;
    };
};
