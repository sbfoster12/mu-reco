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
    return json();
}
