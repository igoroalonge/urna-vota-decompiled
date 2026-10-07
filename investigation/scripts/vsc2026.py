#!/usr/bin/env python3
"""Decode and check the 2026 urna signature files (*-vota.vsc).

Layout: assinatura2026_inferred.asn (inferred by this investigation from the real files; not a TSE
document). For every file the script

  1. decodes it and re-encodes it, comparing bytes;
  2. prints (with --verbose) the urna model and hardware algorithm of the trailing block, the
     algorithms of both signature blocks, the signer names, the key-set label of the software block
     ("conjunto de chaves" tag) and the kind of the hardware certificate (PEM/DER, subject/issuer CN);
  3. checks the file list: SHA-512 and size of every listed file present next to the .vsc, the
     list's own digest (SHA-512 of the encoded list), and that the software and hardware lists agree;
  4. verifies the hardware signatures with the public key of the urna certificate:
       - every listed file:          signature over SHA-512(file digest)
       - the hardware list itself:   signature over SHA-512(list digest)
       - every election of the BU next to it (bu.dat/busa.dat): signature over
         SHA-512(ultimoHashVotosVotavel), plus the control "over the raw hash", which must fail
     UE2020/UE2022 certificates hold Ed521 keys (EdDSA on curve E-521, SHAKE256): verified with the
     pure-Python implementation in sigtools.py (no third-party library needed). UE2013/UE2015 hold
     ECDSA P-521 keys: verified with `cryptography`.
  5. optionally (--roots PEM...) verifies the urna certificate against the issuing CA certificates
     (TSE's "AC UE2020", "AC UE2022", "AC URNA"; obtain them from TSE's published verification
     package -- they are not shipped here);
  6. the software signature (algorithm 3, CEPESC, 4,765 bytes) has no public implementation and is
     NOT verified.

Usage:
  python3 -I vsc2026.py SECTIONS_DIR_OR_FILES... [--repo REPO] [--roots CA.pem ...] [--verbose]
Expected on the 110-section 2026 sample: 110/110 round trips; models UE2020 47, UE2022 44,
UE2015 14, UE2013 5; trailing algorithm = hardware algorithm 110/110; all file digests, list
digests and hardware signatures verify; BU signatures verify over SHA-512(last hash) and never over
the raw hash.
"""
import argparse
import collections
import hashlib
import re
import sys
from pathlib import Path

sys.dont_write_bytecode = True          # keep the scripts directory clean
sys.path.insert(0, str(Path(__file__).resolve().parent))
import kitlib  # noqa: E402
import sigtools  # noqa: E402

ALG = {1: "rsa", 2: "ecdsa", 3: "cepesc", 4: "eddsa"}
HASH = {1: "sha1", 2: "sha256", 3: "sha384", 4: "sha512"}
TIPO_ARQUIVO = {1: "votacaoUE", 2: "votacaoRED", 3: "saMistaMRParcialCedula", 4: "saMistaBUImpressoCedula",
                5: "saManual", 6: "saEletronica"}


def load_roots(paths):
    roots = {}
    for p in paths or []:
        txt = Path(p).read_bytes()
        for m in re.finditer(rb"-----BEGIN CERTIFICATE-----.+?-----END CERTIFICATE-----", txt, re.S):
            der, _ = sigtools.pem_or_der(m.group(0))
            c = sigtools.parse_certificate(der)
            roots[c["subject_cn"]] = c
    return roots


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("inputs", nargs="+", type=Path)
    kitlib.add_repo_arg(ap)
    ap.add_argument("--roots", nargs="*", type=Path, help="PEM file(s) with the issuing CA certificates")
    ap.add_argument("--verbose", action="store_true")
    a = ap.parse_args()

    vsc = kitlib.compile_vsc_spec(numeric_enums=True)
    try:
        bu_spec = kitlib.compile_urna_spec(a.repo, numeric_enums=True)
    except SystemExit as e:
        print(f"note: {e}; BU signatures will not be checked", file=sys.stderr)
        bu_spec = None
    roots = load_roots(a.roots)
    C = collections.defaultdict(collections.Counter)
    fails = []

    def check(name, ok, what=None):
        C[name][bool(ok)] += 1
        if not ok and what:
            fails.append(f"{name}: {what}")

    for f in kitlib.find_files(a.inputs, ".vsc"):
        lab = kitlib.section_label(f)
        data = f.read_bytes()
        try:
            v = vsc.decode("VscFile", data)
        except Exception as e:
            check("decodes with assinatura2026_inferred.asn", False, f"{lab}: {type(e).__name__}: {str(e)[:150]}")
            continue
        check("decodes with assinatura2026_inferred.asn", True)
        check("re-encodes byte for byte", vsc.encode("VscFile", v) == data, lab)
        sw, hw, org = v["softwareBlock"], v["hardwareBlock"], v["hardwareOrigin"]
        model, hw_alg_tail = org["urnaModel"], org["hwAlgorithm"]
        ssw, shw = sw["selfSignature"], hw["selfSignature"]
        swa = (ALG.get(ssw["sigAlgorithm"]["id"]), ssw["sigAlgorithm"]["keyBits"])
        hwa = (ALG.get(shw["sigAlgorithm"]["id"]), shw["sigAlgorithm"]["keyBits"])
        C["urna model (trailing block)"][f"UE{2000 + model}" if model >= 9 else model] += 1
        C["(model, hw algorithm in trailing block)"][(model, ALG.get(hw_alg_tail))] += 1
        C["software signature (algorithm, bits)"][swa] += 1
        C["hardware signature (algorithm, bits)"][hwa] += 1
        C["hash algorithms (sw, hw)"][(HASH.get(ssw["hashAlgorithm"]["id"]), HASH.get(shw["hashAlgorithm"]["id"]))] += 1
        C["software signer name"][ssw["signer"]["name"]] += 1
        C["hardware signer name (digits masked)"][re.sub(r"\d", "#", shw["signer"]["name"])] += 1
        check("trailing algorithm == hardware signature algorithm", hw_alg_tail == shw["sigAlgorithm"]["id"], lab)
        kind_sw, key_sw = sw["keyInfo"]
        kind_hw, key_hw = hw["keyInfo"]
        C["software block key info"][(kind_sw, key_sw if kind_sw == "keySetLabel" else len(key_sw))] += 1

        # --- lists
        lsw = vsc.decode("SignedFileList", sw["signedList"])["files"]
        lhw = vsc.decode("SignedFileList", hw["signedList"])["files"]
        for blk, name in ((sw, "software"), (hw, "hardware")):
            s = blk["selfSignature"]["value"]
            check(f"{name} list digest == SHA-512(list) and size == len(list)",
                  s["digest"] == hashlib.sha512(blk["signedList"]).digest() and s["size"] == len(blk["signedList"]), lab)
        check("software and hardware lists name the same files with the same digests",
              [(x["fileName"], x["signature"]["digest"], x["signature"]["size"]) for x in lsw] ==
              [(x["fileName"], x["signature"]["digest"], x["signature"]["size"]) for x in lhw], lab)
        C["signed files (short names, in order)"][tuple(kitlib.short_name(x["fileName"]) for x in lhw)] += 1
        for x in lhw:
            fp = f.parent / x["fileName"]
            if fp.exists() and fp.is_file():
                content = fp.read_bytes()
                check("published file: SHA-512 and size match the list",
                      hashlib.sha512(content).digest() == x["signature"]["digest"] and len(content) == x["signature"]["size"],
                      f"{lab} {x['fileName']}")

        # --- certificate and hardware signatures
        if kind_hw != "certificate":
            check("hardware block carries a certificate", False, lab)
            continue
        der, form = sigtools.pem_or_der(key_hw)
        cert = sigtools.parse_certificate(der)
        C["hardware certificate (stored as, public-key algorithm, issuer CN)"][(form, cert["spki_alg"], cert["issuer_cn"])] += 1
        key = sigtools.UrnaKey(cert)
        if roots:
            iss = roots.get(cert["issuer_cn"])
            check("urna certificate verifies against the given CA certificate",
                  iss is not None and sigtools.verify_cert_signature(cert, iss), f"{lab} issuer {cert['issuer_cn']}")
        for x in lhw:
            ok = key.verify_value(x["signature"]["digest"], x["signature"]["value"])
            check(f"hardware file signature over SHA-512(file digest) [{key.name}]", ok, f"{lab} {x['fileName']}")
        sv = shw["value"]
        check(f"hardware list signature over SHA-512(list digest) [{key.name}]", key.verify_value(sv["digest"], sv["value"]), lab)

        if bu_spec is not None:
            for bf in sorted(f.parent.glob("*-bu.dat")) + sorted(f.parent.glob("*-busa.dat")):
                env = bu_spec.decode("EntidadeEnvelopeGenerico", bf.read_bytes())
                if "seguranca" in env:
                    continue
                bu = bu_spec.decode("EntidadeBoletimUrna", env["conteudo"])
                nint = bu["urna"]["correspondenciaResultado"]["carga"]["numeroInternoUrna"]
                tipo = TIPO_ARQUIVO.get(bu["urna"]["tipoArquivo"], bu["urna"]["tipoArquivo"])
                # informative: a BU recovered from the RED on another urna is signed by that urna
                C["hardware signer serial == BU numeroInternoUrna (by BU tipoArquivo)"][(tipo, shw["signer"]["serial"] == nint)] += 1
                for r in bu["resultadosVotacaoPorEleicao"]:
                    h, s = r["ultimoHashVotosVotavel"], r["assinaturaUltimoHashVotosVotavel"]
                    check(f"BU election signature over SHA-512(ultimoHashVotosVotavel) [{key.name}]",
                          key.verify_value(h, s), f"{lab} eleicao {r['idEleicao']}")
                    C["control: BU signature over the RAW last hash verifies"][key.verify_prehash(h, s)] += 1
                    C["BU signature length (bytes)"][len(s)] += 1
        if a.verbose:
            print(f"{lab}: UE{2000 + model} hw={hwa[0]}/{hwa[1]} sw={swa[0]}/{swa[1]} "
                  f"sw-signer={ssw['signer']['name']} key-set={key_sw if kind_sw == 'keySetLabel' else '-'} "
                  f"cert={form} {cert['issuer_cn']} files={len(lhw)}")

    print("== Results (check: pass/fail)")
    for k, c in C.items():
        if set(c) <= {True, False} and not k.startswith("control"):
            print(f"  {k:<92} pass={c[True]:>5} fail={c[False]:>4}")
    print("== Inventory")
    for k, c in C.items():
        if not set(c) <= {True, False} or k.startswith("control"):
            print(f"  {k}:")
            for kk, vv in sorted(c.items(), key=lambda kv: repr(kv[0])):
                print(f"      {kk!s:<100} {vv}")
    print("  software signature (CEPESC, algorithm 3): not verified (no public implementation)")
    if not roots:
        print("  certificate chain: not checked (pass --roots with the CA certificates to check it)")
    print(f"failures: {len(fails)}")
    for x in fails[:50]:
        print("FAIL", x)
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
