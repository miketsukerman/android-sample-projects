#pragma once

#include <ios>

class InputStream
{
public:
    InputStream() = default;
    virtual ~InputStream() = default;

    virtual std::streamsize read(char* buffer, std::streamsize count) = 0;
    virtual bool isBufferBased() const = 0;
    virtual bool isSeekable() const = 0;
    virtual std::streamoff seek(std::streamoff offset) = 0;
};

using InputStreamPtr = std::shared_ptr<InputStream>;
