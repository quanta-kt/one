from lib import stmt2sexpr


def test_binary():
    assert stmt2sexpr("a + b;") == "(+ a b)"
    assert stmt2sexpr("a - b;") == "(- a b)"
    assert stmt2sexpr("a / b;") == "(/ a b)"
    assert stmt2sexpr("a * b;") == "(* a b)"
    assert stmt2sexpr("a % b;") == "(% a b)"
    assert stmt2sexpr("a = b;") == "(= a b)"


def test_precedence():
    assert stmt2sexpr("-a + b;") == "(+ (- a) b)"
    assert stmt2sexpr("+a * b;") == "(* (+ a) b)"

    assert stmt2sexpr("a + b * c;") == "(+ a (* b c))"

    assert stmt2sexpr("a + b / c;") == "(+ a (/ b c))"

    assert stmt2sexpr("a - b % c;") == "(- a (% b c))"

    assert stmt2sexpr("-a * b;") == "(* (- a) b)"
    assert stmt2sexpr("+a * b;") == "(* (+ a) b)"
    assert stmt2sexpr("!a * b;") == "(* (! a) b)"

    assert stmt2sexpr("a | b & c;") == "(| a (& b c))"
    assert stmt2sexpr("a & b | c & d;") == "(| (& a b) (& c d))"
    assert stmt2sexpr("(a | b) & c;") == "(& (| a b) c)"
    assert stmt2sexpr("x | y & z | w;") == "(| (| x (& y z)) w)"
    assert stmt2sexpr("a ^ b | c;") == "(| (^ a b) c)"


    assert stmt2sexpr("a < b && c < d;") == "(&& (< a b) (< c d))"
    assert stmt2sexpr("a < b == c < d;") == "(== (< a b) (< c d))"

    assert stmt2sexpr("a & b || c;") == "(|| (& a b) c)"
    assert stmt2sexpr("a && b || c;") == "(|| (&& a b) c)"

    assert stmt2sexpr("-a * !b + c() || d;") == "(|| (+ (* (- a) (! b)) (call c)) d)"

    assert stmt2sexpr("y = a & b || c;") == "(= y (|| (& a b) c))"
    assert stmt2sexpr("y = a && b || c;") == "(= y (|| (&& a b) c))"


def test_associativity():
    assert stmt2sexpr("a * b / c;") == "(/ (* a b) c)"
    assert stmt2sexpr("a % b * c;") == "(* (% a b) c)"

    assert stmt2sexpr("a + b - c;") == "(- (+ a b) c)"

    assert stmt2sexpr("a < b <= c;") == "(<= (< a b) c)"
    assert stmt2sexpr("a > b >= c;") == "(>= (> a b) c)"

    assert stmt2sexpr("a == b != c;") == "(!= (== a b) c)"

    assert stmt2sexpr("a & b & c;") == "(& (& a b) c)"

    assert stmt2sexpr("a ^ b ^ c;") == "(^ (^ a b) c)"

    assert stmt2sexpr("a | b | c;") == "(| (| a b) c)"

    assert stmt2sexpr("a && b && c;") == "(&& (&& a b) c)"

    assert stmt2sexpr("a || b || c;") == "(|| (|| a b) c)"

    assert stmt2sexpr("a(b)(c)(d);") == "(call (call (call a b) c) d)"

    assert stmt2sexpr("a = b = c;") == "(= a (= b c))"
