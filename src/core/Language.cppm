
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

export module beatuy.core.language;

import beatuy.game;

export {
enum class Language : uint8_t {
    AmericanEnglish,
    BritishEnglish,
    Czech,
    Danish,
    German,
    Greek,
    Spanish,
    Finnish,
    French,
    Hungarian,
    Italian,
    Japanese,
    Korean,
    Dutch,
    Norwegian,
    Polish,
    BrazilianPortuguese,
    EuropeanPortuguese,
    Russian,
    Swedish,
    Thai,
    SimplifiedChinese,
    TraditionalChinese,
};

inline constexpr size_t languageCount = size_t(Language::TraditionalChinese) + 1;

template <class Value> using PerLanguage = std::array<Value, languageCount>;

inline constexpr PerLanguage<const char*> languageCodes = std::to_array<const char*>({
    "en-us", "en-gb", "cs-cz", "da-dk", "de-de", "el-gr", "es-es", "fi-fi", "fr-fr", "hu-hu", "it-it", "ja-jp",
    "ko-kr", "nl-nl", "no-no", "pl-pl", "pt-br", "pt-pt", "ru-ru", "sv-se", "th-th", "zh-cn", "zh-tw",
});

template <class Value> constexpr const Value& inLanguage(const PerLanguage<Value>& values, Language language) {
    return values[size_t(language)];
}

constexpr const char* languageCode(Language language) {
    return inLanguage(languageCodes, language);
}

template <class Row> consteval bool inLanguageOrder(const PerLanguage<Row>& rows) {
    for (size_t i = 0; i < rows.size(); ++i)
        if (rows[i].language != Language(i))
            return false;
    return true;
}

Language activeLanguage();
}

module :private;

namespace {
constinit std::atomic<int> knownLanguage{-1};
static_assert(std::atomic<int>::is_always_lock_free);

char16_t lower(char16_t character) {
    return character >= u'A' && character <= u'Z' ? char16_t(character - u'A' + u'a')
           : character == u'_'                    ? u'-'
                                                  : character;
}

std::optional<Language> match(std::u16string_view code) {
    for (size_t compare : std::array<size_t, 2>{5, 2}) {
        if (code.size() < compare)
            continue;
        auto same = [&](const char* known) {
            size_t matched = 0;
            while (matched < compare && lower(code[matched]) == char16_t(known[matched]))
                ++matched;
            return matched == compare;
        };
        if (auto found = std::ranges::find_if(languageCodes, same); found != languageCodes.end())
            return Language(found - languageCodes.begin());
    }
    return std::nullopt;
}

Language fromCommandLine() {
    const wchar_t* line = GetCommandLineW();
    for (const wchar_t* cursor = line; cursor && *cursor; ++cursor) {
        if ((*cursor != L'-' && *cursor != L'/') || (cursor > line && cursor[-1] != L' ' && cursor[-1] != L'"'))
            continue;
        const wchar_t* key = L"locale:";
        size_t matched = 0;
        while (key[matched] && lower(char16_t(cursor[1 + matched])) == char16_t(key[matched]))
            ++matched;
        if (key[matched])
            continue;
        const wchar_t* value = cursor + 1 + matched;
        if (*value == L'"')
            ++value;
        size_t length = 0;
        while (value[length] && value[length] != L'"' && value[length] != L' ' && length < 8)
            ++length;
        if (std::optional<Language> language =
                match(std::u16string_view(reinterpret_cast<const char16_t*>(value), length)))
            return *language;
    }
    return Language::AmericanEnglish;
}
}

Language activeLanguage() {
    if (int known = knownLanguage.load(std::memory_order_relaxed); known >= 0)
        return Language(known);
    void* manager = map::application::localeManager();
    if (!manager)
        return fromCommandLine();
    auto language = game::callVirtual<const char16_t* const*>(manager, 0x14);
    if (!language || !language[0] || language[1] <= language[0])
        return fromCommandLine();
    std::optional<Language> named = match(std::u16string_view(language[0], size_t(language[1] - language[0])));
    if (!named)
        return Language::AmericanEnglish;
    int unknown = -1;
    knownLanguage.compare_exchange_strong(unknown, int(*named), std::memory_order_relaxed);
    return *named;
}

