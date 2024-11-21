from lib import stmt2sexpr

def test_type_annotation():
    assert stmt2sexpr("let a: fn() -> ();") == "(let a :(fn() ()) NULL)"

    # default return type is unit
    assert stmt2sexpr("let a: fn();") == "(let a :(fn() ()) NULL)"


def test_call():
    assert stmt2sexpr("fn() {};") == "(fn () :())"
    assert stmt2sexpr("fn(a: i32, b: string, c: boolean) {};") == "(fn (a :i32 b :string c :boolean) :())"
    assert stmt2sexpr("let a = fn(a: string) { let a = b; };") == "(let a (fn (a :string) :() (let a b)))"

    assert stmt2sexpr("fn() -> string {};") == "(fn () :string)"
    assert stmt2sexpr("fn(a: i32, b: string, c: boolean) -> boolean {};") == "(fn (a :i32 b :string c :boolean) :boolean)"
    assert stmt2sexpr("let a = fn(a: string) -> i32 { let a = b; };") == "(let a (fn (a :string) :i32 (let a b)))"
