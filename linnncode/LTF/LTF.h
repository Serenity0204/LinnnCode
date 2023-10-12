#include <algorithm>
#include <cassert>
#include <chrono>
#include <ctime>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

namespace LTF
{
    template <class T>
    class Singleton
    {
    private:
        Singleton() = delete;
        ~Singleton() = delete;
        Singleton(const Singleton<T>& rhs) = delete;
        Singleton<T>& operator=(const Singleton<T>& rhs) = delete;

    public:
        // the only allowed access point
        inline static T& instance()
        {
            static T s_instance;
            return s_instance;
        }
    };
};

namespace LTF
{
    // success code
    enum LTFCode
    {
        FAIL = 0,
        SUCCESS = 1,
    };

    // status struct that will record the success code and line number
    struct LTFStatus
    {
    public:
        std::size_t line;
        LTFCode code;
        LTFStatus(LTFCode code = LTFCode::FAIL, std::size_t line = 0)
        {
            this->line = line;
            this->code = code;
        }
        ~LTFStatus() {}
    };

    class TestCase
    {
    private:
        std::string m_test_name;
        LTFStatus (*m_function)(bool debug);

        // Function to time other function, in unit of nanoseconds
        inline std::chrono::duration<double, std::nano> time()
        {
            // Start the timer
            auto start = std::chrono::high_resolution_clock::now();

            // Call the input function in debug = false
            m_function(false);

            // Stop the timer
            auto end = std::chrono::high_resolution_clock::now();

            // Calculate the elapsed time in milliseconds
            return end - start;
        }

    public:
        // CTORS
        TestCase();
        TestCase(const std::string& name, LTFStatus (*function)(bool debug));
        ~TestCase();

        // call the function
        LTFStatus run(bool debug, double& time);

        // getters and setters
        void set_test_name(const std::string& name);
        void set_test_function(LTFStatus (*function)(bool debug));
        inline const std::string& get_test_name() const { return this->m_test_name; }
    };
};

LTF::TestCase::TestCase()
{
    this->m_function = nullptr;
    this->m_test_name = "";
}

LTF::TestCase::TestCase(const std::string& name, LTFStatus (*function)(bool debug))
{
    this->m_test_name = name;
    this->m_function = function;
}

LTF::TestCase::~TestCase()
{
}

void LTF::TestCase::set_test_name(const std::string& name)
{
    this->m_test_name = name;
}

void LTF::TestCase::set_test_function(LTFStatus (*function)(bool debug))
{
    this->m_function = function;
}

LTF::LTFStatus LTF::TestCase::run(bool debug, double& time)
{
    if (this->m_function == nullptr) return LTFStatus();
    std::chrono::duration<double, std::milli> elapsed = this->time();
    time = elapsed.count();
    return this->m_function(debug);
}

namespace LTF
{
    class TestSuite
    {
    private:
        // test name vs test case, test name vs pass or not
        std::string m_suite_name;
        std::map<std::string, TestCase> m_test_cases;
        std::map<std::string, LTF::LTFStatus> m_passed_flags;

    public:
        // CTORS
        TestSuite();
        TestSuite(const std::string& name);
        ~TestSuite();
        TestSuite& operator=(const TestSuite& rhs);

        // add the case to the suite by case name vs case
        TestSuite& add(const TestCase& test_case);

        // run all that will return a map of case name vs status
        std::map<std::string, LTF::LTFStatus>& run_all(bool debug, int& test_count, std::map<std::string, double>& times);

        // getters
        inline const std::string& get_suite_name() const { return this->m_suite_name; }
        inline std::size_t get_num_tests() const { return this->m_test_cases.size(); }

        // print all of the cases's name
        void print() const
        {
            std::cout << "test cases:" << std::endl;
            for (const auto& test : this->m_test_cases) std::cout << "name:" << test.first << std::endl;
            std::cout << std::endl;
        }

        // remove test cases
        inline void ignore_tests(const std::vector<std::string>& tests)
        {
            for (const std::string& test : tests) this->m_test_cases.erase(test);
        }
    };

};

LTF::TestSuite::TestSuite()
{
}

LTF::TestSuite::TestSuite(const std::string& name)
{
    this->m_suite_name = name;
    this->m_test_cases = std::map<std::string, TestCase>();
    this->m_passed_flags = std::map<std::string, LTF::LTFStatus>();
}

LTF::TestSuite::~TestSuite()
{
}

LTF::TestSuite& LTF::TestSuite::operator=(const TestSuite& rhs)
{
    this->m_suite_name = rhs.m_suite_name;
    this->m_test_cases = rhs.m_test_cases;
    this->m_passed_flags = rhs.m_passed_flags;
    return *this;
}
LTF::TestSuite& LTF::TestSuite::add(const TestCase& test_case)
{
    std::string name = test_case.get_test_name();
    this->m_test_cases[name] = test_case;
    return *this;
}

std::map<std::string, LTF::LTFStatus>& LTF::TestSuite::run_all(bool debug, int& test_count, std::map<std::string, double>& times)
{
    this->m_passed_flags.clear();
    times.clear();
    for (const auto& test : this->m_test_cases)
    {
        std::string name = test.first;
        TestCase test_case = test.second;
        double time = 0;
        LTFStatus code = test_case.run(debug, time);
        times[name] = time;
        this->m_passed_flags[name] = code;
    }
    test_count = this->m_passed_flags.size();
    return this->m_passed_flags;
}

namespace LTF
{
    enum MODE
    {
        CONSOLE = 0,
        FILE = 1,
    };

    // implement Singleton design pattern
    class LittleTestFramework
    {
    private:
        friend class Singleton<LTF::LittleTestFramework>;

    private:
        // suite name vs suite
        std::map<std::string, TestSuite> m_suites;
        // for messages, function name vs vector of message
        std::map<std::string, std::vector<std::string>> m_messages;
        // for timing, function name vs time in ms
        std::map<std::string, double> m_times;

        inline static void output(const std::string& message, std::ofstream& outs, LTF::MODE mode = LTF::MODE::CONSOLE)
        {
            if (mode == LTF::MODE::FILE) outs << message;
            if (mode == LTF::MODE::CONSOLE) std::cout << message;
        }

    private:
        // CTORS
        LittleTestFramework();
        LittleTestFramework(const LittleTestFramework& rhs) = delete;
        LittleTestFramework& operator=(const LittleTestFramework& rhs) = delete;
        ~LittleTestFramework();

    public:
        // add suite by suite name vs suite
        void add(const TestSuite& suite);

        // getters
        bool suite_exists(const std::string& suite_name);
        TestSuite& get_suite(const std::string& suite_name);
        inline std::size_t get_num_suites() { return this->m_suites.size(); }

        // for testing
        std::map<std::string, TestSuite>& get() { return this->m_suites; }

        // main method to call to run all tests
        void run_all(bool debug = false, LTF::MODE mode = LTF::CONSOLE, const std::string& path = "");

        // remove
        inline void clean()
        {
            this->m_suites.clear();
            this->m_messages.clear();
            this->m_times.clear();
        }

        void ignore_suites(const std::vector<std::string>& suites);
        void ignore_tests(const std::string& suite_name, const std::vector<std::string>& tests);

        // message
        void comment(const std::string& function, const std::string& message);

        // time
        void time(const std::string& function, double time_ns);
    };
};

LTF::LittleTestFramework::LittleTestFramework()
{
    this->m_suites = std::map<std::string, LTF::TestSuite>();
    this->m_messages = std::map<std::string, std::vector<std::string>>();
    this->m_times = std::map<std::string, double>();
}

LTF::LittleTestFramework::~LittleTestFramework()
{
    this->clean();
}

void LTF::LittleTestFramework::add(const LTF::TestSuite& suite)
{
    this->m_suites[suite.get_suite_name()] = suite;
}

bool LTF::LittleTestFramework::suite_exists(const std::string& suite_name)
{
    return this->m_suites.count(suite_name) > 0;
}

LTF::TestSuite& LTF::LittleTestFramework::get_suite(const std::string& suite_name)
{
    assert(this->suite_exists(suite_name));
    return this->m_suites[suite_name];
}

void LTF::LittleTestFramework::ignore_suites(const std::vector<std::string>& suites)
{
    for (const std::string& suite : suites) this->m_suites.erase(suite);
}

void LTF::LittleTestFramework::ignore_tests(const std::string& suite_name, const std::vector<std::string>& tests)
{
    // if suite doesn't exist then do early return
    if (!this->suite_exists(suite_name)) return;
    // get the suite, remove the tests, then add it back
    LTF::TestSuite suite = this->get_suite(suite_name);
    suite.ignore_tests(tests);
    this->add(suite);
}

void LTF::LittleTestFramework::run_all(bool debug, MODE mode, const std::string& path)
{
    std::ofstream outs;
    if (mode == LTF::MODE::FILE) outs.open(path);

    // no suites
    if (this->get_num_suites() == 0)
    {
        LTF::LittleTestFramework::output("\nLTF: NO SUITES ADDED\n", outs, mode);
        LTF::LittleTestFramework::output("EXIT LTF...\n", outs, mode);

        if (mode == LTF::MODE::FILE) outs.close();
        return;
    }

    // constants
    const std::string SPACE = "         ";
    const std::string LINE = "---------------------------------------------------------------------------------------------------------";

    // count the number of tests and suites
    std::vector<std::size_t> tests_count;
    std::size_t suites_count = 0;
    for (auto& suite : this->m_suites)
    {
        tests_count.push_back(suite.second.get_num_tests());
        ++suites_count;
    }

    LTF::LittleTestFramework::output("\nLTF RUNNING ALL " + std::to_string(suites_count) + " TEST SUITE" + (suites_count != 1 ? "S" : "") + ":\n", outs, mode);
    LTF::LittleTestFramework::output(LINE + "\n\n", outs, mode);

    int i = 0;
    int success_total = 0, fail_total = 0;
    for (auto& suite : this->m_suites)
    {
        LTF::LittleTestFramework::output(std::to_string(i + 1) + "." + suite.first + ":\n", outs, mode);

        LTF::LittleTestFramework::output(SPACE + "RUNNING " + std::to_string(tests_count[i]) + " TEST" + (tests_count[i] != 1 ? "S" : "") + " FROM " + suite.first + "\n", outs, mode);
        LTF::LittleTestFramework::output(LINE + "\n", outs, mode);

        // run all and get the tests and times
        int num_test = 0;
        std::map<std::string, double> times;
        auto pass_map = suite.second.run_all(debug, num_test, times);

        // success and fail count
        int count = 1, success_count = 0, fail_count = 0;
        for (const auto& flag : pass_map)
        {
            LTF::LittleTestFramework::output(SPACE + "RUNNING...\n", outs, mode);
            std::string result = SPACE + SPACE + std::to_string(count) + ".TEST NAME:" + flag.first + " ----> ";

            // check timeout first, if timeout then automatically fail
            bool timed = false;
            // make sure the function exists
            assert(times.count(flag.first) > 0);

            // if timed
            if (this->m_times.count(flag.first) > 0)
            {
                // if actual is greater than or equal to expected
                if (this->m_times[flag.first] <= times[flag.first])
                {
                    ++fail_count;
                    result += "[FAIL]";
                    if (debug) result += " AT LINE [" + std::to_string(flag.second.line) + "]";
                    result += "\n" + SPACE + SPACE + SPACE + "TIME LIMIT EXCEEDED: EXPECTED LESS THAN " + std::to_string(m_times[flag.first]) + " NANOSECONDS";
                    timed = true;
                }
            }

            // if not timed, then compare success code
            if (!timed)
            {
                // check success or fail
                if (flag.second.code == LTF::SUCCESS)
                {
                    ++success_count;
                    result += "[SUCCESS]";
                }
                if (flag.second.code == LTF::FAIL)
                {
                    ++fail_count;
                    result += "[FAIL]";
                    if (debug) result += " AT LINE [" + std::to_string(flag.second.line) + "]";
                }
            }

            // output result
            LTF::LittleTestFramework::output(result, outs, mode);

            // output the time
            std::string time_output = "";
            double nanoseconds = times[flag.first];
            LTF::LittleTestFramework::output(std::string(", TIME:") + std::to_string(nanoseconds) + " NANOSECONDS\n", outs, mode);

            // if there's log message then start outputting them
            if (this->m_messages.count(flag.first))
            {
                std::vector<std::string> messages = this->m_messages[flag.first];
                std::string messages_output = "";
                // concat messages
                for (int i = 0; i < messages.size(); ++i) messages_output += SPACE + SPACE + std::to_string(i + 1) + ":" + messages[i] + "\n";
                std::string output_str = "\n" + SPACE + SPACE + std::string("MESSAGE") + (messages.size() != 1 ? "S" : "") + std::string(" FROM:") + flag.first + ":\n" + messages_output;
                LTF::LittleTestFramework::output(output_str, outs, mode);
            }
            ++count;
        }

        // showing total test success and fail
        LTF::LittleTestFramework::output("\nTOTAL:\n", outs, mode);
        if (success_count != 0) LTF::LittleTestFramework::output(SPACE + "(" + std::to_string(success_count) + ") TEST" + (success_count != 1 ? "S" : "") + " SUCCESS\n", outs, mode);
        if (fail_count != 0) LTF::LittleTestFramework::output(SPACE + "(" + std::to_string(fail_count) + ") TEST" + (fail_count != 1 ? "S" : "") + " FAIL\n", outs, mode);

        success_total += success_count;
        fail_total += fail_count;

        LTF::LittleTestFramework::output("\n" + LINE + "\n", outs, mode);
        ++i;
    }

    // showing summary
    LTF::LittleTestFramework::output("\nSUMMARY:\n", outs, mode);
    int total_num_of_test = success_total + fail_total;
    LTF::LittleTestFramework::output(SPACE + "(" + std::to_string(total_num_of_test) + ") TOTAL TEST" + (success_total != 1 ? "S" : "") + " RUN\n", outs, mode);

    if (success_total != 0) LTF::LittleTestFramework::output(SPACE + "(" + std::to_string(success_total) + ") TOTAL TEST" + (success_total != 1 ? "S" : "") + " SUCCESS\n", outs, mode);

    if (fail_total != 0) LTF::LittleTestFramework::output(SPACE + "(" + std::to_string(fail_total) + ") TOTAL TEST" + (fail_total != 1 ? "S" : "") + " FAIL\n", outs, mode);

    LTF::LittleTestFramework::output("\n\n---------------------------------------------THE END OF TEST---------------------------------------------\n\n", outs, mode);

    if (mode == LTF::MODE::FILE) outs.close();
}

void LTF::LittleTestFramework::comment(const std::string& function, const std::string& message)
{
    // if the key exists, then get the vector
    if (this->m_messages.count(function) > 0)
    {
        // check if message already exists, same message should not be output twice anyways
        bool exist = std::find(this->m_messages[function].begin(), this->m_messages[function].end(), message) != this->m_messages[function].end();
        if (exist) return;
        // insert if does not exist
        this->m_messages[function].push_back(message);
        return;
    }
    std::vector<std::string> newMessage = {message};
    this->m_messages[function] = newMessage;
}

void LTF::LittleTestFramework::time(const std::string& function, double time_ns)
{
    // if exists, then don't add
    if (this->m_times.count(function) > 0) return;
    // else add
    this->m_times[function] = time_ns;
}

namespace LTF
{
    namespace LTFDebug
    {
        const bool debug = false;
    };

// the macro for commenting
#define LTF_COMMENT(message) LTF::Singleton<LTF::LittleTestFramework>::instance().comment(__FUNCTION__, message);

// time macro for timing in ns
#define LTF_TIME(time_ns) LTF::Singleton<LTF::LittleTestFramework>::instance().time(__FUNCTION__, time_ns);

// TEST SUITE Associate to test function
// if suite does not exist, create one and add the function into that suite and add it to the manager through singleton instance
// else get the existed suite, add the function, and then add it back to the manager through singleton instance
#define LTF_TEST(suite_name, test_case)                                                                                               \
                                                                                                                                      \
    struct TestRegistration_##suite_name##_##test_case                                                                                \
    {                                                                                                                                 \
        TestRegistration_##suite_name##_##test_case()                                                                                 \
        {                                                                                                                             \
            if (LTF::Singleton<LTF::LittleTestFramework>::instance().suite_exists(#suite_name))                                       \
            {                                                                                                                         \
                LTF::TestCase test = LTF::TestCase(#test_case, test_case);                                                            \
                LTF::TestSuite suite = LTF::Singleton<LTF::LittleTestFramework>::instance().get_suite(#suite_name).add(test);         \
                LTF::Singleton<LTF::LittleTestFramework>::instance().add(suite);                                                      \
                if (LTF::LTFDebug::debug) std::cout << #suite_name << " exists and created " << #test_case << std::endl;              \
                return;                                                                                                               \
            }                                                                                                                         \
                                                                                                                                      \
            LTF::TestSuite suite(#suite_name);                                                                                        \
            suite.add(LTF::TestCase(#test_case, test_case));                                                                          \
            LTF::Singleton<LTF::LittleTestFramework>::instance().add(suite);                                                          \
            if (LTF::LTFDebug::debug) std::cout << #suite_name << " does not exist and created self and " << #test_case << std::endl; \
        }                                                                                                                             \
    } g_TestRegistration_##suite_name##_##test_case;

    // Run All
    inline void LTF_RUN_ALL(bool debug = false, MODE mode = LTF::CONSOLE, const std::string& path = "")
    {
        LTF::Singleton<LTF::LittleTestFramework>::instance().run_all(debug, mode, path);
        LTF::Singleton<LTF::LittleTestFramework>::instance().clean();
    }

    // Ignore test suites
    inline void LTF_IGNORE_SUITES(const std::vector<std::string>& ignored_suites)
    {
        LTF::Singleton<LTF::LittleTestFramework>::instance().ignore_suites(ignored_suites);
    }

    // Ignore test cases by test suites
    inline void LTF_IGNORE_TEST_CASES(const std::string& suite_name, const std::vector<std::string>& ignored_tests)
    {
        LTF::Singleton<LTF::LittleTestFramework>::instance().ignore_tests(suite_name, ignored_tests);
    }
};

namespace LTF
{
    class Logger
    {
    public:
        enum class Level
        {
            DEBUG = 0,
            INFO = 1,
            WARNING = 2,
            ERROR = 3,
            FATAL = 4,
        };

        struct Info
        {
        public:
            std::string file;
            int line;
            Info(const std::string& file = "", int line = 0)
            {
                this->file = file;
                this->line = line;
            }
        };

    private:
        friend class Singleton<LTF::Logger>;

    private:
        // class attributes
        std::ofstream m_file;
        std::mutex m_lock;
        LTF::Logger::Level m_level;
        std::string m_path;
        int m_message_count = 0;

    private:
        // CTORS
        Logger() = default;
        ~Logger();

        Logger(const Logger& rhs) = delete;
        Logger& operator=(const Logger& rhs) = delete;

    private:
        // helper methods
        std::string level_str(LTF::Logger::Level level) const;
        std::string format_output(const std::string& level, const std::string& time, const LTF::Logger::Info& info, const std::string& message);
        std::string get_time_stamp();
        inline bool should_print(LTF::Logger::Level level) const { return level <= this->m_level; }

        // TBD
        void rotate_log_file();

    public:
        // core fucntionalities
        void log(LTF::Logger::Level level, const std::string& message, const Info& info);
        void level(LTF::Logger::Level level);
        void open(const std::string& path);

        // for testing
        inline int count() { return this->m_message_count; }
    };
};

LTF::Logger::~Logger()
{
    if (!this->m_file.is_open()) return;
    this->m_file.close();
}

std::string LTF::Logger::level_str(LTF::Logger::Level level) const
{
    std::string str = "";
    switch (level)
    {
    case LTF::Logger::Level::DEBUG:
        str = "[DEBUG]";
        break;
    case LTF::Logger::Level::ERROR:
        str = "[ERROR]";
        break;
    case LTF::Logger::Level::FATAL:
        str = "[FATAL]";
        break;
    case LTF::Logger::Level::INFO:
        str = "[INFO]";
        break;
    case LTF::Logger::Level::WARNING:
        str = "[WARN]";
        break;
    default:
        throw std::runtime_error("input level does not exist");
        break;
    }
    return str;
}

std::string LTF::Logger::get_time_stamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    // Convert to Pacific Time (UTC-7 or UTC-8 depending on daylight saving time)
    std::tm pacific_time;

    // platform specific
#ifdef _WIN32
    gmtime_s(&pacific_time, &current_time);
#else
    gmtime_r(&current_time, &pacific_time);
#endif

    pacific_time.tm_hour -= 7;                         // UTC-7
    if (pacific_time.tm_isdst) pacific_time.tm_hour--; // Adjust for daylight saving time (UTC-8)

    // Ensure hours are within 0-23 range
    if (pacific_time.tm_hour < 0)
    {
        pacific_time.tm_hour += 24;
        pacific_time.tm_mday -= 1; // Go back one day
    }

    // Convert to string
    std::ostringstream oss;
    oss << std::put_time(&pacific_time, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string LTF::Logger::format_output(const std::string& level, const std::string& time, const LTF::Logger::Info& info, const std::string& message)
{
    std::string output_str = "";
    output_str += time + " " + level + " " + info.file + ":" + std::to_string(info.line) + " " + message;
    return output_str;
}

void LTF::Logger::log(LTF::Logger::Level level, const std::string& message, const Info& info)
{
    std::lock_guard<std::mutex> lock(this->m_lock);

    // if level too high then not log
    // check if the log file is opened, if not should not record the log message
    if (!this->should_print(level) || !this->m_file.is_open()) return;

    std::string time_output = this->get_time_stamp();
    std::string level_output = this->level_str(level);
    std::string output = this->format_output(level_output, time_output, info, message);

    // check if the file needs to rotate
    // logic goes here

    // log the message
    this->m_file << output << "\n";
    this->m_file.flush();
    ++this->m_message_count;
}

void LTF::Logger::level(LTF::Logger::Level level)
{
    this->m_level = level;
}

void LTF::Logger::open(const std::string& path)
{
    if (this->m_file.is_open()) this->m_file.close();
    this->m_file.open(path, std::ios::out | std::ios::app);
    if (this->m_file.fail()) throw std::logic_error("cannot open the file at " + path);
    this->m_path = path;
}

// TBD
void LTF::Logger::rotate_log_file()
{
}

namespace LTF
{
// set path
#define LTF_LOG_INIT(path) LTF::Singleton<LTF::Logger>::instance().open(path);

// set level
#define LTF_LOG_LEVEL(input_level) LTF::Singleton<LTF::Logger>::instance().level(input_level);

// debug
#define LTF_DEBUG(message) LTF::Singleton<LTF::Logger>::instance().log(LTF::Logger::Level::DEBUG, message, LTF::Logger::Info(__FILE__, __LINE__));

// info
#define LTF_INFO(message) LTF::Singleton<LTF::Logger>::instance().log(LTF::Logger::Level::INFO, message, LTF::Logger::Info(__FILE__, __LINE__));

// warning
#define LTF_WARNING(message) LTF::Singleton<LTF::Logger>::instance().log(LTF::Logger::Level::WARNING, message, LTF::Logger::Info(__FILE__, __LINE__));

// error
#define LTF_ERROR(message) LTF::Singleton<LTF::Logger>::instance().log(LTF::Logger::Level::ERROR, message, LTF::Logger::Info(__FILE__, __LINE__));

// fatal
#define LTF_FATAL(message) LTF::Singleton<LTF::Logger>::instance().log(LTF::Logger::Level::FATAL, message, LTF::Logger::Info(__FILE__, __LINE__));
};
