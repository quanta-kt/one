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
    Invokes onec for converting `code` into a S-expressions.
    """

    stdout, _ = invoke_onec(["--s-expr"], f"{code}\n")
    return stdout.strip()


def stmt2sexpr(stmt: str) -> str:
    template = "".join("fn main() {{ {0} }}")

    code = template.format(stmt)
    sexpr = code2sexpr(code)

    prefix = "(fn main () :() "
    suffix = ")"

    return sexpr[len(prefix) : -len(suffix)]
