from lib import stmt2sexpr


def test_blocks_can_nest():
    code = """
    {
        {
            let a = 0;
            let b = 1;
        }

        {
            let a = 11;
            let b = 12;

            {
                if (a > b) {
                    a;
                } else {
                    b;
                }
            }
        }
    }
    """

    expected = (
        "(block "
        "(block (let a 0.000000e+00) (let b 1.000000e+00)) "
        "(block (let a 1.100000e+01) (let b 1.200000e+01) "
        "(block (if (> a b) (block a) (block b)))))"
    )

    assert stmt2sexpr(code) == expected
