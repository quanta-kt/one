from lib import code2sexpr

def test_call():
    assert code2sexpr("a();") == "(call a)"
    assert code2sexpr("a(b);") == "(call a b)"
    assert code2sexpr("a(b, c);") == "(call a b c)"
    assert code2sexpr("a(b, c, d);") == "(call a b c d)"

    # Trailing comma
    assert code2sexpr("a(b,);") == "(call a b)"
    assert code2sexpr("a(b, c,);") == "(call a b c)"
    assert code2sexpr("a(b, c, d,);") == "(call a b c d)"

    # Error: arguments should be separated by a comma
    assert code2sexpr("a(b c);") == ""
    assert code2sexpr("a(b c d);") == ""
