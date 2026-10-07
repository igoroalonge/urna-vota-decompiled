#!/usr/bin/env python3
"""Real urna logs -> message templates per program -> where VOTA's templates are in src/.

Input: the *-log.jez files (ZIP) of the sections. Only the member `logd.dat` is read, in memory;
nothing is extracted to disk. logd.dat is Latin-1, one event per line, 6 TAB-separated fields:
date-time, level, urna id, program, message, 16-hex authenticator.

Templates
  fine    digits runs -> '#', a few identifiers (media id, printer serial, generator machine) -> <...>
  coarse  also collapses package/file names, media serials, sizes, keyboard keys, fingerprint scores
Mapping (VOTA, SCUE, ATUE; GAP/LOGD/INITJE without composition): every double-quoted literal of
src/ (code and comments) is indexed; a template is matched as
  exact        the message is a literal
  format       the message matches a std::format literal ("{}" placeholders) with enough fixed text
  format(weak) it matches one, but with little fixed text
  comment-placeholder  matches a literal from a comment written with <...> placeholders
  composed     the message is a concatenation of up to 4 literals
  NOT FOUND    none of the above

PRIVACY: real logs carry poll workers' títulos eleitorais in clear text (and SCUE/GAP lines carry
the media-generation computer and Windows user). This script never prints raw lines: it prints
templates only, masks every 10-12 digit run, and replaces the computer/user values by <masked>.
Do not commit raw logs or extracted logd.dat files.

Usage:
  python3 -I log_templates.py SECTIONS_DIR_OR_JEZ_FILES... [--repo REPO] [--tsv OUT.tsv] [--all]
    --tsv   write the full template table (program, lines, sections, levels, match, template, source)
    --all   also print every VOTA template with its source location
Expected on the 110-section 2026 sample: VOTA 566,963 lines, 191 coarse templates: 86 exact,
92 format, 5 composed, 8 NOT FOUND (Início/Fim de impressão, four printer messages,
'Votação suspensa', one microterminal LCD error).
"""
import argparse
import collections
import io
import os
import re
import sys
import zipfile
from pathlib import Path

sys.dont_write_bytecode = True          # keep the scripts directory clean
sys.path.insert(0, str(Path(__file__).resolve().parent))
import kitlib  # noqa: E402

MAX_LOG_BYTES = 512 * 1024 * 1024
PROGRAMS = ["VOTA", "SCUE", "ATUE", "GAP", "LOGD", "INITJE"]


# ------------------------------------------------------------------------------ reading the logs

def read_logd(jez_path):
    with zipfile.ZipFile(jez_path) as z:
        info = z.getinfo("logd.dat")
        if info.file_size > MAX_LOG_BYTES:
            raise ValueError("logd.dat too large")
        data = z.read(info)
    rows = []
    for raw in data.split(b"\n"):
        if not raw:
            continue
        p = raw.rstrip(b"\r").decode("latin-1").split("\t")
        if len(p) != 6:
            raise ValueError(f"{jez_path.name}: line with {len(p)} fields")
        rows.append(p)       # dt, level, urna, program, message, code
    return rows


# ------------------------------------------------------------------------------ templates

def norm(msg):
    t = re.sub(r"(gerada pelo computador \(nome - tpm - instalação\): ).*", r"\1<NOME> - <TPM> - <INST>", msg)
    t = re.sub(r"(Identificador da mídia de carga: )[0-9A-F]{8}$", r"\1<HEX>", t)
    t = re.sub(r"(Detectada impressora com serial )\S+", r"\1<SERIAL>", t)
    return re.sub(r"-?\d+(?:[.,]\d+)*", "#", t)


def coarse(msg):
    t = msg
    t = re.sub(r"\[(Pacote|Arquivo): [^\]]*\]", r"[\1: <p>]", t)
    t = re.sub(r"(Verificação de assinatura - Etapa \[\d+\] - )\[[^\]]*\]", r"\1[<p>]", t)
    t = re.sub(r"(Serial da MV(?: original da urna que está registrada na MI)?: )\S+", r"\1<HEX>", t)
    t = re.sub(r"(O arquivo temporário )\S+( foi encontrado)", r"\1<f>\2", t)
    t = re.sub(r"(Tecla (?:pressionada|esperada):? )(\d|CORRIGE|CONFIRMA|BRANCO)", r"\1<tecla>", t)
    t = re.sub(r"(Esperado )(\d|CORRIGE|CONFIRMA|BRANCO)(, pressionado )(\d|CORRIGE|CONFIRMA|BRANCO)",
               r"\1<tecla>\3<tecla>", t)
    t = re.sub(r"(Na tecla )\S+:", r"\1<tecla>:", t)
    t = re.sub(r"\[(\d+(?:\.\d+)?) (MB|GB|KB)\]", r"[<tam>]", t)
    t = re.sub(r": (\d+(?:\.\d+)?) (MB|GB|KB)$", r": <tam>", t)
    t = re.sub(r"\[score \d+\]", "[score #]", t)
    return norm(t)


def mask(s):
    s = kitlib.mask_ids(s)
    s = re.sub(r"(gerada pelo computador \(nome - tpm - instalação\): )(.*)", r"\1<masked>", s)
    s = re.sub(r"(gerada pelo usuário: )(.*)", r"\1<masked>", s)
    return s


# ------------------------------------------------------------------------------ source literals

LIT = re.compile(r'"((?:[^"\\\n]|\\.)*)"')


def _unescape(s):
    return re.sub(r"\\(.)", lambda m: {"n": "\n", "t": "\t", "\\": "\\", '"': '"'}.get(m.group(1), m.group(1)), s)


def source_literals(src):
    """(relpath, line, text, in_comment) of every "..." literal in src/ (adjacent literals merged too)."""
    out = []
    for root, dirs, files in os.walk(src):
        dirs.sort()
        for fn in sorted(files):
            if not fn.endswith((".cpp", ".h", ".hpp")):
                continue
            p = os.path.join(root, fn)
            rel = os.path.relpath(p, os.path.dirname(src))
            with open(p, encoding="utf-8", errors="replace") as fh:
                for ln, line in enumerate(fh, 1):
                    prev = prev_end = None
                    for m in LIT.finditer(line):
                        before = line[:m.start()]
                        ci, inq, i = -1, False, 0
                        while i < len(before):
                            c = before[i]
                            if c == "\\" and inq:
                                i += 2
                                continue
                            if c == '"':
                                inq = not inq
                            elif not inq and before.startswith("//", i):
                                ci = i
                                break
                            i += 1
                        in_comment = ci >= 0 or line.lstrip().startswith(("*", "/*"))
                        txt = _unescape(m.group(1))
                        out.append((rel, ln, txt, in_comment))
                        if prev is not None and line[prev_end:m.start()].strip() == "":
                            prev = prev + txt
                            out.append((rel, ln, prev, in_comment))
                        else:
                            prev = txt
                        prev_end = m.end()
    return out


class Locator:
    PH = re.compile(r"\{[^{}]*\}")
    CPH = re.compile(r"<[^<>]{1,40}>")

    def __init__(self, literals):
        self.occ = collections.defaultdict(list)
        for f, ln, txt, com in literals:
            self.occ[txt].append((f, ln, com))
        PH, CPH = self.PH, self.CPH

        def fmt_regex(s):
            return re.compile("^" + "(.+?)".join(re.escape(p) for p in PH.split(s)) + "$", re.S)

        def cmt_regex(s):
            return re.compile("^" + "(.+?)".join(re.escape(p) for p in CPH.split(PH.sub("<x>", s))) + "$", re.S)

        fmt_lits = [(s, fmt_regex(s)) for s in self.occ if PH.search(s) and len(PH.sub("", s).strip()) >= 3]
        cmt_lits = [(s, cmt_regex(s)) for s in self.occ if CPH.search(s) and len(CPH.sub("", s).strip()) >= 3]
        self.FMT = [(s, rx, max(PH.split(s), key=len)) for s, rx in fmt_lits]
        self.CMT = [(s, rx, max(CPH.split(PH.sub("<x>", s)), key=len)) for s, rx in cmt_lits]
        self.PREF = {s: re.compile(rx.pattern[:-1], re.S) for s, rx in fmt_lits}
        self.plain_by4 = collections.defaultdict(list)
        for s in self.occ:
            if len(s) >= 4 and not PH.search(s):
                self.plain_by4[s[:4]].append(s)
        self.fmt_by4 = collections.defaultdict(list)
        self.fmt_lead = []
        for s, rx, lp in self.FMT:
            first = PH.split(s)[0]
            if len(first) >= 4:
                self.fmt_by4[first[:4]].append(s)
            elif first == "":
                self.fmt_lead.append((s, lp))
        self._memo = {}

    def fixed_len(self, s):
        return len(self.CPH.sub("", self.PH.sub("", s)))

    def where(self, s):
        o = self.occ[s]
        code = [x for x in o if not x[2]]
        return code or o

    def compose(self, msg, depth=0):
        if msg == "":
            return []
        if depth >= 4:
            return None
        key = (msg, depth)
        if key in self._memo:
            return self._memo[key]
        cands = []
        for s in self.plain_by4.get(msg[:4], []):
            if msg.startswith(s):
                cands.append((s, len(s)))
        for s in self.fmt_by4.get(msg[:4], []):
            m = self.PREF[s].match(msg)
            if m:
                cands.append((s, m.end()))
        for s, lp in self.fmt_lead:
            if lp and lp in msg:
                m = self.PREF[s].match(msg)
                if m and m.end() > len(lp):
                    cands.append((s, m.end()))
        cands.sort(key=lambda x: -x[1])
        res = None
        for s, n in cands[:6]:
            rest = self.compose(msg[n:], depth + 1)
            if rest is not None:
                res = [s] + rest
                break
        self._memo[key] = res
        return res

    def locate(self, msg, allow_compose=True):
        if msg in self.occ:
            return "exact", [msg]
        best = None
        for s, rx, lp in self.FMT:
            if lp in msg and rx.match(msg):
                if best is None or self.fixed_len(s) > self.fixed_len(best):
                    best = s
        if best and self.fixed_len(best) >= 0.3 * len(msg) and len(max(self.PH.split(best), key=len).strip()) >= 6:
            return "format", [best]
        if best:
            return "format(weak)", [best]
        for s, rx, lp in self.CMT:
            if len(lp.strip()) >= 10 and self.fixed_len(s) >= 0.4 * len(msg) and lp in msg and rx.match(msg):
                if best is None or self.fixed_len(s) > self.fixed_len(best):
                    best = s
        if best:
            return "comment-placeholder", [best]
        pcs = self.compose(msg) if allow_compose else None
        if pcs and len(pcs) > 1:
            return "composed", pcs
        return "NOT FOUND", []


# ------------------------------------------------------------------------------ main

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("inputs", nargs="+", type=Path)
    kitlib.add_repo_arg(ap)
    ap.add_argument("--tsv", type=Path, help="write the full (masked) template table here")
    ap.add_argument("--all", action="store_true", help="print every VOTA template and its source")
    a = ap.parse_args()
    src = Path(a.repo) / "src"
    if not src.is_dir():
        sys.exit(f"source directory not found: {src} (use --repo)")

    lines_per_prog = collections.Counter()
    secs_per_prog = collections.defaultdict(set)
    tmpl = collections.defaultdict(collections.Counter)
    tmpl_secs = collections.defaultdict(set)
    reps = collections.defaultdict(list)
    levels = collections.defaultdict(collections.Counter)
    fine = collections.defaultdict(set)
    n_logs = 0
    for jez in kitlib.find_files(a.inputs, "-log.jez"):
        sec = kitlib.section_label(jez)
        try:
            rows = read_logd(jez)
        except (KeyError, zipfile.BadZipFile, ValueError) as e:
            print(f"skip {sec}: {e}", file=sys.stderr)
            continue
        n_logs += 1
        for _dt, lvl, _urna, prog, msg, _code in rows:
            lines_per_prog[prog] += 1
            secs_per_prog[prog].add(sec)
            t = coarse(msg)
            fine[prog].add(norm(msg))
            tmpl[prog][t] += 1
            tmpl_secs[(prog, t)].add(sec)
            levels[(prog, t)][lvl] += 1
            if msg not in reps[(prog, t)] and len(reps[(prog, t)]) < 6:
                reps[(prog, t)].append(msg)

    loc = Locator(source_literals(str(src)))
    table = []
    progs = [p for p in PROGRAMS if p in tmpl] + sorted(p for p in tmpl if p not in PROGRAMS)
    for prog in progs:
        for t, n in tmpl[prog].most_common():
            kinds = collections.Counter()
            where = []
            for m in reps[(prog, t)][:3]:
                k, pcs = loc.locate(m, prog in ("VOTA", "SCUE", "ATUE"))
                kinds[k] += 1
                if not where and pcs:
                    for p in pcs:
                        o = loc.where(p)
                        where.append("%s  @ %s" % (p.replace("\n", "\\n"), "; ".join(
                            "%s:%d%s" % (f, ln, " (comment)" if c else "") for f, ln, c in o[:3])))
            table.append(dict(prog=prog, n=n, nsec=len(tmpl_secs[(prog, t)]), levels=dict(levels[(prog, t)]),
                              template=mask(t), match=kinds.most_common(1)[0][0], where=[mask(w) for w in where]))

    print(f"logs read: {n_logs}; lines: {sum(lines_per_prog.values())}")
    for prog, n in lines_per_prog.most_common():
        print(f"{prog:7s} lines={n:7d} sections={len(secs_per_prog[prog]):3d} "
              f"fine-templates={len(fine[prog]):4d} coarse-templates={len(tmpl[prog]):4d}")
    print("\nmatch kinds per program (templates ; lines):")
    for prog in progs:
        ct, cl = collections.Counter(), collections.Counter()
        for r in table:
            if r["prog"] == prog:
                ct[r["match"]] += 1
                cl[r["match"]] += r["n"]
        print(f"  {prog:7s} {dict(ct)} ; {dict(cl)}")
    print("\nVOTA/SCUE/ATUE templates NOT FOUND in src/ (program, lines, sections, template):")
    for r in table:
        if r["prog"] in ("VOTA", "SCUE", "ATUE") and r["match"] in ("NOT FOUND", "format(weak)"):
            if r["prog"] == "VOTA" or r["match"] == "NOT FOUND":
                print(f"  {r['prog']}\t{r['n']}\t{r['nsec']}\t{r['match']}\t{r['template']}")
    if a.all:
        print("\nVOTA templates and source locations:")
        for r in table:
            if r["prog"] == "VOTA":
                print(f"  [{r['match']}] {r['n']:7d}  {r['template']}")
                for w in r["where"]:
                    print(f"        {w}")
    if a.tsv:
        with open(a.tsv, "w", encoding="utf-8") as f:
            f.write("prog\tlines\tsections\tlevels\tmatch\ttemplate\tsource (literal @ file:line)\n")
            for r in table:
                lv = ",".join("%s=%d" % kv for kv in sorted(r["levels"].items()))
                f.write(f"{r['prog']}\t{r['n']}\t{r['nsec']}\t{lv}\t{r['match']}\t{r['template']}\t{' || '.join(r['where'])}\n")
        print(f"\nwrote {a.tsv}")


if __name__ == "__main__":
    main()
