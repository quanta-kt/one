from lib import typecheck_passes


def test_top_level_fn_in_scope():
    assert typecheck_passes("""
    fn add(a: i32, b: i32) -> i32 {}

    fn main() {
        let binary: fn(i32, i32) -> i32;
        let sum = add;
        let binary = sum;
    }
    """)


def test_fn_param_in_scope():
    assert typecheck_passes("""
    fn sum(a: i32, b: i32) {
        let s: i32 = a + b;
    }
    """)

def test_function_environments_are_isolated():
    assert not typecheck_passes("""
    fn foo() {
        let a: string;
    }

    fn bar() {
        let b: string = a;
    }
    """)

def test_empty_code_is_valid():
    assert typecheck_passes("")
