# VOTA, reconstructed from TSE's public voting simulator

*[Versão em português](README.md)*

A readable C++ reconstruction of the TSE code compiled into `vota_web_wasm.wasm`, the WebAssembly
module behind the voting simulator that the Brazilian Electoral Court (TSE) publishes at
<https://www.justicaeleitoral.jus.br/simulador-votacao/>.

| | |
|---|---|
| module | `vota_web_wasm.wasm`, 7,713,699 bytes |
| SHA-256 | `8579ef8d1a93819152cecdbf52906d2e4dd0f57999cb2e1e8c0d1161cd1007e2` |
| version string in the binary | `10.23.0.1 - DESENVOLVIMENTO` (a development build) |

## What is here

| path | content |
|---|---|
| `src/uenux2/src/app/vota/` | the voting application: voter terminal, mesário (poll-worker) terminal, start and end of day, BU generation |
| `src/uenux2/src/app/comum/` | code shared by the urna applications: election data, RDV, reports, result writers, ASN.1 converters, logs |
| `src/uenux2/src/api/` | the urna platform layer: GUI toolkit, service registry, threads and messages, files, timers |
| `src/uenux2/mock/`, `src/uenux2/wasm/` | the small web layer: the `vota*` entry points and the browser stand-ins for screen, keypad and sound |
| `src/ecourna/` | TSE's `ecourna` library: crypto wrappers, SQLite wrapper, ZIP, data model and its ASN.1 converters |
| `src/asn1/` | the 32 ASN.1 modules (data formats) recovered from the binary |
| `docs_en/modules/` | one chapter per reconstruction unit (41), each ending in a function → symbol → file table; start at `docs_en/modules/README.md` |
| `docs_en/data-model/asn1-schemas.md` | how the ASN.1 modules were recovered, and what each one describes |
| `docs_en/glossary.md` | Portuguese and TSE terms |
| `investigation/` | the October 2026 investigation: this reconstruction checked against the files real urnas wrote in the 2026 election and against TSE's public documents, with the scripts that reproduce it; start at `investigation/README.md` |

1,184 C++ files, 83,183 lines. The file paths are the original ones, recovered from the binary.
The module contains 3,463 TSE functions. All of them are reconstructed here except 4 trivial ones:
three compiler-generated string destructors and one unnamed helper. No third-party library code
(OpenSSL, SQLite, RHVoice, Boost, zlib, 7-Zip, libc++) is included.

The documentation is in `docs_en/` (English original) and `docs/` (Portuguese translation).

## What this is not

* **Not TSE's original source code.** It is a reconstruction from the compiled binary. Names come
  from data the compiler left in the module (`std::source_location` records with file, function
  and line; RTTI class names; vtables), and are marked when inferred. A second reviewer checked each
  function against the decompiled code, but errors are possible.
* **It does not compile.** It is meant to be read next to the binary. 325 of the 815 project headers
  that the files include were never reconstructed, and the third-party headers are absent.
* **Not the program in the sealed urnas.** The simulator is a development build of the same source
  tree, with web stand-ins for the hardware and placeholder signatures
  (`assinatura simulada para vota_web_wasm`). The real urnas of the 2026 election ran
  `10.23.0.0 - Praia da Barra do Cahy`. The result files and logs they wrote are consistent with this
  code, which suggests the same source tree but does not prove the binaries are the same. The logs
  also show gaps in the reconstruction: 8 of the 191 VOTA log messages that real urnas wrote are not
  in `src/` (the printer driver's, `Votação suspensa`, and a mesário-terminal display error); the
  key-off shutdown ends after the key line, where real urnas go on to log `Desligando a urna` and
  `Finalização de aplicativo`; and two log calls (`Título … para suspender a votação`) were
  reconstructed as INFO, while the urnas log them as ALERTA. The published logs stop when `log.jez`
  is packed, so the signing and copying of the result files never appear in them (2026 urna data:
  `investigation/README.md`, findings C1, C2, C3 and G13).

## Reading the code

* `func N`: the function with index N in the module, imports included. It is the number that
  `wasm2wat` prints as `(func (;N;)`, so any reconstructed function can be compared with the
  original instructions:

  ```sh
  wasm2wat vota_web_wasm.wasm -o vota.wat     # wabt: https://github.com/WebAssembly/wabt
  grep -n '(func (;7840;)' vota.wat           # votaInit, exported as "Eb"
  ```
* `@N`: an address in the module's linear memory (strings, vtables, global variables).
* `srcloc file.cpp:L`: a `std::source_location` record that gives the original file and line.
* `uNN §x`: a section of the chapter `docs_en/modules/uNN-*.md`.
* Mentions of `analysis/`, `decompiled/`, `tools/`, `work/`, `upstream/`, `samples/`, `q.py`, and of
  chapters `00`–`12`, `bu/` or `libraries/`, refer to the analysis project this repository was taken
  from. Those files are not included, and links to them were turned into plain text.

## Data

* The source paths compiled into the binary are kept exactly as they appear there.
* The simulator's training scenarios include a voter record (name, título and birth date). Those
  values are replaced here by `NOME OMITIDO`, `XXXXXXXXXXXX` and `AAAA`. Test inputs with valid
  check digits (a CPF and a título) are masked the same way.

## Note

Built only from public material: the module and data files served by TSE's simulator page, and
files that TSE itself publishes. Made for study (WebAssembly, and how the published simulator
works). Not affiliated with or endorsed by TSE. The rights to the original software belong to their
owners, and this repository does not grant a license to it.

**Takedown requests.** If TSE or any other competent authority asks for it, this repository will be
taken down promptly. To ask, open an issue here or get in touch through the GitHub profile.
