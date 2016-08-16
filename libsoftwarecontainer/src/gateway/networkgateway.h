/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once

#include "gateway.h"
#include "generators.h"

class NetworkGateway :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("NETG", "Network gateway");

public:
    static constexpr const char *ID = "network";

    static constexpr const char *BRIDGE_INTERFACE = "lxcbr0";

    NetworkGateway();
    ~NetworkGateway();

    ReturnCode readConfigElement(const JSonElement &element) override;

    /*!
     *  Implements Gateway::activateGateway
     */
    bool activateGateway() override;

    /*!
     * Implements Gateway::teardownGateway
     */
    bool teardownGateway() override;

    /*! Returns the IP of the container
     */
    const std::string ip();
private:
    /*! Generate IP address for the container
     *
     * Retrieves an IP from DHCP.
     *
     * Note that a file on the system acts as a placeholder for the DHCP server.
     * The file keeps track of the highest used IP address.
     *
     * \return true  Upon success
     * \return false Upon failure
     */
    bool generateIP();

    /*! Set route to default gateway
     *
     * Sets the route to the default gateway.
     * To be able to access anything outside the container, this method must be
     * called after the network interface has been enabled. This is also true for
     * cases when a network interface that was previously enabled has been disabled
     * and then enabled again.
     *
     * \return true  Upon success
     * \return false Upon failure
     */
    bool setDefaultGateway();

    /*! Enable the default network interface
     *
     * Enables the network interface and calls NetworkGateway::setDefaultGateway().
     *
     * When this is done for the first time, i.e. during the first call to activate()
     * the IP and netmask are also set. During subsequent calls, this merely brings
     * up the existing network interface and calls setDefaultGateway().
     *
     * \return true  Upon success
     * \return false Upon failure
     */
    bool up();

    /*! Disable the default network interface
     *
     * Disables the network interface.
     *
     * \return true  Upon success
     * \return false Upon failure
     */
    bool down();

    /*! Check the availability of the network bridge on the host
     *
     * Checks the availability of the required network bridge on the host.
     *
     * \return true  If bridge interface is available
     * \return false If bridge interface is not available
     */
    bool isBridgeAvailable();

    std::string m_ip;
    std::string m_gateway;
    bool m_internetAccess;
    bool m_interfaceInitialized;

    Generator m_generator;
};
