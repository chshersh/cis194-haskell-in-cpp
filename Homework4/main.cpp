#include <print>
#include <vector>
#include <ranges>
#include <memory>
#include <span>
#include <algorithm>
#include <functional>
#include <array>
#include <unordered_set>

template <typename T, typename F>
auto iterate(T x, F&& f) {
    return std::views::iota(0)
        | std::views::transform([state = std::move(x), step = std::forward<F>(f)](int) mutable {
            auto result = state;
            state = step(state);
            return result;
        });
}

template <typename T>
struct Tree {
    int height;
    T val;
    std::unique_ptr<Tree<T>> left;
    std::unique_ptr<Tree<T>> right;
};

template <typename T>
int height(const std::unique_ptr<Tree<T>>& tree) {
    return tree ? tree->height : 0;
}

template <typename T>
void print_tree(const std::unique_ptr<Tree<T>>& tree, std::string padding) {
    if (!tree) return;
    std::println("{}{}", padding, tree->val);
    print_tree(tree->left, padding + "  ");
    print_tree(tree->right, padding + "  ");
}

template <typename T>
std::unique_ptr<Tree<T>> build_tree(std::span<T> items) {
    auto build = [items](auto self, size_t l, size_t r) -> std::unique_ptr<Tree<T>> {
        if (l >= r) return nullptr;

        size_t mid = l + (r - l) / 2;
        auto left  = self(self, l, mid);
        auto right = self(self, mid + 1, r);
        int h = 1 + std::max(height(left), height(right));

        return std::make_unique<Tree<T>>(
            h,
            items[mid],
            std::move(left),
            std::move(right)
        );
    };

    return build(build, 0, items.size());
}

int func1(std::span<int> xs) {
    auto pipe = xs
        | std::views::filter([](int x) { return x % 2 == 0; })
        | std::views::transform([](int x) { return x - 2; });

    return std::ranges::fold_left(pipe, 1, std::multiplies<>{});
}

// Returns primes in range: 3...2 * n + 2
std::vector<int> sieve_sundaram(int n) {
    auto sieve =
          std::views::iota(1, n + 1)
        | std::ranges::to<std::unordered_set>();

    auto sieved_out =
          std::views::iota(1, n + 1)
        | std::views::transform([n](int j) {
            return
                  std::views::iota(j, n + 1)
                | std::views::transform([j](int i) { return i + j + 2 * i * j; })
                | std::views::take_while([n](int x) { return x <= n; });
        })
        | std::views::join;

    for (int out : sieved_out)
        sieve.erase(out);

    auto primes = sieve
        | std::views::transform([](int x) { return x * 2 + 1; })
        | std::ranges::to<std::vector>();

    std::ranges::sort(primes);

    return primes;
}

int main() {
    auto incs = iterate(1, [](int x) { return x * 2; }) | std::views::take(10);

    for (auto x : incs) {
        std::print("{} ", x);
    }
    std::println();

    std::array arr = {1, 8, 3, 4, 5};
    std::println("Func1: {}", func1(arr));
    std::println("Primes: {}", sieve_sundaram(16));

    std::array items = {1, 3, 5, 7, 8, 10};
    auto tree = build_tree(std::span{items.begin(), items.end()});
    std::println("Here!");
    print_tree(tree, "");
}
