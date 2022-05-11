#ifndef LUNARMP_SRC_LUNARMP_UTILS_STRINGUTILS_H_
#define LUNARMP_SRC_LUNARMP_UTILS_STRINGUTILS_H_

#include <sstream>

namespace lunarmp {

//c++11 no longer supplies a strcasecmp, so define our own version.
static inline int stringcasecompare(const char* a, const char* b)
{
    while(*a && *b)
    {
        if (tolower(*a) != tolower(*b))
            return tolower(*a) - tolower(*b);
        a++;
        b++;
    }
    return *a - *b;
}

/*!
 * Efficient conversion of micron integer type to millimeter string.
 *
 * The integer type is half the size of the normal integer type because of implementation details.
 * However, half the integer type should suffice, because we made the basic coord_t twice as big as necessary
 * so as to support multiplication within the same integer type.
 *
 * \param coord The micron unit to convert
 * \param ss The output stream to write the string to
 */
static inline void writeInt2mm(const int32_t coord, std::ostream& ss)
{
    constexpr size_t buffer_size = 24;
    char buffer[buffer_size];
    int char_count = sprintf(buffer, "%d", coord); // we haven't found any way for the windows compiler to accept formatting of a coord_t, so it has to be int32_t instead
#ifdef DEBUG
    if (char_count + 1 >= int(buffer_size)) // + 1 for the null character
    {
        logError("Cannot write %ld to buffer of size %i", coord, buffer_size);
    }
    if (char_count < 0)
    {
        logError("Encoding error while writing %ld", coord);
    }
#endif // DEBUG
    int end_pos = char_count; // the first character not to write any more
    int trailing_zeros = 1;
    while (trailing_zeros < 4 && buffer[char_count - trailing_zeros] == '0')
    {
        trailing_zeros++;
    }
    trailing_zeros--;
    end_pos = char_count - trailing_zeros;
    if (trailing_zeros == 3)
    { // no need to write the decimal dot
        buffer[char_count - trailing_zeros] = '\0';
        ss << buffer;
        return;
    }
    if (char_count <= 3)
    {
        int start = 0; // where to start writing from the buffer
        if (coord < 0)
        {
            ss << '-';
            start = 1;
        }
        ss << "0.";
        for (int nulls = char_count - start; nulls < 3; nulls++)
        { // fill up to 3 decimals with zeros
            ss << '0';
        }
        buffer[char_count - trailing_zeros] = '\0';
        ss << (static_cast<char*>(buffer) + start);
    }
    else
    {
        char prev = '.';
        int pos;
        for (pos = char_count - 3; pos <= end_pos; pos++)
        { // shift all characters and insert the decimal dot
            char next_prev = buffer[pos];
            buffer[pos] = prev;
            prev = next_prev;
        }
        buffer[pos] = '\0';
        ss << buffer;
    }
}

/*!
 * Struct to make it possible to inline calls to writeInt2mm with writing other stuff to the output stream
 */
struct MMtoStream
{
    int64_t value; //!< The coord in micron

    friend inline std::ostream& operator<< (std::ostream& out, const MMtoStream precision_and_input)
    {
        writeInt2mm(precision_and_input.value, out);
        return out;
    }
};

/*!
 * Efficient writing of a double to a stringstream
 *
 * writes with \p precision digits after the decimal dot, but removes trailing zeros
 *
 * \warning only works with precision up to 9 and input up to 10^14
 *
 * \param precision The number of (non-zero) digits after the decimal dot
 * \param coord double to output
 * \param ss The output stream to write the string to
 */
static inline void writeDoubleToStream(const unsigned int precision, const double coord, std::ostream& ss)
{
    char format[5] = "%.xF"; // write a float with [x] digits after the dot
    format[2] = '0' + precision; // set [x]
    constexpr size_t buffer_size = 400;
    char buffer[buffer_size];
    int char_count = sprintf(buffer, format, coord);
#ifdef DEBUG
    if (char_count + 1 >= int(buffer_size)) // + 1 for the null character
    {
        logError("Cannot write %f to buffer of size %i", coord, buffer_size);
    }
    if (char_count < 0)
    {
        logError("Encoding error while writing %f", coord);
    }
#endif // DEBUG
    if (char_count <= 0)
    {
        return;
    }
    if (buffer[char_count - precision - 1] == '.')
    {
        int non_nul_pos = char_count - 1;
        while (buffer[non_nul_pos] == '0')
        {
            non_nul_pos--;
        }
        if (buffer[non_nul_pos] == '.')
        {
            buffer[non_nul_pos] = '\0';
        }
        else
        {
            buffer[non_nul_pos + 1] = '\0';
        }
    }
    ss << buffer;
}

/*!
 * Struct to make it possible to inline calls to writeDoubleToStream with writing other stuff to the output stream
 */
struct PrecisionedDouble
{
    unsigned int precision; //!< Number of digits after the decimal mark with which to convert to string
    double value; //!< The double value

    friend inline std::ostream& operator<< (std::ostream& out, const PrecisionedDouble precision_and_input)
    {
        writeDoubleToStream(precision_and_input.precision, precision_and_input.value, out);
        return out;
    }
};

/*!
 * Struct for writing a string to a stream in an escaped form
 */
struct Escaped
{
    const char* str;

    /*!
     * Streaming function which replaces escape sequences with extra slashes
     */
    friend inline std::ostream& operator<<(std::ostream& os, const Escaped& e)
    {
        for (const char* char_p = e.str; *char_p != '\0'; char_p++)
        {
            switch (*char_p)
            {
                case '\a':  os << "\\a"; break;
                case '\b':  os << "\\b"; break;
                case '\f':  os << "\\f"; break;
                case '\n':  os << "\\n"; break;
                case '\r':  os << "\\r"; break;
                case '\t':  os << "\\t"; break;
                case '\v':  os << "\\v"; break;
                case '\\':  os << "\\\\"; break;
                case '\'':  os << "\\'"; break;
                case '\"':  os << "\\\""; break;
                case '\?':  os << "\\\?"; break;
                default: os << *char_p;
            }
        }
        return os;
    }
};

static inline std::string camel2snake(std::string &str) {
    std::stringstream res;
    for (int i = 0; i < str.length(); ++i) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            res << "_" << (char)(str[i] + 32);
        } else {
            res << str[i];
        }
    }
    return res.str();
}

static inline std::string sToLower(const std::string& s) {
    std::string res(s);
    transform(s.begin(), s.end(), res.begin(), ::tolower);
    return res;
}

constexpr static unsigned int str2int(const char* str, int h = 0) {
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

}

#endif  // LUNARMP_SRC_LUNARMP_UTILS_STRINGUTILS_H_
