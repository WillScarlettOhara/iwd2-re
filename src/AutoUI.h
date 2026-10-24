#ifndef AUTOUI_H_
#define AUTOUI_H_

// TEST SCAFFOLDING -- not a mirror of IWD2.exe.
//
// Same shape as AutoLoad.h: an isolated namespace, inert unless an environment
// variable points it at a script. It adds
// no game behaviour; it only replays the input the engine already accepts.

class CWarp;

namespace Iwd2AutoUI {

// True only when IWD2_RE_UI_SCRIPT names a readable script. Everything else in
// this module is unreachable when this returns false.
bool IsEnabled();

// A test action borrows live UI/stats and can lock the back surface. Finish an
// in-flight frame first, and let the main thread skip frames while it runs so
// that thread can still process window messages. Inert outside AutoUI runs.
class SynchronousUpdateGuard {
public:
    SynchronousUpdateGuard();
    ~SynchronousUpdateGuard();
    explicit operator bool() const { return m_acquired; }
    SynchronousUpdateGuard(const SynchronousUpdateGuard&) = delete;
    SynchronousUpdateGuard& operator=(const SynchronousUpdateGuard&) = delete;
private:
    bool m_enabled;
    bool m_acquired;
};

// One action per call, from the tail of CChitin::AsynchronousUpdate. Never
// waits for a scenario condition: such steps return and retry next tick.
void Tick(CWarp* pActiveEngine);

}

#endif
