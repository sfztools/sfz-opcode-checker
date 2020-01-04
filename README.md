# sfz-opcode-checker

[![Travis Build Status]](https://travis-ci.com/sfztools/sfz-opcode-checker)
[![AppVeyor Build Status]](https://ci.appveyor.com/project/sfztools/sfz-opcode-checker)

The **sfz-opcode-checker** utility scans opcodes from any number of given SFZ
files, and checks their existence in the public specification.

The source of known opcodes is the [YAML database] from the [reference site].

## Usage

The basic usage of this program is:
```
sfz-opcode-checker validate -d syntax.yml *.sfz
```

Run the program without arguments for additional information.


[Travis Build Status]:   https://img.shields.io/travis/com/sfztools/sfz-opcode-checker.svg?label=Linux&style=popout&logo=travis
[AppVeyor Build Status]: https://img.shields.io/appveyor/ci/sfztools/sfz-opcode-checker.svg?label=Windows&style=popout&logo=appveyor
[YAML database]:  https://github.com/sfzformat/sfzformat.github.io/blob/source/_data/sfz/syntax.yml
[reference site]: https://sfzformat.github.io/
