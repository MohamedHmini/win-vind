#include "errlogger.hpp"

#include "path.hpp"
#include "util/debug.hpp"
#include "util/winwrap.hpp"
#include "version.hpp"

#include <unordered_map>
#include <windows.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>


namespace
{
    void remove_files_over(
            const std::filesystem::path& log_dir,
            std::string pattern_withex,
            std::size_t num) {
        std::vector<std::filesystem::path> files ;

        WIN32_FIND_DATAW wfd = {} ;

        auto query = log_dir / pattern_withex ;
        query.make_preferred() ;

        auto handle = FindFirstFileW(query.wstring().c_str(), &wfd) ;
        if(handle == INVALID_HANDLE_VALUE) {
            return ;
        }

        do {
            auto log_path = log_dir / wfd.cFileName ;
            log_path.make_preferred() ;
            files.push_back(std::move(log_path)) ;
        } while(FindNextFileW(handle, &wfd)) ;

        FindClose(handle) ;

        if(files.size() <= num) {
            return ;
        }

        std::sort(files.begin(), files.end(), std::greater<std::wstring>{}) ;
        for(std::size_t i = num ; i < files.size() ; i ++) {
            std::error_code ec ;
            if(!std::filesystem::remove(files[i], ec)) {
                PRINT_ERROR(ec.message()) ;
            }
        }
    }
}

namespace vind
{
    namespace core {
        struct Logger::Impl {
            std::ofstream stream_ ;

            std::string head_ ;
            std::size_t keep_log_num_ ;
            std::size_t header_align_width_ ;

            // When writing to a stream in a multi-threaded manner,
            // an exclusion process is performed so that the contents do not get mixed up.
            std::mutex mtx_ ;

            template <typename String>
            Impl(
                String&& filename_head,
                std::size_t keeping_log_num,
                std::size_t align_width_of_header)
            : stream_(),
              head_(std::forward<String>(filename_head)),
              keep_log_num_(keeping_log_num),
              header_align_width_(align_width_of_header),
              mtx_()
            {}

            template <typename Key, typename... Vals>
            void add_spec(Key&& key, Vals&&... vals) {
                stream_ << std::right ;
                stream_ << std::setw(header_align_width_) ;
                stream_ << std::forward<Key>(key) ;
                stream_ << std::left ;
                stream_ << std::setw(0) ;
                ((stream_ << std::forward<Vals>(vals)), ...) ;
                stream_ << std::endl ;
                stream_.flush() ;
            }
        } ;

        Logger::Logger(
            std::string&& filename_head,
            std::size_t keeping_log_num,
            std::size_t align_width_of_header)
        : pimpl(std::make_unique<Impl>(
                    std::move(filename_head),
                    keeping_log_num,
                    align_width_of_header))
        {}

        Logger::~Logger() noexcept = default ;

        /**
         * NOTE: Inspired by bitcoin's logger implementation,
         * it dynamically allocates local static variables.
         * This allows us to leave the freeing of variables to
         * the OS/libc and survive until the end in the process.
         * Using this design pattern, Logger is available
         * in destructors of objects with undefined release order,
         * such as static/global.
         *
         * ref.(https://github.com/bitcoin/bitcoin/blob/57982f419e36d0023c83af2dd0d683ca3160dc2a/src/logging.cpp#L17-L36)
         */
        Logger& Logger::get_instance() {
            static auto instance = new Logger("syslog_", 10, 15) ;
            return *instance ;
        }

        void Logger::init() {
            auto log_dir = ROOT_PATH() / "log" ;

            SYSTEMTIME stime ;
            GetLocalTime(&stime) ;

            std::ostringstream ss ;
            ss << stime.wYear \
                << std::setw(2) << std::setfill('0') << stime.wMonth \
                << std::setw(2) << std::setfill('0') << stime.wDay \
                << std::setw(2) << std::setfill('0') << stime.wHour \
                << std::setw(2) << std::setfill('0') << stime.wMinute ;

            if(!std::filesystem::exists(log_dir)) {
                std::filesystem::create_directories(log_dir) ;
            }

            auto filepath = log_dir / (pimpl->head_ + ss.str() + ".log") ;

            pimpl->stream_.open(filepath, std::ios::app) ;

            // Export system infomation for handling issues.
            pimpl->stream_ << "========== System Infomation ==========\n" ;
            pimpl->stream_ << "[Windows]\n" ;

            auto [major, minor, build] = util::get_Windows_versions() ;

            pimpl->add_spec("Edition: ", util::get_Windows_edition(major, minor)) ;
            pimpl->add_spec("Version: ", util::get_Windows_display_version()) ;
            pimpl->add_spec("Build Numbers: ", major, ".", minor, ".", build) ;
            pimpl->add_spec("Architecture: ", util::get_Windows_architecture()) ;

            pimpl->stream_ << std::endl ;

            pimpl->stream_ << "[win-vind]\n" ;
            pimpl->add_spec("Version: ", WIN_VIND_VERSION) ;

            pimpl->stream_ << "=======================================\n" ;
            pimpl->stream_.flush() ;

             //If the log files exists over five, remove old files.
            remove_files_over(log_dir, pimpl->head_ + "*.log", pimpl->keep_log_num_) ;
        }

        void Logger::error(const std::string& msg, const std::string& scope) {
            if(pimpl->stream_.is_open()) {
                std::lock_guard<std::mutex> scoped_lock{pimpl->mtx_} ;

                auto win_ercode = GetLastError() ;
                if(win_ercode) {
                    pimpl->stream_ << "[Error] " ;

                    LPSTR msgbuf = nullptr ;
                    if(auto size = FormatMessageA(
                                FORMAT_MESSAGE_ALLOCATE_BUFFER | \
                                FORMAT_MESSAGE_FROM_SYSTEM | \
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, win_ercode,
                                MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                reinterpret_cast<LPSTR>(&msgbuf),
                                0, NULL)) {

                        // print message without \r\n
                        pimpl->stream_ << std::string(msgbuf, size - 2) ;
                        LocalFree(msgbuf) ;
                    }
                    else {
                        pimpl->stream_ << "Windows Error Code: [" << win_ercode << "]" ;
                    }
                }
                pimpl->stream_ <<  std::endl ;

                pimpl->stream_ << "[Error] " << msg ;
                if(!scope.empty()) {
                    pimpl->stream_ << " (" << scope << ")" ;
                }
                pimpl->stream_ << std::endl ;

                pimpl->stream_.flush() ;
            }
        }

        void Logger::message(const std::string& msg, const std::string& scope) {
            if(pimpl->stream_.is_open()) {
                std::lock_guard<std::mutex> scoped_lock{pimpl->mtx_} ;

                pimpl->stream_ << "[Message] " << msg ;
                if(!scope.empty()) {
                    pimpl->stream_ << " (" << scope << ")" ;
                }
                pimpl->stream_ << std::endl ;
                pimpl->stream_.flush() ;
            }
        }

        void Logger::warning(const std::string& msg, const std::string& scope) {
            if(pimpl->stream_.is_open()) {
                std::lock_guard<std::mutex> scoped_lock{pimpl->mtx_} ;

                pimpl->stream_ << "[Warning] " << msg ;
                if(!scope.empty()) {
                    pimpl->stream_ << " (" << scope << ")" ;
                }
                pimpl->stream_ << std::endl ;
                pimpl->stream_.flush() ;
            }
        }
    }
}
