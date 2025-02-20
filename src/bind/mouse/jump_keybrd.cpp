#include "jump_keybrd.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "core/background.hpp"
#include "core/entry.hpp"
#include "core/errlogger.hpp"
#include "core/inputgate.hpp"
#include "core/keycode.hpp"
#include "core/keylayout.hpp"
#include "core/keylog.hpp"
#include "core/ntypelogger.hpp"
#include "core/path.hpp"
#include "core/settable.hpp"
#include "opt/dedicate_to_window.hpp"
#include "opt/optionlist.hpp"
#include "opt/suppress_for_vim.hpp"
#include "opt/uiacachebuild.hpp"
#include "opt/vcmdline.hpp"
#include "util/debug.hpp"
#include "util/def.hpp"
#include "util/screen_metrics.hpp"
#include "util/string.hpp"


namespace vind
{
    namespace bind
    {
        struct JumpWithKeybrdLayout::Impl {
            float max_keybrd_xposs_ = 0 ;
            float max_keybrd_yposs_ = 0 ;

            using KeyPos = std::array<float, 256> ;
            KeyPos xposs_{} ;
            KeyPos yposs_{} ;

            core::Background bg_{opt::all_global_options()} ;

            Impl()
            : max_keybrd_xposs_(0),
              max_keybrd_yposs_(0),
              xposs_(),
              yposs_(),
              bg_(opt::ref_global_options_bynames(
                    opt::AsyncUIACacheBuilder().name(),
                    opt::Dedicate2Window().name(),
                    opt::SuppressForVim().name(),
                    opt::VCmdLine().name()
              ))
            {}
        } ;

        JumpWithKeybrdLayout::JumpWithKeybrdLayout()
        : BindedFuncVoid("jump_cursor_with_keybrd_layout"),
          pimpl(std::make_unique<Impl>())
        {}
        JumpWithKeybrdLayout::~JumpWithKeybrdLayout() noexcept            = default ;
        JumpWithKeybrdLayout::JumpWithKeybrdLayout(JumpWithKeybrdLayout&&)            = default ;
        JumpWithKeybrdLayout& JumpWithKeybrdLayout::operator=(JumpWithKeybrdLayout&&) = default ;

        void JumpWithKeybrdLayout::sprocess() const {
            auto& igate = core::InputGate::get_instance() ;
            //reset key state (binded key)
            core::InstantKeyAbsorber ika ;

            //ignore toggle keys (for example, CapsLock, NumLock, IME....)
            auto toggle_keys = igate.pressed_list() ;

            auto box = util::get_conbined_metrics() ;

            auto width  = box.width() ;
            auto height = box.height() ;

            while(true) {
                pimpl->bg_.update() ;

                if(igate.is_pressed(KEYCODE_ESC)) {
                    return ;
                }

                auto log = igate.pop_log() - toggle_keys ;
                if(log.empty()) continue ;

                try {
                    for(const auto& keycode : log) {
                        if(keycode.is_unreal()) {
                            continue ;
                        }

                        auto x_pos = static_cast<int>( \
                                pimpl->xposs_[keycode.to_code()] / pimpl->max_keybrd_xposs_ * width) ;
                        auto y_pos = static_cast<int>( \
                                pimpl->yposs_[keycode.to_code()] / pimpl->max_keybrd_yposs_ * height) ;

                        auto& settable = core::SetTable::get_instance() ;
                        if(x_pos == width) {
                            x_pos -= settable.get("jump_margin").get<int>() ;
                        }

                        if(y_pos == height) {
                            y_pos -= settable.get("jump_margin").get<int>() ;
                        }

                        SetCursorPos(x_pos, y_pos) ;

                        for(const auto& key : log) {
                            igate.release_keystate(key) ;
                        }
                        return ;
                    }
                }
                catch(const std::out_of_range&) {
                    continue ;
                }
            }
        }
        void JumpWithKeybrdLayout::sprocess(core::NTypeLogger& parent_lgr) const {
            if(!parent_lgr.is_long_pressing()) {
                sprocess() ;
            }
        }
        void JumpWithKeybrdLayout::sprocess(const core::CharLogger& UNUSED(parent_lgr)) const {
            sprocess() ;
        }


        void JumpWithKeybrdLayout::reconstruct() {
            auto& settable = core::SetTable::get_instance() ;
            auto layoutfile = settable.get("keybrd_layout").get<std::string>() ;
            std::filesystem::path filepath ;
            if(!layoutfile.empty()) {
                filepath = core::CONFIG_PATH() / layoutfile ;
            }
            else {
                auto locale_id = GetKeyboardLayout(0) ;
                auto lang_id = static_cast<LANGID>(reinterpret_cast<std::size_t>(locale_id) & 0xffff) ;
                filepath = core::get_keylayout_filepath(lang_id) ;
            }

            if(filepath.empty()) {
                throw RUNTIME_EXCEPT("The file path of keyboard layout is empty.") ;
            }

            pimpl->max_keybrd_xposs_ = 0 ;
            pimpl->max_keybrd_yposs_ = 0 ;

            pimpl->xposs_.fill(0) ;
            pimpl->yposs_.fill(0) ;

            std::ifstream ifs(filepath, std::ios::in) ;
            if(!ifs.is_open()) {
                throw RUNTIME_EXCEPT("Could not open \"" + filepath.u8string() + "\"") ;
            }
            std::string buf ;
            int lnum = 0 ;

            auto ep = [&lnum, &buf, &filepath](auto msg) {
                PRINT_ERROR(buf + msg + "\"" + filepath.u8string() + "\", L" + std::to_string(lnum) + ".") ;
            } ;

            while(getline(ifs, buf)) {
                try {
                    lnum ++ ;

                    if(buf.empty()) {
                        continue ;
                    }

                    //if top character is #, this line is regarded as comment-out.
                    if(buf.front() == '#') {
                        continue ;
                    }

                    auto vec = util::split(buf, " ") ;

                    if(vec.size() != 3) {
                        ep(" is bad syntax in ") ;
                        continue ;
                    }

                    auto x = std::stof(vec[0]) ;
                    auto y = stof(vec[1]) ;

                    if(x > pimpl->max_keybrd_xposs_) pimpl->max_keybrd_xposs_ = x ;
                    if(y > pimpl->max_keybrd_yposs_) pimpl->max_keybrd_yposs_ = y ;

                    //specific code
                    auto code = vec[2] ;
                    //is ascii code
                    if(code.size() == 1) {
                        core::KeyCode keycode(code.front()) ;
                        if(keycode) {
                            //overwrite
                            pimpl->xposs_[keycode.to_code()] = x ;
                            pimpl->yposs_[keycode.to_code()] = y ;
                            continue ;
                        }
                        ep(" is not supported in ") ;
                        continue ;
                    }

                    code = util::A2a(code) ;
                    if(code.front() != '<' && code.back() != '>') {
                        ep(" is invalid syntax in ") ;
                    }

                    code = code.substr(1, code.length() - 2) ;
                    core::KeyCode keycode(code) ;

                    if(keycode) {
                        pimpl->xposs_[keycode.to_code()] = x ;
                        pimpl->yposs_[keycode.to_code()] = y ;
                        continue ;
                    }

                    ep(" is invalid system key code in ") ;
                }
                catch(const std::runtime_error& e) {
                    PRINT_ERROR(e.what()) ;
                    continue ;
                }
            }
        }
    }
}
