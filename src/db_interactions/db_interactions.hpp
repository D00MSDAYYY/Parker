#include "libpq-fe.h"

#include <iostream>
#include <memory>
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
};

class Employees_Table : public Table
{
public:
    Employees_Table(PGconn* connection);

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
};

class Data_Base
{
private:
    PGconn*         _connection{nullptr};

    Employees_Table _employees{nullptr};

public:
    Data_Base(std::string name = "");

    Table&
    employees()
    {
        return _employees;
    };

    // auto&
    // licensepl_weekday()
    // {
    //     return _licensepl_weekday;
    // };
};
