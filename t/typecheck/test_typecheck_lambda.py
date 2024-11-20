from lib import typecheck_passes


def test_lambda_compat():
    assert typecheck_passes("""
    fn main() {
        let binary: fn(number, number) -> number;

        let add = fn(a: number, b: number) -> number {
            a + b;
        };
        let mul = fn(a: number, b: number) -> number {
            a * b;
        };

        binary = add;
        binary = mul;
    }
    """)

    # differing return types
    assert not typecheck_passes("""
    fn main() {
        let binary: fn(number, number) -> number;
        let concat = fn(a: number, b: number) -> string {};
        binary = concat;
    }
    """)

    # differing arity
    assert not typecheck_passes("""
    fn main() {
        let binary: fn(number, number) -> number;
        let square = fn(a: number) -> number {};
        binary = square;
    }
    """)

    # differing arguments
    assert not typecheck_passes("""
    fn main() {
        let unary: fn(number) -> number;
        let parse_int = fn(a: string) -> number {};
        binary = parse_int;
    }
    """)


def test_lambda_params():
    assert typecheck_passes("""
    fn main() {
        let repeat = fn(what: string, times: number) -> string {
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
        let repeat = fn(what: string, times: number) -> string {
            let i = 0;
            let result: number = what;

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
