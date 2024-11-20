from lib import stmt2sexpr


def test_let():
    assert stmt2sexpr("let a = b;") == "(let a b)"
    assert stmt2sexpr("let mut a = b;") == "(let-mut a b)"
    assert stmt2sexpr("let a;") == "(let a NULL)"
    assert stmt2sexpr("let mut a;") == "(let-mut a NULL)"

    assert stmt2sexpr("let a: number;") == "(let a :number NULL)"
    assert stmt2sexpr("let a: boolean;") == "(let a :boolean NULL)"
    assert stmt2sexpr("let a: string;") == "(let a :string NULL)"
    assert stmt2sexpr("let a: fn() -> number;") == "(let a :(fn() number) NULL)"
    assert stmt2sexpr("let a: fn(string) -> number;") == "(let a :(fn(string) number) NULL)"
    assert stmt2sexpr("let a: fn(string, number) -> number;") == "(let a :(fn(string number) number) NULL)"
    assert stmt2sexpr("let a: fn(string, number, boolean) -> number;") == "(let a :(fn(string number boolean) number) NULL)"

    assert stmt2sexpr("let a: fn() -> fn() -> number;") == "(let a :(fn() (fn() number)) NULL)"
    assert stmt2sexpr("let a: fn() -> fn() -> fn() -> number;") == "(let a :(fn() (fn() (fn() number))) NULL)"
