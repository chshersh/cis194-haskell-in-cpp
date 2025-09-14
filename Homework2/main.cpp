#include <cassert>
#include <charconv>
#include <iostream>
#include <variant>
#include <string>
#include <ranges>
#include <vector>
#include <optional>

/*
data MessageType = Info | Warning | Error Int
*/
struct Info {
    auto operator<=>(const Info&) const = default;
};

struct Warning {
    auto operator<=>(const Warning&) const = default;
};
struct Error {
    int code;

    auto operator<=>(const Error&) const = default;
};

using MessageType = std::variant<Info, Warning, Error>;

// type TimeStamp = Int  // Datetime
// data LogMessage = Message MessageType TimeStamp String | Unknown String
using Timestamp = int;

struct Message {
    MessageType type;
    Timestamp timestamp;
    std::string msg;

    auto operator<=>(const Message&) const = default;
};

struct Unknown {
    std::string msg;

    auto operator<=>(const Unknown&) const = default;
};

using LogMessage = std::variant<Message, Unknown>;

std::optional<int> to_int(std::string_view sv)
{
    int r;
    auto result = std::from_chars(sv.data(), sv.data() + sv.size(), r);
    if (result.ec == std::errc())
        return r;
    else
        return std::nullopt;
}

std::string unwords(const std::vector<std::string>& words, int to_drop) {
    auto words_pipe = words
        | std::views::drop(to_drop)
        | std::views::join_with(' ');

    return std::string(words_pipe.begin(), words_pipe.end());
}

LogMessage parseMessage(std::string str) {
    // Step 1: Split into words
    auto words_pipe = str
        | std::views::split(' ')
        | std::views::transform([](const auto& range) {
            return std::string{range.begin(), range.end()};
        });

    // Step 2: Combine words
    std::vector<std::string> words;
    for (auto&& word : words_pipe) {
        words.push_back(std::move(word));
    };

    auto get = [&words](size_t i) -> std::optional<std::string> {
        if (i < words.size()) {
            return words[i];
        } else {
            return std::nullopt;
        }
    };

    return
        get(0)
        .and_then([&get, &words](const std::string& msgTypeStr) {
            std::optional<LogMessage> result;

            if (msgTypeStr == "I") {
                result =
                    get(1)
                    .and_then(to_int)
                    .transform([&words](int timestamp) {
                        return LogMessage{ Message{ Info{}, timestamp, unwords(words, 2) } };
                    });
            } else if (msgTypeStr == "W") {
                result =
                    get(1)
                    .and_then(to_int)
                    .transform([&words](int timestamp) {
                        return LogMessage{ Message{ Warning{}, timestamp, unwords(words, 2) } };
                    });
            } else if (msgTypeStr == "E") {
                result =
                    get(1)
                    .and_then([&words, &get](const std::string& codeStr) {
                        return to_int(codeStr).and_then([&words, &get](int code) {
                            return get(2)
                                .and_then(to_int)
                                .transform([&words, code](int timestamp) {
                                     return LogMessage{ Message{ Error{ code }, timestamp, unwords(words, 3) } };
                                });
                        });
                    });
            } else {
                result = std::nullopt;
            }

            return result;
        })
        .value_or(Unknown{str});
}

int main() {
    std::cout << "========= Start Here =========\n";
    assert((parseMessage("I 29 la la la") ==
      LogMessage{ Message{ Info{}, 29, "la la la" } }));
    assert((parseMessage("E 2 562 help help") ==
      LogMessage{ Message{ Error{2}, 562, "help help" } }));
    assert((parseMessage("This is not in the right format") ==
      LogMessage{ Unknown{ "This is not in the right format" } }));

}
