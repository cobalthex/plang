#include <vector>
#include <string>

class Location { };
template <class T> class Array { };

enum class ReportType
{
    Info,
    Message,
    Warning,
    Error,

    Count
};

enum class ReportLevel
{
    Informational,
    Deprecated,
    Trivial,
    Moderate,
    Severe,
};

//report codes must be registered
using ReportCode = size_t;

class Report
{
public:
    ReportType type;
    ReportLevel level;
    ReportCode code;

    Location location;
    size_t filePos; //file name in `location`

    Array<std::string> arguments; //possibly just inline to two/three args

    std::string message; //optional message
};

class Reporter
{
public:
//report(report, fatal)

protected:
    static const ReportCode baseCodeVal = 0x1000;
};