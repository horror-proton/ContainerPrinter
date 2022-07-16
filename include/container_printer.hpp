#include <type_traits>
#include <tuple>
#include <iterator>

#define CONTAINER_PRINTER_STREAM_INSERTION cp_stream_insertion

namespace container_printer {

template<typename Stream, typename T, typename Enable = void>
struct has_stream_insertion : std::false_type {};

template<typename S, typename T>
struct has_stream_insertion<S, T, std::void_t<decltype(std::declval<S>() << std::declval<T>())>> : std::true_type {};

template<typename T, typename Enable = void>
struct is_range_expression : std::false_type {};

template<typename T>
struct is_range_expression<T, std::enable_if_t<
    std::is_convertible_v<
        typename std::iterator_traits<decltype(begin(std::declval<T>()))>::iterator_category,
        std::forward_iterator_tag
    >
>> : std::true_type {
};

template<typename T, typename Enable = void>
struct is_tuple : std::false_type {};

template<typename T>
struct is_tuple<T, std::void_t<decltype(get<0>(std::declval<T>())), std::tuple_size<T>>> : std::true_type {};

// TODO: size of other tuple

template<typename Stream, typename T>
Stream &CONTAINER_PRINTER_STREAM_INSERTION(Stream &, const T &);

namespace detail {

template<typename Stream, typename TupleT, std::size_t N = std::tuple_size_v<TupleT>, std::size_t I = 0>
struct tuple_printer_impl {
    static constexpr void apply(Stream &s, const TupleT &t) {
        CONTAINER_PRINTER_STREAM_INSERTION(s, ",");
        CONTAINER_PRINTER_STREAM_INSERTION(s, get<I>(t));
        tuple_printer_impl<Stream, TupleT, N, I + 1>::apply(s, t);
    }
};

template<typename Stream, typename TupleT, std::size_t N>
struct tuple_printer_impl<Stream, TupleT, N, 0> {
    static constexpr void apply(Stream &s, const TupleT &t) {
        CONTAINER_PRINTER_STREAM_INSERTION(s, "[");
        CONTAINER_PRINTER_STREAM_INSERTION(s, get<0>(t));
        tuple_printer_impl<Stream, TupleT, N, 1>::apply(s, t);
    }
};

template<typename Stream, typename TupleT, std::size_t N>
struct tuple_printer_impl<Stream, TupleT, N, N> {
    static constexpr void apply(Stream &s, const TupleT &) {
        CONTAINER_PRINTER_STREAM_INSERTION(s, "]");
    }
};

}

template<typename Stream, typename T>
Stream &CONTAINER_PRINTER_STREAM_INSERTION(Stream &stream, const T &value) {
    if constexpr (has_stream_insertion<Stream, T>::value) {
        stream << value;
        return stream;
    } else if constexpr (is_range_expression<T>::value) {
        CONTAINER_PRINTER_STREAM_INSERTION(stream, "{");
        std::string_view comma{};
        for (const auto &e : value) {
            CONTAINER_PRINTER_STREAM_INSERTION(stream, comma);
            CONTAINER_PRINTER_STREAM_INSERTION(stream, e);
            comma = ", ";
        }
        CONTAINER_PRINTER_STREAM_INSERTION(stream, "}");
        return stream;
    } else if constexpr (is_tuple<T>::value) {
        detail::tuple_printer_impl<Stream, T>::apply(stream, value);
        return stream;
    } else {
        stream << "object";
        return stream;
    }
    // TODO: constructable to string
    // TODO: has .to_string()
}

}
