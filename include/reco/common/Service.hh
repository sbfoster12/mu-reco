// Service.hh
#ifndef SERVICE_HH
#define SERVICE_HH

#include <TObject.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace reco {

    class Service : public TObject {
    public:
        virtual ~Service() = default;
        virtual void Configure(const nlohmann::json& config) = 0;

        void SetLabel(const std::string& label) { label_ = label; }
        const std::string& GetLabel() const { return label_; }

    private:
        std::string label_;

        ClassDef(Service, 1);
    };
}

#endif
