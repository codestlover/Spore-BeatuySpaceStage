
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <array>

export module beatuy.feature.generator:texts;

import beatuy.core.language;

namespace generator {
struct Translation {
    Language language;
    const char16_t* name;
    const char16_t* description;
};

consteval bool complete(const PerLanguage<Translation>& rows) {
    for (const Translation& translation : rows)
        if (!translation.name || !*translation.name || !translation.description || !*translation.description)
            return false;
    return true;
}

inline constexpr PerLanguage<Translation> translations = std::to_array<Translation>({
    {Language::AmericanEnglish, u"Glow Generator",
     u"Place on one of your colonies or on an unclaimed planet to tint the glow of the planet's atmosphere in your "
     u"empire's color, unless you choose another. Double-click the generator to open the color palette. Health: 1000."},
    {Language::BritishEnglish, u"Glow Generator",
     u"Place on one of your colonies or on an unclaimed planet to tint the glow of the planet's atmosphere in your "
     u"empire's color, unless you choose another. Double-click the generator to open the color palette. Health: 1000."},
    {Language::Czech, u"Generátor záře",
     u"Umístěním na svou kolonii nebo na nezabranou planetu zbarvíte záři atmosféry planety barvou svého impéria, "
     u"pokud nezvolíte jinou. Dvojitým kliknutím na generátor otevřete paletu barev. 1000 životů."},
    {Language::Danish, u"Glødegenerator",
     u"Placér på en af dine kolonier eller på en uerobret planet for at give gløden i planetens atmosfære dit "
     u"imperiums farve, medmindre du vælger en anden. Dobbeltklik på generatoren for at åbne farvepaletten. "
     u"Holdbarhed: 1000."},
    {Language::German, u"Leuchtgenerator",
     u"Auf einer deiner Kolonien oder auf einem unbesetzten Planeten platzieren, um das Leuchten der "
     u"Planetenatmosphäre einzufärben – in der Farbe deines Reiches, sofern du keine andere wählst. Doppelklicke auf "
     u"den Generator, um die Farbpalette zu öffnen. Gesundheit: 1000."},
    {Language::Greek, u"Γεννήτρια λάμψης",
     u"Τοποθετήστε τη σε μία από τις αποικίες σας ή σε έναν αδιεκδίκητο πλανήτη για να χρωματίσετε τη λάμψη της "
     u"ατμόσφαιρας του πλανήτη με το χρώμα της αυτοκρατορίας σας, εκτός αν επιλέξετε άλλο. Κάντε διπλό κλικ στη "
     u"γεννήτρια για να ανοίξετε την παλέτα. 1.000 μονάδες υγείας."},
    {Language::Spanish, u"Generador de resplandor",
     u"Se coloca en una de tus colonias o en un planeta sin reclamar para teñir el resplandor de la atmósfera del "
     u"planeta con el color de tu imperio, salvo que elijas otro. Haz doble clic en el generador para abrir la "
     u"paleta. Salud: 1000."},
    {Language::Finnish, u"Hehkugeneraattori",
     u"Sijoita siirtokuntaasi tai valtaamattomalle planeetalle, niin se sävyttää planeetan ilmakehän hehkun "
     u"valtakuntasi värillä, ellet valitse toista. Kaksoisnapsauta generaattoria, niin värivalikoima avautuu. Kunto: "
     u"1000."},
    {Language::French, u"Générateur de lueur",
     u"À placer sur une de vos colonies ou sur une planète sans propriétaire pour que la lueur atmosphérique de la "
     u"planète prenne la couleur de votre empire, sauf si vous en choisissez une autre. Double-cliquez sur le "
     u"générateur pour ouvrir la palette des couleurs. État : 1000."},
    {Language::Hungarian, u"Ragyogásgenerátor",
     u"Helyezd el egyik kolóniádon vagy egy még nem foglalt bolygón, és a birodalmad színére festi a bolygó "
     u"légkörének ragyogását, hacsak nem választasz más színt. A színpaletta megnyitásához kattints duplán a "
     u"generátorra. Életerő: 1000."},
    {Language::Italian, u"Generatore di bagliore",
     u"Posiziona su una delle tue colonie o su un pianeta non ancora conquistato per tingere il bagliore "
     u"dell'atmosfera del pianeta con il colore del tuo impero, a meno che tu non ne scelga un altro. Clicca due "
     u"volte sul generatore per aprire il repertorio dei colori. 1.000 punti salute."},
    {Language::Japanese, u"輝き発生装置",
     u"自身のコロニーか、まだ誰にも獲得されていない惑星に設置することで、惑星の大気の輝きを帝国の固有カラーに染めます"
     u"。設置した装置をダブルクリックするとパレットが開き、別の色を選べます。耐久力は1000です。"},
    {Language::Korean, u"광채 발생기",
     u"당신의 식민지나 아직 주인이 없는 행성에 배치하면 행성 대기의 광채를 제국의 고유색으로 물들입니다. 다른 색은 "
     u"발생기를 더블 클릭하여 팔레트에서 고를 수 있습니다. 체력 1000."},
    {Language::Dutch, u"Gloedgenerator",
     u"Plaats in een van je kolonies of op een ongeclaimde planeet om de gloed van de planeetatmosfeer de kleur van "
     u"je rijk te geven, tenzij je een andere kiest. Dubbelklik op de generator om het kleurenpalet te openen. 1000 "
     u"gezondheidspunten."},
    {Language::Norwegian, u"Glødegenerator",
     u"Plasseres på en av koloniene dine eller en planet som ingen har gjort krav på, for å gi gløden i planetens "
     u"atmosfære fargen til imperiet ditt, med mindre du velger en annen. Dobbeltklikk på generatoren for å åpne "
     u"fargepaletten. 1000 helsepoeng."},
    {Language::Polish, u"Generator poświaty",
     u"Umieść w swojej kolonii lub na niezajętej planecie, aby zabarwić poświatę atmosfery planety kolorem swojego "
     u"imperium, chyba że wybierzesz inny. Kliknij dwukrotnie generator, aby otworzyć paletę. Wytrzymałość: 1000."},
    {Language::BrazilianPortuguese, u"Gerador de Brilho",
     u"Coloque em uma das suas colônias ou em um planeta não reclamado para tingir o brilho da atmosfera do planeta "
     u"com a cor do seu império, a menos que você escolha outra. Clique duas vezes no gerador para abrir a paleta. "
     u"Saúde: 1000."},
    {Language::EuropeanPortuguese, u"Gerador de Brilho",
     u"Coloque numa das suas colónias ou num planeta por reclamar para tingir o brilho da atmosfera do planeta com a "
     u"cor do seu império, a menos que escolha outra. Faça duplo clique no gerador para abrir a paleta. Saúde: 1000."},
    {Language::Russian, u"Генератор сияния",
     u"Размещается в вашей колонии или на незанятой планете и окрашивает сияние атмосферы в цвет вашей империи, если "
     u"вы не выберете другой. Двойной щелчок на генераторе открывает палитру. Запас прочности: 1000."},
    {Language::Swedish, u"Glödgenerator",
     u"Placera på en av dina kolonier eller på en planet som inte ägs av någon, för att färga glöden i planetens "
     u"atmosfär. Glöden får ditt imperiums färg om du inte väljer en annan. Dubbelklicka på generatorn för att öppna "
     u"paletten. Hälsa: 1000."},
    {Language::Thai, u"เครื่องกำเนิดแสงเรือง",
     u"วางบนอาณานิคมของคุณหรือบนดาวเคราะห์ที่ยังไม่ถูกยึด "
     u"เพื่อเปลี่ยนสีแสงเรืองของชั้นบรรยากาศบนดาวเคราะห์ให้เป็นสีประจำจักรวรรดิของคุณ เว้นแต่จะเลือกสีอื่น  "
     u"ดับเบิลคลิกที่เครื่องกำเนิดแสงเรืองเพื่อเปิดถาดเลือกสี  พลังชีวิต 1000"},
    {Language::SimplifiedChinese, u"辉光产生器",
     u"安放在你的殖民地或未被占领的行星上，可为行星大气的辉光染色，默认使用你帝国的颜色，也可另选颜色。双击产生器可打"
     u"开调色板。生命值：1000。"},
    {Language::TraditionalChinese, u"輝光產生裝置",
     u"設置在你的殖民地或未被佔領的星球上，可為星球大氣的輝光染色，預設使用你帝國的顏色，也可另選顏色。雙擊此裝置即可"
     u"開啟調色盤。耐久值：1000。"},
});
static_assert(inLanguageOrder(translations) && complete(translations));
}

