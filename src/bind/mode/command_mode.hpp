#ifndef _COMMAND_MODE_HPP
#define _COMMAND_MODE_HPP

#include "bind/bindedfunc.hpp"

namespace vind
{
    namespace bind
    {
        class ToCommand : public BindedFuncFlex<ToCommand> {
        private:
            struct Impl ;
            std::unique_ptr<Impl> pimpl ;

        public:
            explicit ToCommand() ;

            SystemCall sprocess() const ;
            SystemCall sprocess(core::NTypeLogger& parent_lgr) const ;
            SystemCall sprocess(const core::CharLogger& parent_lgr) const ;

            virtual ~ToCommand() noexcept ;
            ToCommand(ToCommand&&) ;
            ToCommand& operator=(ToCommand&&) ;
            ToCommand(const ToCommand&)            = delete ;
            ToCommand& operator=(const ToCommand&) = delete ;

            void reconstruct() override ;
        } ;
    }
}

#endif
