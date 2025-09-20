#include <algorithm>
#include <cassert>
#include <charconv>
#include <fstream>
#include <iostream>
#include <optional>
#include <memory>
#include <print>
#include <ranges>
#include <string>
#include <variant>
#include <vector>

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

struct MessageTypeToStringVisitor {
    std::string operator()(Info) const { return "I"; }
    std::string operator()(Warning) const { return "W"; }
    std::string operator()(Error error) const {
        return std::format("E {}", error.code);
    }
};

std::string toString(const MessageType& msgType) {
    return std::visit(MessageTypeToStringVisitor{}, msgType);
}

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

LogMessage make_message(MessageType type, Timestamp timestamp, std::string msg) {
    return Message{ type, timestamp, std::move(msg) };
}

struct LogMessageToStringVisitor {
    std::string operator()(const Message& msg) const {
        return std::format("{} {} {}", toString(msg.type), msg.timestamp, msg.msg);
    }

    std::string operator()(const Unknown& unknown) const {
        return std::format("Unknown: {}", unknown.msg);
    }
};

std::string toString(const LogMessage& logMessage) {
    return std::visit(LogMessageToStringVisitor{}, logMessage);
}

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
    return words
        | std::views::drop(to_drop)
        | std::views::join_with(' ')
        | std::ranges::to<std::string>();
}

LogMessage parseMessage(std::string str) {
    // Step 1: Split into words
    std::vector<std::string> words = str
        | std::views::split(' ')
        | std::views::transform([](const auto& range) {
            return std::string{range.begin(), range.end()};
        })
        | std::ranges::to<std::vector>();

    auto get = [&words](size_t i) -> std::optional<std::string> {
        if (i < words.size()) {
            return words[i];
        } else {
            return std::nullopt;
        }
    };

    auto get_int = [&get](size_t i) -> std::optional<int> {
        return get(i).and_then(to_int);
    };

    return
        get(0)
        .and_then([&get_int, &words](const std::string& msgTypeStr) {
            std::optional<LogMessage> result;

            if (msgTypeStr == "I") {
                result =
                    get_int(1)
                    .transform([&words](int timestamp) {
                        return make_message(Info{}, timestamp, unwords(words, 2));
                    });
            } else if (msgTypeStr == "W") {
                result =
                    get_int(1)
                    .transform([&words](int timestamp) {
                        return make_message(Warning{}, timestamp, unwords(words, 2));
                    });
            } else if (msgTypeStr == "E") {
                result =
                    get_int(1)
                    .and_then([&words, &get_int](int code) {
                        return
                            get_int(2)
                            .transform([&words, code](int timestamp) {
                                return make_message(Error{ code }, timestamp, unwords(words, 3));
                            });
                    });
            } else {
                result = std::nullopt;
            }

            return result;
        })
        .value_or(Unknown{str});
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary); // open in binary to avoid newline translation
    return std::string((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
}

std::vector<LogMessage> parseFile(std::string filename) {
    std::string file_contents = readFile(filename);
    return file_contents
        | std::views::split('\n')
        | std::views::transform([](const auto& line) -> LogMessage {
          return parseMessage(std::string{line.begin(), line.end()});
        })
        | std::ranges::to<std::vector>();
}

/*
data MessageTree = Leaf | Node MessageTree LogMessage MessageTree
*/

struct Leaf {};
struct Node;

using MessageTree = std::variant<
    Leaf,
    std::unique_ptr<Node>
>;

struct Node {
    Message msg;
    MessageTree left;
    MessageTree right;
};

struct InsertVisit {
    Message msg;

    MessageTree operator()(Leaf) const {
        return std::make_unique<Node>(msg, Leaf{}, Leaf{});
    }

    MessageTree operator()(std::unique_ptr<Node>& node) const {
        Timestamp cur_value = node->msg.timestamp;
        if (msg.timestamp < cur_value) {
            node->left = std::visit(*this, node->left);
        } else if (msg.timestamp > cur_value) {
            node->right = std::visit(*this, node->right);
        }
        return std::move(node);
    }
};

MessageTree insert(const Message& message, MessageTree& tree) {
    return std::visit(InsertVisit{message}, tree);
}

MessageTree insert(MessageTree& tree, const LogMessage& logMessage) {
    if (const auto* _ = std::get_if<Unknown>(&logMessage)) {
        return std::move(tree);
    }

    const auto* message = std::get_if<Message>(&logMessage);
    return insert(*message, tree);
}

MessageTree build(const std::vector<LogMessage>& messages) {
    return std::ranges::fold_left(messages, Leaf{},
        [](MessageTree tree, const LogMessage& logMessage) {
            return insert(tree, logMessage);
        }
    );
}

struct InOrderVisit {
    std::vector<Message> messages;

    void operator()(Leaf) {}

    void operator()(const std::unique_ptr<Node>& node) {
        std::visit(*this, node->left);
        messages.push_back(node->msg);
        std::visit(*this, node->right);
    }
};

std::vector<Message> in_order(const MessageTree& tree) {
    auto visitor = InOrderVisit{std::vector<Message>()};
    std::visit(visitor, tree);
    return visitor.messages;
}

void what_went_wrong(std::string file_name) {
    auto logMessages = parseFile(file_name);
    auto messageTree = build(logMessages);
    auto relevantMessages = in_order(messageTree);

    auto importantMessages = relevantMessages
        | std::views::filter([](const Message& msg) {
            if (const auto* error = std::get_if<Error>(&msg.type)) {
                return error->code >= 50;
            }
            return false;
        })
        | std::views::transform([](const Message& msg) {
            return msg.msg;
        });

    for (auto&& msg : importantMessages) {
        std::println("{}", msg);
    }
}

int main() {
    std::println("========= Start Here =========");

    assert((parseMessage("I 29 la la la") ==
      LogMessage{ Message{ Info{}, 29, "la la la" } }));
    assert((parseMessage("E 2 562 help help") ==
      LogMessage{ Message{ Error{2}, 562, "help help" } }));
    assert((parseMessage("This is not in the right format") ==
      LogMessage{ Unknown{ "This is not in the right format" } }));
    std::println("========= PASS: 'parseMessage' tests =========");

    what_went_wrong("Homework2/sample.log");
}
