from lib import stmt2sexpr

def test_escapes():
    assert stmt2sexpr(r'"\a\t\b\n\r\\";') == "(str '\a\t\b\n\r\\')"
