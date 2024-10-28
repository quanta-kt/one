from lib import stmt2sexpr


def test_let():
    assert stmt2sexpr("let a = b;") == "(let a b)"
    assert stmt2sexpr("let mut a = b;") == "(let-mut a b)"
    assert stmt2sexpr("let a;") == "(let a NULL)"
    assert stmt2sexpr("let mut a;") == "(let-mut a NULL)"

