#pragma once

#include "EngineCore.h"

namespace Toad
{
    class Object;

    class Tags
    {
    public:
        ENGINE_API explicit Tags(const Object& obj);

        ENGINE_API bool HasTag(const std::string& tag) const;
        ENGINE_API void AddTag(const std::string& tag);
        ENGINE_API void RemoveTag(const std::string& tag);

    private:
        static inline std::unordered_map<std::string, std::set<std::string>> tags {};
        const Object& obj;
    };
}
