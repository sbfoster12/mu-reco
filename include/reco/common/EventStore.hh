#ifndef EVENTSTORE_HH
#define EVENTSTORE_HH

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include <data_products/common/DataProduct.hh>

namespace reco {

    class EventStore {
    public:
        EventStore() = default;
        ~EventStore() = default;

        // Store a collection of typed shared_ptr<T>
        template <typename T>
        void put(const std::string& reco_label, const std::string& data_label, std::vector<std::shared_ptr<T>> collection) {

            //Check that data_label and reco_label have no underscores
            if (data_label.find('_') != std::string::npos || reco_label.find('_') != std::string::npos) {
                throw std::runtime_error("Data label or Reco label cannot contain underscores: " + data_label + ", " + reco_label);
            }

            std::string name = reco_label + "_" + data_label;

            if (store_.count(name)) {
                throw std::runtime_error("Data product already exists: " + name);
            }
            dataProducts::DataProductPtrCollection baseCollection;
            baseCollection.reserve(collection.size());
            for (auto& item : collection) {
                baseCollection.push_back(std::static_pointer_cast<dataProducts::DataProduct>(item));
            }
            store_[name] = std::move(baseCollection);
        }

        // Store a collection of DataProductPtr directly (from your unpacker)
        void put(const std::string& reco_label, const std::string& data_label, const dataProducts::DataProductPtrCollection& collection) {
            
            //Check that data_label and reco_label have no underscores
            if (data_label.find('_') != std::string::npos || reco_label.find('_') != std::string::npos) {
                throw std::runtime_error("Data label or Reco label cannot contain underscores: " + data_label + ", " + reco_label);
            }

            std::string name = reco_label + "_" + data_label;
            if (store_.count(name)) {
                throw std::runtime_error("Data product already exists: " + name);
            }
            store_[name] = collection;  // copy shared_ptr vector
        }

        void put_odb(std::shared_ptr<dataProducts::DataProduct> odb) {
            if (odb_) {
                throw std::runtime_error("ODB data product already exists");
            }
            odb_ = odb;
        }

        template <typename T>
        std::vector<std::shared_ptr<T>> get(const std::string& reco_label, const std::string& data_label) const {
            
            //Check that data_label and reco_label have no underscores
            if (data_label.find('_') != std::string::npos || reco_label.find('_') != std::string::npos) {
                throw std::runtime_error("Data label or Reco label cannot contain underscores: " + data_label + ", " + reco_label);
            }

            std::string name = reco_label + "_" + data_label;
            auto it = store_.find(name);
            if (it == store_.end()) {
                throw std::runtime_error("Data product not found: " + name);
            }
            const auto& baseVec = it->second;

            std::vector<std::shared_ptr<T>> typedVec;
            typedVec.reserve(baseVec.size());

            for (const auto& basePtr : baseVec) {
                auto casted = std::dynamic_pointer_cast<T>(basePtr);
                if (!casted) {
                    throw std::runtime_error("Bad cast for data product: " + name);
                }
                typedVec.push_back(std::move(casted));
            }
            return typedVec;
        }

        std::shared_ptr<dataProducts::DataProduct> GetODB() const {
            return odb_;
        }

        std::map<std::string, dataProducts::DataProductPtrCollection> GetStore() const {
            return store_;
        }

        void putHistogram(const std::string& name, std::shared_ptr<TH1> hist) {
            if (histograms_.count(name)) {
                std::cerr << "Warning: Histogram with name " << name << " already exists. Overwriting.\n";
            }
            histograms_[name] = std::move(hist);
        }

        std::shared_ptr<TH1> GetHistogram(const std::string& name) const {
            auto it = histograms_.find(name);
            if (it == histograms_.end()) {
                throw std::runtime_error("Histogram not found: " + name);
            }
            return it->second;
        }

        const std::map<std::string, std::shared_ptr<TH1>>& GetAllHistograms() const {
            return histograms_;
        }

        void clear() {
            store_.clear();
        }

    private:
        std::map<std::string, dataProducts::DataProductPtrCollection> store_; //data products to store in the tree eventually
        std::shared_ptr<dataProducts::DataProduct> odb_;  // ODB data product, if any
        std::map<std::string, std::shared_ptr<TH1>> histograms_; //histograms

    };
} //namespace reco

#endif  // EVENTSTORE_HH
