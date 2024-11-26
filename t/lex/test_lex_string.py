from lib import code2token_list

def test_string_can_escape_quote():
    actual = code2token_list(r'"He said \"wow!\""')
    expected = """\
1:1 "He said \\"wow!\\""
"""

    assert actual == expected

    actual = code2token_list(r'"\"wow!\", he exclaimed"')
    expected = """\
1:1 "\\"wow!\\", he exclaimed"
"""

    assert actual == expected
