# sfz-opcode-checker

The **sfz-opcode-checker** utility scans opcodes from any number of given SFZ
files, and checks their existence in the public specification.

The source of known opcodes is the [YAML database](https://github.com/sfzformat/sfzformat.github.io/blob/source/_data/sfz/syntax.yml)
from the [reference site](https://sfzformat.github.io/).

## Usage

The basic usage of this program is:
```
sfz-opcode-checker validate -d syntax.yml *.sfz
```

Run the program without arguments for additional information.
