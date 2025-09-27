#include <algorithm>
#include <print>
#include <vector>
#include <ranges>
#include <string>

/*
{1, 2, 3, 4, 5, 6}
*/
template <typename T>
std::vector<std::vector<T>> skips(const std::vector<T>& vec) {
    int size = vec.size();
    return
          std::views::iota(1, size + 1)
        | std::views::transform([&vec](int n) {
            return vec
                | std::views::drop(n - 1)
                | std::views::stride(n)
                | std::ranges::to<std::vector>();
        })
        | std::ranges::to<std::vector>();
}

std::string to_str(const std::vector<int>& vec) {
    using namespace std::literals::string_literals;
    return vec
        | std::views::transform([](int x) {
            return std::to_string(x);
        })
        | std::views::join_with(", "s)
        | std::ranges::to<std::string>();
}

// local_maxima({2, 9, 5, 6, 1}) => {9, 6}
std::vector<int> local_maxima(const std::vector<int>& vec) {
    return vec
        | std::views::slide(3)
        | std::views::filter([](auto&& window) {
            return window[0] < window[1] && window[1] > window[2];
        })
        | std::views::transform([](auto&& window) {
            return window[1];
        })
        | std::ranges::to<std::vector>();
}

std::string histogram(const std::vector<int>& vec) {
    using namespace std::literals::string_literals;

    std::array<int, 10> cnt;
    std::ranges::fill(cnt, 0);
    for (int n : vec) cnt[n]++;
    int maximum = std::ranges::max(cnt);

    std::vector graph{"0123456789"s, "=========="s};
    for (int h = 1; h <= maximum; ++h) {
        std::string line;
        for (int k = 0; k <= 9; ++k) {
            line += cnt[k] >= h ? "*" : " ";
        }
        graph.push_back(line);
    }

    return graph
        | std::views::reverse
        | std::views::join_with("\n"s)
        | std::ranges::to<std::string>();
}

int main() {
    std::println("=== Exercise 1: Skips ===");
    std::vector<int> vec{1, 2, 3, 4, 5, 6};
    auto skipped = skips(vec);
    for (const auto& sk : skipped)
        std::println("{}", to_str(sk));

    std::println("=== Exercise 2: Local Maxima ===");
    std::vector<int> vec1{2, 9, 5, 6, 1};
    std::vector<int> vec2{2, 3, 4, 1, 5};
    std::vector<int> vec3{1, 2, 3, 4, 5, 6};
    std::println("{}", to_str(local_maxima(vec1)));
    std::println("{}", to_str(local_maxima(vec2)));
    std::println("{}", to_str(local_maxima(vec3)));

    std::println("=== Exercise 3: Histogram ===");
    std::vector h1{1,1,1,5};
    std::vector h2{1,4,5,4,6,6,3,4,2,4,9};
    std::println("{}", histogram(h1));
    std::println("{}", histogram(h2));

}
