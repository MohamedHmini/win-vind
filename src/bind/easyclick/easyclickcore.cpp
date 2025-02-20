#include "easyclickcore.hpp"

#include <future>
#include <memory>

#include "bind/easyclick/disphinter.hpp"
#include "bind/easyclick/easyclickhint.hpp"
#include "bind/easyclick/inputhinter.hpp"
#include "bind/easyclick/uiscanner.hpp"
#include "bind/saferepeat.hpp"
#include "core/inputgate.hpp"
#include "core/keycode.hpp"
#include "core/keycodedef.hpp"
#include "core/ntypelogger.hpp"
#include "core/settable.hpp"
#include "opt/uiacachebuild.hpp"
#include "util/container.hpp"
#include "util/debug.hpp"
#include "util/def.hpp"
#include "util/mouse.hpp"
#include "util/rect.hpp"
#include "util/winwrap.hpp"


namespace
{
    using namespace vind ;

    struct ProcessScanInfo {
        DWORD pid ;
        std::vector<util::Point2D>& points ;
    } ;

    BOOL CALLBACK ScanCenterPoint(HWND hwnd, LPARAM lparam) {
        auto obj_list = reinterpret_cast<std::vector<util::Point2D>*>(lparam) ;

        if(!IsWindowVisible(hwnd)) {
            return TRUE ;
        }
        if(!IsWindowEnabled(hwnd)) {
            return TRUE ;
        }

        //also register the position of the other thread window's title bar.
        RECT rect ;
        if(!GetWindowRect(hwnd, &rect)) {
            return TRUE ;
        }

        if(util::width(rect) == 0 || util::height(rect) == 0) {
            return TRUE ;
        }
        if(rect.left < 0 || rect.top < 0 || rect.right < 0 || rect.bottom < 0) {
            return TRUE ;
        }

        obj_list->emplace_back(
                util::center_x(rect),
                util::center_y(rect)) ;
        return TRUE ;
    }

    BOOL CALLBACK EnumerateAllThread(HWND hwnd, LPARAM lparam) {
        auto psinfo = reinterpret_cast<ProcessScanInfo*>(lparam) ;

        DWORD procid ;
        auto thid = GetWindowThreadProcessId(hwnd, &procid) ;

        if(procid == psinfo->pid) {
            //enumerate all threads owned by the parent process.
            EnumThreadWindows(
                    thid, ScanCenterPoint,
                    reinterpret_cast<LPARAM>(&(psinfo->points))) ;
        }

        return TRUE ;
    }
}


namespace vind
{
    namespace bind
    {
        struct EasyClickCore::Impl {
            UIScanner scanner_{} ;
            std::vector<util::SmartElement> elements_{} ;
            std::vector<util::Point2D> positions_{} ;
            std::vector<Hint> hints_{} ;
            std::vector<std::string> strhints_{} ;
            InputHinter input_hinter_{} ;
            DisplayHinter display_hinter_{} ;
        } ;

        EasyClickCore::EasyClickCore()
        : pimpl(std::make_unique<Impl>())
        {
            pimpl->positions_.reserve(2048) ;
            pimpl->elements_.reserve(2048) ;
            pimpl->hints_.reserve(2048) ;
            pimpl->strhints_.reserve(2048) ;

            opt::AsyncUIACacheBuilder::register_properties(
                    pimpl->scanner_.get_properties()) ;
        }

        EasyClickCore::~EasyClickCore() noexcept = default ;

        EasyClickCore::EasyClickCore(EasyClickCore&&)            = default ;
        EasyClickCore& EasyClickCore::operator=(EasyClickCore&&) = default ;

        void EasyClickCore::scan_ui_objects(HWND hwnd) const {
            pimpl->hints_.clear() ;
            pimpl->elements_.clear() ;
            pimpl->positions_.clear() ;
            pimpl->strhints_.clear() ;

            RECT root_rect ;
            if(!GetWindowRect(hwnd, &root_rect)) {
                throw RUNTIME_EXCEPT("Could not get a rectangle of the root window.") ;
            }

            auto& settable = core::SetTable::get_instance() ;
            if(settable.get("uiacachebuild").get<bool>()) {
                auto root_elem = opt::AsyncUIACacheBuilder::get_root_element(hwnd) ;
                pimpl->scanner_.scan(root_elem, pimpl->elements_) ;
            }
            else {
                pimpl->scanner_.scan(hwnd, pimpl->elements_) ;
            }

            for(auto& elem : pimpl->elements_) {
                RECT rect ;
                if(util::is_failed(elem->get_CachedBoundingRectangle(&rect))) {
                    throw RUNTIME_EXCEPT("Could not get a rectangle of a element.") ;
                }

                if(util::is_fully_in_range(rect, root_rect)) {
                    pimpl->positions_.emplace_back(
                            util::center_x(rect),
                            util::center_y(rect)) ;
                }
            }

            // enumerate all window owned by the foreground window process.
            DWORD procid ;
            if(GetWindowThreadProcessId(hwnd, &procid)) {
                ProcessScanInfo psinfo{procid, pimpl->positions_} ;
                if(!EnumWindows(EnumerateAllThread, reinterpret_cast<LPARAM>(&psinfo))) {
                    throw RUNTIME_EXCEPT("Failed to scan for threads in the same process.") ;
                }
            }

            if(pimpl->positions_.empty()) {
                return ;
            }

            util::remove_deplication(pimpl->positions_) ;

            assign_identifier_hints(pimpl->positions_.size(), pimpl->hints_) ;
            convert_hints_to_strings(pimpl->hints_, pimpl->strhints_) ;
        }

        void EasyClickCore::create_matching_loop(
                core::KeyCode sendkey,
                unsigned int repeat_num) const {
            if(pimpl->positions_.empty() || pimpl->hints_.empty()) {
                return ;
            }

            auto ft = pimpl->input_hinter_.launch_async_loop(
                    pimpl->positions_,
                    pimpl->hints_) ;

            using namespace std::chrono ;

            auto prev_draw_num = pimpl->input_hinter_.drawable_hints_num() ;
            while(ft.wait_for(50ms) == std::future_status::timeout) {
                try {
                    auto draw_num = pimpl->input_hinter_.drawable_hints_num() ;

                    if(draw_num == pimpl->hints_.size()) {
                        // Hints were not matched yet, so must draw all hints.
                        pimpl->display_hinter_.paint_all_hints(
                                pimpl->positions_,
                                pimpl->strhints_) ;
                    }
                    else {
                        pimpl->display_hinter_.paint_matching_hints(
                                pimpl->positions_,
                                pimpl->strhints_,
                                pimpl->input_hinter_.matched_counts()) ;

                        if(prev_draw_num != draw_num) {
                            util::refresh_display(NULL) ;
                        }
                    }

                    prev_draw_num = draw_num ;
                }
                catch(const std::exception& e) {
                    pimpl->input_hinter_.cancel() ;
                    throw e ;
                }
            }

            util::refresh_display(NULL) ;

            if(auto pos = ft.get()) {
                if(SetCursorPos(pos->x(), pos->y())) {
                    safe_for(repeat_num, [&sendkey] {
                        util::click(sendkey) ;
                    }) ;
                }
            }

            //Release all keys in order to avoid the next matching right after.
            auto& igate = core::InputGate::get_instance() ;
            for(const auto& key : igate.pressed_list()) {
                igate.release_virtually(key) ;
            }
        }

        void EasyClickCore::reconstruct() {
            pimpl->display_hinter_.load_config() ;
        }
    }
}
