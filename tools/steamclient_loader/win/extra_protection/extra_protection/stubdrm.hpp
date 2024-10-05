#pragma once

namespace stubdrm
{
    bool patch();
    bool restore();

    void set_cleanup_cb(void (*fn)());
}
