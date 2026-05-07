#pragma once

#include <streambuf>
#include <functional>
#include <string>
#include <iostream>

namespace Debugger3DS {

/// @brief RAII stream redirector.
///
/// Replaces the @c streambuf of the given @c ostream on construction and restores it on
/// destruction.  Each complete line (terminated by @c '\\n') is forwarded to the supplied
/// callback.  Output is simultaneously teed to the original @c streambuf so it still appears
/// in the terminal / debug console.
///
/// The object is non-copyable and non-movable because it owns the @c rdbuf slot of the target
/// stream.
///
/// @par Usage
/// @code
/// CerrRedirect redir(std::cerr, [](const std::string& line) {
///     myLog += line + "\n";
/// });
/// @endcode
class StreamRedirect : public std::streambuf {
public:
    /// @brief Callback invoked once per complete output line (without the trailing newline).
    using Callback = std::function<void(const std::string&)>;

    /// @param stream Stream to redirect (e.g. @c std::cerr or @c std::cout).
    /// @param cb     Callback invoked for each complete line.
    explicit StreamRedirect(std::ostream& stream, Callback cb)
        : stream_(stream), callback_(std::move(cb)), old_(stream.rdbuf(this)) {}

    ~StreamRedirect() {
        flush();
        stream_.rdbuf(old_);
    }

    /// @brief Flush any buffered (incomplete) line to the callback immediately.
    void flush() {
        if (!buf_.empty()) {
            callback_(buf_);
            buf_.clear();
        }
    }

    StreamRedirect(const StreamRedirect&)            = delete;
    StreamRedirect& operator=(const StreamRedirect&) = delete;

protected:
    /// @brief Receive a single character — tee it to the original stream and accumulate lines.
    int overflow(int c) override {
        if (c == EOF) return EOF;
        if (old_) old_->sputc(static_cast<char>(c));   // tee to original stream
        if (c == '\n') {
            flush();
        } else {
            buf_ += static_cast<char>(c);
        }
        return c;
    }

    /// @brief Receive a block of characters — forwards each byte through overflow().
    std::streamsize xsputn(const char* s, std::streamsize n) override {
        for (std::streamsize i = 0; i < n; ++i)
            overflow(static_cast<unsigned char>(s[i]));
        return n;
    }

private:
    std::ostream&   stream_;
    Callback        callback_;
    std::string     buf_;
    std::streambuf* old_;
};

/// @brief Convenience alias for redirecting @c std::cerr.
using CerrRedirect = StreamRedirect;
/// @brief Convenience alias for redirecting @c std::cout.
using CoutRedirect = StreamRedirect;

} // namespace Debugger3DS
