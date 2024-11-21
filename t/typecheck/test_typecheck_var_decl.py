from lib import typecheck_passes


def test_var_declaration():
    assert typecheck_passes("""
    fn main() {
        let b: string;
        let a: string = b;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let b: i32;
        let a: string = b;
    }
    """)


def test_var_declaration_implicit():
    assert typecheck_passes("""
    fn main() {
        let b = "";
        let a: string = b;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let b = "";
        let a: i32 = b;
    }
    """)


def test_var_declaration_tuple():
    assert typecheck_passes("""
    fn main() {
        let a: (string, string);
        let b: (string, string) = a;
        let c = b;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: (string, string);
        let b: (i32, string) = a;;
        let c = b;
    }
    """)
