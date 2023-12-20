#include "libpq-fe.h"

#include <array>
#include <optional>
#include <string>

class Table
{
private:
    PGconn* const            _connection{};
    std::vector<std::string> _args{};

public:
    Table(PGconn* connection, std::vector<std::string> args = {})
        : _connection{connection},
          _args{args}
    {
    }

    ~Table() {}

    virtual bool
    add() = 0;
    virtual bool
    remove() = 0;
    virtual bool
    contains() = 0;
    virtual std::optional<int>
    count() = 0;
};

class Data_Base
{
private:
    std::string _name{};

public:
    Data_Base(const std::string& name);

    Table&
    employees(std::optional<const std::string> tg_id,
              std::optional<const std::string> first_name,
              std::optional<const std::string> mid_name,
              std::optional<const std::string> last_name,
              std::optional<const std::string> make_and_model_of_car,
              std::optional<const std::string> license_plate);

    Table&
    id_time(std::optional<const std::string&> tg_id, std::optional<const std::string&> time);

    Table&
    licensepl_weekday(std::optional<const std::string&> license_plate,
                      std::optional<const std::string&> weak_day);
};

class Employees_Table : public Table
{
public:
    Employees_Table(PGconn* connection, std::vector<std::string> args = {})
        : Table{connection, args}
    {
    }

    bool
    add() override;
    bool
    remove() override;
    bool
    contains() override;
    std::optional<int>
    count() override;
};
