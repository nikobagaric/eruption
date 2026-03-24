#ifdef __linux__

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>

namespace Util::File
{
    class Reader {
    public:
        explicit Reader(const std::string &filePath);
        ~Reader();

        Reader(const Reader &) = delete;
        Reader &operator=(const Reader &) = delete;

        const char *data() const { return mData; }
        size_t size() const { return mSize; }
    private:
        int mFileDescriptor{-1};
        char *mData{nullptr};
        size_t mSize{0};
    };
} // namespace Util::File

#endif