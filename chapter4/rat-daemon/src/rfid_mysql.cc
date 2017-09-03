/**
 * RFID MySQL functions for molerat.
 * http://stackoverflow.com/questions/16424828/how-to-connect-mysql-database-using-c#16424893
 * https://dev.mysql.com/doc/connector-cpp/en/connector-cpp-examples-complete-example-1.html
 */


#include "rfid_mysql.hpp"

using namespace std;

namespace mysql {

/**
 * Connect to our local database
 * @param data - our data structure for the mysql data
 * @param conn_string - the mysql connection string
 * @param user - the username
 * @param password - the users password
 * @param database - the database we will use
 * returns an int with the result
 */

int Connect(Data* data, std::string conn_string, std::string user, std::string password, std::string database){ 
  // Create pointer to our local mysql state on the heap
  if (data == nullptr){
    return -1;
  } 
  try {
    data->driver = get_driver_instance(); 
    data->con = data->driver->connect(conn_string.c_str(), user.c_str(), password.c_str());
    data->con->setSchema(database.c_str()); 
  }
  catch (sql::SQLException &e) {
    cerr << "# ERR: SQLException in " << __FILE__;
    cerr << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
    cerr << "# ERR: " << e.what();
    cerr << " (MySQL error code: " << e.getErrorCode();
    cerr << ", SQLState: " << e.getSQLState() << " )" << endl;
    return -1;
  }
  return 0;
}


/**
 * Write RFID sensor to our connected data
 * @param data - our data structure for the mysql data
 * @param num - the sensor number
 * @param tag - the the tag id
 * returns an int with the result
 */

int WriteRFIDData(Data * data, int num, string tag){
  int res = 0;
  try {

    string st = "INSERT INTO nmr_data (tag_id, r_id, time) VALUES (?, ?, CURRENT_TIMESTAMP(2)) ;";   
    data->pst = data->con->prepareStatement(st.c_str());
    data->pst->setString(1,tag);
    data->pst->setInt(2,num);
    res = data->pst->executeUpdate();

  } catch (sql::SQLException &e){
    cerr << "# ERR: SQLException in " << __FILE__;
    cerr << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
    cerr << "# ERR: " << e.what();
    cerr << " (MySQL error code: " << e.getErrorCode();
    cerr << ", SQLState: " << e.getSQLState() << " )" << endl;
    delete data->pst;
    return -1;
  }
  delete data->pst;
  return res;
}

/**
 * Write sensor data internally to our connected data
 * @param data - our data structure for the mysql data
 * @param sensor - the sensor type
 * @param value - the value
 * returns an int with the result
 */

int _WriteSensorData(Data * data, string sensor, int value) {
  int res = 0;
  try {
    string st = "INSERT INTO nmr_env (id, timestamp, sensor, value) VALUES (NULL, CURRENT_TIMESTAMP(2), ?, ?) ;";   
    data->pst = data->con->prepareStatement(st.c_str());
    data->pst->setString(1,sensor.c_str());
    data->pst->setInt(2,value);
    res = data->pst->executeUpdate();

  } catch (sql::SQLException &e){
    cerr << "# ERR: SQLException in " << __FILE__;
    cerr << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
    cerr << "# ERR: " << e.what();
    cerr << " (MySQL error code: " << e.getErrorCode();
    cerr << ", SQLState: " << e.getSQLState() << " )" << endl;
    delete data->pst;
    return -1;
  }
  delete data->pst;
  return res;

}

/**
 * Write RFID sensor to our connected data
 * @param data - our data structure for the mysql data
 * @param raw_data - the raw data off the arduino
 * returns an int with the result
 */

int WriteSensorData(Data * data, string raw_data, int noise_level, float noise_scale){
  int res0 = 0, res1 = 0, res2 = 0, res3 = 0;
  
  int value;
  string sensor;

  // Parse the input data
  vector<string> parsed = s9::SplitStringChars(raw_data,",");
  if (parsed.size() == 6){
    int light_active = s9::FromString<int>(parsed[0]); 
    int ldr_value = s9::FromString<int>(parsed[1]);
    float humidity = s9::FromString<float>(parsed[3]);
    float temperature = s9::FromString<float>(parsed[2]);
    int peak_to_peak = s9::FromString<int>(parsed[4]);
    int volts = s9::FromString<int>(parsed[5]);
 
    // Have we had an hour gap and is it close to the hour?
    std::time_t now = std::time(nullptr);
    if (now - data->hour_difference  >= 60) {
      if (abs(now % 3600 - 3600) < 10 || now % 3600 < 10) {
        data->hour_difference = now;
        res0 = _WriteSensorData(data, "TEMP", temperature);
        res1 = _WriteSensorData(data, "HUM", humidity);
      }
    }
    
    // Other sensor data is passed given the poll rate or previous value
    if (data->prev_light != light_active) {
      res2 = _WriteSensorData(data, "LIGHT", light_active);
      data->prev_light = light_active;
    }    

    float tp = (static_cast<float>(data->prev_ampl) + static_cast<float>(peak_to_peak )) * noise_scale;
    float dd = fabs(static_cast<float>(data->prev_ampl) - static_cast<float>(peak_to_peak ));    

    if (dd  > tp  && dd > noise_level ) { 
      res3 = _WriteSensorData(data, "AMPL", peak_to_peak);
      data->prev_ampl = peak_to_peak;
    }
    
    if (res0 < 0 || res1 < 0 || res2 < 0 || res3 < 0){
      return -1; 
    }
  }
  return 0;
}

/**
 * HelloWorld test DB connection function
 * @param data - our data structure for the mysql data
 * returns an int with the result
 */

void HelloWorld(Data * data) {
  data->stmt = data->con->createStatement();
  data->res = data->stmt->executeQuery("SELECT 'Hello World!' AS _message");
  while (data->res->next()) {
    cout << "Testing MySQL Connection" << endl;
    cout << "\t... MySQL replies: ";
    /* Access column data by alias or column name */
    cout << data->res->getString("_message") << endl;
    cout << "\t... MySQL says it again: ";
    /* Access column data by numeric offset, 1 is the first column */
    cout << data->res->getString(1) << endl;
  }
  delete data->stmt;
}

void Disconnect(Data* data) {
  if (data != nullptr){
    delete data;
  }
}
  
}
