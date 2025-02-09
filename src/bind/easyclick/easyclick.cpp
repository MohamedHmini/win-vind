#include "easyclick.hpp"

#include <future>
#include <memory>

#include "bind/easyclick/disphinter.hpp"
#include "bind/easyclick/easyclickhint.hpp"
#include "bind/easyclick/inputhinter.hpp"
#include "bind/easyclick/uiscanner.hpp"
#include "core/keycodedef.hpp"
#include "core/ntypelogger.hpp"
#include "util/def.hpp"
#include "util/mouse.hpp"
#include "util/winwrap.hpp"

#include "bind/easyclick/easyclickcore.hpp"

namespace vind
{
    namespace bind
    {
        //EasyClickLeft
        struct EasyClickLeft::Impl {
            EasyClickCore core_{} ;
        } ;
        EasyClickLeft::EasyClickLeft()
        : BindedFuncVoid("easy_click_left"),
          pimpl(std::make_unique<Impl>())
        {}
        EasyClickLeft::~EasyClickLeft() noexcept                 = default ;
        EasyClickLeft::EasyClickLeft(EasyClickLeft&&)            = default ;
        EasyClickLeft& EasyClickLeft::operator=(EasyClickLeft&&) = default ;

        void EasyClickLeft::sprocess(unsigned int repeat_num) const {
            auto hwnd = GetForegroundWindow() ;
            if(hwnd == NULL) {
                throw RUNTIME_EXCEPT("Could not find a foreground window.") ;
            }
            pimpl->core_.scan_ui_objects(hwnd) ;
            pimpl->core_.create_matching_loop(KEYCODE_MOUSE_LEFT, repeat_num) ;
        }
        void EasyClickLeft::sprocess(core::NTypeLogger& parent_lgr) const {
            if(!parent_lgr.is_long_pressing()) {
                sprocess(parent_lgr.get_head_num()) ;
            }
        }
        void EasyClickLeft::sprocess(const core::CharLogger& UNUSED(parent_lgr)) const {
            sprocess() ;
        }
        void EasyClickLeft::reconstruct() {
            pimpl->core_.reconstruct() ;
        }


        //EasyClickRight
        struct EasyClickRight::Impl {
            EasyClickCore core_{} ;
        } ;
        EasyClickRight::EasyClickRight()
        : BindedFuncVoid("easy_click_right"),
          pimpl(std::make_unique<Impl>())
        {}
        EasyClickRight::~EasyClickRight() noexcept                  = default ;
        EasyClickRight::EasyClickRight(EasyClickRight&&)            = default ;
        EasyClickRight& EasyClickRight::operator=(EasyClickRight&&) = default ;

        void EasyClickRight::sprocess(unsigned int repeat_num) const {
            auto hwnd = GetForegroundWindow() ;
            if(hwnd == NULL) {
                throw RUNTIME_EXCEPT("Could not find a foreground window.") ;
            }
            pimpl->core_.scan_ui_objects(hwnd) ;
            pimpl->core_.create_matching_loop(KEYCODE_MOUSE_RIGHT, repeat_num) ;
        }
        void EasyClickRight::sprocess(core::NTypeLogger& parent_lgr) const {
            if(!parent_lgr.is_long_pressing()) {
                sprocess(parent_lgr.get_head_num()) ;
            }
        }
        void EasyClickRight::sprocess(const core::CharLogger& UNUSED(parent_lgr)) const {
            sprocess() ;
        }
        void EasyClickRight::reconstruct() {
            pimpl->core_.reconstruct() ;
        }


        //EasyClickMid
        struct EasyClickMid::Impl {
            EasyClickCore core_{} ;
        } ;
        EasyClickMid::EasyClickMid()
        : BindedFuncVoid("easy_click_mid"),
          pimpl(std::make_unique<Impl>())
        {}
        EasyClickMid::~EasyClickMid() noexcept                = default ;
        EasyClickMid::EasyClickMid(EasyClickMid&&)            = default ;
        EasyClickMid& EasyClickMid::operator=(EasyClickMid&&) = default ;

        void EasyClickMid::sprocess(unsigned int repeat_num) const {
            auto hwnd = GetForegroundWindow() ;
            if(hwnd == NULL) {
                throw RUNTIME_EXCEPT("Could not find a foreground window.") ;
            }
            pimpl->core_.scan_ui_objects(hwnd) ;
            pimpl->core_.create_matching_loop(KEYCODE_MOUSE_MID, repeat_num) ;
        }
        void EasyClickMid::sprocess(core::NTypeLogger& parent_lgr) const {
            if(!parent_lgr.is_long_pressing()) {
                sprocess(parent_lgr.get_head_num()) ;
            }
        }
        void EasyClickMid::sprocess(const core::CharLogger& UNUSED(parent_lgr)) const {
            sprocess() ;
        }
        void EasyClickMid::reconstruct() {
            pimpl->core_.reconstruct() ;
        }


        //EasyClickHover
        struct EasyClickHover::Impl {
            EasyClickCore core_{} ;
        } ;
        EasyClickHover::EasyClickHover()
        : BindedFuncVoid("easy_click_hover"),
          pimpl(std::make_unique<Impl>())
        {}
        EasyClickHover::~EasyClickHover() noexcept                  = default ;
        EasyClickHover::EasyClickHover(EasyClickHover&&)            = default ;
        EasyClickHover& EasyClickHover::operator=(EasyClickHover&&) = default ;

        void EasyClickHover::sprocess() const {
            auto hwnd = GetForegroundWindow() ;
            if(hwnd == NULL) {
                throw RUNTIME_EXCEPT("Could not find a foreground window.") ;
            }
            pimpl->core_.scan_ui_objects(hwnd) ;
            pimpl->core_.create_matching_loop() ;
        }
        void EasyClickHover::sprocess(core::NTypeLogger& parent_lgr) const {
            if(!parent_lgr.is_long_pressing()) {
                sprocess() ;
            }
        }
        void EasyClickHover::sprocess(const core::CharLogger& UNUSED(parent_lgr)) const {
            sprocess() ;
        }
        void EasyClickHover::reconstruct() {
            pimpl->core_.reconstruct() ;
        }
    }
}
