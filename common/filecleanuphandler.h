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

#pragma once

#include <cleanuphandler.h>

namespace softwarecontainer {

/**
 * @brief The FileCleanUpHandler class is a subclass of CleanUpHandler that deletes a file. It is
 * most commonly used when the FileToolKitWithUndo destructor is run to clean away cruft from the
 * filesystem
 */
class FileCleanUpHandler :
    public CleanUpHandler
{
public:
    /**
     * @brief FileCleanUpHandler Create a cleanupHandler for the Path, which will be deleted when
     * running the clean() function.
     * @param path The path of the file to delete when clean() is run.
     */
    FileCleanUpHandler(const std::string &path);

    /**
     * @brief clean Perform the cleanupHandler clean.
     * @return ReturnCode::SUCCESS on success
     * @return ReturnCode::FAILURE on failure
     */
    ReturnCode clean() override;

    /**
     * @brief this function is needed to query member name
     *
     * @return a read-only m_path
     */
    const std::string queryName() override;

    std::string m_path;
};

} // namespace softwarecontainer
