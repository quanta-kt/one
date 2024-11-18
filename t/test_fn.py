from lib import code2sexpr


def test_fn():
    assert code2sexpr("fn a() {}") == "(fn a () :())"

    assert code2sexpr("fn a() { b; c; }") == "(fn a () :() b c)"


    assert code2sexpr("fn a(b: number, c: number) { b; c; }") == "(fn a (b :number c :number) :() b c)"

    # trailing comma in param
    assert code2sexpr("fn a(b: number, c: number,) {}") == "(fn a (b :number c :number) :())"


    assert code2sexpr("fn a() -> number {}") == "(fn a () :number)"
    assert code2sexpr("fn a() -> boolean { b; c; }") == "(fn a () :boolean b c)"
    assert code2sexpr("fn a(b: number, c: number) -> string { b; c; }") == "(fn a (b :number c :number) :string b c)"
