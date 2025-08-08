#ifndef EVENTSTORE_HH
#define EVENTSTORE_HH

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include <TClonesArray.h>

#include <data_products/common/DataProduct.hh>
#include <data_products/wfd5/WFD5WaveformFit.hh>

namespace reco {

    class EventStore {
    public:
        EventStore() = default;
        ~EventStore() = default;

        // get or create a TClonesArray for a specific reco_label and data_label
        template <typename T>
        TClonesArray* getOrCreate(const std::string& reco_label, const std::string& data_label) {

            // Check that data_label and reco_label have no underscores
            if (data_label.find('_') != std::string::npos || reco_label.find('_') != std::string::npos) {
                throw std::runtime_error("Data label or Reco label cannot contain underscores: " + data_label + ", " + reco_label);
            }
            std::string label = reco_label + "_" + data_label;

            // Check if buffer already exists
            auto it = buffers_.find(label);
            if (it != buffers_.end()) {
                // Check type matches T
                const char* className = it->second->GetClass()->GetName();
                if (std::string(className) != T::Class()->GetName()) {
                    throw std::runtime_error("Type mismatch for label " + label + 
                                            ": requested " + T::Class()->GetName() + 
                                            ", found " + className);
                }
                return it->second;
            }
            // If not, create a new TClonesArray for the type T
            TClonesArray* arr = new TClonesArray(T::Class()->GetName());
            buffers_[label] = arr;
            bufferKeys_.push_back(label);
            std::cout << "-> reco::EventStore: Created TClonesArray for '" << label << "'." << std::endl;
            return arr;
        }

        // Get a collection by name const
        template <typename T>
        TClonesArray* get(const std::string& reco_label, const std::string& data_label) const {
            
            //Check that data_label and reco_label have no underscores
            if (data_label.find('_') != std::string::npos || reco_label.find('_') != std::string::npos) {
                throw std::runtime_error("Data label or Reco label cannot contain underscores: " + data_label + ", " + reco_label);
            }
            std::string label = reco_label + "_" + data_label;
            auto it = buffers_.find(label);
            if (it == buffers_.end()) {
                throw std::runtime_error("Data product not found: " + label);
            }
            return it->second;

        }

        // Store a collection of DataProductPtr into a TClonesArray (used from the unpacker)
        template <typename T>
        void put(const std::string& reco_label, const std::string& data_label, const dataProducts::DataProductPtrCollection& collection) {
            
            //Check that data_label and reco_label have no underscores
            if (data_label.find('_') != std::string::npos || reco_label.find('_') != std::string::npos) {
                throw std::runtime_error("Data label or Reco label cannot contain underscores: " + data_label + ", " + reco_label);
            }

            // Can't check for duplicates anymore b/c we expect the TClonesArray to stick around
            // std::string name = reco_label + "_" + data_label;
            // if (buffers_.count(name)) {
            //     throw std::runtime_error("Data product already exists: " + name);
            // }
    
            auto buffer = getOrCreate<T>(reco_label, data_label);

            // Fill buffer with copies of unpacked objects
            for (const auto& basePtr : collection) {
                auto* derivedPtr = dynamic_cast<T*>(basePtr.get());
                if (!derivedPtr) {
                    throw std::runtime_error("Bad cast to " + std::string(T::Class()->GetName()));
                }
                new ((*buffer)[buffer->GetEntriesFast()]) T(*derivedPtr);
            }
            buffer->Expand(buffer->GetEntriesFast());

        }

        void put_odb(std::shared_ptr<dataProducts::DataProduct> odb) {
            if (odb_) {
                throw std::runtime_error("ODB data product already exists");
            }
            odb_ = odb;
        }

        std::shared_ptr<dataProducts::DataProduct> GetODB() const {
            return odb_;
        }

        const std::unordered_map<std::string, TClonesArray*>& GetBuffers() const {
            return buffers_;
        }

        const std::vector<std::string>& GetBufferKeys() const {
            return bufferKeys_;
        }

        void putHistogram(const std::string& name, std::shared_ptr<TH1> hist) {
            if (histograms_.count(name)) {
                std::cerr << "Warning: Histogram with name " << name << " already exists. Overwriting.\n";
            }
            std::cout << "-> reco::EventStore: Created histogram '" << name << "' in the event store." << std::endl;
            histograms_[name] = std::move(hist);
        }

        void putSplines(const std::string& name, std::shared_ptr<dataProducts::SplineHolder> splines) {
            if (splines_.count(name)) {
                std::cerr << "Warning: Spline holder with name " << name << " already exists. Overwriting.\n";
            }
            std::cout << "-> reco::EventStore: Created spline holder '" << name << "' in the event store." << std::endl;
            splines_[name] = std::move(splines);
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

        const std::map<std::string, std::shared_ptr<dataProducts::SplineHolder>>& GetAllSplines() const {
            return splines_;
        }

        void clear() {
            for (auto& [key, buffer] : buffers_) {
                buffer->Clear("C");
            }
        }

    private:
        std::unordered_map<std::string, TClonesArray*> buffers_; //buffers for the data product collections
        std::vector<std::string> bufferKeys_; //keys for the data products (need to know insertion order for TRefs)
        std::shared_ptr<dataProducts::DataProduct> odb_;  // ODB data product, if any
        std::map<std::string, std::shared_ptr<TH1>> histograms_; //histograms
        std::map<std::string, std::shared_ptr<dataProducts::SplineHolder>> splines_; //splines





    };
} //namespace reco

#endif  // EVENTSTORE_HH
