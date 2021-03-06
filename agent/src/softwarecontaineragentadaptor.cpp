#include "softwarecontaineragentadaptor.h"

namespace softwarecontainer {

SoftwareContainerAgentAdaptor::~SoftwareContainerAgentAdaptor()
{
}

SoftwareContainerAgentAdaptor::SoftwareContainerAgentAdaptor(::softwarecontainer::SoftwareContainerAgent &agent, bool useSessionBus) :
    com::pelagicore::SoftwareContainerAgent(), m_agent(agent)
{
    std::string agentBusName = "com.pelagicore.SoftwareContainerAgent";
    Gio::DBus::BusType busType = useSessionBus ? Gio::DBus::BUS_TYPE_SESSION
                                               : Gio::DBus::BUS_TYPE_SYSTEM;
    connect(busType, agentBusName);
}

void SoftwareContainerAgentAdaptor::List(SoftwareContainerAgentMessageHelper msg)
{
    std::vector<int> list = m_agent.listContainers();
    msg.ret(list);
}

void SoftwareContainerAgentAdaptor::ListCapabilities(SoftwareContainerAgentMessageHelper msg)
{
    std::vector<std::string> stdStrVec = m_agent.listCapabilities();
    std::vector<Glib::ustring> glibStrVec = SoftwareContainerAgentCommon::stdStringVecToGlibStringVec(stdStrVec);
    msg.ret(glibStrVec);
}

void SoftwareContainerAgentAdaptor::Execute(
    const gint32 containerID,
    const std::string commandLine,
    const std::string workingDirectory,
    const std::string outputFile,
    const std::map<std::string, std::string> env,
    SoftwareContainerAgentMessageHelper msg)
{
    pid_t pid;
    bool success = m_agent.execute(
        containerID,
        commandLine,
        workingDirectory,
        outputFile,
        env,
        pid,
        [this, containerID](pid_t pid, int exitCode) {
            ProcessStateChanged_emitter(containerID, pid, false, exitCode);
            log_info() << "ProcessStateChanged " << pid << " code " << exitCode;
        }
    );
    msg.ret(pid, success);
}

void SoftwareContainerAgentAdaptor::Suspend(const gint32 containerID, SoftwareContainerAgentMessageHelper msg)
{
    bool success = m_agent.suspendContainer(containerID);
    msg.ret(success);
}

void SoftwareContainerAgentAdaptor::Resume(const gint32 containerID, SoftwareContainerAgentMessageHelper msg)
{
    bool success = m_agent.resumeContainer(containerID);
    msg.ret(success);
}

void SoftwareContainerAgentAdaptor::Destroy(const gint32 containerID, SoftwareContainerAgentMessageHelper msg)
{
    bool success = m_agent.shutdownContainer(containerID);
    msg.ret(success);
}

void SoftwareContainerAgentAdaptor::BindMount(
    const gint32 containerID,
    const std::string pathInHost,
    const std::string PathInContainer,
    const bool readOnly,
    SoftwareContainerAgentMessageHelper msg)
{
    bool success = m_agent.bindMount(containerID, pathInHost, PathInContainer, readOnly);
    msg.ret(success);
}

void SoftwareContainerAgentAdaptor::SetCapabilities(
    const gint32 containerID,
    const std::vector<std::string> capabilities,
    SoftwareContainerAgentMessageHelper msg)
{
    bool success = m_agent.setCapabilities(containerID, capabilities);
    msg.ret(success);
}

void SoftwareContainerAgentAdaptor::Create(const std::string config,
                                           SoftwareContainerAgentMessageHelper msg)
{
    gint32 containerID;
    bool success = m_agent.createContainer(config, containerID);
    msg.ret(containerID, success);
}

} // namespace softwarecontainer
