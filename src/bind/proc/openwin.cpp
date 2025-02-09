#include "openwin.hpp"

#include <windows.h>

#include <psapi.h>

#include "core/charlogger.hpp"
#include "core/ntypelogger.hpp"
#include "core/path.hpp"
#include "util/def.hpp"
#include "util/winwrap.hpp"

namespace vind
{
    namespace bind
    {
        //OpenNewWindow
        OpenNewWindow::OpenNewWindow()
        : BindedFuncVoid("open_new_window")
        {}
        void OpenNewWindow::sprocess() {
            auto hwnd = GetForegroundWindow() ;
            if(!hwnd) {
                throw RUNTIME_EXCEPT("The foreground window is not existed") ;
            }

            DWORD proc_id = 0 ;
            GetWindowThreadProcessId(hwnd, &proc_id) ;

            HANDLE hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, proc_id) ;
            if(!hproc) {
                throw RUNTIME_EXCEPT("OpenProcess Failed.") ;
            }

            HMODULE hmod = NULL ;
            DWORD cbneed = 0 ;
            if(!EnumProcessModules(hproc, &hmod, sizeof(HMODULE), &cbneed)) {
                CloseHandle(hproc) ;
                throw RUNTIME_EXCEPT("Cannot enumerate process modules.") ;
            }

            WCHAR path[MAX_PATH] = {0} ;
            if(!GetModuleFileNameExW(hproc, hmod, path, MAX_PATH)) {
                CloseHandle(hproc) ;
                throw RUNTIME_EXCEPT("Cannot get a process path of current window.") ;
            }
            CloseHandle(hproc) ;

            util::create_process(core::HOME_PATH(), util::ws_to_s(path)) ;
            Sleep(100) ;
        }
        void OpenNewWindow::sprocess(core::NTypeLogger& parent_lgr) {
            if(!parent_lgr.is_long_pressing()) {
                sprocess() ;
            }
        }
        void OpenNewWindow::sprocess(const core::CharLogger& UNUSED(parent_lgr)) {
            sprocess() ;
        }
    }
}
