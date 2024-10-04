from lib import code2sexpr


def test_let():
    assert code2sexpr("let a = b;") == "(let a b)"
    assert code2sexpr("let mut a = b;") == "(let-mut a b)"
    assert code2sexpr("let a;") == "(let a NULL)"
    assert code2sexpr("let mut a;") == "(let-mut a NULL)"

