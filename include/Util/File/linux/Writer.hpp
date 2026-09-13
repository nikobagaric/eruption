#ifdef __linux__

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>

namespace Util::File
{
    class Writer
    {
    public:
        explicit Writer(const std::string &filePath);
        ~Writer();

        Writer(const Writer &) = delete;
        Writer &operator=(const Writer &) = delete;

        ssize_t write(const char *data, size_t size);

        size_t size() const { return mSize; }

    private:
        int mFileDescriptor{-1};
        char *mBuffer{nullptr};
        size_t mSize{0};
    };
} // namespace Util::File

#endif