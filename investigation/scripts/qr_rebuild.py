#!/usr/bin/env python3
"""Rebuild the QR-code texts a 2026 urna would print on its BU, from its bu.dat, with the generator
rules of the reconstructed code, and measure them.

Rules (src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp, GeraQRCodes;
ccabecalhoqrcodebuilder.u34/u36.cpp; vota/eleitor/fimvotacao/cgerabu.cpp, MontaQRCodesCertificado):
  * header "ORIG ORLC PROC DTPL PLEI TURN FASE UNFE MUNI ZONA SECA [AGRE] IDUE IDCA HIQT HICA.. VERS
    LOCA APTO APTS APTT COMP FALT HBBM HBBG HBSB DTAB HRAB DTFC HRFC [DTEM HREM for RED]", then per
    election "IDEL", per office in print order "CARG TIPO VERC" + votes + totals
    (proportional: PART/candidates/LEGP/TOTP per party, then APTA APTS APTT NOMI LEGC BRAN NULO TOTC;
    majoritarian: candidates, then APTA APTS APTT NOMI BRAN NULO TOTC; office without candidates:
    APTA APTS APTT CSEC)
  * split into parts of at most 1100 - 277 = 823 characters, cut at the last space
  * each QR: "QRBU:i:n VRQR:6.0 <part> HASH:<SHA-512 hex of the chained parts>", and on the last one
    " ASSI:<hex of the urna's signature over the last hash>"
  * certificate QR codes: the urna certificate as hex, split in ceil(2*bytes/1082) parts:
    "QRCE:i:n IDUE:<urna> MDUE:<model> CERT:<hex part>"

CAVEAT - placeholders. Some values are not in bu.dat and are replaced by fixed-width placeholders,
so lengths are exact only if the real values have the same width: PROC (process id, default 9999),
VERC (candidate-package version, default 12 digits), DTPL (election date, default 20261004),
TURN (default 1). AGRE (aggregated sections) is not in bu.dat and is left out. The ASSI length is taken
from the BU's own election signature (same key and algorithm: 132 bytes Ed521 or 137-139 bytes DER
ECDSA). The certificate is taken from the *-vota.vsc next to the BU as stored (PEM or DER). SA ballot
BUs are not produced by VOTA and are skipped. No printed 2026 BU was available to compare with.

Usage:
  python3 -I qr_rebuild.py SECTIONS_DIR_OR_BU_FILES... [--repo REPO] [--print-texts]
                           [--proc 9999] [--verc 202609011200] [--dtpl 20261004] [--turno 1]
Expected on the 110-section 2026 sample: (BU QR, certificate QR) counts {(1,2):4, (2,2):9, (3,2):70,
(4,2):26, (5,2):1}; the last QR exceeds the manual's 1,100 characters in 13/110 BUs (1,102-1,239).
"""
import argparse
import collections
import hashlib
import math
import re
import sys
from pathlib import Path

sys.dont_write_bytecode = True          # keep the scripts directory clean
sys.path.insert(0, str(Path(__file__).resolve().parent))
import kitlib  # noqa: E402

# Alphanumeric-mode capacity at error-correction level L, QR versions 1..40 (ISO/IEC 18004).
ALNUM_L = [25, 47, 77, 114, 154, 195, 224, 279, 335, 395, 468, 535, 619, 667, 758, 854, 938, 1046, 1153, 1249,
           1352, 1460, 1588, 1704, 1853, 1990, 2132, 2223, 2369, 2520, 2677, 2840, 3009, 3183, 3351, 3537, 3729,
           3927, 4087, 4296]
FASE = {1: "S", 2: "O", 3: "T"}


def qr_version(n):
    return next((v for v, cap in enumerate(ALNUM_L, 1) if n <= cap), None)


def split_parts(texto, limite):
    partes, pos = [], 0
    while pos != len(texto):
        parte = texto[pos:pos + limite]
        tam = len(parte)
        if tam >= limite and parte[-1] != " ":
            tam = parte.rfind(" ")
            if tam <= 0:
                raise ValueError("no space to cut at (the code would loop or throw here)")
            parte = texto[pos:pos + tam]
        partes.append(parte.strip(" "))
        pos += tam
    return partes


def dt(s):
    return s[:8], s[9:15]


def header(b, uf, a):
    urna = b["urna"]
    orig = {1: "VOTA", 2: "RED"}[urna["tipoArquivo"]]
    carga = urna["correspondenciaResultado"]["carga"]
    ids = b["identificacaoSecao"]
    hist = b.get("historicoCodigosCarga") or [carga["codigoCarga"]]
    els = b["resultadosVotacaoPorEleicao"]
    top = max(els, key=lambda r: r["qtdEleitoresAptos"])
    apto, apts, aptt = top["qtdEleitoresAptos"], top["qtdEleitoresAptosSecao"], top["qtdEleitoresAptosTTE"]
    comp = b["qtdEleitoresCompareceram"]
    det = b.get("detalhamentoComparecimento", {})
    ab, fc = b["dadosSecaoSA"][1]["dataHoraAbertura"], b["dadosSecaoSA"][1]["dataHoraEncerramento"]
    vers = re.match(r"[0-9.]+", urna["versaoVotacao"]).group(0)
    h = (f"ORIG:{orig} ORLC:LEG PROC:{a.proc} DTPL:{a.dtpl} PLEI:{b['cabecalho']['idEleitoral'][1]} "
         f"TURN:{a.turno} FASE:{FASE.get(b['fase'], 'O')} UNFE:{uf} "
         f"MUNI:{ids['municipioZona']['municipio']} ZONA:{ids['municipioZona']['zona']} SECA:{ids['secao']} "
         f"IDUE:{carga['numeroInternoUrna']} IDCA:{carga['codigoCarga'][:24]} HIQT:{len(hist)} "
         + "".join(f"HICA:{i}:{c[:24]} " for i, c in enumerate(hist, 1)) + f"VERS:{vers} "
         f"LOCA:{ids['local']} APTO:{apto} APTS:{apts} APTT:{aptt} COMP:{comp} FALT:{apto - comp} "
         f"HBBM:{det.get('qtdEleitoresHabilitadosPorBiometria', 0)} "
         f"HBBG:{det.get('qtdEleitoresHabilitadosPorBiografia', 0)} "
         f"HBSB:{det.get('qtdEleitoresCompareceramSemBiometria', 0)} "
         f"DTAB:{dt(ab)[0]} HRAB:{dt(ab)[1]} DTFC:{dt(fc)[0]} HRFC:{dt(fc)[1]} ")
    if orig == "RED":
        h += f"DTEM:{dt(b['dataHoraEmissao'])[0]} HREM:{dt(b['dataHoraEmissao'])[1]} "
    return h


def body(b, a):
    rows = []
    for r in b["resultadosVotacaoPorEleicao"]:
        for rv in r["resultadosVotacao"]:
            for tc in rv["totaisVotosCargo"]:
                rows.append((tc["ordemImpressao"], r, rv, tc))
    rows.sort(key=lambda x: x[0])
    corpo, last = "", None
    for _, r, rv, tc in rows:
        if r["idEleicao"] != last:
            corpo += f"IDEL:{r['idEleicao']} "
            last = r["idEleicao"]
        tipo = {1: 0, 2: 1, 3: 2}[rv["tipoCargo"]]        # majoritario 0, proporcional 1, consulta 2
        corpo += f"CARG:{tc['codigoCargo'][1]} TIPO:{tipo} VERC:{a.verc} "
        vv = tc["votosVotaveis"]
        aptos = f"APTA:{r['qtdEleitoresAptos']} APTS:{r['qtdEleitoresAptosSecao']} APTT:{r['qtdEleitoresAptosTTE']} "
        if any(v["tipoVoto"] == 5 for v in vv):           # cargoSemCandidato
            corpo += aptos + f"CSEC:{rv['qtdComparecimento']} "
            continue
        nom = [(v["identificacaoVotavel"]["codigo"], v["identificacaoVotavel"]["partido"], v["quantidadeVotos"])
               for v in vv if v["tipoVoto"] == 1]
        leg = {v["identificacaoVotavel"]["partido"]: v["quantidadeVotos"] for v in vv if v["tipoVoto"] == 4}
        bran = sum(v["quantidadeVotos"] for v in vv if v["tipoVoto"] == 2)
        nulo = sum(v["quantidadeVotos"] for v in vv if v["tipoVoto"] == 3)
        nomi, legc = sum(x[2] for x in nom), sum(leg.values())
        if tipo == 1:
            for pt in sorted({x[1] for x in nom} | set(leg)):
                cands = sorted((x for x in nom if x[1] == pt), key=lambda x: x[0])
                corpo += (f"PART:{pt} " + "".join(f"{c}:{q} " for c, _, q in cands) +
                          f"LEGP:{leg.get(pt, 0)} TOTP:{sum(q for *_, q in cands) + leg.get(pt, 0)} ")
            corpo += aptos + f"NOMI:{nomi} LEGC:{legc} BRAN:{bran} NULO:{nulo} TOTC:{nomi + legc + bran + nulo} "
        else:
            corpo += ("".join(f"{c}:{q} " for c, _, q in sorted(nom)) + aptos +
                      f"NOMI:{nomi} BRAN:{bran} NULO:{nulo} TOTC:{nomi + bran + nulo} ")
    return corpo


def qr_texts(partes, sig_hex):
    out, blocos = [], []
    for i, p in enumerate(partes):
        entrada = p if i == 0 else " ".join(blocos) + " " + p
        h = hashlib.sha512(entrada.encode("latin-1")).hexdigest().upper()
        blocos.append(f"{p} HASH:{h}")
        q = f"QRBU:{i + 1}:{len(partes)} VRQR:6.0 {blocos[-1]}"
        if i == len(partes) - 1:
            q += f" ASSI:{sig_hex}"
        out.append(q)
    return out


def cert_qrs(cert, idue, model):
    texto = cert.hex().upper()
    n = math.ceil(2 * len(cert) / 1082)
    tam = math.ceil(len(texto) / n)
    return [f"QRCE:{i + 1}:{n} IDUE:{idue} MDUE:{2000 + model} CERT:{texto[i * tam:(i + 1) * tam]}" for i in range(n)]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("inputs", nargs="+", type=Path)
    kitlib.add_repo_arg(ap)
    ap.add_argument("--proc", default="9999", help="PROC placeholder (process id; not in bu.dat)")
    ap.add_argument("--verc", default="202609011200", help="VERC placeholder (package version; not in bu.dat)")
    ap.add_argument("--dtpl", default="20261004", help="DTPL election date YYYYMMDD (not in bu.dat)")
    ap.add_argument("--turno", default="1")
    ap.add_argument("--max", type=int, default=1100, help="tamanhoMaximo passed by CGeraBU (default 1100)")
    ap.add_argument("--reserve", type=int, default=277, help="characters reserved by the code (default 277)")
    ap.add_argument("--print-texts", action="store_true", help="print the rebuilt QR texts")
    a = ap.parse_args()
    spec = kitlib.compile_urna_spec(a.repo, numeric_enums=True)
    vsc = kitlib.compile_vsc_spec(numeric_enums=True)

    dist, over, lens_all, lens_last = collections.Counter(), [], [], []
    skipped = collections.Counter()
    for p in kitlib.find_files(a.inputs, "-bu.dat"):
        lab = kitlib.section_label(p)
        e = spec.decode("EntidadeEnvelopeGenerico", p.read_bytes())
        if "seguranca" in e:
            skipped["encrypted"] += 1
            continue
        b = spec.decode("EntidadeBoletimUrna", e["conteudo"])
        if b["urna"]["tipoArquivo"] not in (1, 2) or b["dadosSecaoSA"][0] != "dadosSecao":
            skipped["SA BU (not produced by VOTA)"] += 1
            continue
        uf = p.name[6:8].upper() if kitlib.URNA_FILE_PREFIX.match(p.name) else "XX"
        texto = header(b, uf, a) + body(b, a)
        partes = split_parts(texto, a.max - a.reserve)
        els = b["resultadosVotacaoPorEleicao"]
        sig_len = max(len(r["assinaturaUltimoHashVotosVotavel"]) for r in els)
        qrs = qr_texts(partes, "00" * sig_len)          # placeholder signature of the real length
        qlens = [len(q) for q in qrs]
        vf = sorted(p.parent.glob("*-vota.vsc"))
        cqs = []
        if vf:
            v = vsc.decode("VscFile", vf[0].read_bytes())
            kind, cert = v["hardwareBlock"]["keyInfo"]
            if kind == "certificate":
                cqs = cert_qrs(cert, b["urna"]["correspondenciaResultado"]["carga"]["numeroInternoUrna"],
                               v["hardwareOrigin"]["urnaModel"])
        dist[(len(qrs), len(cqs))] += 1
        lens_all += qlens
        lens_last.append(qlens[-1])
        if qlens[-1] > a.max:
            over.append((lab, len(qrs), len(partes[-1]), qlens[-1], sig_len))
        print(f"{lab}: content {len(texto)} chars -> {len(qrs)} BU QR (parts {[len(x) for x in partes]}; "
              f"QR lengths {qlens}; signature {sig_len} B) + {len(cqs)} certificate QR ({[len(c) for c in cqs]} chars)")
        if a.print_texts:
            for q in qrs + cqs:
                print("   ", q)
    n = len(lens_last)
    print()
    print(f"BUs rebuilt: {n}; skipped: {dict(skipped)}")
    print("(BU QR count, certificate QR count) distribution:", dict(sorted(dist.items())))
    if n:
        print(f"last QR longer than {a.max} characters: {len(over)} of {n}; longest QR {max(lens_all)}; "
              f"longest last QR {max(lens_last)}")
        vers = collections.Counter(qr_version(x) for x in lens_all)
        print("QR versions needed (level L, alphanumeric):", dict(sorted(vers.items())))
    for o in over:
        print(f"  over {a.max}: {o[0]}  QRs={o[1]} last part={o[2]} chars, last QR={o[3]} chars, signature={o[4]} B")
    print("CAVEAT: PROC/VERC/DTPL/TURN are placeholders and AGRE is omitted; ASSI is a placeholder of the "
          "real signature length; lengths are predictions from the code, not read from a printed BU.")


if __name__ == "__main__":
    main()
