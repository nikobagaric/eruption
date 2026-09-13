#ifdef __linux__

#include "Util/File/linux/Writer.hpp"
#include <stdexcept>

namespace Util::File
{
    Writer::Writer(const std::string &filePath)
    {
        mFileDescriptor = open(filePath.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (mFileDescriptor == -1)
        {
            throw std::runtime_error("failed to open file for writing: " + filePath);
        }
    }
    
    Writer::~Writer()
    {
        if (mBuffer)
        {
            munmap(mBuffer, mSize);
        }

        if (mFileDescriptor != -1)
        {
            close(mFileDescriptor);
        }
    }

    ssize_t Writer::write(const char *data, size_t size)
    {
        if (ftruncate(mFileDescriptor, size) == -1)
        {
            throw std::runtime_error("failed to set file size");
        }

        mBuffer = static_cast<char *>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, mFileDescriptor, 0));
        if (mBuffer == MAP_FAILED)
        {
            throw std::runtime_error("failed to map file to memory");
        }

        std::copy(data, data + size, mBuffer);
        mSize = size;

        return static_cast<ssize_t>(size);
    }
} // namespace Util::File


#endif