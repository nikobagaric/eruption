#ifdef __linux__

#include "Util/File/linux/Reader.hpp"
#include <stdexcept>

namespace Util::File
{
    Reader::Reader(const std::string &filePath)
    {
        mFileDescriptor = open(filePath.c_str(), O_RDONLY);
        if (mFileDescriptor == -1)
        {
            throw std::runtime_error("failed to open file: " + filePath);
        }

        struct stat fileStat;
        if (fstat(mFileDescriptor, &fileStat) == -1)
        {
            close(mFileDescriptor);
            throw std::runtime_error("failed to get file size: " + filePath);
        }
        mSize = static_cast<size_t>(fileStat.st_size);

        mData = static_cast<char *>(mmap(nullptr, mSize, PROT_READ, MAP_PRIVATE, mFileDescriptor, 0));
        if (mData == MAP_FAILED)
        {
            close(mFileDescriptor);
            throw std::runtime_error("failed to map file: " + filePath);
        }
    }

    Reader::~Reader()
    {
        if (mData)
        {
            munmap(mData, mSize);
            mData = nullptr;
        }

        if (mFileDescriptor != -1)
        {
            close(mFileDescriptor);
            mFileDescriptor = -1;
        }
    }
} // namespace Util::File

#endif