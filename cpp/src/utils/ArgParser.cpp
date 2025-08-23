#include <gtb/ArgParser.h>
#include <gtb/Variadic.h>

#include <algorithm>
#include <stdio.h>
#include <limits.h>

using namespace gtb;

Args::Args()
{
    bError = false;
}

bool Args::hasError() const
{
    return bError;
}

void Args::setError()
{
    bError = true;
}

bool Args::hasArg(const char *arg) const
{
    return args.find(arg) != args.end();
}

std::string Args::getArg(const char *arg) const
{
    auto iter = args.find(arg);
    return iter == args.end() ? std::string() : iter->second.back();
}

std::list<std::string> Args::getArgs(const char *arg) const
{
    auto iter = args.find(arg);
    return iter == args.end() ? std::list<std::string>() : iter->second;
}

int Args::getInt(const char *arg) const
{
    return atoi(getArg(arg).c_str());
}

unsigned int Args::getUInt(const char *arg) const
{
    size_t val = getSizeT(arg);
    return val > UINT_MAX ? UINT_MAX : static_cast<unsigned int>(val);
}

size_t Args::getSizeT(const char *arg) const
{
    size_t ret = 0;
    try {
        ret = std::stoul(getArg(arg));
    } catch (const std::exception &e) {
        fprintf(stderr, "Invalid unsigned integer '%s': %s\n", arg, e.what());
    }
    return ret;
}

bool Args::getBool(const char *arg) const
{
    if (!hasArg(arg))
        return false;

    std::string value = getArg(arg);
    if (atoi(value.c_str()) > 0 || value == "true" || value == "t")
        return true;

    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    return value == "true" || value == "t";
}

void Args::addArg(const std::string &arg, std::string value)
{
    args[arg].push_back(std::move(value));
}

void Args::addArg(const std::string &arg, const char *value)
{
    args[arg].push_back(value);
    originals[arg].push_back(value);
}

void Args::maskArg(const char *arg)
{
    auto iter = originals.find(arg);
    if (iter != originals.end())
    {
        for (const char *p : iter->second)
            const_cast<char &>(*p) = '\0';
    }
}

void ArgParser::addParam(
    char shortcut,
    const char *param,
    const char *description,
    ...)
{
    std::string formatted;
    VARIADIC_STRING(description, formatted);

    checkDuplicate(shortcut);
    checkDuplicate(param);

    order.push_back(param);
    shortArgs[shortcut] = param;
    descriptions[param] = formatted;
}

void ArgParser::addParam(
    const char *param,
    const char *description,
    ...)
{
    std::string formatted;
    VARIADIC_STRING(description, formatted);

    checkDuplicate(param);

    order.push_back(param);
    descriptions[param] = formatted;
}

void ArgParser::addSwitch(
    char shortcut,
    const char *flag,
    const char *description)
{
    checkDuplicate(shortcut);
    checkDuplicate(flag);

    order.push_back(flag);
    shortArgs[shortcut] = flag;
    descriptions[flag] = description;
    switches.insert(flag);
}

void ArgParser::addSwitch(
    const char *flag,
    const char *description)
{
    checkDuplicate(flag);

    order.push_back(flag);
    descriptions[flag] = description;
    switches.insert(flag);
}

Args ArgParser::parse(int argc, const char **argv)
{
    Args args;
    if (!argv)
        return args;

    int pos = 1;
    while (pos < argc)
    {
        // Find out what flag is next
        std::string tk = argv[pos++];
        if (tk.rfind("--", 0) == 0)
        {
            tk = tk.substr(2);
        }
        else if (tk.length() == 2 && tk.rfind("-", 0) == 0)
        {
            char shortcut = tk[1];
            auto iter = shortArgs.find(shortcut);
            if (iter == shortArgs.end())
            {
                fprintf(stderr, "Argument error. Unknown flag '%c'.\n", shortcut);
                continue;
            }

            tk = iter->second;
        }
        else
        {
            fprintf(stderr, "Argument error. Expected flag but received '%s'.\n", tk.c_str());
            continue;
        }

        // Handle --help specially
        if (tk == "help")
        {
            args.setError();
            continue;
        }

        // Is this a flag we know about?
        auto iter = descriptions.find(tk);
        if (iter == descriptions.end())
        {
            fprintf(stderr, "Unknown flag '%s'.\n", tk.c_str());
            args.setError();
            continue;
        }

        // No value if it is a switch
        bool bSwitch = switches.find(tk) != switches.end();
        if (bSwitch)
        {
            args.addArg(tk, "1");
            continue;
        }

        // Do we have a value?
        if (pos >= argc)
        {
            fprintf(stderr, "Expected value for flag '%s'.\n", tk.c_str());
            args.setError();
            continue;
        }

        // Add to args
        args.addArg(tk, argv[pos++]);
    }

    // Always show help on argument parse error
    if (args.hasError())
        printHelp();

    return args;
}

void ArgParser::addCategory(
    const char *category)
{
    order.push_back(category);
}

void ArgParser::printHelp()
{
    constexpr const int DESCRIPTION_INDENTION = 40;

    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\n");

    auto printDesc = [&](int &pos, const std::string &desc) -> void
    {
        // Indent
        while (pos < DESCRIPTION_INDENTION)
            pos += fprintf(stderr, " ");

        fprintf(stderr, "%s\n", desc.c_str());
        pos = 0;
    };

    fprintf(stderr, "General\n");
    int pos = fprintf(stderr, "    --help");
    printDesc(pos, "This help menu");

    for (const std::string &param : order)
    {
        // Category does not have separate description
        auto iterDesc = descriptions.find(param);
        if (iterDesc == descriptions.end())
        {
            fprintf(stderr, "\n%s\n", param.c_str());
            continue;
        }

        // See if there is a short character
        char shortcut = '\0';
        for (const auto &shortPair : shortArgs)
        {
            char curShortcut = shortPair.first;
            const std::string &curParam = shortPair.second;
            if (curParam == param)
            {
                shortcut = curShortcut;
                break;
            }
        }

        // Is this a switch?
        bool bSwitch = switches.find(param) != switches.end();

        pos = 0;
        pos += fprintf(stderr, "    ");
        if (shortcut != '\0')
            pos += fprintf(stderr, "-%c ", shortcut);
        pos += fprintf(stderr, "--%s", param.c_str());

        if (!bSwitch)
            pos += fprintf(stderr, " <Value>");

        // Description
        printDesc(pos, iterDesc->second);
    }

    fflush(stderr);
}

void ArgParser::checkDuplicate(
    char shortcut)
{
    // Programmer error
    if (shortArgs.find(shortcut) != shortArgs.end())
        fprintf(stderr, "Argument flag '%c' specified multiple times.\n", shortcut);
}

void ArgParser::checkDuplicate(
    const char *param)
{
    // Programmer error
    if (descriptions.find(param) != descriptions.end())
        fprintf(stderr, "Argument flag '%s' specified multiple times.\n", param);
}
