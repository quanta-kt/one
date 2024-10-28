import subprocess


def code2sexpr(code: str) -> str:
    """
    Invokes onec for converting `code` into a S-expressions.
    """

    proc = subprocess.Popen(
        ["onec"],
        stdout=subprocess.PIPE,
        stdin=subprocess.PIPE,
    )

    proc.stdin.write(f"{code}\n".encode())
    proc.stdin.close()

    return proc.stdout.read().decode().strip()


def stmt2sexpr(stmt: str) -> str:
    template = "".join("fn main() {{ {0} }}")

    code = template.format(stmt)
    sexpr = code2sexpr(code)

    prefix = "(fn main () "
    suffix = ")"

    return sexpr[len(prefix) : -len(suffix)]
