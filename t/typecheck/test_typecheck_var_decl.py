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
        let b: number;
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
        let a: number = b;
    }
    """)
