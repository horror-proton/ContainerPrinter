#include <type_traits>
#include <tuple>
#include <iterator>


#define CONTAINER_PRINTER_STREAM_INSERTION cp_stream_insertion

template<typename Stream, typename T, typename Enable = void>
struct has_stream_insertion : std::false_type {
};

template<typename S, typename T>
struct has_stream_insertion<S, T, std::void_t<decltype(std::declval<S>() << std::declval<T>())>> : std::true_type {
};

template<typename T, typename Enable = void>
struct is_range_expression : std::false_type {
};

template<typename T>
struct is_range_expression<T, std::enable_if_t<
                              std::is_convertible_v<
                              typename std::iterator_traits<decltype(begin(std::declval<T>()))>::iterator_category,
        std::forward_iterator_tag
>
>> : std::true_type {
};

template<typename T, typename Enable = void>
struct is_tuple : std::false_type {
};

template<typename T>
struct is_tuple<T, std::void_t<decltype(get<0>(std::declval<T>())), decltype(std::tuple_size<T>{})>>
: std::true_type {
};

// TODO: size of other tuple

template<std::size_t ...I, typename TupleT, typename Func, typename...Args>
constexpr auto tuple_for_each_impl(std::index_sequence<I...>, TupleT &t, Func f, Args ...args) {
    using std::get;
    (f(get<I>(t), (args)...), ...);
}

template<typename TupleT, typename Func, typename...Args>
void tuple_for_each(TupleT &t, Func f, Args...other_args) {
    tuple_for_each_impl(std::make_index_sequence<std::tuple_size_v<TupleT>>{}, t, f, (other_args)...);
}

template<typename Stream, typename T>
Stream &CONTAINER_PRINTER_STREAM_INSERTION(Stream &stream, const T &value) {
    if constexpr (has_stream_insertion<Stream, T>::value) {
        stream << value;
        return stream;
    } else if constexpr (is_range_expression<T>::value) {
        CONTAINER_PRINTER_STREAM_INSERTION(stream, "{");
        std::string_view comma{};
        for (const auto &e: value) {
            CONTAINER_PRINTER_STREAM_INSERTION(stream, comma);
            CONTAINER_PRINTER_STREAM_INSERTION(stream, e);
            comma = ", ";
        }
        return stream;
    } else if constexpr (is_tuple<T>::value) {
        CONTAINER_PRINTER_STREAM_INSERTION(stream, "[ ");
        std::string_view comma{};
        tuple_for_each(value, [](const auto &e, Stream &s) {
            CONTAINER_PRINTER_STREAM_INSERTION(s, e);
            CONTAINER_PRINTER_STREAM_INSERTION(s, " ");
        }, std::ref(stream));
        CONTAINER_PRINTER_STREAM_INSERTION(stream, "]");
        return stream;
    } else {
        stream << "object";
        return stream;
    }
    // TODO: constructable to string
    // TODO: has .to_string()
}
