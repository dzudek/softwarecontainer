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

#include "fileconfigloader.h"

namespace softwarecontainer {

std::unique_ptr<Glib::KeyFile> FileConfigLoader::loadConfig()
{
    std::unique_ptr<Glib::KeyFile> configData = std::unique_ptr<Glib::KeyFile>(new Glib::KeyFile);
    try {
        configData->load_from_file(Glib::ustring(this->m_source), Glib::KEY_FILE_NONE);
        log_debug() << "Loaded config file: \"" << this->m_source << "\"";
    } catch (Glib::FileError &error) {
        log_error() << "Could not load SoftwareContainer config: \"" << error.what() << "\"";
        log_debug() << "Config file that failed to load: \"" << this->m_source << "\"";
        throw;
    } catch (Glib::KeyFileError &error) {
        log_error() << "Could not load SoftwareContainer config: \"" << error.what() << "\"";
        log_debug() << "Config file that failed to load: \"" << this->m_source << "\"";
        throw;
    }

    return configData;
}

} // namespace softwarecontainer
