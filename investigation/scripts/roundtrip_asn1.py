#!/usr/bin/env python3
"""Decode and re-encode real urna result files with the recovered ASN.1 (src/asn1), byte for byte.

  *-bu.dat / *-busa.dat  EntidadeEnvelopeGenerico (ModuloEnvelopeGenerico); its `conteudo` is an
                         EntidadeBoletimUrna (ModuloBoletimUrna) when tipoEnvelope is envelopeBoletimUrna
                         and no `seguranca` (encryption) field is present
  *-rdv.dat              a bare EntidadeResultadoRDV (ModuloRegistroDigitalVoto), not enveloped, as
                         CGravadorRDV writes it; the script also confirms that it does NOT decode as an
                         envelope

For every file: decode, re-encode, compare with the original bytes (outer envelope and inner entity
separately). Prints one line per file with --verbose, and the totals.

Usage:
  python3 -I roundtrip_asn1.py SECTIONS_DIR_OR_FILES... [--repo REPO] [--verbose]
Expected on the 110-section 2026 sample: 110/110 BU envelopes and inner BUs and 110/110 RDVs identical;
0 RDVs decodable as an envelope.
"""
import argparse
import collections
import sys
from pathlib import Path

sys.dont_write_bytecode = True          # keep the scripts directory clean
sys.path.insert(0, str(Path(__file__).resolve().parent))
import kitlib  # noqa: E402

INNER = {"envelopeBoletimUrna": "EntidadeBoletimUrna",
         "envelopeRegistroDigitalVoto": "EntidadeResultadoRDV"}


def check_bu(spec, data):
    env = spec.decode("EntidadeEnvelopeGenerico", data)
    outer_ok = spec.encode("EntidadeEnvelopeGenerico", env) == data
    tipo = env["tipoEnvelope"]
    inner_type = INNER.get(tipo)
    if inner_type is None or "seguranca" in env:
        return outer_ok, None, f"tipoEnvelope={tipo} seguranca={'seguranca' in env}"
    inner = spec.decode(inner_type, env["conteudo"])
    inner_ok = spec.encode(inner_type, inner) == env["conteudo"]
    return outer_ok, inner_ok, f"{inner_type} ({len(env['conteudo'])} bytes)"


def check_rdv(spec, data):
    try:
        spec.decode("EntidadeEnvelopeGenerico", data)
        as_env = True
    except Exception:
        as_env = False
    v = spec.decode("EntidadeResultadoRDV", data)
    return spec.encode("EntidadeResultadoRDV", v) == data, as_env


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("inputs", nargs="+", type=Path)
    kitlib.add_repo_arg(ap)
    ap.add_argument("--verbose", action="store_true", help="one line per file")
    a = ap.parse_args()
    spec = kitlib.compile_urna_spec(a.repo)

    c = collections.Counter()
    fails = []
    for f in kitlib.find_files(a.inputs, ("-bu.dat", "-busa.dat", "-rdv.dat")):
        data = f.read_bytes()
        lab = kitlib.section_label(f)
        try:
            if f.name.endswith("-rdv.dat"):
                c["rdv files"] += 1
                ok, as_env = check_rdv(spec, data)
                c["rdv EntidadeResultadoRDV re-encodes identically"] += ok
                c["rdv decodable as EntidadeEnvelopeGenerico"] += as_env
                line = f"{lab} {kitlib.short_name(f)}: identical={ok} decodes-as-envelope={as_env}"
                if not ok:
                    fails.append(line)
            else:
                c["bu files"] += 1
                outer_ok, inner_ok, what = check_bu(spec, data)
                c["bu envelope re-encodes identically"] += outer_ok
                c["bu inner entity re-encodes identically"] += bool(inner_ok)
                line = f"{lab} {kitlib.short_name(f)}: envelope identical={outer_ok} inner identical={inner_ok} [{what}]"
                if not (outer_ok and inner_ok):
                    fails.append(line)
        except Exception as e:
            line = f"{lab} {kitlib.short_name(f)}: DECODE ERROR {type(e).__name__}: {str(e)[:200]}"
            fails.append(line)
        if a.verbose:
            print(line)
    print(f"bu.dat : {c['bu files']} files; envelope identical {c['bu envelope re-encodes identically']}/"
          f"{c['bu files']}; inner EntidadeBoletimUrna identical {c['bu inner entity re-encodes identically']}/"
          f"{c['bu files']}")
    print(f"rdv.dat: {c['rdv files']} files; EntidadeResultadoRDV identical "
          f"{c['rdv EntidadeResultadoRDV re-encodes identically']}/{c['rdv files']}; decodable as "
          f"EntidadeEnvelopeGenerico: {c['rdv decodable as EntidadeEnvelopeGenerico']}")
    print(f"failures: {len(fails)}")
    for x in fails:
        print("FAIL", x)
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
