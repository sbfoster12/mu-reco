#ifndef CONFIGHOLDER_HH
#define CONFIGHOLDER_HH

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace reco {

    class ConfigHolder {
    public:
        ConfigHolder() = default;

        // Load JSON config from a file
        void LoadFromFile(const std::string& filename) {
            std::ifstream ifs(filename);
            if (!ifs.is_open()) {
                throw std::runtime_error("Cannot open config file: " + filename);
            }
            ifs >> config_;
        }

        // Access the whole JSON config
        const json& GetConfig() const {
            return config_;
        }

        // Get a sub-config for a component, e.g. "JitterCorrector"
        json GetSubConfig(const std::string& key) const {
            if (config_.contains(key)) {
                return config_.at(key);
            }
            return json::object();  // return empty object if not found
        }

    private:
        json config_;
    };
} //namespace reco

#endif  // CONFIGHOLDER_HH
