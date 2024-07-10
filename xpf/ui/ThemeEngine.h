#pragma once
#include <xpf/ui/UIProperty.h>
#include <xpf/core/Color.h>
#include <xpf/renderer/common/Font.h>
#include <xpf/renderer/common/FormattedText.h>
#include <unordered_map>
#include <unordered_set>
#include <xpf/core/Tween.h>

namespace xpf {

enum class ThemeId : uint32_t
{
    NotSet,
    Light,
    Dark,
};

class Theme
{
public:
    ThemeId m_themeId;
    std::unordered_map<std::string_view, xpf::Color> m_colors;
    std::unordered_map<std::string_view, xpf::Typeface> m_typefaces;
    std::unordered_set<std::string> m_stringCache;

public:
    Theme(ThemeId id)
        : m_themeId(id)
    {
        m_colors["Window_Background"] = m_themeId == ThemeId::Dark ? xpf::Colors::XpfBlack : xpf::Color(0xe2,0xff);
        m_colors["Focus_Border"] = m_themeId == ThemeId::Dark ? xpf::Colors::Azure : xpf::Colors::DodgerBlue;
        m_colors["Caret_Color_On"] = m_themeId == ThemeId::Dark ? xpf::Colors::Azure : xpf::Colors::DodgerBlue;
        m_colors["Caret_Color_Off"] = m_themeId == ThemeId::Dark ? xpf::Colors::Gray : xpf::Colors::Gray;

        m_colors["Panel_Level1_Background"] = m_themeId == ThemeId::Dark ? xpf::Color(0x2f,0xff) : xpf::Color(0xc2,0xff);
        m_colors["Panel_Level1_Border"] = m_themeId == ThemeId::Dark ? xpf::Color(0x4f,0x80) : xpf::Color(0xc4,0x80);

        m_colors["Background"] = m_themeId == ThemeId::Dark ? xpf::Colors::XpfBlack : xpf::Colors::XpfWhite;
        m_colors["Foreground"] = m_themeId == ThemeId::Dark ? xpf::Colors::XpfWhite : xpf::Colors::XpfBlack;

        m_colors["TextBox_Background"] = m_themeId == ThemeId::Dark ? xpf::Colors::XpfBlack : xpf::Colors::XpfWhite;
        m_colors["TextBox_Foreground"] = m_themeId == ThemeId::Dark ? xpf::Colors::XpfWhite : xpf::Colors::XpfBlack;

        m_colors["Scrollbar_Background_Default"] = xpf::Colors::Transparent;
        m_colors["Scrollbar_Background_Hover"] = m_themeId == ThemeId::Dark ? xpf::Colors::White.with_alpha(10) : xpf::Colors::Black.with_alpha(10);
        m_colors["Scrollbar_Foreground_Default"] = xpf::Colors::Gray;
        m_colors["Scrollbar_Foreground_Hover"] = m_themeId == ThemeId::Dark ? xpf::Colors::DodgerBlue : xpf::Colors::Goldenrod;

        m_colors["SplitPanel_Background_Default"] = xpf::Colors::Transparent;
        m_colors["SplitPanel_Background_Hover"] = m_themeId == ThemeId::Dark ? xpf::Colors::DodgerBlue : xpf::Colors::Goldenrod;

        m_colors["TreeNode_Background"] = m_themeId == ThemeId::Dark ? xpf::Colors::XpfBlack : xpf::Colors::XpfWhite;
        m_colors["TreeNode_Foreground"] = m_themeId == ThemeId::Dark ? xpf::Colors::XpfWhite : xpf::Colors::XpfBlack;
        m_colors["TreeNode_Foreground_Hover"] = m_themeId == ThemeId::Dark ? xpf::Colors::Black : xpf::Colors::Black;
        m_colors["TreeNode_Background_Hover"] = m_themeId == ThemeId::Dark ? xpf::Colors::Azure : xpf::Colors::Honey;
        m_colors["TreeNode_Foreground_Selected"] = m_themeId == ThemeId::Dark ? xpf::Colors::Black : xpf::Colors::Black;
        m_colors["TreeNode_Background_Selected"] = m_themeId == ThemeId::Dark ? xpf::Colors::LightCoral : xpf::Colors::LightCoral;
        m_colors["TreeNode_Background_Selecting"] = m_themeId == ThemeId::Dark ? xpf::Colors::LightPink : xpf::Colors::LightPink;
        m_colors["TreeNode_TypedownLookupMatch_Background"] = xpf::Colors::Aquamarine;

        m_colors["Button_Foreground_Default"] = m_themeId == ThemeId::Dark ? xpf::Color(0xffe9e9e9) : xpf::Color(0xff1e1e1e);
        m_colors["Button_Foreground_Focus"] = m_themeId == ThemeId::Dark ? xpf::Color(0xffe9e9e9) : xpf::Color(0xff1e1e1e);
        m_colors["Button_Foreground_Hover"] = m_themeId == ThemeId::Dark ? xpf::Color(0xffe9e9e9) : xpf::Color(0xff1e1e1e);
        m_colors["Button_Foreground_Pressed"] = m_themeId == ThemeId::Dark ? xpf::Color(0xffe9e9e9) : xpf::Color(0xff1e1e1e);

        m_colors["Button_Background_Default"] = m_themeId == ThemeId::Dark ? xpf::Color(0xff1e1e1e) : xpf::Colors::Honey;
        m_colors["Button_Background_Focus"] = m_themeId == ThemeId::Dark ? xpf::Color(0xff2e2e2e) : xpf::Colors::Azure;
        m_colors["Button_Background_Hover"] = m_themeId == ThemeId::Dark ? xpf::Color(0xff3e3e3e) : xpf::Colors::Green;
        m_colors["Button_Background_Pressed"] = m_themeId == ThemeId::Dark ? xpf::Color(0xff3e3e3e) : xpf::Colors::Yellow;

        m_colors["Button_Border_Default"] = m_themeId == ThemeId::Dark ? xpf::Color(0xff3e3e3e) : xpf::Color(0xffc9c9c9);
        m_colors["Button_Border_Focus"] = m_themeId == ThemeId::Dark ? xpf::Color(0xff3e3e3e) : xpf::Color(0xffc9c9c9);
        m_colors["Button_Border_Hover"] = m_themeId == ThemeId::Dark ? xpf::Color(0xff3e3e3e) : xpf::Color(0xffc9c9c9);
        m_colors["Button_Border_Pressed"] = m_themeId == ThemeId::Dark ? xpf::Color(0xff3e3e3e) : xpf::Color(0xffc9c9c9);
    }

    ThemeId TryGetValue(ThemeId id, std::string_view name, xpf::Color& color) const
    {
        auto iter = m_colors.find(name);
        if (iter != m_colors.cend())
            color = iter->second;

        return m_themeId;
    }

    ThemeId TryGetValue(ThemeId id, std::string_view name, Typeface& typeface) const
    {
        auto iter = m_typefaces.find(name);
        if (iter != m_typefaces.cend())
            typeface = iter->second;

        return m_themeId;
    }

    void SetValue(std::string_view name, xpf::Color color)
    {
        m_colors[name] = color;
    }

    void SetValue(std::string_view name, Typeface&& typeface)
    {
        m_typefaces[name] = std::move(typeface);
    }
};

class ThemeEngine
{
protected:
    static inline ThemeId m_currentTheme;
    static inline std::unordered_map<ThemeId, std::unique_ptr<Theme>> m_themes;
    

public:
    static void Initialize()
    {
        m_currentTheme = ThemeId::Light;
        m_themes[ThemeId::Dark] = std::make_unique<Theme>(ThemeId::Dark);
        m_themes[ThemeId::Light] = std::make_unique<Theme>(ThemeId::Light);
    }

    static void ToggleTheme()
    { 
        m_currentTheme = (m_currentTheme == ThemeId::Dark ? ThemeId::Light : ThemeId::Dark);
    }

    static ThemeId GetCurrentThemeId() { return m_currentTheme; }

    template<typename T>
    static ThemeId TryGetValue(ThemeId, std::string_view, T&) { return ThemeId(-1); }

    static ThemeId TryGetValue(ThemeId id, std::string_view name, xpf::Color& color)
    {
        return m_themes[m_currentTheme]->TryGetValue(id, name, color);
    }

    static void AddThemeColor(std::string_view name, xpf::Color lightTheme, xpf::Color darkTheme)
    {
        m_themes[ThemeId::Light]->SetValue(name, lightTheme);
        m_themes[ThemeId::Dark]->SetValue(name, darkTheme);
    }

    static void AddThemeFont(std::string_view name, Typeface&& lightTypeface, Typeface&& darkTypeface)
    {
        m_themes[ThemeId::Light]->SetValue(name, std::move(lightTypeface));
        m_themes[ThemeId::Dark]->SetValue(name, std::move(darkTypeface));
    }

    static xpf::Color GetColor(std::string_view name, xpf::Color fallback = xpf::Colors::Purple)
    {
        xpf::Color color = fallback;
        m_themes[m_currentTheme]->TryGetValue(m_currentTheme, name, color);
        return color;
    }

    static Typeface GetTypeface(std::string_view name)
    {
        xpf::Typeface typeface;
        m_themes[m_currentTheme]->TryGetValue(m_currentTheme, name, typeface);
        return typeface;
    }
};

template<typename T>
void UIProperty<T>::TryGetValueFromThemeEngine() const
{
    if (m_themeId == ThemeEngine::GetCurrentThemeId())
        return;

    m_themeId = ThemeEngine::TryGetValue(m_themeId, m_themeName, m_value);
}

struct ThemedColor
{
protected:
    mutable ThemeId themeId = ThemeId::NotSet;
    mutable xpf::Color m_color;
    std::string_view m_name;

public:
    ThemedColor(std::string_view name) : m_name(name) { }

    xpf::Color Get() const
    {
        auto currentThemeId = ThemeEngine::GetCurrentThemeId();
        if (currentThemeId != themeId)
        {
            themeId = currentThemeId;
            m_color = ThemeEngine::GetColor(m_name);
        }
        return m_color;
    }
};

struct ThemeImage
{
    mutable ThemeId themeId = ThemeId::NotSet;
    mutable std::shared_ptr<ITexture> spTexture;
    mutable rectf_t txtcoords;
    std::string_view name;
};

struct ThemeTypeface
{
    mutable ThemeId themeId = ThemeId::NotSet;
    mutable Typeface typeface;
    std::string_view name;
    Typeface Get() const
    {
        auto currentThemeId = ThemeEngine::GetCurrentThemeId();
        if (currentThemeId != themeId)
        {
            themeId = currentThemeId;
            typeface = ThemeEngine::GetTypeface(name);
        }
        return typeface;
    }
};

class ThemedFormattedText
{
protected:
    mutable ThemeId m_themeId = ThemeId::NotSet;
    mutable FormattedText m_ft;
    std::string_view m_name;

public:
    ThemedFormattedText(std::string_view name) : m_name(name) { }

    void SetText(std::string_view text) { m_ft.SetText(text); }
    const std::string& GetText() const { return m_ft.GetText(); }
    void SetThemeId(std::string_view themeId) { m_name = themeId; m_themeId = ThemeId::NotSet; }

    FormattedText& Get() const
    {
        auto currentThemeId = ThemeEngine::GetCurrentThemeId();
        if (currentThemeId != m_themeId)
        {
            m_themeId = currentThemeId;
            m_ft.SetFont(ThemeEngine::GetTypeface(m_name));
        }

        return m_ft;
    }
};

} // xpf