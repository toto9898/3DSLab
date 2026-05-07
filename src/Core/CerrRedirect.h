#pragma once

#include <streambuf>
#include <functional>
#include <string>
#include <iostream>

// RAII stream redirector.
// Installs itself as the streambuf of the given ostream on construction, restores on destruction.
// Each complete line (terminated by '\n') is passed to the callback.
// Output is also teed to the original streambuf so it still appears in the terminal.
class StreamRedirect : public std::streambuf {
public:
    using Callback = std::function<void(const std::string&)>;

    explicit StreamRedirect(std::ostream& stream, Callback cb)
        : stream_(stream), callback_(std::move(cb)), old_(stream.rdbuf(this)) {}

    ~StreamRedirect() {
        flush();
        stream_.rdbuf(old_);
    }

    void flush() {
        if (!buf_.empty()) {
            callback_(buf_);
            buf_.clear();
        }
    }

    // Non-copyable, non-movable (owns the rdbuf slot)
    StreamRedirect(const StreamRedirect&)            = delete;
    StreamRedirect& operator=(const StreamRedirect&) = delete;

protected:
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

// Convenience aliases
using CerrRedirect = StreamRedirect;
using CoutRedirect = StreamRedirect;
