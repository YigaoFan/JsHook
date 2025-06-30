export module InputStream;

import IParser;
import std;

using std::string_view;
using std::move;
using std::string;
using std::ifstream;

export
{
    class StringViewStream
    {
    private:
        string_view mStr;
        unsigned mCurrentPos;
    public:
        StringViewStream(string_view str, unsigned startPos = 0) : mStr(str), mCurrentPos(startPos)
        {
        }

        auto NextChar() -> Char
        {
            return { this->mStr[this->mCurrentPos++], };
        }

        auto Copy() -> StringViewStream
        {
            return { this->mStr, this->mCurrentPos, };
        }
    };

    // TODO test copy method work as expect
    class FileStream
    {
    private:
        string mFilename;
        ifstream mFile;
    public:
        static auto New(string filename) -> FileStream
        {
            ifstream f{ filename, };
            return { filename, move(f), };
        }

        auto NextChar() -> Char
        {
            char c;
            this->mFile >> c;
            return { c, };
        }

        auto Copy() -> FileStream
        {
            auto c = FileStream::New(this->mFilename);
            c.mFile.seekg(this->mFile.tellg());
        }
    private:
        FileStream(string filename, ifstream file) : mFilename(move(filename)), mFile(move(file))
        {
        }
    };

    /// <summary>
    /// this is function only for compliance check for concept
    /// </summary>
    void Compliance()
    {
        static_assert(ParserInput<StringViewStream>);
        static_assert(ParserInput<FileStream>);
    }
}