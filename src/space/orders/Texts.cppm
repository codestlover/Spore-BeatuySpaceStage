
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <array>
#include <cstddef>

export module beatuy.feature.orders:texts;

import :book;
import beatuy.core.language;

export namespace feature {
struct OrderWords {
    const char16_t* caption;
    const char16_t* menuCaption;
    const char16_t* description;
};

struct OrdersTranslation {
    Language language;
    const char16_t* tab;
    const char16_t* tabDescription;
    const char16_t* everyone;
    const char16_t* everyoneDescription;
    PerOrder<OrderWords> orders;
};

inline constexpr PerLanguage<OrdersTranslation> ordersTranslations = std::to_array<OrdersTranslation>({
    {Language::AmericanEnglish, u"Orders", u"Choose how each allied ship fights.", u"All",
     u"Protects every allied ship and your own.",
     {{
         {u"Free", u"Free (default)", u"Acts on its own, as allied ships usually do."},
         {u"Protect me", u"Protect my ship", u"Fights only those who attack your ship."},
         {u"Protect others", u"Protect others...", u"Fights only those who attack the allied ships you choose."},
         {u"Protect itself", u"Protect itself", u"Fights only those who attack it."},
         {u"Attack nearby", u"Attack hostiles nearby", u"Attacks the hostile ships near it."},
     }}},
    {Language::BritishEnglish, u"Orders", u"Choose how each allied ship fights.", u"All",
     u"Protects every allied ship and your own.",
     {{
         {u"Free", u"Free (default)", u"Acts on its own, as allied ships usually do."},
         {u"Protect me", u"Protect my ship", u"Fights only those who attack your ship."},
         {u"Protect others", u"Protect others...", u"Fights only those who attack the allied ships you choose."},
         {u"Protect itself", u"Protect itself", u"Fights only those who attack it."},
         {u"Attack nearby", u"Attack hostiles nearby", u"Attacks the hostile ships near it."},
     }}},
    {Language::Czech, u"Příkazy", u"Zvolte, jak bojuje každá spojenecká loď.", u"Všechny",
     u"Chrání všechny spojenecké lodě i vaši loď.",
     {{
         {u"Volně", u"Volně (výchozí)", u"Jedná po svém, jako obvykle."},
         {u"Chraň mě", u"Chraň mou loď", u"Bojuje jen s těmi, kdo útočí na vaši loď."},
         {u"Chraň ostatní", u"Chraň ostatní…", u"Bojuje jen s těmi, kdo útočí na vámi zvolené spojenecké lodě."},
         {u"Chraň se", u"Chraň se", u"Bojuje jen s těmi, kdo útočí na ni."},
         {u"Útoč poblíž", u"Útoč na nepřátele poblíž", u"Útočí na nepřátelské lodě ve své blízkosti."},
     }}},
    {Language::Danish, u"Ordrer", u"Vælg, hvordan hvert allieret rumskib kæmper.", u"Alle",
     u"Beskytter alle allierede rumskibe og dit eget.",
     {{
         {u"Fri", u"Fri (standard)", u"Handler på egen hånd som sædvanligt."},
         {u"Beskyt mig", u"Beskyt mit skib", u"Kæmper kun mod dem, der angriber dit skib."},
         {u"Beskyt andre", u"Beskyt andre ...", u"Kæmper kun mod dem, der angriber de allierede rumskibe, du vælger."},
         {u"Selvforsvar", u"Selvforsvar", u"Kæmper kun mod dem, der angriber det."},
         {u"Angrib", u"Angrib fjender tæt på", u"Angriber fjendtlige rumskibe i nærheden."},
     }}},
    {Language::German, u"Befehle", u"Lege fest, wie jedes verbündete Schiff kämpft.", u"Alle",
     u"Schützt alle verbündeten Schiffe und dein Schiff.",
     {{
         {u"Frei", u"Frei (Standard)", u"Handelt wie gewohnt nach eigenem Ermessen."},
         {u"Mich schützen", u"Mein Schiff schützen", u"Kämpft nur gegen die, die dein Schiff angreifen."},
         {u"Bündnisschutz", u"Verbündete schützen ...",
          u"Kämpft nur gegen Angreifer der verbündeten Schiffe, die du wählst."},
         {u"Selbstschutz", u"Selbstschutz", u"Kämpft nur gegen die, die es angreifen."},
         {u"Angreifen", u"Nahe Feinde angreifen", u"Greift feindliche Schiffe in seiner Nähe an."},
     }}},
    {Language::Greek, u"Εντολές", u"Επιλέξτε πώς πολεμά κάθε συμμαχικό σκάφος.", u"Όλα",
     u"Προστατεύει όλα τα συμμαχικά σκάφη και το δικό σας.",
     {{
         {u"Ελεύθερο", u"Ελεύθερο (προεπιλογή)", u"Ενεργεί με δική του πρωτοβουλία, όπως συνήθως."},
         {u"Φύλαξέ με", u"Φύλαξε το σκάφος μου", u"Πολεμά μόνο όσους επιτίθενται στο σκάφος σας."},
         {u"Φύλαξε άλλους", u"Φύλαξε άλλους...", u"Πολεμά μόνο όσους επιτίθενται στα συμμαχικά σκάφη που επιλέγετε."},
         {u"Αυτοάμυνα", u"Αυτοάμυνα", u"Πολεμά μόνο όσους του επιτίθενται."},
         {u"Επίθεση", u"Επίθεση σε κοντινούς εχθρούς", u"Επιτίθεται στα εχθρικά σκάφη κοντά του."},
     }}},
    {Language::Spanish, u"Órdenes", u"Elige cómo combate cada nave aliada.", u"Todas",
     u"Protege a todas las naves aliadas y a la tuya.",
     {{
         {u"Libre", u"Libre (por defecto)", u"Actúa por su cuenta, como de costumbre."},
         {u"Protegerme", u"Proteger mi nave", u"Solo lucha contra quienes atacan tu nave."},
         {u"Proteger otras", u"Proteger otras...", u"Solo lucha contra quienes atacan las naves aliadas que elijas."},
         {u"Protegerse", u"Protegerse", u"Solo lucha contra quienes la atacan."},
         {u"Atacar", u"Atacar enemigos cerca", u"Ataca a las naves hostiles que estén cerca."},
     }}},
    {Language::Finnish, u"Komennot", u"Valitse, miten kukin liittolaisalus taistelee.", u"Kaikki",
     u"Suojelee kaikkia liittolaisaluksia ja omaa alustasi.",
     {{
         {u"Vapaa", u"Vapaa (oletus)", u"Toimii omin päin tavalliseen tapaan."},
         {u"Suojele minua", u"Suojele alustani", u"Taistelee vain aluksesi kimppuun hyökkääviä vastaan."},
         {u"Suojele muita", u"Suojele muita...",
          u"Taistelee vain valitsemiesi liittolaisalusten kimppuun hyökkääviä vastaan."},
         {u"Itsepuolustus", u"Itsepuolustus", u"Taistelee vain kimppuunsa hyökkääviä vastaan."},
         {u"Hyökkää", u"Hyökkää lähellä oleviin", u"Hyökkää lähellään olevien vihamielisten alusten kimppuun."},
     }}},
    {Language::French, u"Ordres", u"Choisissez comment combat chaque soucoupe alliée.", u"Toutes",
     u"Protège toutes les soucoupes alliées et la vôtre.",
     {{
         {u"Libre", u"Libre (par défaut)", u"Agit à sa guise, comme d'habitude."},
         {u"Me protéger", u"Protéger ma soucoupe", u"Combat uniquement ceux qui attaquent votre soucoupe."},
         {u"Protéger alliés", u"Protéger des alliés...",
          u"Combat uniquement ceux qui attaquent les soucoupes alliées que vous choisissez."},
         {u"Se protéger", u"Se protéger", u"Combat uniquement ceux qui l'attaquent."},
         {u"Attaquer", u"Attaquer à proximité", u"Attaque les vaisseaux hostiles proches."},
     }}},
    {Language::Hungarian, u"Parancsok", u"Add meg, hogyan harcoljon minden szövetséges hajó.", u"Mind",
     u"Minden szövetséges hajót és a hajódat is megvédi.",
     {{
         {u"Szabad", u"Szabad (alapbeállítás)", u"A szokott módon, saját belátása szerint cselekszik."},
         {u"Védj engem", u"Védd a hajómat", u"Csak azokkal harcol, akik a hajódat támadják."},
         {u"Védj másokat", u"Védj másokat...",
          u"Csak azokkal harcol, akik az általad választott szövetséges hajókat támadják."},
         {u"Védd magad", u"Védd magad", u"Csak a saját támadóival harcol."},
         {u"Támadás", u"Támadás a közelben", u"Megtámadja a közelében lévő ellenséges hajókat."},
     }}},
    {Language::Italian, u"Ordini", u"Scegli come combatte ogni nave alleata.", u"Tutte",
     u"Protegge tutte le navi alleate e la tua.",
     {{
         {u"Libero", u"Libero (predefinito)", u"Agisce di sua iniziativa, come al solito."},
         {u"Proteggimi", u"Proteggi la mia nave", u"Combatte solo chi attacca la tua nave."},
         {u"Proteggi altre", u"Proteggi altre navi...", u"Combatte solo chi attacca le navi alleate che scegli."},
         {u"Proteggiti", u"Proteggiti", u"Combatte solo chi la attacca."},
         {u"Attacca", u"Attacca nemici vicini", u"Attacca le navi ostili vicine."},
     }}},
    {Language::Japanese, u"命令", u"艦隊の各宇宙船の戦い方を選べます。", u"すべて",
     u"艦隊のすべての宇宙船とあなたの宇宙船を守ります。",
     {{
         {u"自由", u"自由（デフォルト）", u"いつも通り、自らの判断で行動します。"},
         {u"あなたを守る", u"あなたの宇宙船を守る", u"あなたの宇宙船を攻撃する敵とだけ戦います。"},
         {u"他の船を守る", u"他の船を守る…", u"選んだ味方の宇宙船を攻撃する敵とだけ戦います。"},
         {u"自衛", u"自衛", u"自らを攻撃する敵とだけ戦います。"},
         {u"敵を攻撃", u"近くの敵を攻撃", u"近くにいる敵対的な宇宙船を攻撃します。"},
     }}},
    {Language::Korean, u"명령", u"각 동맹 우주선의 전투 방식을 고릅니다.", u"모두",
     u"모든 동맹 우주선과 당신의 우주선을 보호합니다.",
     {{
         {u"자유", u"자유 (기본)", u"평소처럼 스스로 판단해 행동합니다."},
         {u"나를 보호", u"내 우주선 보호", u"당신의 우주선을 공격하는 적과만 싸웁니다."},
         {u"아군 보호", u"아군 보호…", u"당신이 고른 동맹 우주선을 공격하는 적과만 싸웁니다."},
         {u"자기 방어", u"자기 방어", u"자신을 공격하는 적과만 싸웁니다."},
         {u"적 공격", u"근처의 적 공격", u"근처의 적대적인 우주선을 공격합니다."},
     }}},
    {Language::Dutch, u"Bevelen", u"Kies hoe elk schip van je bondgenoten vecht.", u"Alle",
     u"Beschermt alle schepen van je bondgenoten en je eigen schip.",
     {{
         {u"Vrij", u"Vrij (standaard)", u"Handelt zelfstandig, zoals gewoonlijk."},
         {u"Bescherm mij", u"Bescherm mijn schip", u"Vecht alleen tegen wie je schip aanvalt."},
         {u"Bescherm anderen", u"Bescherm anderen…",
          u"Vecht alleen tegen wie de door jou gekozen schepen van bondgenoten aanvalt."},
         {u"Zelfverweer", u"Zelfverweer", u"Vecht alleen tegen wie het zelf aanvalt."},
         {u"Aanvallen", u"Aanvallen in de buurt", u"Valt vijandelijke schepen in de buurt aan."},
     }}},
    {Language::Norwegian, u"Ordrer", u"Velg hvordan hvert alliert skip kjemper.", u"Alle",
     u"Beskytter alle allierte skip og ditt eget.",
     {{
         {u"Fri", u"Fri (standard)", u"Handler på egen hånd, som vanlig."},
         {u"Beskytt meg", u"Beskytt skipet mitt", u"Kjemper bare mot dem som angriper skipet ditt."},
         {u"Beskytt andre", u"Beskytt andre...", u"Kjemper bare mot dem som angriper de allierte skipene du velger."},
         {u"Selvforsvar", u"Selvforsvar", u"Kjemper bare mot dem som angriper det."},
         {u"Angrip", u"Angrip fiender i nærheten", u"Angriper fiendtlige skip i nærheten."},
     }}},
    {Language::Polish, u"Rozkazy", u"Wybierz, jak walczy każdy statek sprzymierzeńców.", u"Wszystkie",
     u"Chroni wszystkie statki sprzymierzeńców i Twój statek.",
     {{
         {u"Swobodny", u"Swobodny (domyślny)", u"Działa na własną rękę, jak zwykle."},
         {u"Chroń mnie", u"Chroń mój statek", u"Walczy tylko z tymi, którzy atakują Twój statek."},
         {u"Chroń inne", u"Chroń inne statki...",
          u"Walczy tylko z tymi, którzy atakują wybrane przez Ciebie statki sprzymierzeńców."},
         {u"Samoobrona", u"Samoobrona", u"Walczy tylko z tymi, którzy go atakują."},
         {u"Atakuj wroga", u"Atakuj wroga w pobliżu", u"Atakuje wrogie statki w pobliżu."},
     }}},
    {Language::BrazilianPortuguese, u"Ordens", u"Escolha como cada nave aliada combate.", u"Todas",
     u"Protege todas as naves aliadas e a sua.",
     {{
         {u"Livre", u"Livre (padrão)", u"Age por conta própria, como de costume."},
         {u"Proteger-me", u"Proteger minha nave", u"Só combate quem ataca a sua nave."},
         {u"Proteger outras", u"Proteger outras...", u"Só combate quem ataca as naves aliadas que você escolher."},
         {u"Proteger-se", u"Proteger-se", u"Só combate quem a ataca."},
         {u"Atacar", u"Atacar inimigos perto", u"Ataca as naves hostis próximas a ela."},
     }}},
    {Language::EuropeanPortuguese, u"Ordens", u"Escolha como combate cada nave aliada.", u"Todas",
     u"Protege todas as naves aliadas e a sua.",
     {{
         {u"Livre", u"Livre (predefinido)", u"Age por iniciativa própria, como habitualmente."},
         {u"Proteger-me", u"Proteger a minha nave", u"Só combate quem ataca a sua nave."},
         {u"Proteger outras", u"Proteger outras...", u"Só combate quem ataca as naves aliadas que escolher."},
         {u"Proteger-se", u"Proteger-se", u"Só combate quem a ataca."},
         {u"Atacar", u"Atacar inimigos perto", u"Ataca as naves hostis que estão por perto."},
     }}},
    {Language::Russian, u"Приказы", u"Выберите, как сражается каждый корабль союзников.", u"Все",
     u"Защищает все корабли союзников и ваш корабль.",
     {{
         {u"Вольный", u"Вольный (стандарт)", u"Действует по своему усмотрению, как обычно."},
         {u"Защита игрока", u"Защита корабля игрока", u"Сражается только с теми, кто нападает на ваш корабль."},
         {u"Защита других", u"Защита союзников...",
          u"Сражается только с теми, кто нападает на выбранных вами союзников."},
         {u"Самозащита", u"Самозащита", u"Сражается только с теми, кто нападает на него."},
         {u"Атака врагов", u"Атака врагов рядом", u"Атакует вражеские корабли поблизости."},
     }}},
    {Language::Swedish, u"Order", u"Välj hur varje allierat skepp strider.", u"Alla",
     u"Skyddar alla allierade skepp och ditt eget.",
     {{
         {u"Fri", u"Fri (standard)", u"Agerar på eget initiativ, som vanligt."},
         {u"Skydda mig", u"Skydda mitt skepp", u"Strider bara mot dem som anfaller ditt skepp."},
         {u"Skydda andra", u"Skydda andra ...", u"Strider bara mot dem som anfaller de allierade skepp du väljer."},
         {u"Självförsvar", u"Självförsvar", u"Strider bara mot dem som anfaller det."},
         {u"Anfall", u"Anfall fiender i närheten", u"Anfaller fientliga skepp i närheten."},
     }}},
    {Language::Thai, u"คำสั่ง", u"เลือกวิธีต่อสู้ของยานพันธมิตรแต่ละลำ", u"ทั้งหมด",
     u"คุ้มครองยานพันธมิตรทุกลำและยานของคุณ",
     {{
         {u"อิสระ", u"อิสระ (ปกติ)", u"ปฏิบัติการเองตามปกติ"},
         {u"คุ้มครองคุณ", u"คุ้มครองยานของคุณ", u"สู้เฉพาะกับผู้ที่โจมตียานของคุณ"},
         {u"คุ้มครองยานอื่น", u"คุ้มครองยานอื่น...", u"สู้เฉพาะกับผู้ที่โจมตียานพันธมิตรที่คุณเลือก"},
         {u"ป้องกันตัวเอง", u"ป้องกันตัวเอง", u"สู้เฉพาะกับผู้ที่โจมตีตัวมันเอง"},
         {u"โจมตีศัตรู", u"โจมตีศัตรูใกล้ๆ", u"โจมตียานข้าศึกที่อยู่ใกล้ๆ"},
     }}},
    {Language::SimplifiedChinese, u"命令", u"选择每艘盟军飞船的作战方式。", u"全部", u"保护所有盟军飞船和你的飞船。",
     {{
         {u"自由", u"自由（默认）", u"像平常一样自主行动。"},
         {u"保护我", u"保护我的飞船", u"只与攻击你飞船的敌人作战。"},
         {u"保护其他", u"保护其他飞船……", u"只与攻击你所选盟军飞船的敌人作战。"},
         {u"自卫", u"自卫", u"只与攻击它的敌人作战。"},
         {u"攻击敌人", u"攻击附近的敌人", u"攻击附近的敌对飞船。"},
     }}},
    {Language::TraditionalChinese, u"命令", u"選擇每艘同盟船艦的戰鬥方式。", u"全部", u"保護所有同盟船艦和你的船隻。",
     {{
         {u"自由", u"自由（預設）", u"像平常一樣自行行動。"},
         {u"保護我", u"保護我的船隻", u"只與攻擊你船隻的敵人作戰。"},
         {u"保護其他", u"保護其他船艦...", u"只與攻擊你所選同盟船艦的敵人作戰。"},
         {u"自衛", u"自衛", u"只與攻擊它的敵人作戰。"},
         {u"攻擊敵人", u"攻擊附近的敵人", u"攻擊附近的敵方船艦。"},
     }}},
});

inline const OrdersTranslation& ordersTexts() {
    const size_t language = size_t(activeLanguage());
    return language < ordersTranslations.size() ? ordersTranslations[language] : ordersTranslations[0];
}
}

namespace feature {
consteval bool ordersTranslationsComplete(const PerLanguage<OrdersTranslation>& rows) {
    auto present = [](const char16_t* text) { return text && *text; };
    for (const OrdersTranslation& translation : rows) {
        if (!present(translation.tab) || !present(translation.tabDescription) || !present(translation.everyone) ||
            !present(translation.everyoneDescription))
            return false;
        for (const OrderWords& words : translation.orders)
            if (!present(words.caption) || !present(words.menuCaption) || !present(words.description))
                return false;
    }
    return true;
}

static_assert(inLanguageOrder(ordersTranslations) && ordersTranslationsComplete(ordersTranslations));
}

