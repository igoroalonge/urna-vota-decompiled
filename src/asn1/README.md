# src/asn1: TSE ASN.1 modules recovered from `vota_web_wasm.wasm`

32 ASN.1 modules (`Modulo*.asn`), rebuilt from the III ASN.1 type tables compiled into the
simulator: 118 SEQUENCE, 10 CHOICE, 58 ENUMERATED and 5 named string types. The method, the
meaning of each module, a comparison with the TSE's published `bu.asn1`/`rdv.asn1` and the
mapping from scenario files to types are in
[`docs/data-model/asn1-schemas.md`](../../docs/data-model/asn1-schemas.md).

Each type is preceded by a comment such as

```
-- info @1144144; name: rtti+infoname (high)
```

giving the address of its table in wasm memory and where its name comes from. `inferred` marks
a placeholder name that is not in the binary. Member names, order, OPTIONAL, tags, ranges and
enum values are always read from the binary.

The modules compile with an independent ASN.1 compiler (Python `asn1tools`), and every scenario
and state file of the simulator decodes and re-encodes byte-identically with them. So do the BU and
RDV files that real urnas wrote in the 2026 election and TSE published. The extraction and
verification tools are part of the analysis project this repository was taken from, and are not
included here.
