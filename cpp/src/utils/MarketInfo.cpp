#include <gtb/MarketInfo.h>
#include <gtb/Log.h>

#include <ctime>
#include <math.h>
#include <stdint.h>

using namespace gtb;

namespace
{

// -----------------------------------------------------------------------------
// Helper functions for manual DST and time conversion using US rules.
// We assume input "time" is in microseconds since Unix epoch (UTC).
// We use gmtime() only to break down UTC (which is safe) and then adjust manually.
// -----------------------------------------------------------------------------

// Compute weekday for a given date (year, month, day) in the Gregorian calendar.
// Returns: 0=Sunday, 1=Monday, …, 6=Saturday.
// This uses Zeller’s Congruence.
int getWeekday(int year, int month, int day)
{
    if (month < 3) {
        month += 12;
        year -= 1;
    }
    int K = year % 100;
    int J = year / 100;
    // h: 0=Saturday, 1=Sunday, …, 6=Friday.
    int h = (day + (13*(month+1))/5 + K + K/4 + J/4 + 5*J) % 7;
    // Convert so that 0 = Sunday.
    int weekday = (h + 6) % 7;
    return weekday;
}

// For a given year, compute the day (of month) of the second Sunday in March.
int secondSundayInMarch(int year)
{
    // March 1
    int w = getWeekday(year, 3, 1);
    int firstSunday = (w == 0) ? 1 : (8 - w);
    return firstSunday + 7;
}

// For a given year, compute the day (of month) of the first Sunday in November.
int firstSundayInNovember(int year)
{
    int w = getWeekday(year, 11, 1);
    return (w == 0) ? 1 : (8 - w);
}

// Given a UTC time (in seconds), compute the Eastern offset in seconds.
// Returns -14400 (UTC-4) if DST is in effect; otherwise, -18000 (UTC-5).
int getEasternOffset(uint64_t utcSecs, bool &inDST)
{
    // Break down the UTC time.
    std::tm utc_tm;
    time_t utcSecsT = static_cast<time_t>(utcSecs);
    gmtime_r(&utcSecsT, &utc_tm);
    int year = utc_tm.tm_year + 1900;

    // Compute candidate Eastern time assuming standard offset (UTC-5).
    std::tm cand;
    uint64_t candidate = static_cast<uint64_t>(utcSecs - 5 * 3600);
    time_t candidateT = static_cast<time_t>(candidate);
    gmtime_r(&candidateT, &cand);

    // Compute the DST boundaries for this year.
    int secondSundayMarch = secondSundayInMarch(year);
    int firstSundayNovember = firstSundayInNovember(year);

    // Use the candidate's local date (as if Eastern standard time) to determine DST.
    // DST is in effect if the candidate's date is:
    // - After March (i.e. months 4 through 10), OR
    // - In March, if it is after the second Sunday, or on the second Sunday after 2:00 AM, OR
    // - In November, if it is before the first Sunday, or on the first Sunday before 2:00 AM.
    int month = cand.tm_mon + 1; // tm_mon is 0-based; month in 1..12
    if (month > 3 && month < 11)
    {
        inDST = true;
    }
    else if (month == 3)
    {
        if (cand.tm_mday > secondSundayMarch)
            inDST = true;
        else if (cand.tm_mday == secondSundayMarch && cand.tm_hour >= 2)
            inDST = true;
    }
    else if (month == 11)
    {
        if (cand.tm_mday < firstSundayNovember)
            inDST = true;
        else if (cand.tm_mday == firstSundayNovember && cand.tm_hour < 2)
            inDST = true;
    }

    return inDST ? -4 * 3600 : -5 * 3600;
}

// Convert a UTC time (in microseconds) to Eastern local time
std::tm toEastern(uint64_t utcMicros)
{
    uint64_t utcSecs = utcMicros / 1000000;
    bool inDST = false;
    int offset = getEasternOffset(utcSecs, inDST);
    uint64_t localMicros = utcMicros + static_cast<uint64_t>(offset) * 1000000;

    // Given a time in microseconds (in local time), build a std::tm using gmtime() on seconds.
    // (We assume the provided time is already “local” in the chosen zone.)
    std::tm tmLocal;
    uint64_t secs = localMicros / 1000000;
    time_t secsT = static_cast<time_t>(secs);
    gmtime_r(&secsT, &tmLocal);
    tmLocal.tm_isdst = inDST;
    return tmLocal;
}

uint64_t fromEastern(const std::tm &tm)
{
    uint64_t localSecs = static_cast<uint64_t>(timegm(const_cast<std::tm *>(&tm)));
    uint64_t utcSecs = localSecs + (tm.tm_isdst ? 4 : 5) * 3600;
    return utcSecs * 1'000'000;
}

// Convert microseconds to minutes since midnight, given a broken-down time (provided as a std::tm).
uint64_t minutesSinceMidnight(const std::tm &tmVal)
{
    return static_cast<uint64_t>(tmVal.tm_hour * 60 + tmVal.tm_min);
}

// Add days to a microseconds timestamp (local time). For simplicity, add days in seconds.
uint64_t addDays(uint64_t localMicros, int days)
{
    // Compute absolute offset as an unsigned 64-bit value.
    unsigned long long offset = (days >= 0
                        ? static_cast<unsigned long long>(days)
                        : static_cast<unsigned long long>(-days))
                      * 86400ULL * 1000000ULL;
    return (days >= 0 ? localMicros + offset : localMicros - offset);
}

// For these functions we compute "next open" or "next close" manually using Eastern local times.
uint64_t nextStockOpen(uint64_t utcMicros)
{
    std::tm tmLocal = toEastern(utcMicros);
    uint64_t mins = minutesSinceMidnight(tmLocal);
    uint64_t openMins = 9 * 60 + 30;
    // If before today's open and it's a weekday.
    if (mins < openMins && tmLocal.tm_wday >= 1 && tmLocal.tm_wday <= 5)
    {
        tmLocal.tm_hour = 9;
        tmLocal.tm_min = 30;
        tmLocal.tm_sec = 0;
        return fromEastern(tmLocal);
    }

    // Otherwise, find the next weekday open.
    // Add one day until weekday between Monday and Friday.
    utcMicros = addDays(utcMicros, 1);
    tmLocal = toEastern(utcMicros);
    while (tmLocal.tm_wday == 0 || tmLocal.tm_wday == 6)
    {
        utcMicros = addDays(utcMicros, 1);
        tmLocal = toEastern(utcMicros);
    }
    tmLocal.tm_hour = 9;
    tmLocal.tm_min = 30;
    tmLocal.tm_sec = 0;
    return fromEastern(tmLocal);
}

uint64_t nextStockClose(uint64_t utcMicros)
{
    std::tm tmLocal = toEastern(utcMicros);
    uint64_t mins = minutesSinceMidnight(tmLocal);
    uint64_t closeMins = 16 * 60;
    if (mins < closeMins && tmLocal.tm_wday >= 1 && tmLocal.tm_wday <= 5)
    {
        tmLocal.tm_hour = 16;
        tmLocal.tm_min = 0;
        tmLocal.tm_sec = 0;
        return fromEastern(tmLocal);
    }

    // Otherwise, next close is tomorrow's 16:00 on a weekday.
    utcMicros = addDays(utcMicros, 1);
    tmLocal = toEastern(utcMicros);
    while (tmLocal.tm_wday == 0 || tmLocal.tm_wday == 6)
    {
        utcMicros = addDays(utcMicros, 1);
        tmLocal = toEastern(utcMicros);
    }

    tmLocal.tm_hour = 16;
    tmLocal.tm_min = 0;
    tmLocal.tm_sec = 0;
    return fromEastern(tmLocal);
}

uint64_t nextBFOpen(uint64_t utcMicros)
{
    // For Bitcoin Futures: if current Eastern local time is in a closed period,
    // next open is Sunday 18:00 if in weekend closure; if in daily maintenance, next open is today at 18:00.
    std::tm tmLocal = toEastern(utcMicros);
    uint64_t mins = minutesSinceMidnight(tmLocal);

    // If daily maintenance closure (17:00-18:00) on a weekday.
    if (tmLocal.tm_wday >= 1 && tmLocal.tm_wday <= 4 && mins >= 17*60 && mins < 18*60)
    {
        tmLocal.tm_hour = 18;
        tmLocal.tm_min = 0;
        tmLocal.tm_sec = 0;
        return fromEastern(tmLocal);
    }
    // If Friday after 17:00, or Saturday, or Sunday before 18:00:
    if ((tmLocal.tm_wday == 5 && mins >= 17*60) || tmLocal.tm_wday == 6 ||
        (tmLocal.tm_wday == 0 && mins < 18*60))
    {
        // Find next Sunday:
        while (tmLocal.tm_wday != 0)
        {
            utcMicros = addDays(utcMicros, 1);
            tmLocal = toEastern(utcMicros);
        }

        tmLocal.tm_hour = 18;
        tmLocal.tm_min = 0;
        tmLocal.tm_sec = 0;
        return fromEastern(tmLocal);
    }
    // Otherwise, assume currently open.
    return utcMicros;
}

uint64_t nextBFClose(uint64_t utcMicros)
{
    // For Bitcoin Futures: if before 17:00 on an open weekday, next close is today at 17:00.
    std::tm tmLocal = toEastern(utcMicros);
    uint64_t mins = minutesSinceMidnight(tmLocal);
    if (mins < 17*60 && tmLocal.tm_wday >= 1 && tmLocal.tm_wday <= 4)
    {
        tmLocal.tm_hour = 17;
        tmLocal.tm_min = 0;
        tmLocal.tm_sec = 0;
        return fromEastern(tmLocal);
    }
    // Otherwise, if after 18:00 on a weekday, next close is tomorrow at 17:00 (skipping weekend if needed)
    utcMicros = addDays(utcMicros, 1);
    tmLocal = toEastern(utcMicros);
    while (tmLocal.tm_wday == 0 || tmLocal.tm_wday == 6)
    {
        utcMicros = addDays(utcMicros, 1);
        tmLocal = toEastern(utcMicros);
    }
    tmLocal.tm_hour = 17;
    tmLocal.tm_min = 0;
    tmLocal.tm_sec = 0;
    return fromEastern(tmLocal);
}

}

MarketInfo::MarketInfo(
    Market market,
    uint64_t time)
        : market(market)
        , time(time)
{
}

bool MarketInfo::isOpen() const
{
    if (market == Market::None)
        return false;

    if (market == Market::StockMarket)
    {
        // Stock market: Monday-Friday, open 9:30-16:00 Eastern.
        std::tm tmLocal = toEastern(time);

        // tm_wday: 0 = Sunday, 6 = Saturday.
        if (tmLocal.tm_wday == 0 || tmLocal.tm_wday == 6)
            return false;

        uint64_t mins = minutesSinceMidnight(tmLocal);
        uint64_t openMins = 9 * 60 + 30;
        uint64_t closeMins = 16 * 60;
        return (mins >= openMins && mins < closeMins);
    }
    else if (market == Market::BitcoinFutures)
    {
        // Bitcoin Futures (Coinbase Nano):
        // Daily: Open Sunday–Friday from 6:00 PM to 5:00 PM Eastern,
        // with daily maintenance closure 5:00-6:00 PM.
        // Weekend closure: closed Friday 5:00PM until Sunday 6:00PM.
        std::tm tmLocal = toEastern(time);
        uint64_t mins = minutesSinceMidnight(tmLocal);

        // Check weekend closure.
        // tm_wday: 0 = Sunday, 5 = Friday, 6 = Saturday.
        if (tmLocal.tm_wday == 6)
            return false;
        if (tmLocal.tm_wday == 0 && mins < (18*60))
            return false;
        if (tmLocal.tm_wday == 5 && mins >= (17*60))
            return false;
        // Daily maintenance: closed from 17:00 to 18:00.
        if (mins >= 17*60 && mins < 18*60)
            return false;
        return true;
    }

    return false;
}

bool MarketInfo::isClosed() const
{
    return !isOpen();
}

bool MarketInfo::isWeekend() const
{
    if (market == Market::None)
        return false;

    // For both markets, convert UTC time to Eastern local time manually.
    std::tm tmLocal = toEastern(time);
    uint64_t mins = minutesSinceMidnight(tmLocal);

    if (market == Market::StockMarket)
    {
        // Stock Market is closed Saturday and Sunday,
        // and is considered in the weekend if it is after close on Friday (>= 16:00)
        // or before open on Monday (< 9:30).
        if (tmLocal.tm_wday == 6 || tmLocal.tm_wday == 0)
            return true;
        if (tmLocal.tm_wday == 5 && mins >= (16 * 60))
            return true;
        return false;
    }
    else if (market == Market::BitcoinFutures)
    {
        // Bitcoin Futures (Coinbase Nano) are closed from Friday 17:00 until Sunday 18:00.
        if (tmLocal.tm_wday == 5 && mins >= (17 * 60))
            return true;
        if (tmLocal.tm_wday == 6)
            return true;
        if (tmLocal.tm_wday == 0 && mins < (18 * 60))
            return true;
        return false;
    }

    return false;
}

bool MarketInfo::isWeekendNext() const
{
    if (market == Market::None)
        return false;

    if (market == Market::StockMarket)
    {
        // For the stock market, if the market is currently open
        // and today is Friday, then the next closing (today at 16:00)
        // will lead to the weekend closure.
        std::tm tmLocal = toEastern(time);
        uint64_t mins = minutesSinceMidnight(tmLocal);
        // Check that today is a weekday (Mon-Fri) and currently open:
        if (tmLocal.tm_wday >= 1 && tmLocal.tm_wday <= 5)
        {
            uint64_t openMins = 9 * 60 + 30;
            uint64_t closeMins = 16 * 60;
            if (mins >= openMins && mins < closeMins)
            {
                // If today is Friday, the next close is at 16:00 on Friday, which is weekend.
                if (tmLocal.tm_wday == 5)
                    return true;
            }
        }

        return false;
    }
    else if (market == Market::BitcoinFutures)
    {
        // For Bitcoin futures, if the market is open now but the next closing time
        // (computed by nextBFClose()) falls into the long weekend period, then return true.
        uint64_t nextClose = nextBFClose(time);
        std::tm tmClose = toEastern(nextClose);
        uint64_t closeMins = minutesSinceMidnight(tmClose);
        if (tmClose.tm_wday == 5 && closeMins >= (17 * 60))
            return true;
        if (tmClose.tm_wday == 6)
            return true;
        if (tmClose.tm_wday == 0 && closeMins < (18 * 60))
            return true;
        return false;
    }

    return false;
}

uint64_t MarketInfo::tillOpen() const
{
    if (market == Market::None)
        return 0;
    if (isOpen())
        return 0;

    uint64_t candidate = 0;
    if (market == Market::StockMarket)
        candidate = nextStockOpen(time);
    else if (market == Market::BitcoinFutures)
        candidate = nextBFOpen(time);

    return (candidate > time ? candidate - time : 0);
}

uint64_t MarketInfo::tillClosed() const
{
    if (market == Market::None)
        return 0;
    if (!isOpen())
        return 0;

    uint64_t candidate = 0;
    if (market == Market::StockMarket)
        candidate = nextStockClose(time);
    else if (market == Market::BitcoinFutures)
        candidate = nextBFClose(time);

    return (candidate > time ? candidate - time : 0);
}

uint64_t MarketInfo::sinceOpen() const
{
    if (market == Market::None)
        return 0;
    if (!isOpen())
        return 0;

    // For simplicity, assume the open time today (or last open for BitcoinFutures).
    uint64_t openTime = 0;
    if (market == Market::StockMarket)
    {
        std::tm tmLocal = toEastern(time);
        uint64_t mins = minutesSinceMidnight(tmLocal);
        uint64_t openMins = 9*60 + 30;
        if (mins >= openMins)
        {
            tmLocal.tm_hour = 9;
            tmLocal.tm_min = 30;
            tmLocal.tm_sec = 0;
            openTime = fromEastern(tmLocal);
        }
        else
        {
            // Should not happen if isOpen() is true.
            openTime = nextStockOpen(time);
        }
    }
    else if (market == Market::BitcoinFutures)
    {
        std::tm tmLocal = toEastern(time);
        uint64_t mins = minutesSinceMidnight(tmLocal);
        if (mins >= 18*60)
        {
            tmLocal.tm_hour = 18;
            tmLocal.tm_min = 0;
            tmLocal.tm_sec = 0;
            openTime = fromEastern(tmLocal);
        }
        else
        {
            // Before 17:00: assume open was yesterday at 18:00.
            uint64_t prev = addDays(time, -1);
            std::tm tprev = toEastern(prev);
            tprev.tm_hour = 18;
            tprev.tm_min = 0;
            tprev.tm_sec = 0;
            openTime = fromEastern(tprev);
        }
    }

    return (time > openTime ? time - openTime : 0);
}

uint64_t MarketInfo::sinceClosed() const
{
    if (market == Market::None)
        return 0;
    if (isOpen())
        return 0;

    // For simplicity, for stock market: if before 9:30, last close was previous day at 16:00;
    // for Bitcoin futures: if in maintenance or weekend, last close is defined accordingly.
    uint64_t closeTime = 0;
    if (market == Market::StockMarket)
    {
        std::tm tmLocal = toEastern(time);
        uint64_t mins = minutesSinceMidnight(tmLocal);
        uint64_t openMins = 9*60+30;
        if (mins < openMins)
        {
            // last close was yesterday at 16:00.
            uint64_t prev = addDays(time, -1);
            std::tm tprev = toEastern(prev);
            tprev.tm_hour = 16;
            tprev.tm_min = 0;
            tprev.tm_sec = 0;
            closeTime = fromEastern(tprev);
        }
        else
        {
            // after close today.
            tmLocal.tm_hour = 16;
            tmLocal.tm_min = 0;
            tmLocal.tm_sec = 0;
            closeTime = fromEastern(tmLocal);
        }
    }
    else if (market == Market::BitcoinFutures)
    {
        std::tm tmLocal = toEastern(time);
        uint64_t mins = minutesSinceMidnight(tmLocal);
        if ((tmLocal.tm_wday == 5 && mins >= 17*60) ||
           tmLocal.tm_wday == 6 ||
           (tmLocal.tm_wday == 0 && mins < 18*60))
        {
            // Last close is Friday 17:00.
            // For simplicity, subtract days until Friday.
            uint64_t temp = time;
            std::tm ttemp = toEastern(temp);
            while (ttemp.tm_wday != 5)
            {
                temp = addDays(temp, -1);
                ttemp = toEastern(temp);
            }
            ttemp.tm_hour = 17;
            ttemp.tm_min = 0;
            ttemp.tm_sec = 0;
            closeTime = fromEastern(ttemp);
        }
        else if (mins >= 17*60 && mins < 18*60)
        {
            tmLocal.tm_hour = 17;
            tmLocal.tm_min = 0;
            tmLocal.tm_sec = 0;
            closeTime = fromEastern(tmLocal);
        }
        else if (mins < 17*60)
        {
            // Last close was yesterday at 17:00.
            uint64_t prev = addDays(time, -1);
            std::tm tprev = toEastern(prev);
            tprev.tm_hour = 17;
            tprev.tm_min = 0;
            tprev.tm_sec = 0;
            closeTime = fromEastern(tprev);
        }
        else
        {
            tmLocal.tm_hour = 17;
            tmLocal.tm_min = 0;
            tmLocal.tm_sec = 0;
            closeTime = fromEastern(tmLocal);
        }
    }

    return (time > closeTime ? time - closeTime : 0);
}

std::string MarketInfo::getTimeString(
    uint64_t time)
{
    std::tm tmLocal = toEastern(time);

    const char *dow = nullptr;
    switch (tmLocal.tm_wday)
    {
        case 0: dow = "Sun"; break;
        case 1: dow = "Mon"; break;
        case 2: dow = "Tue"; break;
        case 3: dow = "Wed"; break;
        case 4: dow = "Thu"; break;
        case 5: dow = "Fri"; break;
        case 6: dow = "Sat"; break;
        default:
            break;
    }

    char szTime[128] = {};
    snprintf(szTime, sizeof(szTime),
        "%04d-%02d-%02d %02d:%02d:%02d (%s)",
        1900 + tmLocal.tm_year,
        1 + tmLocal.tm_mon,
        tmLocal.tm_mday,
        tmLocal.tm_hour,
        tmLocal.tm_min,
        tmLocal.tm_sec,
        dow);
    return szTime;
}

std::string gtb::to_string(MarketInfo::Market e)
{
    switch (e)
    {
        case MarketInfo::Market::StockMarket: return "Stock";
        case MarketInfo::Market::BitcoinFutures: return "Bitcoin";
        default:
        case MarketInfo::Market::None:
            break;
    }

    return "None";
}
