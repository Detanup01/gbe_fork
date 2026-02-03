#pragma once

#if !defined(STUB_EXTRA_DEBUG)
    #if defined(DEBUG) || defined(_DEBUG)
        #define STUB_EXTRA_DEBUG
    #endif
#endif

namespace stubdrm {
    bool patch();
    bool restore();

    void set_cleanup_cb(void (*fn)());
}
