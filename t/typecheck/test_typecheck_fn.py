from lib import typecheck_passes


def test_top_level_fn_in_scope():
    assert typecheck_passes("""
    fn add(a: number, b: number) -> number {}

    fn main() {
        let binary: fn(number, number) -> number;
        let sum = add;
        let binary = sum;
    }
    """)


def test_fn_param_in_scope():
    assert typecheck_passes("""
    fn sum(a: number, b: number) {
        let s: number = a + b;
    }
    """)


def test_empty_code_is_valid():
    assert typecheck_passes("")
