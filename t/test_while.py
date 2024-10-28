from lib import stmt2sexpr


def test_while():
    assert stmt2sexpr("while a < b { a; }") == "(while (< a b) (block a))"


def test_while_can_nest():
    assert (
        stmt2sexpr("while a < b { while b < c { b; } }")
        == "(while (< a b) (block (while (< b c) (block b))))"
    )

    assert (
        stmt2sexpr("while a < b { if a == c { a; } }")
        == "(while (< a b) (block (if (== a c) (block a))))"
    )


def test_while_with_multiple_statements():
    assert stmt2sexpr("while a < b { a; b; }") == "(while (< a b) (block a b))"

    assert (
        stmt2sexpr("while a < b { { a; } { b; } }")
        == "(while (< a b) (block (block a) (block b)))"
    )
