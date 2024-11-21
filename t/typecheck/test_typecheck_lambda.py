from lib import typecheck_passes


def test_lambda_compat():
    assert typecheck_passes("""
    fn main() {
        let binary: fn(i32, i32) -> i32;

        let add = fn(a: i32, b: i32) -> i32 {
            a + b;
        };
        let mul = fn(a: i32, b: i32) -> i32 {
            a * b;
        };

        binary = add;
        binary = mul;
    }
    """)

    # differing return types
    assert not typecheck_passes("""
    fn main() {
        let binary: fn(i32, i32) -> i32;
        let concat = fn(a: i32, b: i32) -> string {};
        binary = concat;
    }
    """)

    # differing arity
    assert not typecheck_passes("""
    fn main() {
        let binary: fn(i32, i32) -> i32;
        let square = fn(a: i32) -> i32 {};
        binary = square;
    }
    """)

    # differing arguments
    assert not typecheck_passes("""
    fn main() {
        let unary: fn(i32) -> i32;
        let parse_int = fn(a: string) -> i32 {};
        binary = parse_int;
    }
    """)


def test_lambda_params():
    assert typecheck_passes("""
    fn main() {
        let repeat = fn(what: string, times: i32) -> string {
            let i = 0;
            let result = what;

            while i < times {
                result = result + what;
                i = i + 1;
            }
        };
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let repeat = fn(what: string, times: i32) -> string {
            let i = 0;
            let result: i32 = what;

            while i < times {
                result = result + what;
                i = i + 1;
            }
        };
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let repeat = fn(what: string, times: string) -> string {
            let i = 0;
            let result = what;

            while i < times {
                result = result + what;
                i = i + 1;
            }
        };
    }
    """)
