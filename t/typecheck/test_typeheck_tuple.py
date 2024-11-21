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
        let a: (string, i32);
        let b: (string, i32) = a;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: (string, (i32, string));
        let b: (string, (i32, string)) = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: (string, (string, i32));
        let b: (string, (i32, string)) = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: (string);
        let b: (i32) = a;
    }
    """)
