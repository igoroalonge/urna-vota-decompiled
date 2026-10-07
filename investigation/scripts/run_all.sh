#!/bin/sh
# Re-run every offline check of the kit on data you have downloaded.
#
#   sh investigation/scripts/run_all.sh SECTIONS_DIR HASH_HTML_DIR [OUT_DIR]
#
#   SECTIONS_DIR   output of fetch_sections.py (one directory per section)
#   HASH_HTML_DIR  the HTML pages (or zips) of TSE's "Resumos digitais (hashes)" for 2026
#   OUT_DIR        where the .out reports go (default: ./investigation-out)
#
# Set PYTHON to an interpreter that has asn1tools and cryptography (default: python3), and REPO to
# the repository root if the scripts are not inside <repo>/investigation/scripts.
# Nothing here touches the network; the outputs contain counts and templates only (no títulos).
set -e
[ $# -ge 2 ] || { echo "usage: $0 SECTIONS_DIR HASH_HTML_DIR [OUT_DIR]"; exit 2; }
SEC=$1
HTML=$2
OUT=${3:-./investigation-out}
PY=${PYTHON:-python3}
HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$OUT"
run() { name=$1; shift; echo "== $name"; "$PY" -I "$HERE/$name.py" "$@" > "$OUT/$name.out" 2>&1 && echo "   ok -> $OUT/$name.out" || echo "   exit $? -> $OUT/$name.out"; }
run hashlists "$HTML" --tsv-out "$OUT/hashlists-tsv"
run roundtrip_asn1 "$SEC" ${REPO:+--repo "$REPO"}
run bu_vs_rdv "$SEC" ${REPO:+--repo "$REPO"}
run vsc2026 "$SEC" ${REPO:+--repo "$REPO"}
run log_templates "$SEC" --tsv "$OUT/log_templates.tsv" ${REPO:+--repo "$REPO"}
run qr_rebuild "$SEC" ${REPO:+--repo "$REPO"}
echo "== summary"
for f in hashlists roundtrip_asn1 bu_vs_rdv vsc2026 log_templates qr_rebuild; do
  grep -H -E "^TOTAL|^bu\.dat|^rdv\.dat|^failures|^\(BU QR|^last QR|^  VOTA " "$OUT/$f.out" || true
done
