#include "excel.hpp"

std::optional<std::string>
createReport(Data_Base& db, std::chrono::year_month_day ymd)
{
    auto entities{db.datetime().selectWhere({.date = std::format("{:%F}", ymd)})};
    if(entities)
    {
        using namespace OpenXLSX;
        XLDocument doc{};
        auto       date_str{std::format("{:%F}", ymd)};

        doc.create("./parker_" + date_str + ".xlsx");
        auto wks{doc.workbook().worksheet("Sheet1")};
        wks.setName(date_str);
        wks.cell(XLCellReference("A1")).value() = "Фамилия";
        wks.cell(XLCellReference("B1")).value() = "Имя";
        wks.cell(XLCellReference("C1")).value() = "Отчество";
        wks.cell(XLCellReference("D1")).value() = "Марка машины";
        wks.cell(XLCellReference("E1")).value() = "Номер машины";
        wks.cell(XLCellReference("F1")).value() = std::format("{:%F}", ymd);
        std::vector<XLCellValue> columns;
        for(int i_entity{0}; auto& row: wks.rows(2, entities->size() + 1))
        {
            columns.clear();
            auto info{db.employees().selectWhere({.tg_id = (*entities)[i_entity].tg_id})};
            if(info)
            {
                columns.push_back((((*info)[0].fname)) ? *(*info)[0].fname : "");
                columns.push_back((((*info)[0].midname)) ? *(*info)[0].midname : "");
                columns.push_back((((*info)[0].lname)) ? *(*info)[0].lname : "");
                columns.push_back((((*info)[0].car)) ? *(*info)[0].car : "");
                columns.push_back((((*info)[0].license)) ? *(*info)[0].license : "");
                row.values() = columns;
            }
            ++i_entity;
        }
        doc.save();
        auto path{doc.path()};
        doc.close();
        return path;
    }
    else
        return {};
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
 