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
        json ParseFile(const std::string& filename) const;
    };

} // namespace reco

#endif // JSONPARSERUTIL_HH