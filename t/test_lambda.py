from lib import stmt2sexpr


def test_call():
    assert stmt2sexpr("fn() {};") == "(fn () :())"
    assert stmt2sexpr("fn(a: number, b: string, c: boolean) {};") == "(fn (a :number b :string c :boolean) :())"
    assert stmt2sexpr("let a = fn(a: string) { let a = b; };") == "(let a (fn (a :string) :() (let a b)))"

    assert stmt2sexpr("fn() -> string {};") == "(fn () :string)"
    assert stmt2sexpr("fn(a: number, b: string, c: boolean) -> boolean {};") == "(fn (a :number b :string c :boolean) :boolean)"
    assert stmt2sexpr("let a = fn(a: string) -> number { let a = b; };") == "(let a (fn (a :string) :number (let a b)))"
