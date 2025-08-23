#pragma once

#include <map>
#include <set>
#include <list>
#include <string>

namespace gtb
{

class ArgParser;

/**
 * Parsed command line arguments
 */
class Args
{
    public:
        Args();
        Args(Args &&) = default;
        Args(const Args &) = default;
        Args &operator=(Args &&) = default;
        Args &operator=(const Args &) = default;
        ~Args() = default;

        bool hasArg(const char *arg) const;

        std::string getArg(const char *arg) const;
        std::list<std::string> getArgs(const char *arg) const;

        int getInt(const char *arg) const;
        unsigned int getUInt(const char *arg) const;
        size_t getSizeT(const char *arg) const;

        bool getBool(const char *arg) const;

        bool hasError() const;

        void maskArg(const char *arg);

    private:
        friend class ArgParser;

        void addArg(const std::string &arg, std::string value);
        void addArg(const std::string &arg, const char *value);
        void setError();

        bool bError;
        std::map<std::string, std::list<std::string>> args;
        std::map<std::string, std::list<const char *>> originals;
};

/**
 * Parse command line arguments from stdargs
 */
class ArgParser
{
    public:
        ArgParser() = default;
        ArgParser(ArgParser &&) = default;
        ArgParser(const ArgParser &) = default;
        ArgParser &operator=(ArgParser &&) = default;
        ArgParser &operator=(const ArgParser &) = default;
        ~ArgParser() = default;

        void addParam(
            char shortcut,
            const char *param,
            const char *description,
            ...)
            __attribute__((format(printf, 4, 5)));

        void addParam(
            const char *param,
            const char *description,
            ...)
            __attribute__((format(printf, 3, 4)));

        void addSwitch(
            char shortcut,
            const char *flag,
            const char *description);

        void addSwitch(
            const char *flag,
            const char *description);

        void addCategory(
            const char *category);

        // Parse
        Args parse(int argc, const char **argv);

        void printHelp();

    private:
        void checkDuplicate(char);
        void checkDuplicate(const char *);

        std::list<std::string> order;
        std::map<char, std::string> shortArgs;
        std::map<std::string, std::string> descriptions;
        std::set<std::string> switches;
};

}
