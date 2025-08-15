#ifndef JSONPARSERUTIL_HH
#define JSONPARSERUTIL_HH

#include <nlohmann/json.hpp>
#include <iostream>

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

        json GetPathAndParseFile(const std::string& file_name, std::string& file_path, bool debug_ = false) const;

        std::string GetFileFromRunSubrun(int run, int subrun, const std::string& topFileName, const std::string& jsonKey) const;

        json ParseFile(const std::string& filename) const;
    };

} // namespace reco

#endif // JSONPARSERUTIL_HH