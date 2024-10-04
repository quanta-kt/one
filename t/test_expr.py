from lib import code2sexpr


def test_binary():
    assert code2sexpr("a + b;") == "(+ a b)"
    assert code2sexpr("a - b;") == "(- a b)"
    assert code2sexpr("a / b;") == "(/ a b)"
    assert code2sexpr("a * b;") == "(* a b)"
    assert code2sexpr("a % b;") == "(% a b)"


def test_precedence():
    assert code2sexpr("-a + b;") == "(+ (- a) b)"
    assert code2sexpr("+a * b;") == "(* (+ a) b)"

    assert code2sexpr("a + b * c;") == "(+ a (* b c))"

    assert code2sexpr("a + b / c;") == "(+ a (/ b c))"

    assert code2sexpr("a - b % c;") == "(- a (% b c))"

    assert code2sexpr("-a * b;") == "(* (- a) b)"
    assert code2sexpr("+a * b;") == "(* (+ a) b)"
    assert code2sexpr("!a * b;") == "(* (! a) b)"

    assert code2sexpr("a | b & c;") == "(| a (& b c))"
    assert code2sexpr("a & b | c & d;") == "(| (& a b) (& c d))"
    assert code2sexpr("(a | b) & c;") == "(& (| a b) c)"
    assert code2sexpr("x | y & z | w;") == "(| (| x (& y z)) w)"
    assert code2sexpr("a ^ b | c;") == "(| (^ a b) c)"


    assert code2sexpr("a < b && c < d;") == "(&& (< a b) (< c d))"
    assert code2sexpr("a < b == c < d;") == "(== (< a b) (< c d))"

    assert code2sexpr("a & b || c;") == "(|| (& a b) c)"
    assert code2sexpr("a && b || c;") == "(|| (&& a b) c)"

    assert code2sexpr("-a * !b + c() || d;") == "(|| (+ (* (- a) (! b)) (call c)) d)"


def test_associativity():
    assert code2sexpr("a * b / c;") == "(/ (* a b) c)"
    assert code2sexpr("a % b * c;") == "(* (% a b) c)"

    assert code2sexpr("a + b - c;") == "(- (+ a b) c)"

    assert code2sexpr("a < b <= c;") == "(<= (< a b) c)"
    assert code2sexpr("a > b >= c;") == "(>= (> a b) c)"

    assert code2sexpr("a == b != c;") == "(!= (== a b) c)"

    assert code2sexpr("a & b & c;") == "(& (& a b) c)"

    assert code2sexpr("a ^ b ^ c;") == "(^ (^ a b) c)"

    assert code2sexpr("a | b | c;") == "(| (| a b) c)"

    assert code2sexpr("a && b && c;") == "(&& (&& a b) c)"

    assert code2sexpr("a || b || c;") == "(|| (|| a b) c)"

    assert code2sexpr("a(b)(c)(d);") == "(call (call (call a b) c) d)"
