from lib import code2token_list


def test_only_one_decimal_point():
    actual = code2token_list("1.1.0")
    expected = "1:1 1.1\n"
    assert actual == expected
