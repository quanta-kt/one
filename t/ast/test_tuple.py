from lib import code2sexpr, stmt2sexpr

def test_tuple_type():
    assert code2sexpr("""
    fn call_network() -> (string, string) {
         
    }
    """) == "(fn call_network () :(string, string))"

    assert stmt2sexpr("let single: (string);") == "(let single :(string) NULL)"

    assert stmt2sexpr("let pair: (string, number);") == "(let pair :(string, number) NULL)"

    assert stmt2sexpr("let pair: ((string, number), (string, number));") == "(let pair :((string, number), (string, number)) NULL)"
