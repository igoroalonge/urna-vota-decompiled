#!/usr/bin/env python3
"""List and download TSE open-data resources through the CKAN API of dadosabertos.tse.jus.br.

Commands:
  search  QUERY                       datasets whose metadata match QUERY (name, title, #resources)
  list    DATASET [--uf UF] [--match REGEX]
                                      resources of a dataset (name, format, URL); --uf keeps the
                                      resources whose name or URL carries that UF code
  get     DATASET --out DIR [--uf UF] [--match REGEX] [--no-verify]
                                      download the selected resources; when the dataset also lists
                                      '<file>.sha512', the download is checked against it

DATASET is the CKAN dataset name, e.g. resultados-2026-boletim-de-urna or
resultados-2026-logs-do-sistema-de-preparacao-das-urnas-eletronicas-gedai-1-turno.

Examples:
  python3 -I fetch_open_data.py search "boletim de urna 2026"
  python3 -I fetch_open_data.py list resultados-2026-boletim-de-urna --uf AC
  python3 -I fetch_open_data.py get resultados-2026-boletim-de-urna --uf AC --out data/cdn

Files land in --out under their published file names. Archives are not extracted. The GEDAI log
bundles contain staff user numbers and computer names: do not commit them or derived raw text.
"""
import argparse
import base64
import hashlib
import json
import re
import sys
import urllib.parse
import urllib.request
from pathlib import Path

API = "https://dadosabertos.tse.jus.br/api/3/action/"
UA = {"User-Agent": "Mozilla/5.0 (X11; Linux x86_64) investigation-kit/1.0", "Accept": "*/*"}
SAFE_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,200}$")


def api(action, **params):
    url = API + action + "?" + urllib.parse.urlencode(params)
    with urllib.request.urlopen(urllib.request.Request(url, headers=UA), timeout=60) as r:
        d = json.loads(r.read().decode("utf-8"))
    if not d.get("success"):
        sys.exit(f"CKAN error for {action}: {d.get('error')}")
    return d["result"]


def resources(dataset, uf=None, match=None):
    res = api("package_show", id=dataset)["resources"]
    out = []
    for r in res:
        name, url = r.get("name") or "", r.get("url") or ""
        if uf:
            u = uf.upper()
            if not (re.search(rf"(^|[^A-Z]){u}([^A-Z]|$)", name) or re.search(rf"[_/-]{u}[_./-]", url)):
                continue
        if match and not (re.search(match, name) or re.search(match, url)):
            continue
        out.append(r)
    return out


def download(url, dest):
    with urllib.request.urlopen(urllib.request.Request(url, headers=UA), timeout=600) as r, \
            open(dest.with_name(dest.name + ".part"), "wb") as f:
        h = hashlib.sha512()
        while True:
            b = r.read(1 << 20)
            if not b:
                break
            h.update(b)
            f.write(b)
    dest.with_name(dest.name + ".part").replace(dest)
    return h


def sha_matches(h, sha_text):
    cands = {h.hexdigest().lower(), base64.b64encode(h.digest()).decode()}
    toks = re.findall(r"[0-9A-Za-z+/=]{80,}", sha_text)
    return any(t.lower() in cands or t in cands for t in toks)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("search")
    s.add_argument("query")
    s.add_argument("--rows", type=int, default=50)
    for name in ("list", "get"):
        p = sub.add_parser(name)
        p.add_argument("dataset")
        p.add_argument("--uf")
        p.add_argument("--match", help="regular expression on resource name or URL")
        if name == "get":
            p.add_argument("--out", required=True, type=Path)
            p.add_argument("--no-verify", action="store_true", help="skip the .sha512 check")
    a = ap.parse_args()

    if a.cmd == "search":
        r = api("package_search", q=a.query, rows=a.rows)
        print(f"{r['count']} datasets")
        for p in r["results"]:
            print(f"{p['name']}\t{len(p['resources'])} resources\t{p['title']}")
        return
    sel = resources(a.dataset, a.uf, a.match)
    if a.cmd == "list":
        for r in sel:
            print(f"{r.get('format') or '-'}\t{r.get('name')}\t{r.get('url')}")
        print(f"{len(sel)} resources", file=sys.stderr)
        return
    a.out.mkdir(parents=True, exist_ok=True)
    allres = {r["url"]: r for r in resources(a.dataset)}
    for r in sel:
        url = r["url"]
        if url.endswith(".sha512"):
            continue
        fn = url.rstrip("/").rsplit("/", 1)[-1]
        if not SAFE_NAME.match(fn):
            print(f"skip unsafe file name {fn!r}")
            continue
        dest = a.out / fn
        print(f"downloading {fn} ...", flush=True)
        h = download(url, dest)
        status = "not checked"
        if not a.no_verify and url + ".sha512" in allres:
            with urllib.request.urlopen(urllib.request.Request(url + ".sha512", headers=UA), timeout=60) as rr:
                sha_txt = rr.read().decode("latin-1")
            status = "SHA-512 matches the published .sha512" if sha_matches(h, sha_txt) else "SHA-512 MISMATCH"
        print(f"  {dest.stat().st_size} bytes, {status}")


if __name__ == "__main__":
    main()
