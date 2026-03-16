import os
from pathlib import Path
import re
import subprocess


def get_filename(args):
    directory = Path("created").joinpath(*args[:-1])
    filename = args[-1] + ".md"
    return directory / filename


def sanitise_line(line):
    # Remove trailing whitespace otherwise the markdown parser can insert an extra \n
    line = line.rstrip()

    # Replace angular brackets with HTML equivalents.
    line = line.replace(r"&", r"&amp;")
    line = line.replace(r"<", r"&lt;")
    line = line.replace(r">", r"&gt;")

    # If there are whitespace characters at the start of the line, replace the first with an &nbsp
    # so that it is not discarded by the markdown parser used by the parsed-literal directive.
    line = re.sub(r"^\s", r"&nbsp;", line)

    return line


# Process a single subcommand, adding new subcommands found to to_process.
def process(args, to_process):
    cmd = args + ["--help"]
    cmd_string = " ".join(cmd)
    filename = get_filename(args)
    filename.parent.mkdir(parents=True, exist_ok=True)

    print(f"Writing '{cmd_string}' to file {filename}")
    p = subprocess.run(cmd, capture_output=True, text=True, check=True)

    # Write output markdown file, identifying subcommands at the same time to provide
    # links to the subcommand markdown files.
    subcommands = []
    with open(filename, "w") as f:
        f.write(f"({filename})=\n")  # Target for links.
        f.write(f"# {' '.join(args)}\n")
        f.write("\n")
        f.write("```{parsed-literal}\n")

        in_subcommand_section = False
        for line in p.stdout.splitlines():
            if in_subcommand_section:
                match = re.match(r"^(  )([\w\-_]+)(\s+.*)$", line)
                if match:
                    subcommand = match.group(2)
                    subcommand_filename = get_filename(args + [subcommand])
                    line = (
                        match.group(1) + f"[{subcommand}]({subcommand_filename})" + match.group(3)
                    )
                    subcommands.append(subcommand)
            elif line.startswith("SUBCOMMANDS:"):
                in_subcommand_section = True

            f.write(sanitise_line(line) + "\n")
        f.write("```\n")

        subcommands.sort()
        to_process.extend(args + [subcommand] for subcommand in subcommands)

        if len(subcommands) > 0:
            # Hidden table of contents for subcommands of this command/subcommand.
            f.write("\n")
            f.write("```{toctree}\n")
            f.write(":hidden:\n")
            for subcommand in subcommands:
                f.write(f"{args[-1]}/{subcommand}\n")
            f.write("```\n")


if __name__ == "__main__":
    # Modify the PATH so that git2cpp is found by name, as using a full path will cause the help
    # pages to write that full path.
    git2cpp_dir = Path(__file__).parent.parent / "build"
    os.environ["PATH"] = f"{git2cpp_dir}{os.pathsep}{os.environ['PATH']}"

    to_process = [["git2cpp"]]
    while len(to_process) > 0:
        subcommand = to_process.pop(0)
        process(subcommand, to_process)
