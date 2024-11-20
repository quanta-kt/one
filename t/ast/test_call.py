from lib import stmt2sexpr

def test_call():
    assert stmt2sexpr("a();") == "(call a)"
    assert stmt2sexpr("a(b);") == "(call a b)"
    assert stmt2sexpr("a(b, c);") == "(call a b c)"
    assert stmt2sexpr("a(b, c, d);") == "(call a b c d)"

    # Trailing comma
    assert stmt2sexpr("a(b,);") == "(call a b)"
    assert stmt2sexpr("a(b, c,);") == "(call a b c)"
    assert stmt2sexpr("a(b, c, d,);") == "(call a b c d)"

    # Error: arguments should be separated by a comma
    assert stmt2sexpr("a(b c);") == ""
    assert stmt2sexpr("a(b c d);") == ""
