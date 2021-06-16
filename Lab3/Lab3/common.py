#!/usr/bin/python3

from sys import stderr


# DEBUG
def si_error(msg: str) -> None:
    print(msg, file=stderr)
