from lib import code2token_list


def test_line_nr():
    actual = code2token_list("""\
fn main() {
    return 0;
}
""")

    expected = """\
1:1 fn
1:4 main
1:8 (
1:9 )
1:11 {
2:5 return
2:12 0
2:13 ;
3:1 }
"""

    assert actual == expected


def test_col_nr():
    actual = code2token_list("fn main > = <= abc 123")
    #                         1234567891111111111122
    #                         0000000000123456789012

    expected = """\
1:1 fn
1:4 main
1:9 >
1:11 =
1:13 <=
1:16 abc
1:20 123
"""
    assert actual == expected
