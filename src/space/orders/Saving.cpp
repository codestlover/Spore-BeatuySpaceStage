
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>

module beatuy.feature.orders;

import beatuy.game;

namespace feature {
namespace {
using MissingFunction = void(__cdecl*)(void* attribute);
using OffsetFunction = void*(__cdecl*)(void* attribute);
using SerializeFunction = bool(__cdecl*)(void* stream, void* data);

struct Attribute {
    const char* name;
    uint32_t identifier;
    uint32_t offset;
    std::array<uint32_t, 3> unknown;
    void* currentObject;
    MissingFunction missingFunction;
    OffsetFunction offsetFunction;
    SerializeFunction readFunction;
    SerializeFunction writeFunction;
    std::array<void*, 4> textFunctions;
};
static_assert(sizeof(Attribute) == 0x3c);
static_assert(offsetof(Attribute, currentObject) == 0x18 && offsetof(Attribute, missingFunction) == 0x1c);
static_assert(offsetof(Attribute, offsetFunction) == 0x20 && offsetof(Attribute, readFunction) == 0x24);
static_assert(offsetof(Attribute, writeFunction) == 0x28 && offsetof(Attribute, textFunctions) == 0x2c);

class SerializerRecord : public game::View {
public:
    using View::View;
    void* bytes() const {
        return call<void*>(0x18);
    }
};

class SerializerStream : public game::View {
public:
    using View::View;
    void* bytes() const {
        const SerializerRecord record(call<void*>(0x20));
        return record ? record.bytes() : nullptr;
    }
    bool good() const {
        return call<bool>(0x1c);
    }
};

constexpr uint32_t recordVersion = 0x02f011e8, classIdentifier = 0x01a80d26;
constexpr size_t serializerByteSize = 0xa14, serializableOffset = 8;
constexpr std::array<uint32_t, 4> gameAttributes{0x022556e7, 0x0436e7e0, 0x0436e7e1, 0x053e9f38};
constexpr uint32_t ordersAttribute = 0x7b028af8;
static_assert(std::ranges::is_sorted(gameAttributes) && gameAttributes.back() < ordersAttribute);

constexpr uint32_t payloadVersion = 1, guardsAllFlag = 1, drainLimit = 0x10000;
constexpr size_t headerWords = 2, fingerprintWords = 4, standingWords = 4;
constexpr size_t payloadCapacity = fingerprintWords + maximumEscort * (standingWords + maximumEscort);
constexpr int bigEndian = 0;

std::array<Attribute, gameAttributes.size() + 2> attributeTable{};
OrdersSaving* attributeOwner = nullptr;

bool __cdecl noText() {
    return true;
}

EmpireFingerprint playerFingerprint() {
    if (game::playerPoliticalId() == uint32_t(-1))
        return {};
    const game::Empire empire = game::playerEmpire();
    if (!empire)
        return {};
    return {empire.politicalId(), empire.homeStarIdentifier(), empire.homePlanetIdentifier()};
}

template <class FunctionPointer> bool gameOwns(const map::Slot<FunctionPointer>& slot) {
    return slot.original == game::address<FunctionPointer>(slot.function);
}

std::optional<std::span<const uint32_t>> receivePayload(void* bytes, uint32_t byteLength, std::span<uint32_t> buffer) {
    if (byteLength % sizeof(uint32_t) || byteLength > drainLimit)
        return std::nullopt;
    uint32_t left = uint32_t(byteLength / sizeof(uint32_t));
    if (left <= buffer.size()) {
        if (left && !map::saving::readWords(bytes, buffer.data(), left, bigEndian))
            return std::nullopt;
        return buffer.first(left);
    }
    while (left) {
        const uint32_t piece = std::min(left, uint32_t(buffer.size()));
        if (!map::saving::readWords(bytes, buffer.data(), piece, bigEndian))
            return std::nullopt;
        left -= piece;
    }
    return std::nullopt;
}
}

bool OrdersSaving::prepare(OrderBook& orderBook) {
    book = &orderBook;
    attributeOwner = this;
    std::array<Attribute, gameAttributes.size() + 1> copied{};
    const std::byte* table = map::saving::playerShipAttributes.at<const std::byte*>();
    if (!game::readable(table, sizeof(copied)))
        return false;
    std::memcpy(copied.data(), table, sizeof(copied));
    for (size_t index = 0; index < gameAttributes.size(); ++index) {
        const Attribute& attribute = copied[index];
        if (attribute.identifier != gameAttributes[index] || attribute.currentObject || !attribute.offsetFunction ||
            !attribute.readFunction || !attribute.writeFunction)
            return false;
    }
    if (copied.back().readFunction && copied.back().writeFunction)
        return false;
    void* const text = std::bit_cast<void*>(&noText);
    std::ranges::copy(std::span(copied).first(gameAttributes.size()), attributeTable.begin());
    attributeTable[gameAttributes.size()] = Attribute{
        .name = "beatuyEscortOrders",
        .identifier = ordersAttribute,
        .offset = 0,
        .unknown = {},
        .currentObject = nullptr,
        .missingFunction = &OrdersSaving::attributeMissing,
        .offsetFunction = &OrdersSaving::attributeData,
        .readFunction = &OrdersSaving::readAttribute,
        .writeFunction = &OrdersSaving::writeAttribute,
        .textFunctions = {text, text, text, text},
    };
    attributeTable.back() = Attribute{};
    ready = true;
    return true;
}

std::optional<bool> OrdersSaving::write(void* serializable, void* stream) {
    if (!ready || !book || !serializable || !stream)
        return std::nullopt;
    if (!gameOwns(map::saving::playerShipWrite))
        return std::nullopt;
    void* bytes = SerializerStream(stream).bytes();
    if (!bytes)
        return std::nullopt;
    dropForeign(*book);
    const uint32_t version = recordVersion;
    map::saving::writeWords(bytes, &version, 1, bigEndian);
    return serialize(serializable, stream, map::saving::classSerializerWrite);
}

std::optional<bool> OrdersSaving::read(void* serializable, void* stream) {
    if (!ready || !book || !serializable || !stream)
        return std::nullopt;
    if (!gameOwns(map::saving::playerShipRead))
        return std::nullopt;
    void* bytes = SerializerStream(stream).bytes();
    if (!bytes)
        return std::nullopt;
    forget();
    uint32_t version = 0;
    map::saving::readWords(bytes, &version, 1, bigEndian);
    return serialize(serializable, stream, map::saving::classSerializerRead);
}

void OrdersSaving::tick(OrderBook& orderBook) {
    dropForeign(orderBook);
}

bool OrdersSaving::serialize(void* serializable, void* stream,
                             const map::Function<bool(__thiscall*)(void*, void*)>& serializerFunction) {
    record = game::PlayerShipManager(static_cast<char*>(serializable) - serializableOffset);
    alignas(4) std::array<std::byte, serializerByteSize> serializer{};
    map::saving::classSerializerConstructor(serializer.data(), record.raw(), attributeTable.data(), classIdentifier);
    const bool done = serializerFunction(serializer.data(), stream);
    record = game::PlayerShipManager();
    return done;
}

void* __cdecl OrdersSaving::attributeData(void*) {
    return attributeOwner;
}

bool __cdecl OrdersSaving::writeAttribute(void* stream, void* data) {
    if (!stream)
        return true;
    const SerializerStream serializerStream(stream);
    std::array<uint32_t, headerWords + payloadCapacity> words{};
    auto* saving = static_cast<OrdersSaving*>(data);
    const size_t payloadSize = saving ? saving->compose(std::span(words).subspan(headerWords)) : fingerprintWords;
    words[0] = payloadVersion;
    words[1] = uint32_t(payloadSize * sizeof(uint32_t));
    if (void* bytes = serializerStream.bytes())
        map::saving::writeWords(bytes, words.data(), uint32_t(headerWords + payloadSize), bigEndian);
    return serializerStream.good();
}

bool __cdecl OrdersSaving::readAttribute(void* stream, void* data) {
    if (!stream)
        return true;
    const SerializerStream serializerStream(stream);
    auto* saving = static_cast<OrdersSaving*>(data);
    void* bytes = serializerStream.bytes();
    std::array<uint32_t, headerWords> header{};
    if (!bytes || !map::saving::readWords(bytes, header.data(), uint32_t(header.size()), bigEndian))
        return serializerStream.good();
    const auto [version, byteLength] = header;
    std::array<uint32_t, payloadCapacity> buffer{};
    const std::optional<std::span<const uint32_t>> payload = receivePayload(bytes, byteLength, buffer);
    const bool usable = payload && version == payloadVersion;
    if (saving && usable)
        saving->load(*payload);
    return serializerStream.good();
}

void __cdecl OrdersSaving::attributeMissing(void*) {
    if (!attributeOwner)
        return;
    attributeOwner->forget();
}

size_t OrdersSaving::compose(std::span<uint32_t> words) const {
    size_t size = 0;
    const auto put = [&](uint32_t word) {
        if (size < words.size())
            words[size++] = word;
    };
    const EmpireFingerprint fingerprint = playerFingerprint();
    const game::Posse escort = record ? record.escort() : game::Posse();
    const auto inEscort = [&escort](uint32_t politicalId) { return escort.has(politicalId); };
    put(fingerprint.politicalId);
    put(fingerprint.homeStar);
    put(fingerprint.homePlanet);
    if (!fingerprint.known() || !escort || !book) {
        put(0);
        return size;
    }
    const std::span<const Standing> kept = book->standings();
    put(uint32_t(std::ranges::count_if(kept, inEscort, &Standing::politicalId)));
    for (const Standing& standing : kept) {
        if (!inEscort(standing.politicalId))
            continue;
        const std::span<const uint32_t> guarded = standing.guardedShips();
        put(standing.politicalId);
        put(uint32_t(standing.order));
        put(standing.guardsAll ? guardsAllFlag : 0);
        put(uint32_t(std::ranges::count_if(guarded, inEscort)));
        for (uint32_t ship : guarded)
            if (inEscort(ship))
                put(ship);
    }
    return size;
}

void OrdersSaving::load(std::span<const uint32_t> payload) {
    if (!book || payload.size() < fingerprintWords)
        return;
    givenFor = EmpireFingerprint{payload[0], payload[1], payload[2]};
    const uint32_t savedCount = payload[3];
    std::span<const uint32_t> rest = payload.subspan(fingerprintWords);
    std::array<Standing, maximumEscort> standings{};
    size_t loaded = 0;
    for (uint32_t entry = 0; entry < savedCount && loaded < standings.size() && rest.size() >= standingWords; ++entry) {
        const uint32_t politicalId = rest[0], order = rest[1], flags = rest[2], guardedCount = rest[3];
        if (guardedCount > maximumEscort || rest.size() - standingWords < guardedCount)
            break;
        const std::span<const uint32_t> guarded = rest.subspan(standingWords, guardedCount);
        rest = rest.subspan(standingWords + guardedCount);
        const std::span<const Standing> earlier = std::span(standings).first(loaded);
        if (std::ranges::find(earlier, politicalId, &Standing::politicalId) != earlier.end())
            continue;
        Standing& standing = standings[loaded++];
        standing = Standing{.politicalId = politicalId,
                            .order = order < orderCount ? static_cast<Order>(order) : Order::Free,
                            .guardsAll = (flags & guardsAllFlag) != 0,
                            .guardedCount = uint8_t(guardedCount)};
        std::ranges::copy(guarded, standing.guarded.begin());
    }
    book->load(std::span(standings).first(loaded));
}

void OrdersSaving::forget() {
    if (book)
        book->clear();
    givenFor = {};
}

void OrdersSaving::dropForeign(OrderBook& orderBook) {
    const EmpireFingerprint current = playerFingerprint();
    if (!current.known())
        return;
    if (orderBook.standings().empty()) {
        givenFor = {};
        return;
    }
    if (!givenFor.known()) {
        givenFor = current;
        return;
    }
    if (givenFor == current)
        return;
    orderBook.clear();
    givenFor = {};
}
}

