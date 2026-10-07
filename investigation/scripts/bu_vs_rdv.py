#!/usr/bin/env python3
"""Recompute the BU from the RDV and the BU tuple hash chain with the rules of the reconstructed code.

For every section (a directory holding *-bu.dat and *-rdv.dat; SA ballot-paper BUs have no RDV and
are only chain-checked) the script checks, per election and per office:

  hash chain   (src/uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp, formats @8483,
                @1120, @1126): the chain starts with
                    SHA-512("{pleito:05}|{eleicao:05}|{municipio:05}|{zona:04}|{secao:04}|{carga:24}")
                and each BU vote line i (ordemGeracaoHash = 1..n per office) is
                    SHA-512("{HEX(previous)}|{i}|{cargo}|{tipoVoto}|{qtd}[|{numero}|{partido}]")
                (HEX = upper-case hex of the previous 64-byte digest); every stored tuple hash and
                ultimoHashVotosVotavel must match
  BU lines     the lines of each office recomputed from the RDV votes with the type table @546352
                (ConverteTipoVoto: RDV legenda->BU legenda (first 2 digits = party), nominal->nominal,
                branco/brancoAposSuspensao->branco, nulo/nuloAposSuspensao/nuloPorRepeticao->nulo,
                nuloCargoSemCandidato/nuloAposSuspensaoCargoSemCandidato->cargoSemCandidato);
                no zero line is written; line order nominal(asc), branco, nulo, legenda(asc)
  counts       #RDV votes(office) = qtdComparecimento x qtdEscolhas (no empty slot);
                sum of BU quantities = #RDV votes; aptos = aptosSecao + aptosTTE;
                RDV votes of an office sorted by (tipoVoto, digitacao) (crdvposicionadorvota.cpp)

Usage:
  python3 -I bu_vs_rdv.py SECTIONS_DIR_OR_FILES... [--repo REPO] [--verbose]
Expected on the 110-section 2026 sample: 0 failures; 216 elections, 13,088 BU lines (tuples),
534 office blocks recomputed exactly from the RDV.
"""
import argparse
import collections
import hashlib
import sys
from pathlib import Path

sys.dont_write_bytecode = True          # keep the scripts directory clean
sys.path.insert(0, str(Path(__file__).resolve().parent))
import kitlib  # noqa: E402

CARGO_NUM = {"presidente": 1, "vicePresidente": 2, "governador": 3, "viceGovernador": 4, "senador": 5,
             "deputadoFederal": 6, "deputadoEstadual": 7, "deputadoDistrital": 8, "primeiroSuplenteSenador": 9,
             "segundoSuplenteSenador": 10, "prefeito": 11, "vicePrefeito": 12, "vereador": 13}
BU_TV = {"nominal": 1, "branco": 2, "nulo": 3, "legenda": 4, "cargoSemCandidato": 5}
RDV_TV = {"legenda": 1, "nominal": 2, "branco": 3, "nulo": 4, "brancoAposSuspensao": 5, "nuloAposSuspensao": 6,
          "nuloPorRepeticao": 7, "nuloCargoSemCandidato": 8, "nuloAposSuspensaoCargoSemCandidato": 9}
RDV2BU = {1: 4, 2: 1, 3: 2, 4: 3, 5: 2, 6: 3, 7: 3, 8: 5, 9: 5}   # table @546352 (ConverteTipoVoto)


def cargo_code(cc):
    return CARGO_NUM[cc[1]] if cc[0] == "cargoConstitucional" else cc[1]


class Checks:
    def __init__(self):
        self.r = collections.defaultdict(collections.Counter)
        self.fails = collections.defaultdict(list)

    def __call__(self, name, ok, detail=None):
        self.r[name][bool(ok)] += 1
        if not ok and len(self.fails[name]) < 10:
            self.fails[name].append(detail)


def chain_check(chk, bu, tag):
    ids = bu["identificacaoSecao"]
    mun, zona, secao = ids["municipioZona"]["municipio"], ids["municipioZona"]["zona"], ids["secao"]
    pleito = bu["cabecalho"]["idEleitoral"][1]
    carga = bu["urna"]["correspondenciaResultado"]["carga"]["codigoCarga"]
    n_lines = 0
    for r in bu["resultadosVotacaoPorEleicao"]:
        el = r["idEleicao"]
        prev = hashlib.sha512(f"{pleito:05}|{el:05}|{mun:05}|{zona:04}|{secao:04}|{carga:24}".encode()).digest()
        ok = True
        for rv in r["resultadosVotacao"]:
            for t in rv["totaisVotosCargo"]:
                cg = cargo_code(t["codigoCargo"])
                for i, vv in enumerate(t["votosVotaveis"], 1):
                    n_lines += 1
                    chk("ordemGeracaoHash == 1..n per office", vv["ordemGeracaoHash"] == i, (tag, el, cg, i))
                    chk("quantidadeVotos > 0 (zero lines never written)", vv["quantidadeVotos"] > 0, (tag, el, cg))
                    txt = f"{prev.hex().upper()}|{vv['ordemGeracaoHash']}|{cg}|{BU_TV[vv['tipoVoto']]}|{vv['quantidadeVotos']}"
                    iv = vv.get("identificacaoVotavel")
                    if iv is not None:
                        txt += f"|{iv['codigo']}|{iv['partido']}"
                    prev = hashlib.sha512(txt.encode()).digest()
                    ok &= prev == vv["hash"]
        chk("hash chain: every tuple hash recomputes", ok, (tag, el))
        chk("hash chain: ultimoHashVotosVotavel == last tuple hash", prev == r["ultimoHashVotosVotavel"], (tag, el))
        chk("aptos == aptosSecao + aptosTTE", r["qtdEleitoresAptos"] == r["qtdEleitoresAptosSecao"] + r["qtdEleitoresAptosTTE"], (tag, el))
    return n_lines


def rdv_check(chk, bu, rdv, tag):
    rdv_by_el = {e["idEleicao"]: e for e in rdv["eleicoes"][1]}
    chk("RDV elections == BU elections (ids, same order)",
        [e["idEleicao"] for e in rdv["eleicoes"][1]] == [r["idEleicao"] for r in bu["resultadosVotacaoPorEleicao"]], tag)
    n_blocks = 0
    for r in bu["resultadosVotacaoPorEleicao"]:
        el = r["idEleicao"]
        rel = rdv_by_el.get(el)
        cargos = {cargo_code(vc["idCargo"]): vc for vc in rel["votosCargos"]} if rel else {}
        chk("RDV offices == BU offices", set(cargos) == {cargo_code(t["codigoCargo"]) for rv in r["resultadosVotacao"]
                                                        for t in rv["totaisVotosCargo"]}, (tag, el))
        for rv in r["resultadosVotacao"]:
            for t in rv["totaisVotosCargo"]:
                cg = cargo_code(t["codigoCargo"])
                vc = cargos.get(cg)
                if vc is None:
                    continue
                n_blocks += 1
                votos, q = vc["votos"], vc["quantidadeEscolhas"]
                chk("#RDV votes(office) == qtdComparecimento x qtdEscolhas (no empty slot)",
                    len(votos) == rv["qtdComparecimento"] * q, (tag, el, cg))
                chk("sum(BU quantities of office) == #RDV votes(office)",
                    sum(v["quantidadeVotos"] for v in t["votosVotaveis"]) == len(votos), (tag, el, cg))
                keys = [(RDV_TV[v["tipoVoto"]], v.get("digitacao", "")) for v in votos]
                chk("RDV votes of an office sorted by (tipoVoto, digitacao)", keys == sorted(keys), (tag, el, cg))
                exp = collections.Counter()
                for v in votos:
                    bt = RDV2BU[RDV_TV[v["tipoVoto"]]]
                    if bt == 1:
                        exp[(1, int(v["digitacao"]))] += 1
                    elif bt == 4:
                        exp[(4, int(v["digitacao"][:2]))] += 1
                    else:
                        exp[(bt, None)] += 1
                got = collections.Counter()
                for vv in t["votosVotaveis"]:
                    iv = vv.get("identificacaoVotavel")
                    got[(BU_TV[vv["tipoVoto"]], iv["codigo"] if iv else None)] += vv["quantidadeVotos"]
                chk("BU lines of office == lines recomputed from the RDV (table @546352)", exp == got,
                    (tag, el, cg, sorted((exp - got).items())[:3], sorted((got - exp).items())[:3]))
                seq = [(BU_TV[vv["tipoVoto"]], (vv.get("identificacaoVotavel") or {}).get("codigo", 0))
                       for vv in t["votosVotaveis"]]
                rank = {1: 0, 2: 1, 3: 2, 4: 3, 5: 4}
                chk("BU line order: nominal(asc), branco, nulo, legenda(asc)",
                    seq == sorted(seq, key=lambda x: (rank[x[0]], x[1])), (tag, el, cg))
    return n_blocks


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("inputs", nargs="+", type=Path)
    kitlib.add_repo_arg(ap)
    ap.add_argument("--verbose", action="store_true")
    a = ap.parse_args()
    spec = kitlib.compile_urna_spec(a.repo)
    chk = Checks()
    tot = collections.Counter()
    for bf in kitlib.find_files(a.inputs, ("-bu.dat", "-busa.dat")):
        tag = kitlib.section_label(bf)
        env = spec.decode("EntidadeEnvelopeGenerico", bf.read_bytes())
        if "seguranca" in env:
            tot["encrypted BU (skipped)"] += 1
            continue
        bu = spec.decode("EntidadeBoletimUrna", env["conteudo"])
        tot["BU files"] += 1
        tot["elections"] += len(bu["resultadosVotacaoPorEleicao"])
        n_lines = chain_check(chk, bu, tag)
        tot["BU lines (tuples)"] += n_lines
        rdvs = sorted(bf.parent.glob("*-rdv.dat"))
        n_blocks = 0
        if rdvs and bf.name.endswith("-bu.dat"):
            rdv = spec.decode("EntidadeResultadoRDV", rdvs[0].read_bytes())["rdv"]
            n_blocks = rdv_check(chk, bu, rdv, tag)
            tot["office blocks compared with the RDV"] += n_blocks
        else:
            tot["BU without RDV (chain only)"] += 1
        if a.verbose:
            print(f"{tag}: {len(bu['resultadosVotacaoPorEleicao'])} elections, {n_lines} lines, {n_blocks} office blocks")
    print("== Totals")
    for k, v in tot.items():
        print(f"  {k}: {v}")
    print("== Checks")
    w = max(len(k) for k in chk.r) if chk.r else 10
    nfail = 0
    for k, v in chk.r.items():
        nfail += v[False]
        print(f"  {k:<{w}}  pass={v[True]:>6}  fail={v[False]:>4}")
    for k, v in chk.fails.items():
        print("FAIL", k, v[:3])
    print(f"failures: {nfail}")
    return 1 if nfail else 0


if __name__ == "__main__":
    sys.exit(main())
