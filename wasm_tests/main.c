
#include "common.h"

int runCallback(CallbackTest callback) {
    return callback();
}

int callback42() {
    return 42;
}