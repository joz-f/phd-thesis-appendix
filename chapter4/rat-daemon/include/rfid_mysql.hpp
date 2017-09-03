#ifndef __RFID_MYSQL_HPP__
#define __RFID_MYSQL_HPP__

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <ctime>

#include "string_utils.hpp"

namespace mysql {

  /// Persistent data that we keep for each MySQL Connection
  struct Data {
    sql::Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::PreparedStatement *pst;
   
    std::time_t hour_difference;
    int prev_ampl;
    int prev_light;
  };
  
  int WriteSensorData(Data * data, std::string raw_data, int noise_level, float noise_scale);
  int WriteRFIDData(Data * data, int num, std::string tag);
  int Connect(Data * data, std::string conn_string, std::string user, std::string password, std::string database);
  void HelloWorld(Data * data);
  void Disconnect(Data * data);

}
#endif
