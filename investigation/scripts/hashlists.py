#!/usr/bin/env python3
"""TSE hash listings ("Resumos digitais (hashes)") -> TSV, HASH GERAL check, urna inventory.

Input: the HTML pages found in TSE's 'Resumos digitais' zips (one table per system: path, SHA-512 in
Base64). Three shapes exist:
  * urna systems (path1u13/u15/u20/u22, one per urna model): "ARQUIVOS BASE" rows, then one block per
    UF ("UF = XX", rows "XX - /dsk/fi/estatico/chave/<file>") closed by "HASH GERAL UF = XX";
  * PC / other systems (pc1*, itnm1*): file rows closed by "HASH GERAL CALCULADO A PARTIR DOS HASHES ACIMA";
  * verification media (HASH_TSE/OAB/MP/CONFEA): file rows only.

What it does
  1. writes one TSV per page: list, group (BASE | UF code | -), kind (file | hash_geral), path, sha512_b64
  2. recomputes every HASH GERAL with the code's chain, CMontadorHash::CalculaHashGeral
     (src/uenux2/src/api/hash/cmontadorhash.cpp:88-100):
         geral = first digest;  geral = Base64(SHA-512(geral_text || next_digest_text)) for each next one
     over the files in listed order; for the urna lists over the base files in listed order and then
     the UF's key files in listed order (the order CGravadorHashes writes them:
     src/uenux2/src/app/comum/gravadores/cgravadorhashes.cpp)
  3. checks that the base-file order is CriaHashesDiretorio's walk (files of a directory sorted
     byte-wise, then its subdirectories, depth first) and that the key files are sorted
  4. prints the per-model inventory diff of the urna base files and the per-UF variability of the
     key files, and digests shared between the urna key files and the other lists

Usage:
  python3 -I hashlists.py HTML_ZIP_OR_DIR [...] [--tsv-out DIR] [--compare-txt DIR]
    inputs              HTML pages, directories (searched recursively for *.html) or the published
                        zips (the *.html members are read in memory, nothing is extracted)
    --compare-txt DIR   also compare every page with a plain-text copy of the same list (*.txt with the
                        same stem, as found in public mirrors) and report digest differences
Expected on the 2026 1st-round pages: 10/10 PC lists and 112/112 urna HASH GERAL values reproduced.
"""
import argparse
import base64
import collections
import hashlib
import re
import sys
import zipfile
from html.parser import HTMLParser
from pathlib import Path

B64_512 = re.compile(r"^[A-Za-z0-9+/]{86}==$")


class _TableParser(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.rows, self.headings = [], {}
        self._row = self._cell = self._head = None

    def handle_starttag(self, tag, attrs):
        if tag == "tr":
            self._row = []
        elif tag in ("td", "th") and self._row is not None:
            self._cell = []
        elif tag in ("h1", "h2"):
            self._head = (tag, [])

    def handle_endtag(self, tag):
        if tag in ("td", "th") and self._cell is not None:
            self._row.append((tag, " ".join("".join(self._cell).split())))
            self._cell = None
        elif tag == "tr" and self._row is not None:
            if self._row:
                self.rows.append(self._row)
            self._row = None
        elif tag in ("h1", "h2") and self._head and self._head[0] == tag:
            self.headings.setdefault(tag, " ".join("".join(self._head[1]).split()))
            self._head = None

    def handle_data(self, data):
        if self._cell is not None:
            self._cell.append(data)
        elif self._head is not None:
            self._head[1].append(data)


def parse_html(path, data=None):
    p = _TableParser()
    p.feed((Path(path).read_bytes() if data is None else data).decode("utf-8-sig", errors="replace"))
    lst = {"source": str(path), "title": p.headings.get("h2", ""), "kind": "files", "files": [],
           "geral": None, "geral_covers": 0, "base": [], "uf": {}, "geral_uf": {}, "rows": []}
    group = None
    for row in p.rows:
        if all(t == "th" for t, _ in row):
            continue
        cells = [c for _, c in row]
        if len(cells) == 1:
            c = cells[0]
            if c == "ARQUIVOS BASE":
                lst["kind"], group = "urna", "BASE"
            elif re.fullmatch(r"UF = [A-Z]{2}", c):
                group = c[-2:]
                lst["uf"][group] = []
            else:
                raise ValueError(f"{path}: unexpected single-cell row {c!r}")
            continue
        if len(cells) != 2:
            raise ValueError(f"{path}: row with {len(cells)} cells")
        name, dig = cells
        if not B64_512.match(dig):
            raise ValueError(f"{path}: not a Base64 SHA-512: {dig!r}")
        m = re.fullmatch(r"HASH GERAL UF = ([A-Z]{2})", name)
        if m:
            lst["geral_uf"][m.group(1)] = dig
            lst["rows"].append((m.group(1), "hash_geral", "", dig))
        elif name.startswith("HASH GERAL"):
            # "CALCULADO A PARTIR DOS HASHES ACIMA": covers the file rows above it only
            lst["geral"], lst["geral_covers"] = dig, len(lst["files"])
            lst["rows"].append(("-", "hash_geral", "", dig))
        elif lst["kind"] == "urna":
            if group == "BASE":
                lst["base"].append((name, dig))
                lst["rows"].append(("BASE", "file", name, dig))
            else:
                m = re.fullmatch(r"([A-Z]{2}) - (/\S+)", name)
                if not m or m.group(1) != group:
                    raise ValueError(f"{path}: key row {name!r} outside its UF block")
                lst["uf"][group].append((m.group(2), dig))
                lst["rows"].append((group, "file", m.group(2), dig))
        else:
            lst["files"].append((name, dig))
            lst["rows"].append(("-", "file", name, dig))
    return lst


def hash_geral(digests):
    """CMontadorHash::CalculaHashGeral applied over a sequence of Base64 digests."""
    g = ""
    for d in digests:
        g = d if not g else base64.b64encode(hashlib.sha512((g + d).encode("ascii")).digest()).decode()
    return g


def walk_order(paths):
    """CriaHashesDiretorio order: per directory, files sorted byte-wise, then subdirectories, depth first."""
    tree = {}
    for p in paths:
        parts = p.strip("/").split("/")
        node = tree
        for x in parts[:-1]:
            node = node.setdefault(x + "/", {})
        node[parts[-1]] = p
    out = []

    def rec(node):
        out.extend(node[f] for f in sorted(k for k in node if not k.endswith("/")))
        for d in sorted(k for k in node if k.endswith("/")):
            rec(node[d])
    rec(tree)
    return out


def write_tsv(lst, dest):
    """Rows in page order: list, group (BASE | UF | -), kind (file | hash_geral), path, sha512_b64."""
    with open(dest, "w", encoding="utf-8", newline="\n") as f:
        f.write("list\tgroup\tkind\tpath\tsha512_b64\n")
        for group, kind, p, d in lst["rows"]:
            f.write(f"{lst['name']}\t{group}\t{kind}\t{p}\t{d}\n")


def txt_pairs(path):
    """(path, digest) pairs of a plain-text copy (either 'path digest' or path line + digest line)."""
    lines = [l.strip() for l in Path(path).read_text(encoding="utf-8", errors="replace").splitlines()]
    out = []
    for i, l in enumerate(lines):
        m = re.fullmatch(r"(.+?) ([A-Za-z0-9+/]{86}==)", l)
        if m:
            out.append((re.sub(r"^HASH GERAL UF = ([A-Z]{2})$", r"HASH GERAL UF = \1", m.group(1)), m.group(2)))
        elif B64_512.match(l) and i > 0:
            out.append((lines[i - 1], l))
    return out


def html_pairs(lst):
    out = []
    for group, kind, p, d in lst["rows"]:
        if kind == "hash_geral":
            out.append((f"HASH GERAL UF = {group}" if group != "-" else
                        "HASH GERAL CALCULADO A PARTIR DOS HASHES ACIMA", d))
        else:
            out.append((f"{group} - {p}" if group not in ("BASE", "-") else p, d))
    return out


def model_of(lst):
    m = re.search(r"Urna (\d{4})", lst["title"])
    return f"UE{m.group(1)}" if m else lst["name"]


def inventory(urna, others):
    M = sorted(urna)
    print("\n== Inventory of the urna lists (models: %s)" % ", ".join(M))
    base = {m: dict(urna[m]["base"]) for m in M}
    allp = sorted(set().union(*base.values()))
    same = [p for p in allp if all(p in base[m] for m in M) and len({base[m][p] for m in M}) == 1]
    print(f"base files: union {len(allp)}; per model " + ", ".join(f"{m}={len(base[m])}" for m in M))
    print(f"present in every model with the same digest: {len(same)}")
    print("paths that are not (presence per model in the order above, distinct digests):")
    for p in allp:
        if p in same:
            continue
        pres = "".join("x" if p in base[m] else "." for m in M)
        nd = len({base[m][p] for m in M if p in base[m]})
        print(f"  {pres}  distinct={nd}  {p}")
    for m in M:
        b = [p for p, _ in urna[m]["base"]]
        ok_walk = walk_order(b) == b
        ok_keys = all([p for p, _ in rows] == sorted(p for p, _ in rows) for rows in urna[m]["uf"].values())
        print(f"{m}: base order is the directory-walk order: {ok_walk}; key files sorted in every UF: {ok_keys}")

    print("\n== Per-UF key files")
    ufs = sorted(set.intersection(*[set(urna[m]["uf"]) for m in M]))
    keys = sorted({p for m in M for uf in ufs for p, _ in urna[m]["uf"][uf]})
    kd = {m: {uf: dict(urna[m]["uf"][uf]) for uf in ufs} for m in M}
    print(f"UFs: {len(ufs)}; key paths: {len(keys)}")
    n_per_uf = n_const = 0
    for p in keys:
        per_model = {m: len({kd[m][uf].get(p) for uf in ufs}) for m in M}
        across = all(len({kd[m][uf].get(p) for m in M}) == 1 for uf in ufs)
        tag = "same in every UF" if set(per_model.values()) == {1} else "differs per UF"
        n_const += tag == "same in every UF"
        n_per_uf += tag == "differs per UF"
        print(f"  {p:45s} distinct over UFs {per_model}  identical across models: {across}  -> {tag}")
    print(f"key files that differ per UF: {n_per_uf}; identical in every UF: {n_const}")
    print("every UF key block identical in all model lists:",
          all(kd[m][uf] == kd[M[0]][uf] for m in M for uf in ufs))
    print("HASH GERAL distinct values per UF across models:",
          sorted({len({urna[m]["geral_uf"].get(uf) for m in M}) for uf in ufs}))
    other_by_digest = collections.defaultdict(list)
    for o in others:
        for p, d in o["files"]:
            other_by_digest[d].append(f"{o['name']}:{p}")
    shared = collections.defaultdict(set)
    for m in M:
        for uf in ufs:
            for p, d in urna[m]["uf"][uf]:
                if d in other_by_digest:
                    shared[p].update(other_by_digest[d])
    print("key files whose digest also appears in another list:")
    for p in sorted(shared):
        print(f"  {p}  ==  {sorted(shared[p])}")
    if not shared:
        print("  (none)")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("inputs", nargs="+", type=Path, help="HTML files or directories searched recursively")
    ap.add_argument("--tsv-out", type=Path, help="write one TSV per page into this directory")
    ap.add_argument("--compare-txt", type=Path, help="directory with plain-text copies (<stem>.txt)")
    a = ap.parse_args()

    files = []      # (path, bytes or None)
    for i in a.inputs:
        if i.is_dir():
            files += [(f, None) for f in sorted(i.rglob("*.html"))]
        elif i.suffix.lower() == ".zip":   # read the pages straight from a 'Resumos digitais' zip
            with zipfile.ZipFile(i) as z:
                for m in sorted(z.namelist()):
                    if m.lower().endswith(".html") and z.getinfo(m).file_size < 50 * 1024 * 1024:
                        files.append((Path(m), z.read(m)))
        else:
            files.append((i, None))
    lists, seen = [], collections.Counter()
    for f, data in files:
        lst = parse_html(f, data)
        stem = f.stem.lower()
        seen[stem] += 1
        if seen[stem] == 1:
            lst["name"] = stem
        else:  # same page name twice (e.g. two dated versions in 'x' and 'x (1)' folders)
            parent = re.sub(r"[^a-z0-9()_.-]+", "_", f.parent.name.lower())
            lst["name"] = parent if parent.startswith(stem) and parent != stem else f"{stem}_{seen[stem]}"
        lists.append(lst)
    if a.tsv_out:
        a.tsv_out.mkdir(parents=True, exist_ok=True)
        for lst in lists:
            write_tsv(lst, a.tsv_out / f"{lst['name']}.tsv")
        print(f"wrote {len(lists)} TSV files to {a.tsv_out}")

    print("== HASH GERAL (CMontadorHash::CalculaHashGeral chain)")
    pc_ok = pc_n = uf_ok = uf_n = 0
    for lst in lists:
        if lst["kind"] == "urna":
            good = []
            for uf, rows in lst["uf"].items():
                if uf not in lst["geral_uf"]:
                    continue
                uf_n += 1
                calc = hash_geral([d for _, d in lst["base"]] + [d for _, d in rows])
                if calc == lst["geral_uf"][uf]:
                    uf_ok += 1
                    good.append(uf)
            print(f"{lst['name']:16s} {model_of(lst)}: base {len(lst['base'])} files; "
                  f"UF HASH GERAL reproduced {len(good)}/{len(lst['geral_uf'])}")
        elif lst["geral"]:
            pc_n += 1
            cov = lst["geral_covers"]
            ok = hash_geral([d for _, d in lst["files"][:cov]]) == lst["geral"]
            pc_ok += ok
            after = len(lst["files"]) - cov
            extra = f" (+{after} files listed after the HASH GERAL row, not covered by it)" if after else ""
            print(f"{lst['name']:16s} {cov:4d} files; HASH GERAL reproduced: {ok}{extra}   [{lst['title']}]")
        else:
            print(f"{lst['name']:16s} {len(lst['files']):4d} files; no HASH GERAL on this page   [{lst['title']}]")
    print(f"TOTAL: lists with one HASH GERAL reproduced {pc_ok}/{pc_n}; "
          f"urna per-UF HASH GERAL reproduced {uf_ok}/{uf_n}")

    if a.compare_txt:
        print("\n== HTML vs plain-text copies")
        for lst in lists:
            cands = [a.compare_txt / f"{lst['name']}.txt", a.compare_txt / f"{lst['name'].split('_')[0]}.txt"]
            t = next((c for c in cands if c.exists()), None)
            if t is None:
                print(f"{lst['name']}: no text copy")
                continue
            h, x = html_pairs(lst), txt_pairs(t)
            print(f"{lst['name']}: {len(h)} entries; identical to {t.name}: {h == x}")

    urna = {model_of(l): l for l in lists if l["kind"] == "urna"}
    if urna:
        inventory(urna, [l for l in lists if l["kind"] != "urna"])


if __name__ == "__main__":
    sys.exit(main())
