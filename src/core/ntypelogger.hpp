#ifndef _NTYPE_LOGGER_HPP
#define _NTYPE_LOGGER_HPP

#include "keylgrbase.hpp"

#define NTYPE_HEAD_NUM(result)  ((result) == -1)
#define NTYPE_EMPTY(result)     ((result) == 0)
#define NTYPE_LOGGED(result)    ((result) > 0)

namespace vind
{
    namespace core
    {
        class NTypeLogger : public KeyLoggerBase {
        private:
            struct Impl ;
            std::unique_ptr<Impl> pimpl ;

            virtual int transition_to_parsing_num_state(const KeyLog& num_only_log) ;

            virtual int do_initial_state(const KeyLog& log) ;
            virtual int do_initial_waiting_state(const KeyLog& log) ;
            virtual int do_waiting_state(const KeyLog& log) ;
            virtual int do_accepted_state(const KeyLog& log) ;
            virtual int do_parsing_num_state(const KeyLog& log) ;

        public:
            explicit NTypeLogger() ;
            virtual ~NTypeLogger() noexcept ;

            NTypeLogger(const NTypeLogger&) ;
            NTypeLogger& operator=(const NTypeLogger&) ;

            NTypeLogger(NTypeLogger&&) ;
            NTypeLogger& operator=(NTypeLogger&&) ;

            virtual int logging_state(const KeyLog& log) override ;
            virtual unsigned int get_head_num() const noexcept ;
            virtual void set_head_num(unsigned int num) noexcept ;
            virtual bool has_head_num() const noexcept ;

            virtual bool is_long_pressing() const noexcept ;

            // If the sequence is rejected, you must call it.
            virtual void reject() noexcept ; // alias of clear
            virtual void clear() noexcept override ;

            // If the sequence is accepted, you must call it.
            virtual void accept() noexcept ;
        } ;
    }
}

#endif
