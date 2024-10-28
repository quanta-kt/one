from lib import code2sexpr


def test_fn():
    assert code2sexpr("fn a() {}") == "(fn a ())"

    assert code2sexpr("fn a() { b; c; }") == "(fn a () b c)"


    assert code2sexpr("fn a(b, c) { b; c; }") == "(fn a (b c) b c)"

    # trailing comma in param
    assert code2sexpr("fn a(b, c,) {}") == "(fn a (b c))"
