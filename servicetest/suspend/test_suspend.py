
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

import os
from time import sleep

from testframework import Container


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return CURRENT_DIR + "/test.log"


# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: '[{"enableWriteBuffer": false}]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}

def isFileGrowing(filename):
    """ Check if a file is growing, by stat:ing it, sleeping for a while,
        and then stat:ing it again.
    """

    sleep(0.1)
    sizeBefore = os.stat(filename).st_size

    sleep(0.1)
    sizeAfter = os.stat(filename).st_size

    return sizeBefore != sizeAfter

@pytest.mark.usefixtures("agent", "assert_no_proxy")
class TestSuspend(object):
    """ This suite tests that suspend/resume can be used with SoftwareContainer with
        expected results.

        The tests use the Agent D-Bus interface to drive the tests.
    """

    def test_suspend_and_resume_works(self):
        """ Test suspending a container works.
        """
        try:
            sc = Container()
            sc.start(DATA)

            suspended = sc.suspend()
            assert suspended is True

            resumed = sc.resume()
            assert resumed is True

        finally:
            sc.terminate()

    def test_suspend_prevents_launching_commands(self):
        """ Test that launching inside a suspended container doesn't work
            until the container has been resumed.
        """
        try:
            sc = Container()
            sc.start(DATA)

            suspended = sc.suspend()
            assert suspended is True

            _, success = sc.launch_command("true")
            assert success is False

            resumed = sc.resume()
            assert resumed is True

            _, success = sc.launch_command("true")
            assert success is True

        finally:
            sc.terminate()

    def test_suspend_halts_execution_and_resumes_on_resume(self):
        """ Test that a running application inside the container is frozen
            when the container is suspended, and that it is resuming execution
            when the container is resumed.
        """

        filename = "yes_output.log"
        try:
            sc = Container()
            sc.start(DATA)

            _, success = sc.launch_command("yes", stdout=filename)
            assert success is True

            assert isFileGrowing(filename)

            # Suspend the container
            suspended = sc.suspend()
            assert suspended is True

            assert not isFileGrowing(filename)

            resumed = sc.resume()
            assert resumed is True

            assert isFileGrowing(filename)

        finally:
            sc.terminate()
            os.remove(filename)

