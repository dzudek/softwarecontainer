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

include(GenerateDBusCpp)
generate_dbuscpp_hfile(${TEMPSERVICE_XML} "TemperatureService")

add_executable(temperatureservice
    main.cpp
    ${GENERATED_ADAPTOR_FILENAME}
    temperatureservice.cpp
    temperatureservicetodbusadapter.cpp
)

TARGET_LINK_LIBRARIES( temperatureservice
    ${DBusCpp_LIBRARIES}
)

install(TARGETS temperatureservice
        RUNTIME DESTINATION share/softwarecontainer/examples/temperature)
