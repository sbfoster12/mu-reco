#include <fstream>
#include <sstream>
#include <stdexcept>

#include "reco/common/JsonParserUtil.hh"

using namespace reco;

JsonParserUtil& JsonParserUtil::instance() {
  static JsonParserUtil parser;
  return parser;
}

json JsonParserUtil::ParseFile(const std::string& filename) const {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open JSON file: " + filename);
    }
    json j;
    file >> j;
    return j;
}

 std::string JsonParserUtil::GetFileFromRunSubrun(int run, int subrun, const std::string& topFileName) const {
  // This function retrieves the appropriate configuration file based on the run and subrun numbers
  // It uses the IOV file located at topFileName

  // We need the singleton
  auto& jsonParserUtil = reco::JsonParserUtil::instance();

  // Open the top level file
  std::string topFilePath = "";
  if (topFileName.find('/') != std::string::npos) {
      // If not a base name, try using this path directly
      topFilePath = topFileName;
  } else {
      // If a base name, prepend the config directory
      topFilePath = std::string(std::getenv("MU_RECO_PATH")) + "/config/" + topFileName;
  }
  std::cout << "-> reco::JsonParserUtil: Loading top level file from " << topFilePath << std::endl;
  auto topJsonFile = jsonParserUtil.ParseFile(topFilePath);

  if (!topJsonFile.contains("config_files")) {
    throw std::runtime_error("reco::JsonParserUtil configuration file must contain 'config_files' key");
  }

  auto configFiles = topJsonFile["config_files"];
  if (!configFiles.is_array()) {
      throw std::runtime_error("'config_files' key must contain an array");
  }

  // Loop over each IOV and find the right one
  for (const auto& configFile : configFiles) {
      if (!configFile.contains("iov") || !configFile["iov"].is_array() || configFile["iov"].size() != 2) {
          throw std::runtime_error("Each configuration file must contain 'iov' key with an array of two elements");
          return "";
      }
      int startRun = configFile["iov"][0];
      int endRun = configFile["iov"][1];
      if (run >= startRun && run <= endRun) {
          if (!configFile.contains("file")) {
              throw std::runtime_error("Configuration file must contain 'file' key");
              return "";
          }
          std::cout << "-> reco::JsonParserUtil: Found configuration file for run: " << run
                  << ", subrun: " << subrun << " -> " << configFile["file"]
                  << " with IOV: [" << startRun << ", " << endRun << "]" << std::endl;
          return configFile["file"];
      }
  }

  return "";
}