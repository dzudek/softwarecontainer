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

include_directories(${LIBSOFTWARECONTAINER_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gateway)

# Enable or disable support for gateways compile-time.
set(GATEWAY_SOURCES "")
set(GATEWAY_DEFINITIONS "")

add_subdirectory(gateway)
add_definitions(${GATEWAY_DEFINITIONS})

# Include the job files
set(JOB_SOURCES "")
add_subdirectory(jobs)

add_library(softwarecontainer SHARED
    container.cpp
    softwarecontainer.cpp
    workspace.cpp
    ${GATEWAY_SOURCES}
    ${JOB_SOURCES}
)

TARGET_LINK_LIBRARIES(softwarecontainer
    softwarecontainercommon
    ${Glibmm_LIBRARIES}
    ${LXC_LIBRARIES}
    ${Jansson_LIBRARIES}
)

install(TARGETS softwarecontainer DESTINATION lib)
set_target_properties(softwarecontainer PROPERTIES VERSION ${VERSION} SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})
