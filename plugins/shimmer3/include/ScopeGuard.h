#pragma once

#ifndef SSI_SCOPEGUARD_H
#define SSI_SCOPEGUARD_H

#include <functional>

namespace ssi {

    /// <summary>
    /// Wrapper to use RAII with arbitrary cleanup function
    /// </summary>
    struct ScopeGuard {
        /// The deferred function.
        std::function<void()> fn;

        /// Constructor.
        explicit ScopeGuard(std::function<void()> fn)
            : fn(std::move(fn)) {}

        /// Destructor.
        /// Calls the deferred function.
        ~ScopeGuard() { fn(); }

        /// Runs the deferred funciton.
        void run() { fn(); fn = []() {}; }
    };


}  // namespace ssi

#endif /* SSI_SCOPEGUARD_H */
