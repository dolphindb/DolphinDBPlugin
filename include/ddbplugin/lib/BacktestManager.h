#pragma once

#include "CoreConcept.h"

namespace ddb {

class BacktestManager {
  protected:
    LocklessFlatHashmap<long long, string> names_;
    Mutex createAndDropLock_;  // create and drop interface are serialized
};

}  // namespace ddb
