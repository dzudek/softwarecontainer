/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */

#include <iostream>
#include <fstream>

#include "temperatureserviceinterface.h"

TemperatureServiceInterface::TemperatureServiceInterface(DBus::Connection &connection,
                                                         std::string logfilepath) :
    m_logfilepath(logfilepath),
    DBus::ObjectProxy(connection,
                      "/com/pelagicore/TemperatureService",
                      "com.pelagicore.TemperatureService")
{
}

std::string TemperatureServiceInterface::echo(const std::string &str)
{
    std::cout << str << std::endl;
    return str;
}

double TemperatureServiceInterface::getTemperature()
{
    return GetTemperature();
}

void TemperatureServiceInterface::setTemperature(const double &temperature)
{
    SetTemperature(temperature);
}

/*
 * On temperature changed signal write the change into logfile
 */
void TemperatureServiceInterface::TemperatureChanged(const double &temperature)
{
    std::ofstream logfile(m_logfilepath.c_str(), std::ofstream::app);
    if(logfile.is_open()) {
        logfile << __TIMESTAMP__ << " - Temperature changed to: "
                << temperature << "°C" << std::endl;
        logfile.close();
    } else {
        std::cerr << m_logfilepath << " could not be opened to write." << std::endl;
    }
}
