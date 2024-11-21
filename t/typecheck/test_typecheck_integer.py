from lib import typecheck_passes


def test_same_integer_types_are_compatible():
    assert typecheck_passes("""
    fn main() {
        let a: u8 = 12;
        let b: u8 = a;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: u16 = 21;
        let b: u16 = a;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: u32 = 32;
        let b: u32 = a;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: i8 = 43;
        let b: i8 = a;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: i16 = 55;
        let b: i16 = a;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let a: i32 = 99;
        let b: i32 = a;
    }
    """)


def test_different_integer_types_are_not_compatible():
    assert not typecheck_passes("""
    fn main() {
        let a: u8;
        let b: u16 = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: u16;
        let b: u32 = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: i8;
        let b: i16 = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: i16;
        let b: iu = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: u16;
        let b: u32 = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: u8;
        let b: i8 = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: u16;
        let b: i16 = a;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let a: u32;
        let b: i32 = a;
    }
    """)


def test_i32_is_default():
    assert typecheck_passes("""
    fn main() {
        let n = 30;
        let a: i32 = n;
        let b = a;
    }
    """)

    assert typecheck_passes("""
    fn main() {
        let n = 30 + 10;
        let a: i32 = n;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let n = 30;
        let a: u32 = n;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let n = 30;
        let a: u8 = n;
    }
    """)

    assert not typecheck_passes("""
    fn main() {
        let n = 30;
        let a: i8 = n;
    }
    """)
