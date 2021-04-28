#ifndef _INPUT_HINTER_HPP
#define _INPUT_HINTER_HPP

#include <future>
#include <memory>

#include "ec_hints.hpp"
#include "point_2d.hpp"

namespace vind
{
    class InputHinter {
    private:
        struct Impl ;
        std::unique_ptr<Impl> pimpl ;

    public:
        explicit InputHinter() ;
        virtual ~InputHinter() noexcept ;

        InputHinter(const InputHinter&)            = delete ;
        InputHinter& operator=(const InputHinter&) = delete ;

        InputHinter(InputHinter&&) ;
        InputHinter& operator=(InputHinter&&) ;

        void load_config() ;

        Point2D::SPtr launch_loop(
                const std::vector<Point2D>& positions,
                const std::vector<Hint>& hints) ;

        std::shared_future<Point2D::SPtr> launch_async_loop(
                const std::vector<Point2D>& positions,
                const std::vector<Hint>& hints) ;

        void cancel() noexcept ;

        const std::vector<unsigned char>& matched_counts() const noexcept ;

        const std::size_t& drawable_hints_num() const noexcept ;
    } ;
}

#endif
