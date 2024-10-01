from lib import code2sexpr


def test_if():
    assert code2sexpr("if a > b { a; }") == "(if (> a b) (block a))"
    assert (
        code2sexpr("if a > b { a; } else { b; }") == "(if (> a b) (block a) (block b))"
    )


def test_if_can_nest():
    assert (
        code2sexpr("if a > b { if b > c { b; } }")
        == "(if (> a b) (block (if (> b c) (block b))))"
    )
