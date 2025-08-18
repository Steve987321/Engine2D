#include "pch.h"
#include "Tags.h"

#include "engine/default_objects/Object.h"

namespace Toad
{
    Tags::Tags(const Object& obj)
        : obj(obj)
    {
    }

    bool Tags::HasTag(const std::string& tag) const
    {
        auto object_tags = tags.find(obj.name);
        if (object_tags != tags.end())
            return object_tags->second.contains(tag);

        return false;
    }

    void Tags::AddTag(const std::string& tag)
    {
        tags[obj.name].emplace(tag);
    }

    void Tags::RemoveTag(const std::string &tag)
    {
        auto object_tags = tags.find(obj.name);
        if (object_tags != tags.end())
            object_tags->second.erase(tag);
    }
}