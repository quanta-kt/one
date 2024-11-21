from lib import typecheck_passes


def test_typecheck_tuple():
    assert typecheck_passes("""
    fn main() {
        let a: ();
        let b: () = a;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: (string);
        let b: (string) = a;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: (string, number);
        let b: (string, number) = a;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: (string, (number, string));
        let b: (string, (number, string)) = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: (string, (string, number));
        let b: (string, (number, string)) = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: (string);
        let b: (number) = a;
    }
    """)
