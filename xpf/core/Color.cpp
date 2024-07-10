#include "Color.h"
//#include "log.h"

typedef uint8_t byte_t;

namespace xpf {

Color xpf::Colors::AliceBlue = xpf::Color(0xFFF0F8FF);
Color xpf::Colors::AntiqueWhite = xpf::Color(0xFFFAEBD7);
Color xpf::Colors::Aqua = xpf::Color(0xFF00FFFF);
Color xpf::Colors::Aquamarine = xpf::Color(0xFF7FFFD4);
Color xpf::Colors::Azure = xpf::Color(0xFFF0FFFF);
Color xpf::Colors::Beige = xpf::Color(0xFFF5F5DC);
Color xpf::Colors::Bisque = xpf::Color(0xFFFFE4C4);
Color xpf::Colors::Black = xpf::Color(0xFF000000);
Color xpf::Colors::BlanchedAlmond = xpf::Color(0xFFFFEBCD);
Color xpf::Colors::Blue = xpf::Color(0xFF0000FF);
Color xpf::Colors::BlueViolet = xpf::Color(0xFF8A2BE2);
Color xpf::Colors::Brown = xpf::Color(0xFFA52A2A);
Color xpf::Colors::BurlyWood = xpf::Color(0xFFDEB887);
Color xpf::Colors::CadetBlue = xpf::Color(0xFF5F9EA0);
Color xpf::Colors::Chartreuse = xpf::Color(0xFF7FFF00);
Color xpf::Colors::Chocolate = xpf::Color(0xFFD2691E);
Color xpf::Colors::Coral = xpf::Color(0xFFFF7F50);
Color xpf::Colors::CornflowerBlue = xpf::Color(0xFF6495ED);
Color xpf::Colors::Cornsilk = xpf::Color(0xFFFFF8DC);
Color xpf::Colors::Crimson = xpf::Color(0xFFDC143C);
Color xpf::Colors::Cyan = xpf::Color(0xFF00FFFF);
Color xpf::Colors::DarkBlue = xpf::Color(0xFF00008B);
Color xpf::Colors::DarkCyan = xpf::Color(0xFF008B8B);
Color xpf::Colors::DarkGoldenrod = xpf::Color(0xFFB8860B);
Color xpf::Colors::DarkGray = xpf::Color(0xFFA9A9A9);
Color xpf::Colors::DarkGreen = xpf::Color(0xFF006400);
Color xpf::Colors::DarkKhaki = xpf::Color(0xFFBDB76B);
Color xpf::Colors::DarkMagenta = xpf::Color(0xFF8B008B);
Color xpf::Colors::DarkOliveGreen = xpf::Color(0xFF556B2F);
Color xpf::Colors::DarkOrange = xpf::Color(0xFFFF8C00);
Color xpf::Colors::DarkOrchid = xpf::Color(0xFF9932CC);
Color xpf::Colors::DarkRed = xpf::Color(0xFF8B0000);
Color xpf::Colors::DarkSalmon = xpf::Color(0xFFE9967A);
Color xpf::Colors::DarkSeaGreen = xpf::Color(0xFF8FBC8F);
Color xpf::Colors::DarkSlateBlue = xpf::Color(0xFF483D8B);
Color xpf::Colors::DarkSlateGray = xpf::Color(0xFF2F4F4F);
Color xpf::Colors::DarkTurquoise = xpf::Color(0xFF00CED1);
Color xpf::Colors::DarkViolet = xpf::Color(0xFF9400D3);
Color xpf::Colors::DeepPink = xpf::Color(0xFFFF1493);
Color xpf::Colors::DeepSkyBlue = xpf::Color(0xFF00BFFF);
Color xpf::Colors::DimGray = xpf::Color(0xFF696969);
Color xpf::Colors::DodgerBlue = xpf::Color(0xFF1E90FF);
Color xpf::Colors::Firebrick = xpf::Color(0xFFB22222);
Color xpf::Colors::FloralWhite = xpf::Color(0xFFFFFAF0);
Color xpf::Colors::ForestGreen = xpf::Color(0xFF228B22);
Color xpf::Colors::Fuchsia = xpf::Color(0xFFFF00FF);
Color xpf::Colors::Gainsboro = xpf::Color(0xFFDCDCDC);
Color xpf::Colors::GhostWhite = xpf::Color(0xFFF8F8FF);
Color xpf::Colors::Gold = xpf::Color(0xFFFFD700);
Color xpf::Colors::Goldenrod = xpf::Color(0xFFDAA520);
Color xpf::Colors::Gray = xpf::Color(0xFF808080);
Color xpf::Colors::Green = xpf::Color(0xFF008000);
Color xpf::Colors::GreenYellow = xpf::Color(0xFFADFF2F);
Color xpf::Colors::Honeydew = xpf::Color(0xFFF0FFF0);
Color xpf::Colors::HotPink = xpf::Color(0xFFFF69B4);
Color xpf::Colors::IndianRed = xpf::Color(0xFFCD5C5C);
Color xpf::Colors::Indigo = xpf::Color(0xFF4B0082);
Color xpf::Colors::Ivory = xpf::Color(0xFFFFFFF0);
Color xpf::Colors::Khaki = xpf::Color(0xFFF0E68C);
Color xpf::Colors::Lavender = xpf::Color(0xFFE6E6FA);
Color xpf::Colors::LavenderBlush = xpf::Color(0xFFFFF0F5);
Color xpf::Colors::LawnGreen = xpf::Color(0xFF7CFC00);
Color xpf::Colors::LemonChiffon = xpf::Color(0xFFFFFACD);
Color xpf::Colors::LightBlue = xpf::Color(0xFFADD8E6);
Color xpf::Colors::LightCoral = xpf::Color(0xFFF08080);
Color xpf::Colors::LightCyan = xpf::Color(0xFFE0FFFF);
Color xpf::Colors::LightGoldenrodYellow = xpf::Color(0xFFFAFAD2);
Color xpf::Colors::LightGray = xpf::Color(0xFFD3D3D3);
Color xpf::Colors::LightGreen = xpf::Color(0xFF90EE90);
Color xpf::Colors::LightPink = xpf::Color(0xFFFFB6C1);
Color xpf::Colors::LightSalmon = xpf::Color(0xFFFFA07A);
Color xpf::Colors::LightSeaGreen = xpf::Color(0xFF20B2AA);
Color xpf::Colors::LightSkyBlue = xpf::Color(0xFF87CEFA);
Color xpf::Colors::LightSlateGray = xpf::Color(0xFF778899);
Color xpf::Colors::LightSteelBlue = xpf::Color(0xFFB0C4DE);
Color xpf::Colors::LightYellow = xpf::Color(0xFFFFFFE0);
Color xpf::Colors::Lime = xpf::Color(0xFF00FF00);
Color xpf::Colors::LimeGreen = xpf::Color(0xFF32CD32);
Color xpf::Colors::Linen = xpf::Color(0xFFFAF0E6);
Color xpf::Colors::Magenta = xpf::Color(0xFFFF00FF);
Color xpf::Colors::Maroon = xpf::Color(0xFF800000);
Color xpf::Colors::MediumAquamarine = xpf::Color(0xFF66CDAA);
Color xpf::Colors::MediumBlue = xpf::Color(0xFF0000CD);
Color xpf::Colors::MediumOrchid = xpf::Color(0xFFBA55D3);
Color xpf::Colors::MediumPurple = xpf::Color(0xFF9370DB);
Color xpf::Colors::MediumSeaGreen = xpf::Color(0xFF3CB371);
Color xpf::Colors::MediumSlateBlue = xpf::Color(0xFF7B68EE);
Color xpf::Colors::MediumSpringGreen = xpf::Color(0xFF00FA9A);
Color xpf::Colors::MediumTurquoise = xpf::Color(0xFF48D1CC);
Color xpf::Colors::MediumVioletRed = xpf::Color(0xFFC71585);
Color xpf::Colors::MidnightBlue = xpf::Color(0xFF191970);
Color xpf::Colors::MintCream = xpf::Color(0xFFF5FFFA);
Color xpf::Colors::MistyRose = xpf::Color(0xFFFFE4E1);
Color xpf::Colors::Moccasin = xpf::Color(0xFFFFE4B5);
Color xpf::Colors::NavajoWhite = xpf::Color(0xFFFFDEAD);
Color xpf::Colors::Navy = xpf::Color(0xFF000080);
Color xpf::Colors::OldLace = xpf::Color(0xFFFDF5E6);
Color xpf::Colors::Olive = xpf::Color(0xFF808000);
Color xpf::Colors::OliveDrab = xpf::Color(0xFF6B8E23);
Color xpf::Colors::Orange = xpf::Color(0xFFFFA500);
Color xpf::Colors::OrangeRed = xpf::Color(0xFFFF4500);
Color xpf::Colors::Orchid = xpf::Color(0xFFDA70D6);
Color xpf::Colors::PaleGoldenrod = xpf::Color(0xFFEEE8AA);
Color xpf::Colors::PaleGreen = xpf::Color(0xFF98FB98);
Color xpf::Colors::PaleTurquoise = xpf::Color(0xFFAFEEEE);
Color xpf::Colors::PaleVioletRed = xpf::Color(0xFFDB7093);
Color xpf::Colors::PapayaWhip = xpf::Color(0xFFFFEFD5);
Color xpf::Colors::PeachPuff = xpf::Color(0xFFFFDAB9);
Color xpf::Colors::Peru = xpf::Color(0xFFCD853F);
Color xpf::Colors::Pink = xpf::Color(0xFFFFC0CB);
Color xpf::Colors::Plum = xpf::Color(0xFFDDA0DD);
Color xpf::Colors::PowderBlue = xpf::Color(0xFFB0E0E6);
Color xpf::Colors::Purple = xpf::Color(0xFF800080);
Color xpf::Colors::Red = xpf::Color(0xFFFF0000);
Color xpf::Colors::RosyBrown = xpf::Color(0xFFBC8F8F);
Color xpf::Colors::RoyalBlue = xpf::Color(0xFF4169E1);
Color xpf::Colors::SaddleBrown = xpf::Color(0xFF8B4513);
Color xpf::Colors::Salmon = xpf::Color(0xFFFA8072);
Color xpf::Colors::SandyBrown = xpf::Color(0xFFF4A460);
Color xpf::Colors::SeaGreen = xpf::Color(0xFF2E8B57);
Color xpf::Colors::SeaShell = xpf::Color(0xFFFFF5EE);
Color xpf::Colors::Sienna = xpf::Color(0xFFA0522D);
Color xpf::Colors::Silver = xpf::Color(0xFFC0C0C0);
Color xpf::Colors::SkyBlue = xpf::Color(0xFF87CEEB);
Color xpf::Colors::SlateBlue = xpf::Color(0xFF6A5ACD);
Color xpf::Colors::SlateGray = xpf::Color(0xFF708090);
Color xpf::Colors::Snow = xpf::Color(0xFFFFFAFA);
Color xpf::Colors::SpringGreen = xpf::Color(0xFF00FF7F);
Color xpf::Colors::SteelBlue = xpf::Color(0xFF4682B4);
Color xpf::Colors::Tan = xpf::Color(0xFFD2B48C);
Color xpf::Colors::Teal = xpf::Color(0xFF008080);
Color xpf::Colors::Thistle = xpf::Color(0xFFD8BFD8);
Color xpf::Colors::Tomato = xpf::Color(0xFFFF6347);
Color xpf::Colors::Transparent = xpf::Color(uint32_t(0x00FFFFFF));
Color xpf::Colors::Turquoise = xpf::Color(0xFF40E0D0);
Color xpf::Colors::Violet = xpf::Color(0xFFEE82EE);
Color xpf::Colors::Wheat = xpf::Color(0xFFF5DEB3);
Color xpf::Colors::White = xpf::Color(0xFFFFFFFF);
Color xpf::Colors::WhiteSmoke = xpf::Color(0xFFF5F5F5);
Color xpf::Colors::Yellow = xpf::Color(0xFFFFFF00);
Color xpf::Colors::YellowGreen = xpf::Color(0xFF9ACD32);
// -------------------
Color xpf::Colors::XpfBlack = Color(0x1e, 0x1e, 0x1e);
Color xpf::Colors::XpfGray = Color(0xc1, 0xc1, 0xc4);
Color xpf::Colors::XpfWhite = Color(220, 220, 222);
Color xpf::Colors::XpfRed = Color(255, 102, 0, 220);
Color xpf::Colors::Honey = Color(255, 165, 0, 128);
Color xpf::Colors::BlueSuede  = Color(0,120,120, 128);

static char read_hex_digit(char b) {
    if (b >= 'A' && b <= 'F')
        return b - 'A' + 10;
    if (b >= 'a' && b <= 'f')
        return b - 'a' + 10;
    if (b >= '0' && b <= '9')
        return b - '0';

    return -1;
}

static byte_t read_hex_two_digits(std::string_view name, size_t& i) {
    uint8_t a = read_hex_digit(name[i++]);
    uint8_t b = read_hex_digit(name[i++]);
    return (byte_t)(a << 4) | b;
}

/*static*/ xpf::Color xpf::Color::to_color(std::string_view name) {
    xpf::Color result;
    if (!try_get_color(name, result))
        result = xpf::Colors::Purple;
    return result;
}

/*static*/ bool xpf::Color::try_get_color(std::string_view name, xpf::Color& result) {
    if (name.empty()) {
        result = xpf::Colors::Transparent;
        return true;
    }

    if (name[0] == '#') {
        result = xpf::Color(0,0,0,255);
        size_t i = 1;
        size_t count = name.length() - 1;
        if (count == 3 || count == 4) {
            if (count == 4) {
                result.a = read_hex_digit(name[i++]); result.a |= (result.a << 4);
            }
            result.r = read_hex_digit(name[i++]); result.r |= (result.r << 4);
            result.g = read_hex_digit(name[i++]); result.g |= (result.g << 4);
            result.b = read_hex_digit(name[i++]); result.b |= (result.b << 4);
            return true;
        } else if (count == 8 || count == 6) {
            if (count == 8)
                result.a = read_hex_two_digits(name, i);
            result.r = read_hex_two_digits(name, i);
            result.g = read_hex_two_digits(name, i);
            result.b = read_hex_two_digits(name, i);
            return true;
        }

        return false;
    }

    static std::unordered_map<std::string_view, xpf::Color> s_colors({
        // wpf Colors -------------------
        { "AliceBlue", xpf::Colors::AliceBlue },
        { "AntiqueWhite", xpf::Colors::AntiqueWhite },
        { "Aqua", xpf::Colors::Aqua },
        { "Aquamarine", xpf::Colors::Aquamarine },
        { "Azure", xpf::Colors::Azure },
        { "Beige", xpf::Colors::Beige },
        { "Bisque", xpf::Colors::Bisque },
        { "Black", xpf::Colors::Black },
        { "BlanchedAlmond", xpf::Colors::BlanchedAlmond },
        { "Blue", xpf::Colors::Blue },
        { "BlueViolet", xpf::Colors::BlueViolet },
        { "Brown", xpf::Colors::Brown },
        { "BurlyWood", xpf::Colors::BurlyWood },
        { "CadetBlue", xpf::Colors::CadetBlue },
        { "Chartreuse", xpf::Colors::Chartreuse },
        { "Chocolate", xpf::Colors::Chocolate },
        { "Coral", xpf::Colors::Coral },
        { "CornflowerBlue", xpf::Colors::CornflowerBlue },
        { "Cornsilk", xpf::Colors::Cornsilk },
        { "Crimson", xpf::Colors::Crimson },
        { "Cyan", xpf::Colors::Cyan },
        { "DarkBlue", xpf::Colors::DarkBlue },
        { "DarkCyan", xpf::Colors::DarkCyan },
        { "DarkGoldenrod", xpf::Colors::DarkGoldenrod },
        { "DarkGray", xpf::Colors::DarkGray },
        { "DarkGreen", xpf::Colors::DarkGreen },
        { "DarkKhaki", xpf::Colors::DarkKhaki },
        { "DarkMagenta", xpf::Colors::DarkMagenta },
        { "DarkOliveGreen", xpf::Colors::DarkOliveGreen },
        { "DarkOrange", xpf::Colors::DarkOrange },
        { "DarkOrchid", xpf::Colors::DarkOrchid },
        { "DarkRed", xpf::Colors::DarkRed },
        { "DarkSalmon", xpf::Colors::DarkSalmon },
        { "DarkSeaGreen", xpf::Colors::DarkSeaGreen },
        { "DarkSlateBlue", xpf::Colors::DarkSlateBlue },
        { "DarkSlateGray", xpf::Colors::DarkSlateGray },
        { "DarkTurquoise", xpf::Colors::DarkTurquoise },
        { "DarkViolet", xpf::Colors::DarkViolet },
        { "DeepPink", xpf::Colors::DeepPink },
        { "DeepSkyBlue", xpf::Colors::DeepSkyBlue },
        { "DimGray", xpf::Colors::DimGray },
        { "DodgerBlue", xpf::Colors::DodgerBlue },
        { "Firebrick", xpf::Colors::Firebrick },
        { "FloralWhite", xpf::Colors::FloralWhite },
        { "ForestGreen", xpf::Colors::ForestGreen },
        { "Fuchsia", xpf::Colors::Fuchsia },
        { "Gainsboro", xpf::Colors::Gainsboro },
        { "GhostWhite", xpf::Colors::GhostWhite },
        { "Gold", xpf::Colors::Gold },
        { "Goldenrod", xpf::Colors::Goldenrod },
        { "Gray", xpf::Colors::Gray },
        { "Green", xpf::Colors::Green },
        { "GreenYellow", xpf::Colors::GreenYellow },
        { "Honeydew", xpf::Colors::Honeydew },
        { "HotPink", xpf::Colors::HotPink },
        { "IndianRed", xpf::Colors::IndianRed },
        { "Indigo", xpf::Colors::Indigo },
        { "Ivory", xpf::Colors::Ivory },
        { "Khaki", xpf::Colors::Khaki },
        { "Lavender", xpf::Colors::Lavender },
        { "LavenderBlush", xpf::Colors::LavenderBlush },
        { "LawnGreen", xpf::Colors::LawnGreen },
        { "LemonChiffon", xpf::Colors::LemonChiffon },
        { "LightBlue", xpf::Colors::LightBlue },
        { "LightCoral", xpf::Colors::LightCoral },
        { "LightCyan", xpf::Colors::LightCyan },
        { "LightGoldenrodYellow", xpf::Colors::LightGoldenrodYellow },
        { "LightGray", xpf::Colors::LightGray },
        { "LightGreen", xpf::Colors::LightGreen },
        { "LightPink", xpf::Colors::LightPink },
        { "LightSalmon", xpf::Colors::LightSalmon },
        { "LightSeaGreen", xpf::Colors::LightSeaGreen },
        { "LightSkyBlue", xpf::Colors::LightSkyBlue },
        { "LightSlateGray", xpf::Colors::LightSlateGray },
        { "LightSteelBlue", xpf::Colors::LightSteelBlue },
        { "LightYellow", xpf::Colors::LightYellow },
        { "Lime", xpf::Colors::Lime },
        { "LimeGreen", xpf::Colors::LimeGreen },
        { "Linen", xpf::Colors::Linen },
        { "Magenta", xpf::Colors::Magenta },
        { "Maroon", xpf::Colors::Maroon },
        { "MediumAquamarine", xpf::Colors::MediumAquamarine },
        { "MediumBlue", xpf::Colors::MediumBlue },
        { "MediumOrchid", xpf::Colors::MediumOrchid },
        { "MediumPurple", xpf::Colors::MediumPurple },
        { "MediumSeaGreen", xpf::Colors::MediumSeaGreen },
        { "MediumSlateBlue", xpf::Colors::MediumSlateBlue },
        { "MediumSpringGreen", xpf::Colors::MediumSpringGreen },
        { "MediumTurquoise", xpf::Colors::MediumTurquoise },
        { "MediumVioletRed", xpf::Colors::MediumVioletRed },
        { "MidnightBlue", xpf::Colors::MidnightBlue },
        { "MintCream", xpf::Colors::MintCream },
        { "MistyRose", xpf::Colors::MistyRose },
        { "Moccasin", xpf::Colors::Moccasin },
        { "NavajoWhite", xpf::Colors::NavajoWhite },
        { "Navy", xpf::Colors::Navy },
        { "OldLace", xpf::Colors::OldLace },
        { "Olive", xpf::Colors::Olive },
        { "OliveDrab", xpf::Colors::OliveDrab },
        { "Orange", xpf::Colors::Orange },
        { "OrangeRed", xpf::Colors::OrangeRed },
        { "Orchid", xpf::Colors::Orchid },
        { "PaleGoldenrod", xpf::Colors::PaleGoldenrod },
        { "PaleGreen", xpf::Colors::PaleGreen },
        { "PaleTurquoise", xpf::Colors::PaleTurquoise },
        { "PaleVioletRed", xpf::Colors::PaleVioletRed },
        { "PapayaWhip", xpf::Colors::PapayaWhip },
        { "PeachPuff", xpf::Colors::PeachPuff },
        { "Peru", xpf::Colors::Peru },
        { "Pink", xpf::Colors::Pink },
        { "Plum", xpf::Colors::Plum },
        { "PowderBlue", xpf::Colors::PowderBlue },
        { "Purple", xpf::Colors::Purple },
        { "Red", xpf::Colors::Red },
        { "RosyBrown", xpf::Colors::RosyBrown },
        { "RoyalBlue", xpf::Colors::RoyalBlue },
        { "SaddleBrown", xpf::Colors::SaddleBrown },
        { "Salmon", xpf::Colors::Salmon },
        { "SandyBrown", xpf::Colors::SandyBrown },
        { "SeaGreen", xpf::Colors::SeaGreen },
        { "SeaShell", xpf::Colors::SeaShell },
        { "Sienna", xpf::Colors::Sienna },
        { "Silver", xpf::Colors::Silver },
        { "SkyBlue", xpf::Colors::SkyBlue },
        { "SlateBlue", xpf::Colors::SlateBlue },
        { "SlateGray", xpf::Colors::SlateGray },
        { "Snow", xpf::Colors::Snow },
        { "SpringGreen", xpf::Colors::SpringGreen },
        { "SteelBlue", xpf::Colors::SteelBlue },
        { "Tan", xpf::Colors::Tan },
        { "Teal", xpf::Colors::Teal },
        { "Thistle", xpf::Colors::Thistle },
        { "Tomato", xpf::Colors::Tomato },
        { "Transparent", xpf::Colors::Transparent },
        { "Turquoise", xpf::Colors::Turquoise },
        { "Violet", xpf::Colors::Violet },
        { "Wheat", xpf::Colors::Wheat },
        { "White", xpf::Colors::White },
        { "WhiteSmoke", xpf::Colors::WhiteSmoke },
        { "Yellow", xpf::Colors::Yellow },
        { "YellowGreen", xpf::Colors::YellowGreen },
        // xpf Colors -------------------
        { "XpfBlack", xpf::Colors::XpfBlack },
        { "XpfGray", xpf::Colors::XpfGray },
        { "XpfWhite", xpf::Colors::XpfWhite },
        { "XpfRed", xpf::Colors::XpfRed },
        { "Honey", xpf::Colors::Honey },
        { "BlueSuede", xpf::Colors::BlueSuede },
    });

    auto iter = s_colors.find(name);
    if (iter == s_colors.cend()) {
        // log::info("Colors - unknown Color: " + name);
        return false;
    }

    result = iter->second;
    return true;
}

} // xpf