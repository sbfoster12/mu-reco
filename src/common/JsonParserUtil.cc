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
        std::cout << "Warning: Could not open file " << filename << std::endl;
        return json();
    }
    json j;
    file >> j;
    return j;
}

json JsonParserUtil::GetPathAndParseFile(const std::string& file_name, std::string& file_path, bool debug_) const {
    // Check if config file exists
    if (file_name.find('/') != std::string::npos) {
        // If not a base name, try using this path directly
        file_path = file_name;
    } else {
        // Check if the file is in the current directory
        if (std::filesystem::exists(file_name)) {
            file_path = file_name;
        } else {
            // Otherwise, prepend the config directory from the environment variable
            file_path = std::string(std::getenv("MU_RECO_PATH")) + "/config/" + file_name;
        }
    }
    return ParseFile(file_path);
}

json JsonParserUtil::GetIOVMatch(const json& iovList, int run, int subrun) const {

    // Loop over each IOV and find the right one
    for (const auto& iovCandidate : iovList) {
        if (!iovCandidate.contains("iov") || !iovCandidate["iov"].is_array() || iovCandidate["iov"].size() != 2) {
            throw std::runtime_error("Each configuration file must contain 'iov' key with an array of two elements");
        }
        int startRun = iovCandidate["iov"][0];
        int endRun = iovCandidate["iov"][1];
        if (run >= startRun && run <= endRun) {
            if (!iovCandidate.contains("file")) {
                throw std::runtime_error("Configuration file must contain 'file' key");
            }
            std::cout << "-> reco::JsonParserUtil: Found configuration file for run: " << run
                    << ", subrun: " << subrun << " -> " << iovCandidate["file"]
                    << " with IOV: [" << startRun << ", " << endRun << "]" << std::endl;
            return iovCandidate;    
        }
    }
    // return an empty json object
    std::cout << "-> reco::JsonParserUtil: No configuration file found for run: " << run
              << ", subrun: " << subrun << std::endl;
    return json();
}


json JsonParserUtil::GetConfigFromIOVList(json config, int run, int subrun, std::string iov_label, bool debug) {
    // Set up the parser
    auto& jsonParserUtil = reco::JsonParserUtil::instance();

    // Check the parameter exists
    if (!config.contains(iov_label)) {
        throw std::runtime_error("Error: Configuration must contain '" + iov_label + "' key");
    }

    // Get the list of configurations per iov
    auto iovConfigListFileName = config.value(iov_label,"");
    std::string iovConfigListFilePath = "";
    auto iovConfigJson = GetPathAndParseFile(iovConfigListFileName, iovConfigListFilePath, debug);
    
    // Check that the list exists
    if (!iovConfigJson.contains(iov_label)) {
        throw std::runtime_error("Error: File " + iovConfigListFilePath + " must contain '" + iov_label + "' key");
    }
    auto iovConfigList = iovConfigJson[iov_label];
    if (!iovConfigList.is_array()) {
        throw std::runtime_error("'" + iov_label + "' key must contain an array");
    }
    
    // Determine the correct configuration based on run and subrun
    auto iovConfigMatch = GetIOVMatch(iovConfigList, run, subrun);
    if (iovConfigMatch.empty()) {
        return json(); // Return an empty json object if no match is found
    }

    // Now get the actual configuration now
    std::string configFileName = iovConfigMatch.value("file","");
    std::string configFilePath = "";
    auto thisConfig = GetPathAndParseFile(configFileName, configFilePath, debug);
    std::cout << "-> reco::JsonParserUtil: Loading configuration from file: " << configFilePath << std::endl;

    return thisConfig;
}
