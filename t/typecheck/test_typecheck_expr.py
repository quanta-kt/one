from lib import typecheck_passes


def test_assign():
    assert typecheck_passes("""
    fn main() {
        let a = 0;
        let b = 1;
        a = b;
        b = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: string = "";
        let b = 1;
        a = b;
        b = a;
    }
    """)


def test_binary_number():
    assert typecheck_passes("""
    fn main() {
        let a: number = 1 + 2;
        let b: number = a + 0;
        let c: number = a + b;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: number = 1 - 2;
        let b: number = a - 0;
        let c: number = a - b;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: number = 1 * 2;
        let b: number = a * 0;
        let c: number = a * b;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: number = 1 / 2;
        let b: number = a / 0;
        let c: number = a / b;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: number = 1 | 2;
        let b: number = a | 0;
        let c: number = a | b;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: number = 1 & 2;
        let b: number = a & 0;
        let c: number = a & b;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: number = 1 ^ 2;
        let b: number = a ^ 0;
        let c: number = a ^ b;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: number = 1 % 2;
        let b: number = a % 0;
        let c: number = a % b;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: number = 1 % 2;
        let b: number = a % 0;
        let c: number = a % b;
    }
    """)


def test_binary_boolean():
    assert typecheck_passes("""
    fn main() {
        let n: number = 3;
        let m: number = 4;

        let a: boolean = 1 > 2;
        let b: boolean = n > 0;
        let c: boolean = n > m;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let n: number = 3;
        let m: number = 4;

        let a: boolean = 1 < 2;
        let b: boolean = n < 0;
        let c: boolean = n < m;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let n: string = "foo";
        let m: string = "bar";

        let a: boolean = "foo" == "bar";
        let b: boolean = n == "foo";
        let c: boolean = n == m;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: boolean = true || false;
        let b: boolean = a || true;
        let c: boolean = a || b;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: boolean = true && false;
        let b: boolean = a && true;
        let c: boolean = a && b;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: boolean = !false;
        let b: boolean = !a;
    }
    """)

def test_call():
    assert typecheck_passes("""
    fn a() -> number {}

    fn main() {
        let num: number = a();
    }
    """)
