
# Copyright (C) 2016 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE

import pytest

# External utility imports
import os
import time

# Test framework and local test helper imports
from testframework.testhelper import EnvironmentHelper
from testframework import Capability
from testframework import StandardManifest
from testframework import Container


##### Useful globals #####

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTOUTPUT_DIR = CURRENT_DIR + "/testoutput"

# The path to where the test app is located:
#  * Will be passed in the 'data' dict when starting the container to set
#    this test modules location as a bind mount visible inside the container.
#  * Is used by the testframework for knowing where this test is located.
HOST_PATH = os.path.dirname(os.path.abspath(__file__))


##### Provide what the testframework requires #####

# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return TESTOUTPUT_DIR + "/environment-test.log"


# This function is used by the testframework 'testhelper' fixture to know where the
# testhelper should be made available when running the tests
def mounted_path_in_host():
    return HOST_PATH


##### Local fixtures #####

@pytest.fixture
def clear_env_files(scope="function"):
    """ Removes the files used by the helper to avoid having false
        positives from previous test runs.

        TODO: Should this be a method on the helper?
    """
    file_path = TESTOUTPUT_DIR + EnvironmentHelper.ENV_VARS_FILE_NAME
    if os.path.exists(file_path):
        os.remove(file_path)


@pytest.fixture
def create_testoutput_dir(scope="module"):
    """ Create a directory for the generated test files.

        This directory is ignored by git but it's nice to have
        somewhere locally to store test output to support
        troubleshooting etc.
    """
    if not os.path.exists(TESTOUTPUT_DIR):
        os.makedirs(TESTOUTPUT_DIR)


##### Globals for setup and configuration of SC #####

# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: "[{\"enableWriteBuffer\": false}]",
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: HOST_PATH,
    Container.READONLY: False
}


#####  Various gateway configs used by the tests. #####
#
# The configs below might be dependent on each other so beware
# when changing them.

GW_CONFIG_ONE_VAR = [
    {
        "name": "MY_ENV_VAR",
        "value": "1234"
    }
]

GW_CONFIG_SAME_VAR = [
    {
        "name": "MY_ENV_VAR",
        "value": "abcd"
    }
]

GW_CONFIG_TWO_VARS = [
     {
        "name": "MY_ENV_VAR",
        "value": "1234"
    },
    {
        "name": "LD_LIBRARY_PATH",
        "value": "/some/non/relevant/path"
    }
]

# This config appends LD_LIBRARY_PATH and should be used
# after e.g. GW_CONFIG_ONE_VAR has been set.
GW_CONFIG_APPEND = [
    {
        "name": "LD_LIBRARY_PATH",
        "value": "/another/path",
        "mode": "append",
        "separator": ":"
    }
]

GW_CONFIG_PREPEND = [
    {
        "name": "LD_LIBRARY_PATH",
        "value": "/another/path",
        "mode": "prepend",
        "separator": ":"
    }
]


""" Cap that appends to a non existent env variable """
test_cap_2 = Capability("environment.test.cap.2",
                        [
                            {"id": "env", "config": GW_CONFIG_APPEND}
                        ])

""" Cap that sets one env variable """
test_cap_4 = Capability("environment.test.cap.4",
                        [
                            {"id": "env", "config": GW_CONFIG_ONE_VAR}
                        ])

""" Cap that sets the same env variable twice """
test_cap_1 = Capability("environment.test.cap.1",
                                  [
                                      {"id": "env", "config": GW_CONFIG_ONE_VAR},
                                      {"id": "env", "config": GW_CONFIG_SAME_VAR}
                                  ])

""" Cap that sets two env variables """
test_cap_6 = Capability("environment.test.cap.6",
                        [
                            {"id": "env", "config": GW_CONFIG_TWO_VARS}
                        ])

""" Cap that sets two env variables and appends one of them """
test_cap_3 = Capability("environment.test.cap.3",
                        [
                            {"id": "env", "config": GW_CONFIG_TWO_VARS},
                            {"id": "env", "config": GW_CONFIG_APPEND}
                        ])

""" Cap that sets two env variables and prepends one of them """
test_cap_5 = Capability("environment.test.cap.5",
                        [
                            {"id": "env", "config": GW_CONFIG_TWO_VARS},
                            {"id": "env", "config": GW_CONFIG_PREPEND}
                        ])

manifest = StandardManifest(TESTOUTPUT_DIR,
                            "environment-test-manifest.json",
                            [
                                test_cap_1,
                                test_cap_2,
                                test_cap_3,
                                test_cap_4,
                                test_cap_5,
                                test_cap_6
                            ])


def service_manifests():
    """ The agent fixture calls this function when it creates the service manifests
        that should be used with this test module. The agent fixture expects a list
        of StandardManifest and/or DefaultManifest objects.
    """
    return [manifest]


##### Test suites #####

@pytest.mark.usefixtures("testhelper", "create_testoutput_dir", "agent", "clear_env_files")
class TestInteractionGatewayAndAPI(object):
    """ This suite tests the interaction between the environment gateway and
        setting of environment variables through the Agent D-Bus API.
    """

    def test_vars_set_by_api_takes_precedence(self):
        """ Setting a varaible through the D-Bus API should mean that value
            takes precedence over a variable previously set by the environment
            gateway.
        """
        sc = Container()
        try:
            sc.start(DATA)
            # Set variable through gateway
            sc.set_capabilities(["environment.test.cap.4"])
            # Set the same variable with LaunchCommand
            env_var = {"MY_ENV_VAR": "new-value"}
            sc.launch_command("python " +
                                  sc.get_bind_dir() +
                                  "/testhelper.py --test-dir " +
                                  sc.get_bind_dir() + "/testoutput" +
                                  " --do-get-env-vars",
                              env=env_var)

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            helper = EnvironmentHelper(TESTOUTPUT_DIR)
            my_env_var = helper.env_var("MY_ENV_VAR")
            # Assert value is the one set with LaunchCommand
            assert my_env_var == "new-value"
        finally:
            sc.terminate()


@pytest.mark.usefixtures("testhelper", "create_testoutput_dir", "agent", "clear_env_files")
class TestEnvironment(object):
    """ This suite does basic tests on the environment variables inside the
        container.
    """

    def test_setting_same_var_fails(self):
        """ Setting a variable twice should result in failed call to set the capability
            that results in the bad config combo.
        """
        sc = Container()
        try:
            sc.start(DATA)

            result = sc.set_capabilities(["environment.test.cap.1"])

            assert result is False
        finally:
            sc.terminate()

    def test_appending_non_existent_var_creates_it(self):
        """ Append to a non existing env var and assert that the variable is created
            as a result.
        """
        sc = Container()
        try:
            sc.start(DATA)
            sc.set_capabilities(["environment.test.cap.2"])
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --test-dir " +
                              sc.get_bind_dir() + "/testoutput" +
                              " --do-get-env-vars")

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            helper = EnvironmentHelper(TESTOUTPUT_DIR)
            variable = helper.env_var("LD_LIBRARY_PATH")
            # Assert the append resulted in the creation of the variable
            assert variable == "/another/path"
        finally:
            sc.terminate()

    def test_env_var_is_appended(self):
        """ Set a config and then an appending config, assert the appended
            variable looks as expected.
        """
        sc = Container()
        try:
            sc.start(DATA)
            sc.set_capabilities(["environment.test.cap.3"])
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --test-dir " +
                              sc.get_bind_dir() + "/testoutput" +
                              " --do-get-env-vars")

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            helper = EnvironmentHelper(TESTOUTPUT_DIR)
            ld_library_path = helper.env_var("LD_LIBRARY_PATH")
            # Assert the append was applied
            assert ld_library_path == "/some/non/relevant/path:/another/path"
            my_env_var = helper.env_var("MY_ENV_VAR")
            # Assert other variables were not affected
            assert my_env_var == "1234"
        finally:
            sc.terminate()

    def test_env_var_is_prepended(self):
        """ Set a config and then an prepending config, assert the prepended
            variable looks as expected.
        """
        sc = Container()
        try:
            sc.start(DATA)
            sc.set_capabilities(["environment.test.cap.5"])
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --test-dir " +
                              sc.get_bind_dir() + "/testoutput" +
                              " --do-get-env-vars")

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            helper = EnvironmentHelper(TESTOUTPUT_DIR)
            ld_library_path = helper.env_var("LD_LIBRARY_PATH")
            # Assert the append was applied
            assert ld_library_path == "/another/path:/some/non/relevant/path"
            my_env_var = helper.env_var("MY_ENV_VAR")
            # Assert other variables were not affected
            assert my_env_var == "1234"
        finally:
            sc.terminate()

    def test_env_var_is_set(self):
        """ Set an Environment gateway config that sets a variable inside
            the container. Use helper to get the inside environment and
            assert the expected result.
        """
        sc = Container()
        try:
            sc.start(DATA)
            sc.set_capabilities(["environment.test.cap.4"])
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --test-dir " +
                              sc.get_bind_dir() + "/testoutput" +
                              " --do-get-env-vars")

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            helper = EnvironmentHelper(TESTOUTPUT_DIR)
            assert helper.env_var("MY_ENV_VAR") == "1234"
        finally:
            sc.terminate()

    def test_multiple_env_vars_are_set(self):
        """ Set an Environment gateway config that sets multiple variables inside
            the container. Use helper to get the inside environment and
            assert the expected reisult.
        """
        sc = Container()
        try:
            sc.start(DATA)
            sc.set_capabilities(["environment.test.cap.6"])
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --test-dir " +
                              sc.get_bind_dir() + "/testoutput" +
                              " --do-get-env-vars")

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            helper = EnvironmentHelper(TESTOUTPUT_DIR)
            assert helper.env_var("MY_ENV_VAR") == "1234"
            assert helper.env_var("LD_LIBRARY_PATH") == "/some/non/relevant/path"
        finally:
            sc.terminate()
