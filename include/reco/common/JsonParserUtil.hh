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