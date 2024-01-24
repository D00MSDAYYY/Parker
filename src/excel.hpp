#pragma once

#include "data_base.hpp"

#include <OpenXLSX.hpp>
#include <chrono>

std::optional<std::string>
createReport(Data_Base& db, std::chrono::year_month_day ymd);

// D00MSDAYY
// 8888888b.   .d8888b.   .d8888b.  888b     d888  .d8888b.  8888888b.        d8888 Y88b   d88P Y88b   d88P Y88b   d88P 
// 888  "Y88b d88P  Y88b d88P  Y88b 8888b   d8888 d88P  Y88b 888  "Y88b      d88888  Y88b d88P   Y88b d88P   Y88b d88P  
// 888    888 888    888 888    888 88888b.d88888 Y88b.      888    888     d88P888   Y88o88P     Y88o88P     Y88o88P   
// 888    888 888    888 888    888 888Y88888P888  "Y888b.   888    888    d88P 888    Y888P       Y888P       Y888P    
// 888    888 888    888 888    888 888 Y888P 888     "Y88b. 888    888   d88P  888     888         888         888     
// 888    888 888    888 888    888 888  Y8P  888       "888 888    888  d88P   888     888         888         888     
// 888  .d88P Y88b  d88P Y88b  d88P 888   "   888 Y88b  d88P 888  .d88P d8888888888     888         888         888     
// 8888888P"   "Y8888P"   "Y8888P"  888       888  "Y8888P"  8888888P" d88P     888     888         888         888     
                                                                                                                    
 