#include "bind/emu/dot.hpp"

#include "bind/bindedfunc.hpp"
#include "core/ntypelogger.hpp"


namespace vind
{
    namespace bind
    {
        const BindedFunc* RepeatLastChange::lastchange_ = nullptr ;
        unsigned int RepeatLastChange::repeat_count_ = 1 ;

        RepeatLastChange::RepeatLastChange()
        : BindedFuncFlex("repeat_last_change")
        {}

        //
        // NOTE:
        // In the original Vim, "last change" include from the moment enter insert mode
        // to the moment exit insert mode. However win-vind is not capturing in insert mode yet,
        // so we repeat the last change of only commands to edit.
        // Recording in insert mode will be a future work due to load concerns.
        //
        SystemCall RepeatLastChange::sprocess() {
            if(!lastchange_) {
                return SystemCall::NOTHING ;
            }
            return lastchange_->process() ;
        }

        SystemCall RepeatLastChange::sprocess(core::NTypeLogger& parent_lgr) {
            if(!lastchange_) {
                return SystemCall::NOTHING ;
            }
            if(repeat_count_ > 1 && parent_lgr.get_head_num() == 1) {
                parent_lgr.set_head_num(repeat_count_) ;
            }
            return lastchange_->process(parent_lgr) ;
        }

        SystemCall RepeatLastChange::sprocess(const core::CharLogger& parent_lgr) {
            if(!lastchange_) {
                return SystemCall::NOTHING ;
            }
            return lastchange_->process(parent_lgr) ;
        }
    }
}
