#include "libpq-fe.h"

#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

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
    add(std::vector<std::string> args) const = 0;
    virtual bool
    remove(std::vector<std::string> args) const = 0;
    virtual bool
    update(std::vector<std::string> args) const = 0;
    virtual bool
    exists(std::vector<std::string> args) const = 0;
    virtual int
    count(std::vector<std::string> args) const = 0;
};

class Employees_Table : public Table
{
public:
    Employees_Table(PGconn* connection);

    bool
    add(std::vector<std::string> args) const override;
    bool
    remove(std::vector<std::string> args) const override;
    virtual bool
    update(std::vector<std::string> args) const override;
    bool
    exists(std::vector<std::string> args) const override;
    int
    count(std::vector<std::string> args) const override;
};

class LicensePl_Weekday_Table : public Table
{
public:
    LicensePl_Weekday_Table(PGconn* connection)
        : Table{connection}
    {
    }

    bool
    add(std::vector<std::string> args) const override;
    bool
    remove(std::vector<std::string> args) const override;
    bool
    exists(std::vector<std::string> args) const override;
    int
    count(std::vector<std::string> args) const override;
};

class Data_Base
{
private:
    PGconn*                          _connection{nullptr};

    std::unique_ptr<Employees_Table> _employees{nullptr};
    std::unique_ptr<Employees_Table> _licensepl_weekday{nullptr};

public:
    Data_Base(std::string name = "");

    auto&
    employees()
    {
        return _employees;
    };

    auto&
    licensepl_weekday()
    {
        return _licensepl_weekday;
    };
};
