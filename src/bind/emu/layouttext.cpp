#include "layouttext.hpp"

#include "bind/saferepeat.hpp"
#include "core/inputgate.hpp"
#include "core/mode.hpp"
#include "core/ntypelogger.hpp"
#include "util/def.hpp"
#include "util/keystroke_repeater.hpp"


namespace vind
{
    namespace bind
    {
        struct JoinNextLine::Impl {
            util::KeyStrokeRepeater ksr{} ;
        } ;

        JoinNextLine::JoinNextLine()
        : ChangeBaseCreator("join_next_line"),
          pimpl(std::make_unique<Impl>())
        {}

        JoinNextLine::~JoinNextLine() noexcept                    = default ;
        JoinNextLine::JoinNextLine(JoinNextLine&&)               = default ;
        JoinNextLine& JoinNextLine::operator=(JoinNextLine&&)    = default ;

        void JoinNextLine::sprocess(unsigned int repeat_num) const {
            safe_for(repeat_num, [] {
                auto& igate = core::InputGate::get_instance() ;
                igate.pushup(KEYCODE_END) ;
                igate.pushup(KEYCODE_DELETE) ;
            }) ;
        }
        void JoinNextLine::sprocess(core::NTypeLogger& parent_lgr) const {
            if(!parent_lgr.is_long_pressing()) {
                sprocess(parent_lgr.get_head_num()) ;
                pimpl->ksr.reset() ;
            }
            else if(pimpl->ksr.is_passed()) {
                sprocess(1) ;
            }
        }
        void JoinNextLine::sprocess(const core::CharLogger& UNUSED(parent_lgr)) const {
            sprocess(1) ;
        }
    }
}
