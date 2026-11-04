
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <array>
#include <cstddef>
#include <span>

module beatuy.feature.civilization;

import beatuy.core.language;

namespace civilization {
namespace {
struct Bribe {
    Language language;
    const char16_t* offer;
};

constexpr PerLanguage<Bribe> bribes = std::to_array<Bribe>({
    {Language::AmericanEnglish,
     u"A unit left without backup offers you # Sporebucks if you stop attacking it. Click to accept."},
    {Language::BritishEnglish,
     u"A unit left without backup offers you # Sporebucks if you stop attacking it. Click to accept."},
    {Language::Czech,
     u"Jednotka, která zůstala bez podpory, vám nabízí # Sporáčů, pokud ji přestanete napadat. Klikněte pro přijetí."},
    {Language::Danish,
     u"En enhed uden forstærkninger tilbyder dig # Sporebucks, hvis du holder op med at angribe den. Klik for at "
     u"acceptere."},
    {Language::German,
     u"Eine Einheit ohne Verstärkung bietet dir # Spore-Dollars, wenn du aufhörst, sie anzugreifen. Klicke, um "
     u"anzunehmen."},
    {Language::Greek,
     u"Μια μονάδα χωρίς υποστήριξη σας προσφέρει # Σποροδολάρια για να σταματήσετε να της επιτίθεστε. Κάντε κλικ για "
     u"αποδοχή."},
    {Language::Spanish, u"Una unidad sin refuerzos te ofrece # esporetas si dejas de atacarla. Haz clic para aceptar."},
    {Language::Finnish,
     u"Ilman tukea jäänyt yksikkö tarjoaa sinulle # Spore-taaleria, jos lakkaat hyökkäämästä sen kimppuun. Hyväksy "
     u"napsauttamalla."},
    {Language::French,
     u"Une unité privée de renforts vous offre # Sporeflouz si vous cessez de l'attaquer. Cliquez pour accepter."},
    {Language::Hungarian,
     u"Egy támogatás nélkül maradt egység # spóradollárt ajánl, ha abbahagyod a támadását. Kattints az elfogadáshoz."},
    {Language::Italian,
     u"Un'unità rimasta senza rinforzi ti offre # Sporecrediti se smetti di attaccarla. Fai clic per accettare."},
    {Language::Japanese,
     u"援軍を失ったユニットが、攻撃をやめれば#スポアバックスを支払うと申し出ています。クリックで受け入れます。"},
    {Language::Korean, u"지원을 잃은 유닛이 공격을 멈추면 # Spore 머니를 주겠다고 제안합니다. 클릭하여 수락하세요."},
    {Language::Dutch,
     u"Een eenheid zonder versterking biedt je # Sporedollars als je stopt met aanvallen. Klik om te accepteren."},
    {Language::Norwegian,
     u"En enhet uten forsterkninger tilbyr deg # Sporespenn hvis du slutter å angripe den. Klikk for å godta."},
    {Language::Polish,
     u"Jednostka pozbawiona wsparcia oferuje ci # sporylionów, jeśli przestaniesz ją atakować. Kliknij, aby przyjąć."},
    {Language::BrazilianPortuguese,
     u"Uma unidade sem reforços oferece # Sporegranas para você parar de atacá-la. Clique para aceitar."},
    {Language::EuropeanPortuguese,
     u"Uma unidade sem reforços oferece-lhe # Sporecoroas se deixar de a atacar. Clique para aceitar."},
    {Language::Russian,
     u"Оставшийся без подмоги юнит предлагает вам # спорл., если вы прекратите его атаковать. Щёлкните, чтобы "
     u"принять."},
    {Language::Swedish,
     u"En enhet utan förstärkningar erbjuder dig # Sporebucks om du slutar anfalla den. Klicka för att acceptera."},
    {Language::Thai, u"หน่วยที่ไม่มีกำลังเสริมเสนอให้คุณ # Sporebaht หากคุณหยุดโจมตี คลิกเพื่อยอมรับ"},
    {Language::SimplifiedChinese, u"失去支援的单位愿意付给你#Spore币，只要你停止攻击它。点击接受。"},
    {Language::TraditionalChinese, u"失去支援的單位願意付給你 # Spore幣，只要你停止攻擊它。點擊接受。"},
});
static_assert(inLanguageOrder(bribes));
}

const wchar_t* Text::bribeOffer(int amount, std::span<wchar_t> buffer) {
    if (!buffer.data() || buffer.empty())
        return L"";
    std::array<wchar_t, 12> digits;
    size_t count = 0;
    unsigned value = amount < 0 ? 0u : unsigned(amount);
    do {
        digits[count++] = wchar_t(L'0' + value % 10);
        value /= 10;
    } while (value && count < 11);
    size_t length = 0;
    const char16_t* offer = inLanguage(bribes, activeLanguage()).offer;
    for (const char16_t* cursor = offer; *cursor && length + 1 < buffer.size(); ++cursor) {
        if (*cursor != u'#') {
            buffer[length++] = wchar_t(*cursor);
            continue;
        }
        for (size_t remaining = count; remaining > 0 && length + 1 < buffer.size(); --remaining)
            buffer[length++] = digits[remaining - 1];
    }
    buffer[length] = 0;
    return buffer.data();
}
}

