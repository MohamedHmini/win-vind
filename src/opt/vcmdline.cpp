#include "opt/vcmdline.hpp"

#include <windows.h>

#include <chrono>
#include <memory>
#include <unordered_map>

#include "core/errlogger.hpp"
#include "core/mode.hpp"
#include "core/path.hpp"
#include "core/settable.hpp"
#include "util/display_text_painter.hpp"
#include "util/screen_metrics.hpp"
#include "util/winwrap.hpp"


namespace vind
{
    namespace opt
    {
        struct VCmdLine::Impl {
            util::DisplayTextPainter dtp_{25, FW_MEDIUM, "Consolas"} ;
            int x_ = 0 ;
            int y_ = 0 ;
            int extra_ = 0 ;
            std::chrono::seconds fadeout_time_{} ;
        } ;

        Message VCmdLine::msg_ ;

        VCmdLine::VCmdLine()
        : OptionCreator("vcmdline"),
          pimpl(std::make_unique<Impl>())
        {}
        VCmdLine::~VCmdLine() noexcept {
            try {
                reset() ;
            }
            catch(const std::exception& UNUSED(e)) {
                PRINT_ERROR("Failed to reset the drawing pixels of the virtual command line.") ;
            }
        }
        VCmdLine::VCmdLine(VCmdLine&&)            = default ;
        VCmdLine& VCmdLine::operator=(VCmdLine&&) = default ;


        void VCmdLine::do_enable() const {
            auto& settable = core::SetTable::get_instance() ;

            pimpl->dtp_.set_font(
                    settable.get("cmd_fontsize").get<long>(),
                    settable.get("cmd_fontweight").get<long>(),
                    settable.get("cmd_fontname").get<std::string>()) ;

            pimpl->dtp_.set_text_color(
                    settable.get("cmd_fontcolor").get<std::string>()) ;

            pimpl->dtp_.set_back_color(
                    settable.get("cmd_bgcolor").get<std::string>()) ;

            auto pos = settable.get("cmd_roughpos").get<std::string>() ;
            auto xma = settable.get("cmd_xmargin").get<int>() ;
            auto yma = settable.get("cmd_ymargin").get<int>() ;

            auto rect = util::get_primary_metrics() ;

            auto w = rect.width() ;
            auto h = rect.height() ;

            constexpr auto midxbuf = 256 ;

            std::unordered_map<std::string, std::pair<int, int>> pos_list {
                {"UpperLeft",  {xma,                yma}},
                {"UpperMid",   {w / 2 - midxbuf,    yma}},
                {"UpperRight", {w - xma - midxbuf,  yma}},
                {"MidLeft",    {xma,                h / 2}},
                {"Center",     {w / 2 - midxbuf,    h / 2}},
                {"MidRight",   {w - xma - midxbuf,  h / 2}},
                {"LowerLeft",  {xma,                h - yma}},
                {"LowerMid",   {w / 2 - midxbuf,    h - yma}},
                {"LowerRight", {w - xma - midxbuf,  h - yma}}
            } ;
            try {
                const auto& p = pos_list.at(
                        settable.get("cmd_roughpos").get<std::string>()) ;
                pimpl->x_ = p.first ;
                pimpl->y_ = p.second ;
            }
            catch(const std::out_of_range& e) {
                const auto& p = pos_list.at("LowerMid") ;
                pimpl->x_ = p.first ;
                pimpl->y_ = p.second ;
                PRINT_ERROR(std::string(e.what()) + "in " + core::SETTINGS().u8string() + \
                        ", " + settable.get("cmd_roughpos").get<std::string>() + "is invalid syntax.") ;
            }

            pimpl->extra_ = settable.get("cmd_fontextra").get<int>() ;
            pimpl->fadeout_time_ = std::chrono::seconds(
                    settable.get("cmd_fadeout").get<int>()) ;
        }

        void VCmdLine::do_disable() const {
            reset() ;
        }

        void VCmdLine::refresh() {
            util::refresh_display(NULL) ;
        }

        void VCmdLine::clear() {
            msg_.clear() ;
        }

        void VCmdLine::reset() {
            if(!msg_.empty()) {
                clear() ;
                refresh() ;
            }
        }

        void VCmdLine::do_process() const {
            if(msg_.empty()) {
                return ;
            }
            if(msg_.fadeoutable()) {
                if(msg_.lifetime() > pimpl->fadeout_time_) {
                    reset() ;
                    return ;
                }
            }

            pimpl->dtp_.draw(msg_, pimpl->x_, pimpl->y_, pimpl->extra_) ;
        }
    }
}
