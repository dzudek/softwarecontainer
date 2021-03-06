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


#include "softwarecontainer.h"

#ifdef ENABLE_PULSEGATEWAY
#include "gateway/pulsegateway.h"
#endif

#ifdef ENABLE_NETWORKGATEWAY
#include "gateway/network/networkgateway.h"
#endif

#ifdef ENABLE_DBUSGATEWAY
#include "gateway/dbus/dbusgateway.h"
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
#include "gateway/devicenode/devicenodegateway.h"
#endif

#ifdef ENABLE_CGROUPSGATEWAY
#include "gateway/cgroups/cgroupsgateway.h"
#endif

#ifdef ENABLE_WAYLANDGATEWAY
#include "gateway/waylandgateway.h"
#endif

#ifdef ENABLE_ENVGATEWAY
#include "gateway/environment/envgateway.h"
#endif

#ifdef ENABLE_FILEGATEWAY
#include "gateway/files/filegateway.h"
#endif

namespace softwarecontainer {

SoftwareContainer::SoftwareContainer(std::shared_ptr<Workspace> workspace, const ContainerID id) :
    m_workspace(workspace),
    m_containerID(id),
    m_container(new Container("SC-" + std::to_string(id),
                              m_workspace->m_containerConfigPath,
                              m_workspace->m_containerRootDir,
                              m_workspace->m_enableWriteBuffer,
                              m_workspace->m_containerShutdownTimeout))
{
    m_containerState = ContainerState::CREATED;
}

SoftwareContainer::~SoftwareContainer()
{
}

void SoftwareContainer::setMainLoopContext(Glib::RefPtr<Glib::MainContext> mainLoopContext)
{
    m_mainLoopContext = mainLoopContext;
}

ReturnCode SoftwareContainer::preload()
{
    log_debug() << "Initializing container";
    if (isError(m_container->initialize())) {
        log_error() << "Could not setup container for preloading";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Creating container";
    if (isError(m_container->create())) {
        return ReturnCode::FAILURE;
    }

    log_debug() << "Starting container";
    ReturnCode result = m_container->start(&m_pcPid);
    if (isError(result)) {
        log_error() << "Could not start the container during preload";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Started container with PID " << m_pcPid;
    m_containerState.setValueNotify(ContainerState::PRELOADED);
    return ReturnCode::SUCCESS;
}

ReturnCode SoftwareContainer::init()
{
    if (m_mainLoopContext->gobj() == nullptr) {
        log_error() << "Main loop context must be set first !";
        return ReturnCode::FAILURE;
    }

    // TODO: Rename this... preloading is a bit wierd name
    if (getContainerState() != ContainerState::PRELOADED) {
        if (isError(preload())) {
            log_error() << "Failed to preload container";
            return ReturnCode::FAILURE;
        }
    }

#ifdef ENABLE_NETWORKGATEWAY
    try {
        addGateway(new NetworkGateway(m_containerID, "10.0.3.1", 16));
    } catch (ReturnCode failure) {
        log_error() << "Given netmask is not appropriate for creating ip address."
                    << "It should be an unsigned value between 1 and 31";
        return failure;
    }
#endif

#ifdef ENABLE_PULSEGATEWAY
    addGateway(new PulseGateway());
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
    addGateway(new DeviceNodeGateway());
#endif

#ifdef ENABLE_DBUSGATEWAY
    std::string containerID = std::string(m_container->id());
    addGateway(new DBusGateway( getGatewayDir(), containerID ));
#endif

#ifdef ENABLE_CGROUPSGATEWAY
    addGateway(new CgroupsGateway());
#endif

#ifdef ENABLE_WAYLANDGATEWAY
    addGateway(new WaylandGateway());
#endif

#ifdef ENABLE_ENVGATEWAY
    addGateway(new EnvironmentGateway());
#endif

#ifdef ENABLE_FILEGATEWAY
    addGateway(new FileGateway());
#endif

    m_initialized = true;
    return ReturnCode::SUCCESS;
}

void SoftwareContainer::addGateway(Gateway *gateway)
{
    gateway->setContainer(m_container);
    m_gateways.push_back(std::move(std::unique_ptr<Gateway>(gateway)));
}

ReturnCode SoftwareContainer::startGateways(const GatewayConfiguration &gwConfig)
{
    ReturnCode result = ReturnCode::SUCCESS;

    result = configureGateways(gwConfig);
    if (isError(result)) {
        return result;
    }

    result = activateGateways();
    if (isError(result)) {
        return result;
    }

    m_containerState.setValueNotify(ContainerState::READY);

    return result;
}

ReturnCode SoftwareContainer::configureGateways(const GatewayConfiguration &gwConfig)
{
    for (auto &gateway : m_gateways) {
        std::string gatewayId = gateway->id();

        json_t *config = gwConfig.config(gatewayId);
        if (config != nullptr) {
            std::string configStr = json_dumps(config,0);
            log_debug() << "Configuring gateway \""
                        << gatewayId
                        << "\" with config: "
                        << configStr;
            try {
                ReturnCode configurationResult = gateway->setConfig(configStr);
                if (isError(configurationResult)) {
                    log_error() << "Failed to apply gateway configuration: " << configStr;
                    return configurationResult;
                }
            } catch (GatewayError &error) {
                /*
                 * Exceptions in gateways during configuration are fatal errors for the whole of SC
                 * as it means one or more capabilities are broken.
                 */
                log_error() << "Fatal error when configuring gateway \""
                            << gatewayId
                            << "\" : "
                            << error.what();
                throw error;
            }
            json_decref(config);
        }
    }

    return ReturnCode::SUCCESS;
}

ReturnCode SoftwareContainer::activateGateways()
{
    for (auto &gateway : m_gateways) {
        std::string gatewayId = gateway->id();

        try {
            if (gateway->isConfigured()) {
                ReturnCode activationResult = gateway->activate();
                if (isError(activationResult)) {
                    log_error() << "Failed to activate gateway \""
                                << gatewayId
                                << "\"";
                    return activationResult;
                }
            }
        } catch (GatewayError &error) {
            /*
             * Exceptions in gateways during activation are fatal errors for the whole of SC
             * as it means one or more gateways will not be active in the way one or more
             * capabilities implies, i.e. the application environment will be in a broken state.
             */
            log_error() << "Fatal error when activating gateway \""
                        << gatewayId
                        << "\" : "
                        << error.what();
            throw error;
        }
    }

    return ReturnCode::SUCCESS;
}

ReturnCode SoftwareContainer::shutdown()
{
    return shutdown(m_workspace->m_containerShutdownTimeout);
}

ReturnCode SoftwareContainer::shutdown(unsigned int timeout)
{
    log_debug() << "shutdown called"; // << logging::getStackTrace();
    if(isError(shutdownGateways())) {
        log_error() << "Could not shut down all gateways cleanly, check the log";
    }

    if(isError(m_container->destroy(timeout))) {
        log_error() << "Could not destroy the container during shutdown";
        return ReturnCode::FAILURE;
    }

    m_containerState.setValueNotify(ContainerState::TERMINATED);
    return ReturnCode::SUCCESS;
}

ReturnCode SoftwareContainer::suspend()
{
    return m_container->suspend();
}

ReturnCode SoftwareContainer::resume()
{
    return m_container->resume();
}


ReturnCode SoftwareContainer::shutdownGateways()
{
    ReturnCode status = ReturnCode::SUCCESS;
    for (auto &gateway : m_gateways) {
        if (gateway->isActivated()) {
            if (isError(gateway->teardown())) {
                log_warning() << "Could not tear down gateway cleanly: " << gateway->id();
                status = ReturnCode::FAILURE;
            }
        }
    }

    m_gateways.clear();
    return status;
}

bool SoftwareContainer::isInitialized() const
{
    return m_initialized;
}

std::shared_ptr<ContainerAbstractInterface> SoftwareContainer::getContainer()
{
    std::shared_ptr<ContainerAbstractInterface> ptrCopy = m_container;
    return ptrCopy;
}

std::string SoftwareContainer::getContainerDir()
{
    const std::string containerID = std::string(m_container->id());
    return m_workspace->m_containerRootDir + "/" + containerID;
}

std::string SoftwareContainer::getGatewayDir()
{
    return getContainerDir() + "/gateways";
}

ObservableProperty<ContainerState> &SoftwareContainer::getContainerState()
{
    return m_containerState;
}

} // namespace softwarecontainer
