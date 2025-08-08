#include "reco/common/ConfigHolder.hh"

using namespace reco;


// Load JSON config from a file
void ConfigHolder::LoadFromFile(const std::string& filename) {

    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        throw std::runtime_error("Cannot open config file: " + filename);
    }
    ifs >> config_;

    std::cout << "-> reco::ConfigHolder: Loaded config from file: " << filename << std::endl;
}