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
