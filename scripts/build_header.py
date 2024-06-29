import re
from pathlib import Path


base_folder = Path(__file__).parent.parent
headers_folder = base_folder / "src"
output_folder = base_folder / "out"


class HeaderParser:
    def __init__(self) -> None:
        self.included_headers = []

    include_directive_pattern = re.compile(
        r"\s*#\s*include\s*(?P<include_args>.*)\s*",
    )
    include_args_pattern = re.compile(
        r"<(?P<h_char_seq>[^>]*)>|\"(?P<q_char_seq>[^\"]*)\"",
    )
    pragma_once_pattern = re.compile(r"\s*#\s*pragma\s*once\s*")

    def parse_header(self, name: str) -> list[str]:
        if name in self.included_headers:
            return []

        self.included_headers.append(name)

        with (headers_folder / name).open() as fs:
            return self.expand_includes(fs.readlines())

    def parse_include_directive(self, line: str) -> str:
        m = self.include_directive_pattern.fullmatch(line)

        if m is None:
            raise ValueError

        include_args = m.group("include_args")
        m = self.include_args_pattern.fullmatch(include_args)

        if m is None:
            raise ValueError

        h_char_seq = m.group("h_char_seq")

        if h_char_seq is not None:
            return h_char_seq

        return m.group("q_char_seq")

    def parse_pragma_once(self, line: str) -> bool:
        return self.pragma_once_pattern.fullmatch(line) is not None

    def expand_includes(self, lines: list[str]) -> list[str]:
        out = []

        for line in lines:
            if line == "\n" or self.parse_pragma_once(line):
                continue

            try:
                include_path = self.parse_include_directive(line)
                out.extend(self.parse_header(include_path))
            except (ValueError, OSError):
                out.append(line)

        return out


def main() -> None:
    parser = HeaderParser()

    with (output_folder / "main.h").open("w") as fs:
        fs.writelines(parser.parse_header("main.h"))


if __name__ == "__main__":
    main()
