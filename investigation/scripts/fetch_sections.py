#!/usr/bin/env python3
"""Download the per-section urna files of the 2026 elections from resultados.tse.jus.br.

The public "arquivo-urna" tree needs browser-like request headers (User-Agent, Referer), which are
sent here. Layout written (the one every other script of this kit reads):

    <out>/<uf>-<mun>-<zona>-<secao>/aux.json                 the section index published by TSE
    <out>/<uf>-<mun>-<zona>-<secao>/<hash[:16]>/<file>       one directory per transmitted hash
                                                            (*-bu.dat, *-rdv.dat, *-log.jez, *-vota.vsc ...)

Two modes:
  sample    N random sections per UF, reproducible with --seed (random.Random(f"{seed}-{uf}").sample
            over the sections in the order of the UF's published configuration file). Sections with
            no published files answer 404 and are reported and skipped.
  sections  explicit sections, written uf-mun-zona-secao (e.g. ac-01074-0004-0042)

Examples:
  python3 -I fetch_sections.py --out data/sections sample --per-uf 4 --seed 1 ac df sp zz
  python3 -I fetch_sections.py --out data/sections sections ac-01074-0004-0042 --types bu,rdv,vota

Downloaded files are untrusted data: keep them in their own directory, never run anything from it,
and read them only with `python3 -I`. The log archives (*-log.jez) contain poll-worker identifiers
(títulos eleitorais in clear text): do not commit them to a public repository.
"""
import argparse
import json
import random
import re
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

UA = ("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) "
      "Chrome/129.0 Safari/537.36")
HEADERS = {"User-Agent": UA, "Accept": "*/*", "Accept-Language": "pt-BR,pt;q=0.9",
           "Referer": "https://resultados.tse.jus.br/"}
SAFE_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,120}$")
SECTION = re.compile(r"^([a-z]{2})-(\d{5})-(\d{4})-(\d{4})$")


def get(url, tries=4, timeout=60):
    for i in range(tries):
        try:
            req = urllib.request.Request(url, headers=HEADERS)
            with urllib.request.urlopen(req, timeout=timeout) as r:
                return r.read()
        except urllib.error.HTTPError as e:
            if e.code == 404 or i == tries - 1:
                raise
            time.sleep(2 ** i)
        except Exception:
            if i == tries - 1:
                raise
            time.sleep(2 ** i)


def write_atomic(path, data):
    tmp = path.with_name(path.name + ".part")
    tmp.write_bytes(data)
    tmp.replace(path)


class Site:
    def __init__(self, election, pleito):
        self.base = f"https://resultados.tse.jus.br/oficial/{election}/arquivo-urna/{pleito}"
        self.pleito = int(pleito)

    def config(self, uf, cache_dir):
        name = f"{uf}-p{self.pleito:06d}-cs.json"
        p = Path(cache_dir) / name
        if not p.exists():
            p.parent.mkdir(parents=True, exist_ok=True)
            write_atomic(p, get(f"{self.base}/config/{uf}/{name}"))
        return json.loads(p.read_bytes().decode("utf-8"))

    def fetch_section(self, uf, mun, zon, sec, out, types=None, delay=0.0):
        d = Path(out) / f"{uf}-{mun}-{zon}-{sec}"
        base = f"{self.base}/dados/{uf}/{mun}/{zon}/{sec}"
        # 404 here = no file published for the section (e.g. not installed / aggregated)
        raw = get(f"{base}/p{self.pleito:06d}-{uf}-m{mun}-z{zon}-s{sec}-aux.json")
        d.mkdir(parents=True, exist_ok=True)
        write_atomic(d / "aux.json", raw)
        aux = json.loads(raw.decode("utf-8", errors="replace"))
        n_ok = n_err = 0
        for h in aux.get("hashes", []):
            hv = str(h.get("hash", ""))
            if not re.fullmatch(r"[A-Za-z0-9]{16,256}", hv):
                print(f"  skip hash entry with unexpected value in {d.name}", file=sys.stderr)
                continue
            hd = d / hv[:16]
            hd.mkdir(exist_ok=True)
            for a in h.get("arq", []):
                nm, tp = str(a.get("nm", "")), str(a.get("tp", ""))
                if types and tp not in types:
                    continue
                if not SAFE_NAME.match(nm):
                    print(f"  skip unsafe file name in {d.name}: {nm!r}", file=sys.stderr)
                    continue
                fp = hd / nm
                if fp.exists():
                    continue
                try:
                    write_atomic(fp, get(f"{base}/{hv}/{nm}"))
                    n_ok += 1
                except Exception as e:  # keep going; record the failure next to the file
                    (hd / (nm + ".ERR")).write_text(str(e)[:500])
                    n_err += 1
                if delay:
                    time.sleep(delay)
        return d, n_ok, n_err


def iter_sections(cfg):
    """(uf, mun, zona, secao) of every section of a UF configuration file, in file order."""
    for ab in cfg["abr"]:
        for mu in ab["mu"]:
            for z in mu["zon"]:
                for s in z["sec"]:
                    yield ab["cd"].lower(), mu["cd"], z["cd"], s["ns"]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", required=True, type=Path, help="output directory (one subdirectory per section)")
    ap.add_argument("--config-dir", type=Path, help="where the per-UF configuration files are cached "
                    "(default: <out>/_config)")
    ap.add_argument("--election", default="ele2026", help="election path element (default ele2026)")
    ap.add_argument("--pleito", default="3220", help="pleito id (default 3220 = 2026 1st round)")
    ap.add_argument("--types", help="comma list of aux.json file types to fetch, e.g. bu,rdv,vota,log "
                    "(default: all)")
    ap.add_argument("--delay", type=float, default=0.2, help="seconds between file requests (default 0.2)")
    sub = ap.add_subparsers(dest="mode", required=True)
    s1 = sub.add_parser("sample", help="random sample of sections per UF")
    s1.add_argument("--per-uf", type=int, required=True)
    s1.add_argument("--seed", required=True)
    s1.add_argument("ufs", nargs="+", help="UF codes (lower case), 'zz' = abroad")
    s2 = sub.add_parser("sections", help="explicit sections uf-mun-zona-secao")
    s2.add_argument("ids", nargs="+")
    s3 = sub.add_parser("count", help="only print the number of sections of each UF")
    s3.add_argument("ufs", nargs="+")
    a = ap.parse_args()

    site = Site(a.election, a.pleito)
    cfgdir = a.config_dir or (a.out / "_config")
    types = set(a.types.split(",")) if a.types else None
    a.out.mkdir(parents=True, exist_ok=True)

    if a.mode == "count":
        for uf in a.ufs:
            print(uf, len(list(iter_sections(site.config(uf.lower(), cfgdir)))))
        return
    if a.mode == "sample":
        todo = []
        for uf in a.ufs:
            uf = uf.lower()
            secs = list(iter_sections(site.config(uf, cfgdir)))
            pick = random.Random(f"{a.seed}-{uf}").sample(secs, min(a.per_uf, len(secs)))
            print(f"{uf}: {len(secs)} sections, sampling {len(pick)}", flush=True)
            todo += pick
    else:
        todo = []
        for s in a.ids:
            m = SECTION.match(s.lower())
            if not m:
                sys.exit(f"bad section id {s!r}; expected uf-mun-zona-secao like ac-01074-0004-0042")
            todo.append(m.groups())
    for uf, mun, zon, sec in todo:
        try:
            d, n_ok, n_err = site.fetch_section(uf, mun, zon, sec, a.out, types, a.delay)
            print(f"{d.name}: {n_ok} files downloaded, {n_err} errors", flush=True)
        except Exception as e:
            print(f"ERROR {uf}-{mun}-{zon}-{sec}: {e}", flush=True)


if __name__ == "__main__":
    main()
