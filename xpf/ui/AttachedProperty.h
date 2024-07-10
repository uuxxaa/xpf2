#pragma once
#include <stdint.h>
#include <memory>
#include <unordered_map>

namespace xpf {

struct AttachedPropertyBase {};

template<typename T, size_t Tid>
struct AttachedProperty : public AttachedPropertyBase {
    typedef T value_type;
    T value;

    AttachedProperty(T v) : value(std::move(v)) { }
    static size_t Id() { return Tid; }
};

template<typename TBase>
class ObjectWithAttachedProperties : public TBase
{
protected:
    std::unordered_map<size_t, std::unique_ptr<AttachedPropertyBase>> m_attachedProperties;

public:
    template<typename T> void Remove()
    {
        m_attachedProperties.erase(T::Id());
    }

    template<typename T> ObjectWithAttachedProperties& Set(T&& item)
    {
        m_attachedProperties[T::Id()] = std::make_unique<T>(std::move(item));
        return *this;
    }

    template<typename T> const typename T::value_type& GetValueOr(const T::value_type& value) const
    {
        auto iter = m_attachedProperties.find(T::Id());
        if (iter == m_attachedProperties.cend()) return value;
        return *(typename T::value_type*)iter->second.get();
    }

    template<typename T> const typename T::value_type* Get() const
    {
        auto iter = m_attachedProperties.find(T::Id());
        if (iter == m_attachedProperties.cend()) return nullptr;
        return (typename T::value_type*)iter->second.get();
    }

    template<typename T> typename T::value_type* Get()
    {
        auto iter = m_attachedProperties.find(T::Id());
        if (iter == m_attachedProperties.cend()) return nullptr;
        return (typename T::value_type*)iter->second.get();
    }

    template<typename T> bool Has() const
    {
        auto iter = m_attachedProperties.find(T::Id());
        return (iter == m_attachedProperties.cend());
    }
};

} // xpf