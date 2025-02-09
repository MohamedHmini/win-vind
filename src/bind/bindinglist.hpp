#ifndef _BINDINGS_LISTS_HPP
#define _BINDINGS_LISTS_HPP

#include "bindedfunc.hpp"

#include <vector>

namespace vind
{
    namespace bind
    {
        const std::vector<BindedFunc::SPtr>& all_global_binded_funcs() ;

        BindedFunc::SPtr ref_global_funcs_bynames(const std::string& name) ;
        BindedFunc::SPtr ref_global_funcs_bynames(std::string&& name) ;

        template <typename String, typename... Strings>
        std::vector<BindedFunc::SPtr> ref_global_funcs_bynames(String&& name, Strings&&... names) {
            std::vector<BindedFunc::SPtr> out ;

            if(auto func = ref_global_funcs_bynames(std::forward<String>(name))) {
                out.push_back(std::move(func)) ;
            }

            auto append = [&out](auto&& ptr) {
                if(ptr) {
                    out.push_back(std::forward<decltype(ptr)>(ptr)) ;
                }
            } ;

            ((append(ref_global_funcs_bynames(std::forward<String>(names)))), ...) ;

            return out ;
        }
    }
}
#endif
