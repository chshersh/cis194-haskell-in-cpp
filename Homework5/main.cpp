#include <memory>
#include <print>
#include <variant>

struct Lit { int val; };
struct Add;
struct Mul;

using Expr = std::variant<
    Lit,
    std::unique_ptr<Add>,
    std::unique_ptr<Mul>
>;

struct Add {
    Expr left;
    Expr right;
};

struct Mul {
    Expr left;
    Expr right;
};

Expr lit(int val) {
    return Lit{val};
}

Expr add(Expr left, Expr right) {
    return std::make_unique<Add>(std::move(left), std::move(right));
}

Expr mul(Expr left, Expr right) {
    return std::make_unique<Mul>(std::move(left), std::move(right));
}

int eval(const Expr& expr) {
    struct Eval {
        int operator()(Lit lit) { return lit.val; }
        int operator()(const std::unique_ptr<Add>& add) {
            int l = std::visit(*this, add->left);
            int r = std::visit(*this, add->right);
            return l + r;
        }
        int operator()(const std::unique_ptr<Mul>& add) {
            int l = std::visit(*this, add->left);
            int r = std::visit(*this, add->right);
            return l * r;
        }
    };

    return std::visit(Eval{}, expr);
}

int main() {
    auto example = mul(add(lit(2), lit(3)), lit(4));
    std::println("{}", eval(example));
}
