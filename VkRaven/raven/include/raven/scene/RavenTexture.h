#pragma once

namespace raven {
    class RavenTexture {
    public:
        std::string m_key{};
        std::string m_name = "RavenTexture";

        std::string m_directory;
        std::string m_uri;
    };
}