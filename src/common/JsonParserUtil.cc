#include <fstream>
#include <sstream>
#include <stdexcept>

#include "reco/common/JsonParserUtil.hh"

namespace reco {

JsonParserUtil& JsonParserUtil::instance() {
  static JsonParserUtil parser;
  return parser;
}

} // namespace util
