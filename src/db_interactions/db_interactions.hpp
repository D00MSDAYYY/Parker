#include "libpq-fe.h"

#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class Table  // interface
{
protected:
    PGconn* _connection{nullptr};

public:
    Table() {}

    ~Table() {}

    void
    setConnection(PGconn* connection);
    virtual bool
    add(std::vector<std::optional<const std::string>> args) const = 0;
    virtual bool
    remove(std::vector<std::optional<const std::string>> args) = 0;
    virtual bool
    contains(std::vector<std::optional<const std::string>> args) = 0;
    virtual int
    count(std::vector<std::optional<const std::string>> args) = 0;
};

class Employees_Table : public Table
{
public:
    Employees_Table() {}

    bool
    add(std::vector<std::optional<const std::string>> args) const override;
    bool
    remove(std::vector<std::optional<const std::string>> args) override{};
    bool
    contains(std::vector<std::optional<const std::string>> args) override{};
    int
    count(std::vector<std::optional<const std::string>> args) override{};
};

class Data_Base
{
private:
    PGconn*         _connection{nullptr};

    Employees_Table _employees{};

public:
    Data_Base(std::string name);

    const Table&
    employees()
    {
        return _employees;
    };

    const Table&
    id_time(){};
    const Table&
    licensepl_weekday(){};
};
