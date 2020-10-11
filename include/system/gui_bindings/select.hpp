#ifndef _SELECT_HPP
#define _SELECT_HPP
#include "binded_func_with_creator.hpp"

struct SelectAll : public BindedFuncWithCreator<SelectAll>
{
    static void sprocess(const bool first_call, const unsigned int repeat_num, const KeyLogger* const parent_logger) ;
    static const std::string sname() noexcept ;
} ;

#endif
