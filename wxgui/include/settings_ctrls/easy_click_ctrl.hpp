#ifndef _EASY_CLICK_CTRL_HPP
#define _EASY_CLICK_CTRL_HPP
#include "ctrl_core.hpp"
#include <memory>

namespace wxGUI
{
    class EasyClcikCtrl : public CtrlCore {
    private:
        struct Impl ;
        std::unique_ptr<Impl> pimpl ;

        void translate() override ;
        void do_load_config() override ;
        void do_load_config_default() override ;
        void do_save_config() override ;
    public:
        explicit EasyClcikCtrl(wxWindow* parent, wxWindowID id=wxID_ANY) ;

        virtual ~EasyClcikCtrl() noexcept ;
        EasyClcikCtrl(EasyClcikCtrl&&)            = delete ;
        EasyClcikCtrl& operator=(EasyClcikCtrl&&) = delete ;
        EasyClcikCtrl(const EasyClcikCtrl&)       = delete ;
        EasyClcikCtrl& operator=(EasyClcikCtrl&)  = delete ;

        const wxString name() override ;
    } ;
}

#endif
