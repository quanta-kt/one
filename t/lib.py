import subprocess


def invoke_onec(args: list[str], stdin: str) -> tuple[str, int]:
    proc = subprocess.Popen(
        ["onec", *args],
        stdout=subprocess.PIPE,
        stdin=subprocess.PIPE,
    )

    proc.stdin.write(stdin.encode())
    proc.stdin.close()

    exit_code = proc.wait()

    return (proc.stdout.read().decode(), exit_code)


def code2sexpr(code: str) -> str:
    """
    Converts 'code' to an S-expression.
    """
    proc = subprocess.Popen(
        ["code2sexpr", code],
        stdout=subprocess.PIPE,
    )

    proc.wait()
    return proc.stdout.read().decode().strip()


def stmt2sexpr(stmt: str) -> str:
    template = "".join("fn main() {{ {0} }}")

    code = template.format(stmt)
    sexpr = code2sexpr(code)

    prefix = "(fn main () :() "
    suffix = ")"

    return sexpr[len(prefix) : -len(suffix)]


def typecheck_passes(code: str) -> bool:
    """
    Invokes onec for typechecking, returns true if code passes type-check.
    """

    proc = subprocess.Popen(["typecheck", code])

    match proc.wait():
        case 1:
            return False
        case 0:
            return True
        case _:
            raise Exception("Failed to typecheck")

def code2token_list(code: str) -> str:
    """
    Invokes code2token_list that in turn tokenizes
    the code with the lexer and returns string with
    new-line separated list of tokens.
    """

    proc = subprocess.Popen(
        ["code2token-list", code],
        stdout=subprocess.PIPE,
    )

    proc.wait()
    return proc.stdout.read().decode()
