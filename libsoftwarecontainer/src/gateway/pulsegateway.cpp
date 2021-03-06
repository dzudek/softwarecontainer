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

#include "pulsegateway.h"

namespace softwarecontainer {

PulseGateway::PulseGateway() :
    Gateway(ID),
    m_enableAudio(false)
{
}

PulseGateway::~PulseGateway()
{
}

ReturnCode PulseGateway::readConfigElement(const json_t *element)
{
    bool configValue = false;

    if (!JSONParser::read(element, "audio", configValue)) {
        log_error() << "Either \"audio\" key is missing, or not a bool in json configuration";
        return ReturnCode::FAILURE;
    }

    if (!m_enableAudio) {
        m_enableAudio = configValue;
    }

    return ReturnCode::SUCCESS;
}

ReturnCode PulseGateway::enablePulseAudio() {
    bool hasPulse = false;
    std::string dir = Glib::getenv(PULSE_AUDIO_SERVER_ENVIRONMENT_VARIABLE_NAME, hasPulse);

    if (!hasPulse) {
        log_error() << "Should enable pulseaudio gateway, but "
                    << std::string(PULSE_AUDIO_SERVER_ENVIRONMENT_VARIABLE_NAME) << " is not defined";
        return ReturnCode::FAILURE;
    }

    log_info() << "enabling pulseaudio gateway. Socket location : " << dir;
    std::string pathInContainer = "/gateways/" + std::string(SOCKET_FILE_NAME);
    ReturnCode result = getContainer()->bindMountFileInContainer(std::string(dir), pathInContainer, false);

    if (isError(result)) {
        log_error() << "Could not bind mount pulseaudio socket in container";
        return ReturnCode::FAILURE;
    }

    std::string unixPath = "unix:" + pathInContainer;
    setEnvironmentVariable(PULSE_AUDIO_SERVER_ENVIRONMENT_VARIABLE_NAME, unixPath);
    return ReturnCode::SUCCESS;
}

bool PulseGateway::activateGateway()
{
    if (!m_enableAudio) {
        log_debug() << "Audio will be disabled";
        return true;
    }
    log_debug() << "Audio will be enabled";

    return isSuccess(enablePulseAudio());
}

bool PulseGateway::teardownGateway()
{
    return true;
}

} // namespace softwarecontainer
