#pragma once

// reflection for variables in scripts
// should be used on script class member variables

// this will be used for the editor, and scenes to serialize variables

#include "EngineCore.h"
#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

namespace Toad
{

using json = nlohmann::ordered_json;

// Define string identifier for type X
#define SERIALIZABLE_TYPE(X) \
template<>  \
inline std::string_view type_name<X>() { return #X; }

template<typename T>
inline std::string_view type_name()
{
    return "Unknown";
}

struct SerializedEnum
{
    union Data
    {    
        int value;
        int* ptr;
    } data;

    bool is_ptr = false;
    
    std::string_view enum_name;
    std::vector<std::string_view> enumerators; 

    json Serialize() const 
    {
        json data;
        data["is_ptr"] = is_ptr;
        data["enum_name"] = enum_name;
        data["enumerators"] = {};
        for (const auto& v : enumerators)
            data["enumerators"].push_back(v);
        data["data"] = data;
        return data;
    }
};

SERIALIZABLE_TYPE(SerializedEnum)
SERIALIZABLE_TYPE(bool*)
SERIALIZABLE_TYPE(int8_t*)
SERIALIZABLE_TYPE(int16_t*)
SERIALIZABLE_TYPE(int32_t*)
SERIALIZABLE_TYPE(int64_t*)
SERIALIZABLE_TYPE(float*)
SERIALIZABLE_TYPE(std::string*)

#define ReflectTypes SerializedEnum, bool*, int8_t*, int16_t*, int32_t*, int64_t*, float*, std::string*

template<typename T>
struct ReflectVarsOfT
{
    std::unordered_map<std::string_view, T> data;

    json Serialize() const
    {
        if (data.empty())
            return {};
        
        std::string_view id = type_name<T>();

        json res;
        res[id] = {};

        for(const auto& [n, v] : data)
        {
            json var;
            std::remove_pointer_t<T> value;

            if constexpr (std::is_pointer_v<T>)
                value = *v;
            else 
                value = v;

            if constexpr (std::is_same_v<T, SerializedEnum>)
                var[n] = v.Serialize();
            else 
                var[n] = value; 

            res[id].push_back(var);
        }

        return res;
    }
};

template<typename... Ts>
struct ReflectVarsCopy : ReflectVarsOfT<std::remove_pointer_t<Ts>>...
{
    template<typename T>
    ENGINE_API ReflectVarsOfT<T>& Get()
    {
        return static_cast<ReflectVarsOfT<T>&>(*this);
    }

    template<typename T>
    ENGINE_API const ReflectVarsOfT<T>& GetCRef() const 
    {
        return static_cast<const ReflectVarsOfT<T>&>(*this);
    }

    template<typename T>
    ENGINE_API T& GetVar(std::string_view name)
    {
        return Get<T>().data[name];
    }

    template<typename T>
    ENGINE_API void Add(std::string_view name, T value)
    {
        if (std::is_enum_v<T>)
            AddEnum(name, value);
        else
            ReflectVarsOfT<T>::data[name] = value;
    }

    template<typename T>
    ENGINE_API void Add(const ReflectVarsOfT<T>& vars)
    {
        ReflectVarsOfT<T>::data.insert(vars.data.begin(), vars.data.end());
    }
};

template<typename... Ts>
struct ReflectVars : ReflectVarsOfT<Ts>...
{
    ReflectVars()
        : ReflectVarsOfT<Ts>()...
    {}

    template<typename T>
    ENGINE_API ReflectVarsOfT<T>& Get()
    {
        return (ReflectVarsOfT<T>&)(*this);
    }

    template<typename T>
    ENGINE_API T& GetVar(std::string_view name)
    {
        return GetVars<T>().data[name];
    }

    template<typename T>
    ENGINE_API void Add(std::string_view name, T value)
    {
        if constexpr (std::is_enum_v<T>)
            AddEnum(name, value);
        else
            ReflectVarsOfT<T>::data[name] = value;
    }

    template<typename T>
    ENGINE_API void AddEnum(std::string_view name, T* value)
    {
        auto enum_values = magic_enum::enum_values<T>();
        SerializedEnum serialized;
        serialized.enum_name = magic_enum::enum_name(*value);
        serialized.data.ptr = (int*)value;
        serialized.is_ptr = true;
        serialized.enumerators = {enum_values.begin(), enum_values.end()};
        ReflectVarsOfT<SerializedEnum>::data[name] = serialized;
    }

    template<typename T>
    ENGINE_API ReflectVarsOfT<std::remove_pointer_t<T>> CopyVars() const 
    {
        ReflectVarsOfT<std::remove_pointer_t<T>> res;
        for (const auto& [n, v] : ReflectVarsOfT<T>::data)
        {
            if constexpr (std::is_same_v<T, SerializedEnum>)
                res.data[n] = v;
            else 
                res.data[n] = *v;
        }

        return res;
    }

    ENGINE_API ReflectVarsCopy<Ts...> Copy() const 
    {
        ReflectVarsCopy<Ts...> res;
        (res.template Add<std::remove_pointer_t<Ts>>(CopyVars<Ts>()), ...);
        return res; 
    }

    ENGINE_API void Clear()
    {
        (ReflectVarsOfT<Ts>::data.clear(), ...);
    }

    ENGINE_API json Serialize() 
    {
        json res; 
        (res.push_back(Get<Ts>().Serialize()), ...);
        return res;
    }

    // update overlapping vars 
    ENGINE_API void Update(const ReflectVarsCopy<Ts...>& reflection)
    {
        (std::for_each(Get<Ts>().data.begin(), Get<Ts>().data.end(),
            [&reflection](auto& e)
            {
                auto& [name, value] = e;
                for (const auto& [newname, newvalue] : reflection.template GetCRef<std::remove_pointer_t<Ts>>().data)
                    if (newname == name)
                    {
                        if constexpr (std::is_same_v<Ts, SerializedEnum>)
                            value = newvalue;
                        else 
                            *value = newvalue;
                    }
            }), ...);
    }
};

using Reflection = ReflectVars<ReflectTypes>;
using ReflectionCopy = ReflectVarsCopy<ReflectTypes>;

}