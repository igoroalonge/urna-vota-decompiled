"""Shared helpers for the investigation scripts (standard library + asn1tools).

The repository root is found relative to this file (<repo>/investigation/scripts/kitlib.py), so the
recovered ASN.1 modules are read from <repo>/src/asn1. Every script also accepts --repo to point
elsewhere. Scripts are meant to be run with `python3 -I`; under -I the script directory is not on
sys.path, so each script adds its own directory explicitly before importing this module.
"""
import json
import os
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO_DEFAULT = Path(__file__).resolve().parents[2]

# Portable file-name pattern of the published urna files: o03220ac0107400040042-bu.dat
URNA_FILE_PREFIX = re.compile(r"^[ost]\d{5}[a-z]{2}\d{13}-")

# A 10-12 digit run that is not part of a longer number (título eleitoral is 12 digits, CPF 11).
_TITULO_LIKE = re.compile(r"(?<!\d)\d{10,12}(?!\d)")


def mask_ids(text):
    """Replace every isolated 10-12 digit run by '#' characters (títulos, CPFs, staff numbers)."""
    return _TITULO_LIKE.sub(lambda m: "#" * len(m.group(0)), text)


def add_repo_arg(parser):
    parser.add_argument("--repo", type=Path, default=REPO_DEFAULT,
                        help="repository root holding src/asn1 (default: two levels above this script)")


def asn1_dir(repo):
    d = Path(repo) / "src" / "asn1"
    if not d.is_dir():
        sys.exit(f"ASN.1 directory not found: {d} (use --repo <repository root>)")
    return d


def _asn1tools():
    try:
        import asn1tools
    except ImportError:
        sys.exit("this script needs asn1tools: pip install asn1tools cryptography")
    return asn1tools


def compile_urna_spec(repo, numeric_enums=False):
    """Compile every src/asn1/*.asn module (BER) with asn1tools."""
    asn1tools = _asn1tools()
    files = sorted(str(p) for p in asn1_dir(repo).glob("*.asn"))
    return asn1tools.compile_files(files, codec="ber", numeric_enums=numeric_enums)


def compile_vsc_spec(numeric_enums=True):
    """Compile the inferred 2026 signature-file layout shipped next to the scripts."""
    asn1tools = _asn1tools()
    return asn1tools.compile_files([str(HERE / "assinatura2026_inferred.asn")], codec="ber",
                                   numeric_enums=numeric_enums)


def find_files(roots, suffixes):
    """Every file under the given roots (files or directories) whose name ends with one of the
    suffixes, sorted. Symbolic links are not followed."""
    if isinstance(suffixes, str):
        suffixes = (suffixes,)
    out = []
    for r in roots:
        r = Path(r)
        if r.is_file():
            if r.name.endswith(tuple(suffixes)):
                out.append(r)
            continue
        for dp, dns, fns in os.walk(r):
            dns.sort()
            for fn in sorted(fns):
                if fn.endswith(tuple(suffixes)):
                    out.append(Path(dp) / fn)
    return out


def section_label(path, root=None):
    """Short label of a urna file: '<section dir>/<hash dir>' when the standard layout
    <root>/<uf-mun-zona-secao>/<hash16>/<file> is used, else the parent directory name."""
    p = Path(path)
    parts = p.parts
    if len(parts) >= 3:
        return f"{parts[-3]}/{parts[-2]}"
    return p.parent.name


def short_name(fn):
    """'o03220ac0107400040042-bu.dat' -> 'bu.dat'."""
    return URNA_FILE_PREFIX.sub("", Path(fn).name)


def load_json_any(path):
    raw = Path(path).read_bytes()
    try:
        return json.loads(raw.decode("utf-8"))
    except UnicodeDecodeError:
        return json.loads(raw.decode("latin-1"))


def counter_lines(counter, indent="    "):
    return [f"{indent}{k!s:<60} {v}" for k, v in sorted(counter.items(), key=lambda kv: repr(kv[0]))]
