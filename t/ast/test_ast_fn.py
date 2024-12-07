from lib import code2sexpr, invoke_onec
import subprocess


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
    proc = subprocess.Popen(
        ["code2sexpr", ""],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    assert proc.wait() == 0
    assert proc.stdout.read().decode() == ""

    # single (invalid) character program must fail to parse
    proc = subprocess.Popen(
        ["code2sexpr", "       a"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    assert proc.wait() == 1
    assert proc.stdout.read().decode() == ""
