#include "data_base.hpp"

#include <OpenXLSX.hpp>
#include <chrono>
#include <fstream>
#include <string>

OpenXLSX::XLDocument
createReport(Data_Base& db, std::tm to_time);
