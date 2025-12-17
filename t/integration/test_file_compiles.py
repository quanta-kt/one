import tempfile

from lib import invoke_onec


def test_file_compiles():
    with tempfile.NamedTemporaryFile() as tmp:
        tmp.write(b"""
        fn main() {
            let message: string = "hello";

            let a = 1;
            let b = 2;
            let c = 1 + 2;
        }
        """)
        tmp.flush()
        (_, status) = invoke_onec([tmp.name])
        assert status == 0

