"""Signature helpers for the urna's 2026 hardware signatures (standard library + cryptography).

* der_*        a minimal DER reader, enough to take an X.509 certificate apart (TBS bytes,
               algorithms, subject/issuer common names, public key, signature)
* ed521_verify EdDSA over the Edwards curve E-521 ("Ed521") with SHAKE256 (132-byte output) and
               the domain prefix "SigEd521" || 0x00 || 0x00 -- the scheme the UE2020/UE2022 urna
               certificates use (public-key OID 1.3.6.1.4.1.44588.2.1). Own pure-Python
               implementation from the published curve constants; no third-party code.
* ecdsa_p521_verify  ECDSA over P-521 with SHA-512 (UE2013/UE2015), through `cryptography`.

How the urna signs (observed on the real 2026 files, see vsc2026.py): what is signed is the SHA-512
*of* the value (a file's SHA-512 digest, a BU's last tuple hash): ECDSA-with-SHA-512 over the value
(= ECDSA on the pre-hash SHA-512(value)), or Ed521 with the 64-byte SHA-512(value) as the message.
"""
import base64
import hashlib

# ----------------------------------------------------------------------------------------- DER

OID_EC_PUBLIC_KEY = "1.2.840.10045.2.1"
OID_ED521 = "1.3.6.1.4.1.44588.2.1"
OID_ECDSA_SHA512 = "1.2.840.10045.4.3.4"
OID_SECP521R1 = "1.3.132.0.35"
OID_CN = "2.5.4.3"


def der_tlv(buf, i=0):
    """(tag, header_len, value_start, value_end) of the TLV at buf[i]."""
    tag = buf[i]
    j = i + 1
    if tag & 0x1F == 0x1F:
        while buf[j] & 0x80:
            j += 1
        j += 1
    ln = buf[j]
    j += 1
    if ln & 0x80:
        n = ln & 0x7F
        ln = int.from_bytes(buf[j:j + n], "big")
        j += n
    return tag, j - i, j, j + ln


def der_children(buf, start, end):
    out = []
    i = start
    while i < end:
        tag, _, vs, ve = der_tlv(buf, i)
        out.append((tag, i, vs, ve))
        i = ve
    return out


def der_oid(b):
    first = b[0]
    parts = [first // 40, first % 40] if first < 80 else [2, first - 80]
    v = 0
    for x in b[1:]:
        v = (v << 7) | (x & 0x7F)
        if not x & 0x80:
            parts.append(v)
            v = 0
    return ".".join(map(str, parts))


def pem_or_der(blob):
    """Certificate bytes as stored (PEM text or DER, possibly with trailing bytes) -> DER."""
    if blob.lstrip().startswith(b"-----BEGIN"):
        body = b"".join(l.strip() for l in blob.splitlines() if l.strip() and not l.startswith(b"-----"))
        return base64.b64decode(body), "PEM"
    tag, _, _, ve = der_tlv(blob, 0)
    return blob[:ve], "DER"


def _name_cn(buf, s, e):
    for _, _, rs, re_ in der_children(buf, s, e):            # SET
        for _, _, as_, ae in der_children(buf, rs, re_):     # SEQUENCE {oid, value}
            kids = der_children(buf, as_, ae)
            if len(kids) == 2 and der_oid(buf[kids[0][2]:kids[0][3]]) == OID_CN:
                return buf[kids[1][2]:kids[1][3]].decode("utf-8", "replace")
    return None


def parse_certificate(der):
    """Minimal X.509 parse -> dict(tbs, sig_alg, signature, spki_alg, spki_params, public_key,
    subject_cn, issuer_cn)."""
    _, _, cs, ce = der_tlv(der, 0)
    tbs_t, sig_alg_t, sig_t = der_children(der, cs, ce)
    tbs = der[tbs_t[1]:tbs_t[3]]
    sig_alg = der_oid(der[der_children(der, sig_alg_t[2], sig_alg_t[3])[0][2]:
                          der_children(der, sig_alg_t[2], sig_alg_t[3])[0][3]])
    signature = der[sig_t[2] + 1:sig_t[3]]                     # BIT STRING, skip unused-bits byte
    f = der_children(der, tbs_t[2], tbs_t[3])
    k = 1 if f[0][0] == 0xA0 else 0                            # optional [0] version
    issuer, subject, spki = f[k + 2], f[k + 4], f[k + 5]
    alg_seq, key_bits = der_children(der, spki[2], spki[3])
    alg_kids = der_children(der, alg_seq[2], alg_seq[3])
    spki_alg = der_oid(der[alg_kids[0][2]:alg_kids[0][3]])
    params = der_oid(der[alg_kids[1][2]:alg_kids[1][3]]) if len(alg_kids) > 1 and alg_kids[1][0] == 6 else None
    return dict(tbs=tbs, sig_alg=sig_alg, signature=signature, spki_alg=spki_alg, spki_params=params,
                public_key=der[key_bits[2] + 1:key_bits[3]],
                subject_cn=_name_cn(der, subject[2], subject[3]),
                issuer_cn=_name_cn(der, issuer[2], issuer[3]))


# --------------------------------------------------------------------------------------- Ed521
# Curve E-521 (twisted Edwards form with a = 1): x^2 + y^2 = 1 + d x^2 y^2 over GF(2^521 - 1).
P = 2 ** 521 - 1
D = (-376014) % P
N = 0x7ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd15b6c64746fc85f736b8af5e7ec53f04fbd8c4569a8f1f4540ea2435f5180d6b
GX = 0x752cb45c48648b189df90cb2296b2878a3bfd9f42fc6c818ec8bf3c9c0c6203913f6ecc5ccc72434b1ae949d568fc99c6059d0fb13364838aa302a940a2f19ba6c
GY = 12
SIZE = 66


def _add(p1, p2):
    """Unified addition in extended coordinates (X:Y:Z:T), x = X/Z, y = Y/Z, T = XY/Z, a = 1."""
    X1, Y1, Z1, T1 = p1
    X2, Y2, Z2, T2 = p2
    A = X1 * X2 % P
    B = Y1 * Y2 % P
    C = T1 * D % P * T2 % P
    Dd = Z1 * Z2 % P
    E = ((X1 + Y1) * (X2 + Y2) - A - B) % P
    F = (Dd - C) % P
    G = (Dd + C) % P
    H = (B - A) % P
    return (E * F % P, G * H % P, F * G % P, E * H % P)


def _mul(k, pt):
    acc = (0, 1, 1, 0)
    while k:
        if k & 1:
            acc = _add(acc, pt)
        pt = _add(pt, pt)
        k >>= 1
    return acc


def _affine(x, y):
    return (x % P, y % P, 1, x * y % P)


def _eq(p1, p2):
    return (p1[0] * p2[2] - p2[0] * p1[2]) % P == 0 and (p1[1] * p2[2] - p2[1] * p1[2]) % P == 0


def ed521_decode(enc):
    if len(enc) != SIZE:
        raise ValueError("Ed521 point must be 66 bytes")
    b = bytearray(enc)
    sign = b[-1] >> 7
    b[-1] &= 0x7F
    y = int.from_bytes(b, "little")
    if y >= P:
        raise ValueError("y out of range")
    yy = y * y % P
    xx = (1 - yy) * pow((1 - D * yy) % P, P - 2, P) % P
    x = pow(xx, (P + 1) // 4, P)
    if x * x % P != xx:
        raise ValueError("not a point on Ed521")
    if (x & 1) != sign:
        x = (P - x) % P
    return _affine(x, y)


def ed521_encode(pt):
    zi = pow(pt[2], P - 2, P)
    x, y = pt[0] * zi % P, pt[1] * zi % P
    b = bytearray(y.to_bytes(SIZE, "little"))
    if x & 1:
        b[-1] |= 0x80
    return bytes(b)


def ed521_verify(public_key, message, signature):
    if len(signature) != 2 * SIZE:
        return False
    try:
        A = ed521_decode(public_key)
        R = ed521_decode(signature[:SIZE])
    except ValueError:
        return False
    S = int.from_bytes(signature[SIZE:], "little")
    if S >= N:
        return False
    h = hashlib.shake_256(b"SigEd521\x00\x00" + signature[:SIZE] + ed521_encode(A) + message).digest(132)
    k = int.from_bytes(h, "little") % N
    return _eq(_mul(S, _affine(GX, GY)), _add(R, _mul(k, A)))


# --------------------------------------------------------------------------------------- ECDSA

def ecdsa_p521_verify(public_point, message, der_signature, prehashed=False):
    """ECDSA P-521 / SHA-512 via `cryptography`; public_point = 04||X||Y. With prehashed=True the
    message is already the 64-byte SHA-512 digest."""
    try:
        from cryptography.exceptions import InvalidSignature
        from cryptography.hazmat.primitives import hashes
        from cryptography.hazmat.primitives.asymmetric import ec, utils
    except ImportError:
        raise SystemExit("ECDSA verification needs cryptography: pip install cryptography")
    key = ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP521R1(), public_point)
    algo = ec.ECDSA(utils.Prehashed(hashes.SHA512())) if prehashed else ec.ECDSA(hashes.SHA512())
    try:
        key.verify(der_signature, message, algo)
        return True
    except (InvalidSignature, ValueError):
        return False


class UrnaKey:
    """Public key of an urna certificate, with the urna's signing convention."""

    def __init__(self, cert):
        self.alg = cert["spki_alg"]
        self.public_key = cert["public_key"]
        if self.alg not in (OID_ED521, OID_EC_PUBLIC_KEY):
            raise ValueError(f"unsupported public-key algorithm {self.alg}")

    @property
    def name(self):
        return "Ed521 (EdDSA, SHAKE256)" if self.alg == OID_ED521 else "ECDSA P-521"

    def verify_prehash(self, digest, signature):
        """Signature over a 64-byte value treated as the SHA-512 pre-hash: for ECDSA the value is the
        digest that is signed; for Ed521 it is the EdDSA message."""
        if self.alg == OID_ED521:
            return ed521_verify(self.public_key, digest, signature)
        return ecdsa_p521_verify(self.public_key, digest, signature, prehashed=True)

    def verify_value(self, value, signature):
        """The urna's convention: what is signed is SHA-512(value)."""
        return self.verify_prehash(hashlib.sha512(value).digest(), signature)


def verify_cert_signature(cert, issuer_cert):
    """Check `cert` against the public key of `issuer_cert` (both parse_certificate() dicts)."""
    if cert["sig_alg"] == OID_ED521:
        return ed521_verify(issuer_cert["public_key"], cert["tbs"], cert["signature"])
    if cert["sig_alg"] == OID_ECDSA_SHA512:
        return ecdsa_p521_verify(issuer_cert["public_key"], cert["tbs"], cert["signature"])
    raise ValueError(f"unsupported certificate signature algorithm {cert['sig_alg']}")
