#ifndef JSONPARSERUTIL_HH
#define JSONPARSERUTIL_HH

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace reco {

    class JsonParserUtil {
    public:
        static JsonParserUtil& instance();  // Return by reference

    private:
        JsonParserUtil() = default;
        JsonParserUtil(const JsonParserUtil&) = delete;
        JsonParserUtil& operator=(const JsonParserUtil&) = delete;
    
    public: 

        json GetPathAndParseFile(const std::string& file_name, bool debug_ = false) const
        {
            std::string file_path_ = "";
            if (file_name.find('/') != std::string::npos) {
                // If not a base name, try using this path directly
                file_path_ = file_name;
            } else {
                // If a base name, prepend the config directory
                file_path_ = std::string(std::getenv("MU_RECO_PATH")) + "/config/" + file_name;
            }
            if (!std::filesystem::exists(file_path_)) {
                throw std::runtime_error("JsonParserUtil: File not found: " + file_path_);
            }
            if (debug_) std::cout << "-> JsonParserUtil: Configuring with file: " << file_path_ << std::endl;
            return ParseFile(file_path_);  // Example usage of JsonParserUtil
        }

        json ParseFile(const std::string& filename) const {
            std::ifstream file(filename);
            if (!file.is_open()) {
                throw std::runtime_error("Could not open JSON file: " + filename);
            }
            json j;
            file >> j;
            return j;
        }
    };

} // namespace reco

#endif // JSONPARSERUTIL_HH