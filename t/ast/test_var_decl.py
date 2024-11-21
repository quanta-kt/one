from lib import stmt2sexpr


def test_let():
    assert stmt2sexpr("let a = b;") == "(let a b)"
    assert stmt2sexpr("let mut a = b;") == "(let-mut a b)"
    assert stmt2sexpr("let a;") == "(let a NULL)"
    assert stmt2sexpr("let mut a;") == "(let-mut a NULL)"

    assert stmt2sexpr("let a: i32;") == "(let a :i32 NULL)"
    assert stmt2sexpr("let a: boolean;") == "(let a :boolean NULL)"
    assert stmt2sexpr("let a: string;") == "(let a :string NULL)"
    assert stmt2sexpr("let a: fn() -> i32;") == "(let a :(fn() i32) NULL)"
    assert stmt2sexpr("let a: fn(string) -> i32;") == "(let a :(fn(string) i32) NULL)"
    assert stmt2sexpr("let a: fn(string, i32) -> i32;") == "(let a :(fn(string i32) i32) NULL)"
    assert stmt2sexpr("let a: fn(string, i32, boolean) -> i32;") == "(let a :(fn(string i32 boolean) i32) NULL)"

    assert stmt2sexpr("let a: fn() -> fn() -> i32;") == "(let a :(fn() (fn() i32)) NULL)"
    assert stmt2sexpr("let a: fn() -> fn() -> fn() -> i32;") == "(let a :(fn() (fn() (fn() i32))) NULL)"
