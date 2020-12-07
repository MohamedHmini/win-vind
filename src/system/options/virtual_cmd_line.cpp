#include "screen_metrics.hpp"
#include "virtual_cmd_line.hpp"

#include <windows.h>
#include <unordered_map>

#include "utility.hpp"
#include "mode_manager.hpp"
#include "path.hpp"
#include "i_params.hpp"

using namespace std ;
std::string VirtualCmdLine::outstr{} ;
std::chrono::system_clock::time_point VirtualCmdLine::msg_start{} ;
bool VirtualCmdLine::msg_showing = false ;

namespace VCLUtility
{
    inline static const auto _hex2COLOREF(string hex) {
        if(hex.front() == '#') {
            hex.erase(0, 1) ;
        }

        unsigned char r = 0 ;
        unsigned char g = 0 ;
        unsigned char b = 0 ;

        if(hex.length() == 6) {
            auto r_hex = hex.substr(0, 2) ;
            auto g_hex = hex.substr(2, 2) ;
            auto b_hex = hex.substr(4, 2) ;
            r = static_cast<unsigned char>(strtol(r_hex.c_str(), nullptr, 16)) ;
            g = static_cast<unsigned char>(strtol(g_hex.c_str(), nullptr, 16)) ;
            b = static_cast<unsigned char>(strtol(b_hex.c_str(), nullptr, 16)) ;
        }

        return RGB(r, g, b) ;
    }
}
using namespace VCLUtility ;

struct VirtualCmdLine::Impl
{
    LOGFONT lf{} ;
    COLORREF color{RGB(0, 0, 0)} ;
    COLORREF bkcolor{RGB(0, 0, 0)} ;
    int x     = 0 ;
    int y     = 0 ;
    int extra = 0 ;

    std::chrono::seconds fadeout_time{} ;
} ;

VirtualCmdLine::VirtualCmdLine()
: pimpl(std::make_unique<Impl>())
{
    //default setting
    pimpl->lf.lfHeight         = 25 ;
    pimpl->lf.lfWidth          = 0 ;
    pimpl->lf.lfEscapement     = 0 ;
    pimpl->lf.lfOrientation    = 0 ;
    pimpl->lf.lfWeight         = FW_MEDIUM ;
    pimpl->lf.lfItalic         = FALSE ;
    pimpl->lf.lfUnderline      = FALSE ;
    pimpl->lf.lfStrikeOut      = FALSE ;
    pimpl->lf.lfCharSet        = ANSI_CHARSET ;
    pimpl->lf.lfOutPrecision   = OUT_TT_ONLY_PRECIS ;
    pimpl->lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS ;
    pimpl->lf.lfQuality        = ANTIALIASED_QUALITY ;
    pimpl->lf.lfPitchAndFamily = 0 ;
    pimpl->lf.lfFaceName[0]    = '\0' ;
}
VirtualCmdLine::~VirtualCmdLine() noexcept                  = default ;
VirtualCmdLine::VirtualCmdLine(VirtualCmdLine&&)            = default ;
VirtualCmdLine& VirtualCmdLine::operator=(VirtualCmdLine&&) = default ;

const string VirtualCmdLine::sname() noexcept
{
    return "virtual_cmd_line" ;
}

void VirtualCmdLine::do_enable() const
{
    reset() ;
    pimpl->lf.lfHeight = iParams::get_l("cmd_font_size") ;
    pimpl->lf.lfWeight = iParams::get_l("cmd_font_weight") ;

    pimpl->color   = _hex2COLOREF(iParams::get_s("cmd_font_color")) ;
    pimpl->bkcolor = _hex2COLOREF(iParams::get_s("cmd_font_bkcolor")) ;

    const auto pos = iParams::get_s("cmd_pos") ;
    const auto xma = iParams::get_i("cmd_xmargin") ;
    const auto yma = iParams::get_i("cmd_ymargin") ;

    static const ScreenMetrics met{} ;

    constexpr auto midxbuf = 256 ;

    const std::unordered_map<std::string, std::pair<int, int>> pos_list {
        {"UpperLeft",  {xma, yma}},
        {"UpperMid",   {met.primary_width() / 2 - midxbuf, yma}},
        {"UpperRight", {met.primary_width() - xma - midxbuf, yma}},
        {"MidLeft",    {xma, met.primary_height() / 2}},
        {"Center",     {met.primary_width() / 2 - midxbuf, met.primary_height() / 2}},
        {"MidRight",   {met.primary_width() - xma - midxbuf, met.primary_height() / 2}},
        {"LowerLeft",  {xma, met.primary_height() - yma}},
        {"LowerMid",   {met.primary_width() / 2 - midxbuf, met.primary_height() - yma}},
        {"LowerRight", {met.primary_width() - xma - midxbuf, met.primary_height() - yma}}
    } ;
    try {
        const auto& p = pos_list.at(iParams::get_s("cmd_pos")) ;
        pimpl->x = p.first ;
        pimpl->y = p.second ;
    }
    catch(const std::out_of_range& e) {
        const auto& p = pos_list.at("LowerMid") ;
        pimpl->x = p.first ;
        pimpl->y = p.second ;
        ERROR_PRINT(std::string(e.what()) + "in " + Path::SETTINGS() + ", " + iParams::get_s("cmd_pos") + "is invalid syntax.") ;
    }

    pimpl->extra = iParams::get_i("cmd_font_extra") ;
    pimpl->fadeout_time = std::chrono::seconds(iParams::get_i("cmd_fadeout_time")) ;
}

void VirtualCmdLine::do_disable() const
{
}

void VirtualCmdLine::cout(std::string&& str) noexcept
{
    outstr = std::move(str) ;
}
void VirtualCmdLine::cout(const std::string& str) noexcept
{
    outstr = str ;
}

void VirtualCmdLine::msgout(std::string str) noexcept
{
    if(str.empty()) return ;
    outstr = std::move(str) ;
    msg_start = std::chrono::system_clock::now() ;
    msg_showing = true ;
}

void VirtualCmdLine::refresh() {
    if(!InvalidateRect(NULL, NULL, TRUE)) {
        throw RUNTIME_EXCEPT(" failed refresh display") ;
    }
}

void VirtualCmdLine::clear() noexcept
{
    outstr.clear() ;
}

void VirtualCmdLine::reset() noexcept
{
    clear() ;
    msg_showing = false ;
    refresh() ;
}

void VirtualCmdLine::do_process() const
{
    if(outstr.empty()) return ;
    if(msg_showing) {
        if(std::chrono::system_clock::now() - msg_start > pimpl->fadeout_time) {
            reset() ;
            return ;
        }
    }

    auto hdc = CreateDCA("DISPLAY", NULL, NULL, NULL) ;
    if(!hdc) {
        throw RUNTIME_EXCEPT("CreateDC") ;
    }

    auto font = CreateFontIndirect(&pimpl->lf) ;
    if(!font) {
        throw RUNTIME_EXCEPT("CreateFontIndirectA") ;
    }

    if(!SelectObject(hdc, font)) {
        throw RUNTIME_EXCEPT("SelectObject") ;
    }

    if(SetBkColor(hdc, pimpl->bkcolor) == CLR_INVALID) {
        throw RUNTIME_EXCEPT("SetBkColor") ;
    }

    if(SetTextColor(hdc, pimpl->color) == CLR_INVALID) {
        throw RUNTIME_EXCEPT("SetTextColor") ;
    }

    if(SetTextCharacterExtra(hdc, pimpl->extra) == static_cast<int>(0x80000000)) {
        throw RUNTIME_EXCEPT("SetTextCharacterExtra") ;
    }

    if(!TextOutA(hdc, pimpl->x, pimpl->y, outstr.c_str(), lstrlenA(outstr.c_str()))) {
        throw RUNTIME_EXCEPT("TextOutA") ;
    }

    if(!DeleteDC(hdc)) {
        throw RUNTIME_EXCEPT("DeleteDC") ;
    }

    if(!DeleteObject(font)) {
        throw RUNTIME_EXCEPT("DeleteObject") ;
    }
}
