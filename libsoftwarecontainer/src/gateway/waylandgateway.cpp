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

#include <string>
#include <unistd.h>
#include "waylandgateway.h"

namespace softwarecontainer {

// These lines are needed in order to define the fields, which otherwise would
// yield linker errors.
constexpr const char *WaylandGateway::ENABLED_FIELD;
constexpr const char *WaylandGateway::SOCKET_FILE_NAME;
constexpr const char *WaylandGateway::WAYLAND_RUNTIME_DIR_VARIABLE_NAME;

WaylandGateway::WaylandGateway() :
    Gateway(ID),
    m_enabled(false)
{
}

WaylandGateway::~WaylandGateway()
{
}

ReturnCode WaylandGateway::readConfigElement(const json_t *element)
{
    bool configValue = false;

    if (!JSONParser::read(element, ENABLED_FIELD, configValue)) {
        log_error() << "Key " << ENABLED_FIELD << " missing or not bool in json configuration";
        return ReturnCode::FAILURE;
    }

    if (!m_enabled) {
        m_enabled = configValue;
    }

    return ReturnCode::SUCCESS;
}

bool WaylandGateway::activateGateway()
{
    if (!m_enabled) {
        log_info() << "Wayland gateway disabled";
        return true;
    }

    bool hasWayland = false;
    std::string dir = Glib::getenv(WAYLAND_RUNTIME_DIR_VARIABLE_NAME, hasWayland);
    if (!hasWayland) {
        log_error() << "Should enable wayland gateway, but " << WAYLAND_RUNTIME_DIR_VARIABLE_NAME << " is not defined";
        return false;
    }

    log_info() << "enabling Wayland gateway. Socket dir:" << dir;
    std::string pathOnHost = logging::StringBuilder() << dir << "/" << SOCKET_FILE_NAME;
    std::string pathInContainer = logging::StringBuilder() << "/gateways/" << SOCKET_FILE_NAME;
    ReturnCode result = getContainer()->bindMountFileInContainer(pathOnHost, pathInContainer, false);

    if (isError(result)) {
        log_error() << "Could not bind mount the wayland socket into the container";
        return false;
    }

    setEnvironmentVariable(WAYLAND_RUNTIME_DIR_VARIABLE_NAME, parentPath(pathInContainer));

    return true;
}

bool WaylandGateway::teardownGateway()
{
    return true;
}

} // namespace softwarecontainer
