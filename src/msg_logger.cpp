#include "msg_logger.hpp"

#include <windows.h>
#include <mutex>

namespace Logger
{
    static std::mutex mtx ;
    static constexpr auto _er_tag = "[Error] " ;

    template <typename T>
    inline void _error(T&& msg, const char* scope) {
        std::lock_guard<std::mutex> lock(mtx) ;
        error_stream << _er_tag << "Windows Error Code: [" << GetLastError() << "], " << msg << " (" << scope << ")\n" ;
    }
    void error(const char* msg, const char* scope) {
        _error(msg, scope) ;
    }
    void error(std::string&& msg, const char* scope) {
        _error(std::move(msg), scope) ;
    }
    void error(const std::string& msg, const char* scope) {
        _error(msg, scope) ;
    }
}
