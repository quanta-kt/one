from lib import code2sexpr, invoke_onec


def test_fn():
    assert code2sexpr("fn a() {}") == "(fn a () :())"

    assert code2sexpr("fn a() { b; c; }") == "(fn a () :() b c)"


    assert code2sexpr("fn a(b: i32, c: i32) { b; c; }") == "(fn a (b :i32 c :i32) :() b c)"

    # trailing comma in param
    assert code2sexpr("fn a(b: i32, c: i32,) {}") == "(fn a (b :i32 c :i32) :())"


    assert code2sexpr("fn a() -> i32 {}") == "(fn a () :i32)"
    assert code2sexpr("fn a() -> boolean { b; c; }") == "(fn a () :boolean b c)"
    assert code2sexpr("fn a(b: i32, c: i32) -> string { b; c; }") == "(fn a (b :i32 c :i32) :string b c)"


def test_empty_program_is_valid():
    output, exit_code = invoke_onec(["--s-expr"], "")
    assert output == ""
    assert exit_code == 0

    # single (invalid) character program must fail to parse
    output, exit_code = invoke_onec(["--s-expr"], "    a")
    assert output == ""
    assert exit_code == 1
