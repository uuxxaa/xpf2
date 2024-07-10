#pragma once
#include <functional>
#include <xpf/math/hash.h>

namespace xpf {

class UIElement;

enum class Invalidates {
    None,
    SelfLayout,
    ParentLayout,
    Visuals,
};

enum class ThemeId : uint32_t;

template<typename T>
class UIProperty {
public:
    static inline std::function<void(UIElement*, T newValue)> NoopFn = [](UIElement*, T) {};

protected:
    UIElement* m_pOwner = nullptr;
    const Invalidates m_invalidates = Invalidates::Visuals;
    mutable T m_value = {};
    bool m_isSet = false;
    bool m_isReadOnly = true;
    std::function<const T&(T)> m_getter;
    std::function<void(UIProperty<T>&)> m_setter;
    std::function<void(UIElement*, T newValue)> m_onChanged = NoopFn;
    mutable ThemeId m_themeId = ThemeId(0);
    std::string_view m_themeName;

public:
    UIProperty() = default;

    UIProperty(
        UIElement* p,
        T defaultValue = {},
        Invalidates invalidates = Invalidates::Visuals,
        const std::function<void(UIElement*, T oldvalue)>& onChanged = NoopFn,
        std::string_view name = "")
        : m_pOwner(p)
        , m_invalidates(invalidates)
        , m_value(defaultValue)
        , m_onChanged(onChanged)
        , m_themeName(name)
    {}

    const T& Get() const
    {
        if (m_isSet)
            return m_value;

        TryGetValueFromThemeEngine();

        if (m_getter != nullptr)
        {
            auto value = m_getter(m_value);
            const_cast<UIProperty*>(this)->Set(value);
        }

        return m_value;
    }

    void TryGetValueFromThemeEngine() const;

    void Set(const T& f);
    void SetDefaultValue(T f) { if (!m_isSet) { m_value = f; } }
    void Unset() { m_isSet = false; m_value = {}; }
    bool IsSet() const { return m_isSet; }
    void SetIsReadOnly(bool value) { m_isReadOnly = value; }

    void SetThemeId(std::string_view name) { m_themeName = name; }
    bool IsSetOrIsThemed() const { return m_isSet || !m_themeName.empty(); }

    void SetGetter(std::function<T(T)>&& getter) { m_getter = std::move(getter); }
    void SetSetter(std::function<void(UIProperty<T>&)>&& setter) { m_setter = std::move(setter); m_setter(*this); }
    void SetOnChanged(std::function<void(UIElement*, T newValue)>&& onChanged) { m_onChanged = std::move(onChanged); }

    const T& ValueOr(const T& v) { return m_isSet ? m_value : v; }

    UIProperty& operator =(T f) { Set(f); return *this; }
    operator const T&() const { return Get(); }
    bool operator==(T t) const { return t == m_value; }
    bool operator!=(T t) const { return t != m_value; }
};

typedef std::function<void(UIProperty<float>&)> bind_float;

} // xpf

#define DECLARE_PROPERTY(TClass, TType, TName, TDefaultValue, TInvalidates) \
protected: \
    xpf::UIProperty<TType> m_##TName = {this, TDefaultValue, TInvalidates}; \
public: \
    TType Get##TName() const { return m_##TName.Get(); } \
    TClass& Set##TName(TType value) { m_##TName.Set(value); return *this; } \
    xpf::UIProperty<TType>& Get##TName##Property() { return m_##TName; }

#define DECLARE_THEMED_PROPERTY(TClass, TType, TName, TDefaultValue, TInvalidates) \
protected: \
    xpf::UIProperty<TType> m_##TName = {this, TDefaultValue, TInvalidates, xpf::UIProperty<TType>::NoopFn, #TClass "_" #TName}; \
public: \
    TType Get##TName() const { return m_##TName.Get(); } \
    TClass& Set##TName(TType value) { m_##TName.Set(value); return *this; } \
    xpf::UIProperty<TType>& Get##TName##Property() { return m_##TName; }

#define DECLARE_PROPERTY_GETONLY(TClass, TType, TName, TDefaultValue, TInvalidates) \
protected: \
    xpf::UIProperty<TType> m_##TName = {this, TDefaultValue, TInvalidates}; \
public: \
    TType Get##TName() const { return m_##TName.Get(); } \
    xpf::UIProperty<TType>& Get##TName##Property() { return m_##TName; }
