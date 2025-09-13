#include <algorithm>
#include <cassert>
#include <iostream>
#include <ranges>
#include <vector>
#include <utility>

bool validate1(long long card_number) {
    int sum = 0;
    bool is_even = false;

    while (card_number > 0) {
        int digit = card_number % 10;
        card_number /= 10;

        if (is_even) {
            digit *= 2;
            int digit1 = digit % 10, digit2 = digit / 10;
            sum += digit1 + digit2;
        } else {
            sum += digit;
        }
        is_even = !is_even;
    }

    return sum % 10 == 0;
}

// Pipeline:
//   1. Number -> Digits
//   2. Digits -> Doubled Every Second Digit
//   3. Doubled Digits -> Single Digits
//   4. Single Digits -> Sum of digits

std::vector<int> digits(long long n) {
    if (n == 0) return {0};

    std::vector<int> result;

    while (n > 0) {
        int digit = n % 10;
        n /= 10;
        result.push_back(digit);
    }

    return result;
}

// 4 4 3 5
// 1 2 3 4

auto flatten_digits() {
    return
        std::views::transform([](int doubled_digit) {
          return digits(doubled_digit);
        })
        | std::views::join;
}

bool validate2(long long card_number) {
    auto card_digits = digits(card_number);

    auto doubled_digits =
          std::views::zip(std::views::iota(1), card_digits)
        | std::views::transform([](const auto& pair) {
            auto [i, d] = pair;
            return i % 2 == 0 ? d * 2 : d;
        })
        | flatten_digits() ;

    int sum = std::ranges::fold_left(doubled_digits, 0, std::plus{});

    return sum % 10 == 0;
}

int main() {
    std::cout << "======== Program Starts Here ========\n";

    assert(validate1(4012888888881881));
    assert(!validate1(4012888888881882));

    assert(validate2(4012888888881881));
    assert(!validate2(4012888888881882));
}
