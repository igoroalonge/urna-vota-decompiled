# investigation/scripts — reproducible checks against public 2026 TSE data

This directory holds small scripts that check the reconstructed VOTA code in `src/` against public
data of the 2026 Brazilian elections:

- TSE's published hash listings ("Resumos digitais (hashes)").
- The per-section urna files on resultados.tse.jus.br: `bu.dat`, `rdv.dat`, `*-vota.vsc` and `*-log.jez`.

Each script prints counts and pass/fail tables. It is meant to reproduce one numbered finding of the
investigation (see `MANIFEST.json`).

## Requirements

- Python ≥ 3.9
- `pip install asn1tools cryptography`

`hashlists.py`, `log_templates.py`, `fetch_sections.py` and `fetch_open_data.py` need only the
standard library.

No other third-party code is needed. Ed521 (EdDSA on curve E-521 with SHAKE256), used by the
UE2020/UE2022 urna certificates, is implemented in pure Python in `sigtools.py`. You do not need
ECPy or any other EdDSA library.

## Conventions

- **Run every script with `python3 -I`.** Downloaded files are untrusted data. Keep them in their own
  directory, outside this one. Never run anything from inside a data directory.
- **Repository root.** It is found from the script's location (`Path(__file__).resolve().parents[2]`),
  so `src/asn1/*.asn` and `src/` are read from the repository. Use `--repo PATH` to point elsewhere.
- **Paths.** Every path is a command-line argument; nothing is hard-coded.
- **Section layout.** All section tools use the layout written by `fetch_sections.py`:
  `<dir>/<uf>-<mun>-<zona>-<secao>/aux.json` and `<dir>/<section>/<hash[:16]>/<files>`.
  You can also pass single files.

## Getting the data

```sh
# Per-section urna files (resultados.tse.jus.br; browser-like headers are built in)
python3 -I investigation/scripts/fetch_sections.py --out data/sections \
    sample --per-uf 4 --seed 2026 ac al am ap ba ce df es go ma mg ms mt pa pb pe pi pr rj rn ro rr rs sc se sp to zz
python3 -I investigation/scripts/fetch_sections.py --out data/sections sections ac-01074-0004-0042 --types bu,rdv,vota,log
python3 -I investigation/scripts/fetch_sections.py --out data/sections count ac zz     # sections per UF

# TSE open data through the CKAN API (dadosabertos.tse.jus.br); checked against the published .sha512
python3 -I investigation/scripts/fetch_open_data.py search "boletim de urna 2026"
python3 -I investigation/scripts/fetch_open_data.py list resultados-2026-boletim-de-urna --uf AC
python3 -I investigation/scripts/fetch_open_data.py get  resultados-2026-boletim-de-urna --uf AC --out data/cdn
```

About the sample:

- Sampling is `random.Random(f"{seed}-{uf}").sample(...)` over the sections of each UF's published
  configuration file, so it is reproducible.
- `--seed 2026 --per-uf 4` selects 112 sections.
  - 106 of them are the investigation's 110-section sample.
  - 6 have no published files (HTTP 404) and are skipped.
  - The other 4 sample sections were added with the `sections` mode.

Hash listings: download the 2026 "Resumos digitais (hashes)" zips. `hashlists.py` reads the HTML
pages directly from the zips, or from a directory where you extracted them.

## Scripts

| Script | What it does |
|---|---|
| `fetch_sections.py` | Downloads per-section 2026 urna files (`sample` with seed, `sections`, `count`). Unsafe file names in `aux.json` are refused. |
| `fetch_open_data.py` | CKAN `search` / `list` / `get` by dataset name, UF and regex; `get` checks downloads against the `.sha512` resource. |
| `hashlists.py` | Hash-list HTML (or zip) → one TSV per page; recomputes every HASH GERAL; checks file order; prints the urna inventory diff and per-UF key variability. |
| `roundtrip_asn1.py` | Decodes and re-encodes `bu.dat` and `rdv.dat` with `src/asn1` and compares bytes. |
| `assinatura2026_inferred.asn` | **Our** ASN.1 layout of the 2026 `*-vota.vsc`, inferred from the real files (own type names; not a TSE document). |
| `vsc2026.py` | Decodes `.vsc` files, checks file digests and signatures (see below). |
| `sigtools.py` | Minimal DER/X.509 reader, Ed521 verification, ECDSA P-521 through `cryptography`. Library only. |
| `bu_vs_rdv.py` | Recomputes the BU tuple hash chain and every BU vote line from the RDV, with the code's rules. |
| `log_templates.py` | Log messages → templates per program → location in `src/`. |
| `qr_rebuild.py` | Rebuilds the 2026 BU and certificate QR texts with the code's rules and measures them. |
| `kitlib.py` | Shared helpers: repository root, ASN.1 compilation, file discovery, masking of 10–12 digit runs. |
| `run_all.sh` | Runs all offline checks and prints a summary. |

### hashlists.py — finding A1 (HASH GERAL), A2/A4 (inventory)

What it checks:

- **HASH GERAL** is recomputed with `CMontadorHash::CalculaHashGeral`
  (`src/uenux2/src/api/hash/cmontadorhash.cpp:88-100`):
  - `geral = first digest`
  - then, for each next digest: `geral = Base64(SHA-512(geral_text || next_digest_text))`
  - Files are taken in listed order. For the urna lists, the base files come first, then the UF's
    key files (the order `CGravadorHashes` writes them).
  - A "HASH GERAL CALCULADO A PARTIR DOS HASHES ACIMA" row covers only the rows above it.
    `pc1sis1` lists 2 more files after that row.
- **File order:** the base-file order must be `CriaHashesDiretorio`'s walk (files of a directory
  sorted byte-wise, then its subdirectories, depth first). Key files must be sorted.

```sh
python3 -I investigation/scripts/hashlists.py hashes/ --tsv-out out/hash-tsv [--compare-txt hashes-txt/]
```

Expected output:

```
TOTAL: lists with one HASH GERAL reproduced 10/10; urna per-UF HASH GERAL reproduced 112/112
base files: union 173; per model UE2013=135, UE2015=136, UE2020=140, UE2022=140
present in every model with the same digest: 123
key files that differ per UF: 11; identical in every UF: 6
```

The six keys that are identical in every UF have the same digests as Holocron's
`chaves\legal\o*.ber.pub`.

### roundtrip_asn1.py — finding B1

- `bu.dat` = `EntidadeEnvelopeGenerico` → `EntidadeBoletimUrna`.
- `rdv.dat` = a bare `EntidadeResultadoRDV`. It is not enveloped, as `CGravadorRDV` writes it.

```sh
python3 -I investigation/scripts/roundtrip_asn1.py data/sections [--verbose]
```

Expected output:

```
bu.dat : 110 files; envelope identical 110/110; inner EntidadeBoletimUrna identical 110/110
rdv.dat: 110 files; EntidadeResultadoRDV identical 110/110; decodable as EntidadeEnvelopeGenerico: 0
```

### vsc2026.py (+ assinatura2026_inferred.asn) — 2026 signature files

For each `.vsc` file, the script:

- decodes it with our inferred layout, re-encodes it and compares bytes;
- prints the urna model and hardware algorithm of the trailing block, both blocks' algorithms,
  the signer names and the key-set label (the "conjunto de chaves" tag);
- checks:
  - the SHA-512 and size of every published file next to the `.vsc`;
  - each list's own digest;
  - that the software and hardware lists agree;
- verifies with the urna certificate's key:
  - each listed file's hardware signature;
  - the hardware list signature;
  - every BU election signature.

  The urna signs **SHA-512 of the value**: ECDSA-with-SHA-512 over the value, or Ed521 with the
  64-byte SHA-512 as message. A control check verifies over the raw last hash; it must fail.

Limits:

- `--roots CA.pem` checks the certificate chain. It needs TSE's "AC UE2020", "AC UE2022" and
  "AC URNA" certificates, which you obtain from TSE's verification package; they are not shipped here.
- The software signature (algorithm 3, CEPESC) has no public implementation and is **not** verified.

```sh
python3 -I investigation/scripts/vsc2026.py data/sections [--roots tse_ca.pem] [--verbose]
```

Expected output:

- 110/110 round trips.
- Models: UE2020 47, UE2022 44, UE2015 14, UE2013 5.
- The trailing algorithm equals the hardware algorithm in 110/110 files (eddsa on UE2020/22,
  ecdsa on UE2013/15).
- Signatures that verify:
  - 1,001 Ed521 and 197 ECDSA file signatures;
  - 91 + 19 list signatures;
  - 182 + 34 BU signatures.
- The control check verifies 0 of 216; there are 0 failures.

### bu_vs_rdv.py — finding B (hash chain, BU from RDV)

**Hash chain** (`cconversorentidadebu.cpp` formats @8483/@1120/@1126):

- seed: `SHA-512("{pleito:05}|{eleicao:05}|{mun:05}|{zona:04}|{secao:04}|{carga:24}")`
- each line: `SHA-512("{HEX(prev)}|{i}|{cargo}|{tipo}|{qtd}[|{numero}|{partido}]")`

**BU lines** are recomputed from the RDV with the `ConverteTipoVoto` table @546352. The script also
checks the count, ordering and "no zero line" rules.

```sh
python3 -I investigation/scripts/bu_vs_rdv.py data/sections
```

Expected output:

- 216 elections, 13,088 tuples and 534 office blocks.
- Every check passes; failures: 0.

### log_templates.py — finding C1

- Reads `logd.dat` from each `*-log.jez` in memory; nothing is extracted to disk.
- Normalises the messages into fine and coarse templates for each program.
- Maps the templates to string literals in `src/`: exact, `std::format`, composed, or not found.

```sh
python3 -I investigation/scripts/log_templates.py data/sections [--tsv out/log_templates.tsv] [--all]
```

Expected output:

- 622,567 lines in total: VOTA 566,963, GAP 33,538, ATUE 10,260, SCUE 9,329, LOGD 2,214, INITJE 263.
- VOTA has 191 coarse templates: 86 exact, 92 format, 5 composed and 8 NOT FOUND:
  - Início de impressão / Fim de impressão
  - four printer messages
  - Votação suspensa
  - one microterminal LCD error

### qr_rebuild.py — finding H5

Rules: VRQR 6.0 header and body, parts of 1100−277 = 823 characters cut at the last space, chained
SHA-512 `HASH`, `ASSI` on the last QR, and certificate QR codes of `ceil(2·bytes/1082)` parts.
Sources:

- `src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp`
- `ccabecalhoqrcodebuilder.u34/u36.cpp`
- `vota/eleitor/fimvotacao/cgerabu.cpp`

**Caveat:** some values are not in `bu.dat`, so the lengths are predictions from the code, not
readings of a printed BU.

- PROC, VERC, DTPL and TURN are fixed-width placeholders (`--proc --verc --dtpl --turno`).
- AGRE is left out.
- ASSI uses the length of the BU's own signature.

```sh
python3 -I investigation/scripts/qr_rebuild.py data/sections [--print-texts]
```

Expected output:

```
(BU QR count, certificate QR count) distribution: {(1, 2): 4, (2, 2): 9, (3, 2): 70, (4, 2): 26, (5, 2): 1}
last QR longer than 1100 characters: 13 of 110; longest QR 1239; longest last QR 1239
```

### run_all.sh

```sh
PYTHON=python3 sh investigation/scripts/run_all.sh data/sections hashes/ out/
```

Set `REPO=...` if the scripts are not inside `<repo>/investigation/scripts`.

A full run on 110 sections takes about 1 minute. Most of the time is spent on Ed521 verification
and the log scan.

## Data privacy

- **Raw urna logs contain poll-worker identifiers.** `*-log.jez` / `logd.dat` carry mesários'
  títulos eleitorais in clear text. SCUE/GAP lines carry the media-generation computer name and the
  Windows user.
- **GEDAI log bundles** from open data contain staff user numbers and computer names.
- **Never commit** raw logs, extracted `logd.dat` files, or anything quoted from them, to a public
  repository. Keep downloaded data outside the repository, or git-ignore it.
- The scripts print counts and templates only:
  - `log_templates.py` never prints raw lines;
  - it masks every 10–12 digit run;
  - it replaces the computer and user values with `<masked>`.
- `vsc2026.py` masks the digits in the hardware signer names.
- **Review any output before publishing it.**
