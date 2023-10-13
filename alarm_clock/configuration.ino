// struct Config {
//     char cfg_alarmTime[10];
//     int cfg_volume;
// };

// const char *configFilename = "/config.txt"
// Config config;

// // Loads the configuration from a file
// void loadConfiguration(const char *filename, Config &config) {
//     // Open file for reading
//     File file = SD.open(filename);

//     // Allocate a temporary JsonDocument
//     // Don't forget to change the capacity to match your requirements.
//     // Use arduinojson.org/v6/assistant to compute the capacity.
//     StaticJsonDocument<512> doc;

//     // Deserialize the JSON document
//     DeserializationError error = deserializeJson(doc, file);
//     if (error)
//         Serial.println(F("Failed to read file, using default configuration"));

//     // Copy values from the JsonDocument to the Config
//     config.port = doc["cfg_volume"] | 10;
//     strlcpy(config.cfg_alarmTime,            // <- destination
//             doc["cfg_alarmTime"] | "00:00",  // <- source
//             sizeof(config.cfg_alarmTime));   // <- destination's capacity

//     // Close the file (Curiously, File's destructor doesn't close the file)
//     file.close();
// }

// // Saves the configuration to a file
// void saveConfiguration(const char *filename, const Config &config) {
//     // Delete existing file, otherwise the configuration is appended to the file
//     SD.remove(filename);

//     // Open file for writing
//     File file = SD.open(filename, FILE_WRITE);
//     if (!file) {
//         Serial.println(F("Failed to create file"));
//         return;
//     }

//     // Allocate a temporary JsonDocument
//     // Don't forget to change the capacity to match your requirements.
//     // Use arduinojson.org/assistant to compute the capacity.
//     StaticJsonDocument<256> doc;

//     // Set the values in the document
//     doc["hostname"] = config.hostname;
//     doc["port"] = config.port;

//     // Serialize JSON to file
//     if (serializeJson(doc, file) == 0) {
//         Serial.println(F("Failed to write to file"));
//     }

//     // Close the file
//     file.close();
// }