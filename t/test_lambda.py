from lib import stmt2sexpr


def test_call():
    assert stmt2sexpr("fn() {};") == "(fn ())"
    assert stmt2sexpr("fn(a, b, c) {};") == "(fn (a b c))"
    assert stmt2sexpr("let a = fn(a) { let a = b; };") == "(let a (fn (a) (let a b)))"
