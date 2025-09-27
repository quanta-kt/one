from lib import code2error_list as errors

def test_error_invalid_item():
    assert errors("""
    func main() {
        let a = 0;
    }
    """) == ["E0506"]

    assert errors("""
    ;;

    fn main() {
        let a = 0;
    }
    """) == ["E0506"]

    assert errors("""
    any number of meaningless tokens here

    fn main() {
        let a = 0;
    }
    """) == ["E0506"]

