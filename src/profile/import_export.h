#pragma once
#include "profile_schema.h"
#include <string>

namespace wz {

// Serializes/deserializes profiles to/from JSON using nlohmann/json
class ImportExport {
public:
    static bool SaveToFile(const Profile& profile, const std::string& path);
    static bool LoadFromFile(const std::string& path, Profile& out);

    // Convert to/from JSON string
    static std::string ToJsonString(const Profile& profile);
    static bool FromJsonString(const std::string& json, Profile& out);
};

} // namespace wz
