// Copyright (c) Facebook, Inc. and its affiliates
// SPDX-License-Identifier: MIT OR Apache-2.0

#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

// Basic implementation for 128-bit unsigned integers.
struct uint128_t {
    uint64_t high;
    uint64_t low;

    friend bool operator==(const uint128_t &, const uint128_t &);
};

bool operator==(const uint128_t &lhs, const uint128_t &rhs) {
    return lhs.high == rhs.high && lhs.low == lhs.low;
}

// 128-bit signed integers.
struct int128_t {
    int64_t high;
    uint64_t low;

    friend bool operator==(const int128_t &, const int128_t &);
};

bool operator==(const int128_t &lhs, const int128_t &rhs) {
    return lhs.high == rhs.high && lhs.low == lhs.low;
}

// Trait to enable serialization of values of type T.
// This is similar to the `serde::Serialize` trait in Rust.
template <typename T>
struct Serializable {
    template <typename Serializer>
    static void serialize(const T &value, Serializer &serializer);
};

// Trait to enable deserialization of values of type T.
// This is similar to the `serde::Deserialize` trait in Rust.
template <typename T>
struct Deserializable {
    template <typename Deserializer>
    static T deserialize(Deserializer &deserializer);
};

// --- Implementation of Serializable for base types ---

// string
template <>
struct Serializable<std::string> {
    template <typename Serializer>
    static void serialize(const std::string &value, Serializer &serializer) {
        serializer.serialize_str(value);
    }
};

// unit
template <>
struct Serializable<std::monostate> {
    template <typename Serializer>
    static void serialize(const std::monostate &_value,
                          Serializer &serializer) {
        serializer.serialize_unit();
    }
};

// bool
template <>
struct Serializable<bool> {
    template <typename Serializer>
    static void serialize(const bool &value, Serializer &serializer) {
        serializer.serialize_bool(value);
    }
};

// UTF-8 char
template <>
struct Serializable<char32_t> {
    template <typename Serializer>
    static void serialize(const char32_t &value, Serializer &serializer) {
        serializer.serialize_char(value);
    }
};

// f32
template <>
struct Serializable<float> {
    template <typename Serializer>
    static void serialize(const float &value, Serializer &serializer) {
        serializer.serialize_f32(value);
    }
};

// f64
template <>
struct Serializable<double> {
    template <typename Serializer>
    static void serialize(const double &value, Serializer &serializer) {
        serializer.serialize_f64(value);
    }
};

// u8
template <>
struct Serializable<uint8_t> {
    template <typename Serializer>
    static void serialize(const uint8_t &value, Serializer &serializer) {
        serializer.serialize_u8(value);
    }
};

// u16
template <>
struct Serializable<uint16_t> {
    template <typename Serializer>
    static void serialize(const uint16_t &value, Serializer &serializer) {
        serializer.serialize_u16(value);
    }
};

// u32
template <>
struct Serializable<uint32_t> {
    template <typename Serializer>
    static void serialize(const uint32_t &value, Serializer &serializer) {
        serializer.serialize_u32(value);
    }
};

// u64
template <>
struct Serializable<uint64_t> {
    template <typename Serializer>
    static void serialize(const uint64_t &value, Serializer &serializer) {
        serializer.serialize_u64(value);
    }
};

// u128
template <>
struct Serializable<uint128_t> {
    template <typename Serializer>
    static void serialize(const uint128_t &value, Serializer &serializer) {
        serializer.serialize_u128(value);
    }
};

// i8
template <>
struct Serializable<int8_t> {
    template <typename Serializer>
    static void serialize(const int8_t &value, Serializer &serializer) {
        serializer.serialize_i8(value);
    }
};

// i16
template <>
struct Serializable<int16_t> {
    template <typename Serializer>
    static void serialize(const int16_t &value, Serializer &serializer) {
        serializer.serialize_i16(value);
    }
};

// i32
template <>
struct Serializable<int32_t> {
    template <typename Serializer>
    static void serialize(const int32_t &value, Serializer &serializer) {
        serializer.serialize_i32(value);
    }
};

// i64
template <>
struct Serializable<int64_t> {
    template <typename Serializer>
    static void serialize(const int64_t &value, Serializer &serializer) {
        serializer.serialize_i64(value);
    }
};

// i128
template <>
struct Serializable<int128_t> {
    template <typename Serializer>
    static void serialize(const int128_t &value, Serializer &serializer) {
        serializer.serialize_i128(value);
    }
};

// --- Derivation of Serializable for composite types ---

// Unique pointers (non-nullable)
template <typename T, typename Deleter>
struct Serializable<std::unique_ptr<T, Deleter>> {
    template <typename Serializer>
    static void serialize(const std::unique_ptr<T, Deleter> &value,
                          Serializer &serializer) {
        Serializable<T>::serialize(*value, serializer);
    }
};

// Options
template <typename T>
struct Serializable<std::optional<T>> {
    template <typename Serializer>
    static void serialize(const std::optional<T> &option,
                          Serializer &serializer) {
        if (option.has_value()) {
            serializer.serialize_u8(1);
            Serializable<T>::serialize(option.value(), serializer);
        } else {
            serializer.serialize_u8(0);
        }
    }
};

// Vectors (sequences)
template <typename T, typename Allocator>
struct Serializable<std::vector<T, Allocator>> {
    template <typename Serializer>
    static void serialize(const std::vector<T, Allocator> &value,
                          Serializer &serializer) {
        serializer.serialize_len(value.size());
        for (const T &item : value) {
            Serializable<T>::serialize(item, serializer);
        }
    }
};

// Fixed-size arrays
template <typename T, std::size_t N>
struct Serializable<std::array<T, N>> {
    template <typename Serializer>
    static void serialize(const std::array<T, N> &value,
                          Serializer &serializer) {
        for (const T &item : value) {
            Serializable<T>::serialize(item, serializer);
        }
    }
};

// Maps
template <typename K, typename V, typename Allocator>
struct Serializable<std::map<K, V, Allocator>> {
    template <typename Serializer>
    static void serialize(const std::map<K, V, Allocator> &value,
                          Serializer &serializer) {
        serializer.serialize_len(value.size());
        for (const auto &item : value) {
            Serializable<K>::serialize(item.first, serializer);
            Serializable<V>::serialize(item.second, serializer);
        }
    }
};

// Tuples
template <class... Types>
struct Serializable<std::tuple<Types...>> {
    template <typename Serializer>
    static void serialize(const std::tuple<Types...> &value,
                          Serializer &serializer) {
        // Visit each of the type components.
        std::apply(
            [&serializer](Types const &... args) {
                (Serializable<Types>::serialize(args, serializer), ...);
            },
            value);
    }
};

// Enums
template <class... Types>
struct Serializable<std::variant<Types...>> {
    template <typename Serializer>
    static void serialize(const std::variant<Types...> &value,
                          Serializer &serializer) {
        // Write the variant index.
        serializer.serialize_variant_index(value.index());
        // Visit the inner type.
        std::visit(
            [&serializer](const auto &arg) {
                using T = typename std::decay<decltype(arg)>::type;
                Serializable<T>::serialize(arg, serializer);
            },
            value);
    }
};

// --- Implementation of Deserializable for base types ---

// string
template <>
struct Deserializable<std::string> {
    template <typename Deserializer>
    static std::string deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_str();
    }
};

// unit
template <>
struct Deserializable<std::monostate> {
    template <typename Deserializer>
    static std::monostate deserialize(Deserializer &deserializer) {
        deserializer.deserialize_unit();
        return {};
    }
};

// bool
template <>
struct Deserializable<bool> {
    template <typename Deserializer>
    static bool deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_bool();
    }
};

// f32
template <>
struct Deserializable<float> {
    template <typename Deserializer>
    static float deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_f32();
    }
};

// f64
template <>
struct Deserializable<double> {
    template <typename Deserializer>
    static double deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_f64();
    }
};

// UTF-8 char
template <>
struct Deserializable<char32_t> {
    template <typename Deserializer>
    static char32_t deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_char();
    }
};

// u8
template <>
struct Deserializable<uint8_t> {
    template <typename Deserializer>
    static uint8_t deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_u8();
    }
};

// u16
template <>
struct Deserializable<uint16_t> {
    template <typename Deserializer>
    static uint16_t deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_u16();
    }
};

// u32
template <>
struct Deserializable<uint32_t> {
    template <typename Deserializer>
    static uint32_t deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_u32();
    }
};

// u64
template <>
struct Deserializable<uint64_t> {
    template <typename Deserializer>
    static uint64_t deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_u64();
    }
};

// u128
template <>
struct Deserializable<uint128_t> {
    template <typename Deserializer>
    static uint128_t deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_u128();
    }
};

// i8
template <>
struct Deserializable<int8_t> {
    template <typename Deserializer>
    static int8_t deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_i8();
    }
};

// i16
template <>
struct Deserializable<int16_t> {
    template <typename Deserializer>
    static int16_t deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_i16();
    }
};

// i32
template <>
struct Deserializable<int32_t> {
    template <typename Deserializer>
    static int32_t deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_i32();
    }
};

// i64
template <>
struct Deserializable<int64_t> {
    template <typename Deserializer>
    static int64_t deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_i64();
    }
};

// i128
template <>
struct Deserializable<int128_t> {
    template <typename Deserializer>
    static int128_t deserialize(Deserializer &deserializer) {
        return deserializer.deserialize_i128();
    }
};

// --- Derivation of Deserializable for composite types ---

// Unique pointers
template <typename T>
struct Deserializable<std::unique_ptr<T>> {
    template <typename Deserializer>
    static std::unique_ptr<T> deserialize(Deserializer &deserializer) {
        return std::make_unique<T>(
            Deserializable<T>::deserialize(deserializer));
    }
};

// Options
template <typename T>
struct Deserializable<std::optional<T>> {
    template <typename Deserializer>
    static std::optional<T> deserialize(Deserializer &deserializer) {
        auto tag = deserializer.deserialize_u8();
        if (tag == 0) {
            return {};
        } else {
            if (tag != 1) {
                throw "invalid option tag";
            }
            return {Deserializable<T>::deserialize(deserializer)};
        }
    }
};

// Vectors
template <typename T, typename Allocator>
struct Deserializable<std::vector<T, Allocator>> {
    template <typename Deserializer>
    static std::vector<T> deserialize(Deserializer &deserializer) {
        std::vector<T> result;
        size_t len = deserializer.deserialize_len();
        for (size_t i = 0; i < len; i++) {
            result.push_back(Deserializable<T>::deserialize(deserializer));
        }
        return result;
    }
};

// Maps
template <typename K, typename V>
struct Deserializable<std::map<K, V>> {
    template <typename Deserializer>
    static std::map<K, V> deserialize(Deserializer &deserializer) {
        std::map<K, V> result;
        size_t len = deserializer.deserialize_len();
        for (size_t i = 0; i < len; i++) {
            auto key = Deserializable<K>::deserialize(deserializer);
            auto value = Deserializable<V>::deserialize(deserializer);
            result.insert({key, value});
        }
        return result;
    }
};

// Fixed-size arrays
template <typename T, std::size_t N>
struct Deserializable<std::array<T, N>> {
    template <typename Deserializer>
    static std::array<T, N> deserialize(Deserializer &deserializer) {
        std::array<T, N> result;
        for (T &item : result) {
            item = Deserializable<T>::deserialize(deserializer);
        }
        return result;
    }
};

// Tuples
template <class... Types>
struct Deserializable<std::tuple<Types...>> {
    template <typename Deserializer>
    static std::tuple<Types...> deserialize(Deserializer &deserializer) {
        // Visit each of the type components.
        return std::make_tuple(
            Deserializable<Types>::deserialize(deserializer)...);
    }
};

// Enums
template <class... Types>
struct Deserializable<std::variant<Types...>> {
    template <typename Deserializer>
    static std::variant<Types...> deserialize(Deserializer &deserializer) {
        // A "case" is analog to a particular branch in switch-case over the
        // index. Given the variant type `T` known statically, we create a
        // closure that will deserialize a value `T` and return it as a variant.
        using Case = std::function<std::variant<Types...>(Deserializer &)>;
        auto make_case = [](auto tag) -> Case {
            // Obtain the type `T` encoded in the type of `tag ==
            // std::common_type<T>{}`.
            using T = typename decltype(tag)::type;
            auto f = [](Deserializer &de) {
                return std::variant<Types...>(
                    Deserializable<T>::deserialize(de));
            };
            return f;
        };

        // The static array of all the cases for this variant.
        static const std::array<Case, sizeof...(Types)> cases = {
            make_case(std::common_type<Types>{})...};

        // Read the variant index and execute the corresponding case.
        auto index = deserializer.deserialize_variant_index();
        return cases.at(index)(deserializer);
    }
};
