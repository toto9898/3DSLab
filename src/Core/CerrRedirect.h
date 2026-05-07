#pragma once

#include <streambuf>
#include <functional>
#include <string>
#include <iostream>

// RAII cerr redirector.
// Installs itself as std::cerr's streambuf on construction, restores on destruction.
// Each complete line (terminated by '\n') is passed to the callback.
// Output is also teed to the original cerr streambuf so it still appears in the terminal.
class CerrRedirect : public std::streambuf {
public:
    using Callback = std::function<void(const std::string&)>;

    explicit CerrRedirect(Callback cb)
        : callback_(std::move(cb)), old_(std::cerr.rdbuf(this)) {}

    ~CerrRedirect() {
        if (!buf_.empty()) {
            callback_(buf_);
            buf_.clear();
        }
        std::cerr.rdbuf(old_);
    }

    // Non-copyable, non-movable (owns the rdbuf slot)
    CerrRedirect(const CerrRedirect&)            = delete;
    CerrRedirect& operator=(const CerrRedirect&) = delete;

protected:
    int overflow(int c) override {
        if (c == EOF) return EOF;
        if (old_) old_->sputc(static_cast<char>(c));   // tee to original cerr
        if (c == '\n') {
            if (!buf_.empty()) {
                callback_(buf_);
                buf_.clear();
            }
        } else {
            buf_ += static_cast<char>(c);
        }
        return c;
    }

    std::streamsize xsputn(const char* s, std::streamsize n) override {
        for (std::streamsize i = 0; i < n; ++i)
            overflow(static_cast<unsigned char>(s[i]));
        return n;
    }

private:
    Callback        callback_;
    std::string     buf_;
    std::streambuf* old_;
};
