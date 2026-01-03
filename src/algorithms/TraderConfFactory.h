#pragma once

#include <gtb/HodlTrader.h>
#include <gtb/MomentumTrader.h>
#include <gtb/MovingAverageTrader.h>
#include <gtb/SpreadTrader.h>
#include <gtb/TimeTrader.h>
#include <gtb/VolumeTrader.h>
#include <gtb/WindowTrader.h>

namespace gtb
{

/**
 * Named configuration sets for traders
 */
namespace TraderConfFactory
{
    namespace Window
    {
        WindowTrader::Config quick();
        WindowTrader::Config core();
        WindowTrader::Config drift();
    }

    namespace Moving
    {
        MovingAverageTrader::Config medium();
    }

    namespace Momentum
    {
        MomentumTrader::Config swing();
    }

    namespace Volume
    {
        VolumeTrader::Config churn();
        VolumeTrader::Config stockChurn();
        VolumeTrader::Config drip();
    }

    namespace Hodl
    {
        HodlTrader::Config hodl();
    }

    namespace Spread
    {
        SpreadTrader::Config breakEven();
        SpreadTrader::Config small();
        SpreadTrader::Config medium();
        SpreadTrader::Config large();
        SpreadTrader::Config allHoursProbe();
    }

    namespace Time
    {
        TimeTrader::Config small();
        TimeTrader::Config medium();
        TimeTrader::Config large();
    }
}

}
