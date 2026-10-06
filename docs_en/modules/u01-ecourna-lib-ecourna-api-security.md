# u01: `ecourna::api::security` (AES block cipher, Base64, key loader, CEPESC data objects)

Unit u01 covers 17 wasm functions from the TSE library **ecourna** (`ecourna-lib/ecourna/api/security/…`,
built from the Conan cache `/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/…`). They are the
urna's general-purpose symmetric-crypto layer:

* **`CAesCipher`** and **`CBlockCipher<CAesCipher, CTrng>`**: AES through OpenSSL 3.0.17 `EVP_*`. The
  application uses it to encrypt the **RDV** (*Registro Digital do Voto*, the file with the anonymised
  record of every vote) and to decipher key files.
* **`EncodeBase64`**: Base64 text for display. In this binary it survives only inside a caller that prints
  the first 8 characters of the hash of the data packages ("Dados: …").
* **`CKeyLoader::DecipherKeyIfNeeded`**: unwraps a key envelope (`ModuloEnvelopeChave::EntidadeChave`) read
  from `/dsk/fi/estatico/chave/`. The BU code uses it to load the key of the *código verificador*.
* **CEPESC data objects** `CPlainText`, `CCipheredOut` and `CInfoSalt`. CEPESC is the TSE's encryption for
  result files, named after the *Centro de Pesquisa e Desenvolvimento para a Segurança das Comunicações*.
  These classes are the input and output of `CCepescCipher`. **In the web build `CCepescCipher` does not
  encrypt** (§4.1).

Reconstructed sources: `src/ecourna/api/security/{caescipher,cblockcipher,base64}.*`,
`src/ecourna/api/security/asn1/ckeyloader.*`, `src/ecourna/api/security/cepesc/{cinfosalt,cplaintext,ccipheredout}.*`.

Five of the 17 functions ran during the recorded votes (`analysis/runtime/*.functions.tsv`): 6154, 9487,
9493, 9494 and 9495. All five ran during `votaInit`, when the RDV is saved (§3.4).

---

## 1. Classes and how they relate

RTTI (`q.py cls`) and the layouts recovered from constructors:

```
pattern::NonCopyable
 └─ ISymmetricCipher                         typeinfo @1114024, vtable @1114100   (isymmetriccipher.cpp, not in u01)
     └─ CBlockCipher<CAesCipher, CTrng>      typeinfo @1113944, vtable @1113928   (cblockcipher.hpp)
            vf0 ~dtor (9484, ICF with ~ISymmetricCipher), vf1 deleting dtor (9488),
            vf2 Encrypt (9487), vf3 Decrypt (9485)
ISymmetricCipherFactory
 └─ CSymmetricCipherFactory                  vtable @1113840: vf2 Create(const std::string&) (1884),
                                                              vf3 Create(const CAesKey&)    (9489)
IRng
 └─ CTrng                                    vtable @1114668 (std::random_device)
IAsymmetricCipher<CPlainText, CCipheredOut, CCipheredIn, std::vector<uebyte>>
 └─ cepesc::CCepescCipher                    vtable @1115004: vf2 Encrypt (2681), vf3 Decrypt (5171)

non-polymorphic: CAesCipher (48 bytes), CAesKey (28), CKeyLoader (8),
                 cepesc::CPlainText (64), cepesc::CCipheredOut (32), cepesc::CInfoSalt (24)
error type:      exception::CBaseError<security::ESecurityError, SErrorLimits{1325, 1725}>
                 typeinfo @1112704, vtable @1112932 (built by thunk 9505 -> shared body 1011)
```

| class | layout (wasm32) |
|---|---|
| `CAesKey` | `+0 vector<uebyte> key`, `+12 ESymmetricCipherKeySize keySize` (default **256**), `+16 vector<uebyte> iv` |
| `ISymmetricCipher` | `+0 vptr`, `+4 CAesKey m_key` |
| `CBlockCipher<A,R>` | `ISymmetricCipher`, `+32 EOperationModes m_mode` (36 bytes) |
| `CAesCipher` | `+0 const EVP_CIPHER*`, `+4 mode`, `+8 shared_ptr<IRng>`, `+16 CAesKey`, `+44 int blockSize` |
| `CKeyLoader` | `+0 TSharedSymmetricCipher` (= `std::shared_ptr<ISymmetricCipher>`) |
| `CInfoSalt` | `+0 vector salt`, `+12 vector info` |
| `CPlainText` | `+0 uebyte tipoArquivo`, `+1 uebyte idCriptografia`, `+2 ueword zona?`, `+4 ueword seção?`, `+8 tabelaCriptografia`, `+20 numeroAleatorio`, `+32 chave`, `+44 conteudo`, `+56 shared_ptr<CInfoSalt>` |
| `CCipheredOut` | `+0 vector chave`, `+12 vector conteudo`, `+24 shared_ptr<CInfoSalt>` |

Enumerations (values from the code; the names are inferred):

* `EOperationModes`: ECB = 0, CBC = 1, CTR = 4. Values 2 and 3 are not in the cipher table.
* `ESymmetricCipherKeySize`: the key size in **bits**: 128, 192 or 256.

---

## 2. `CAesCipher` and `CBlockCipher`: control flow

### 2.1 Choosing the algorithm (func 9495, `caescipher.cpp:54`)

A global `std::map<pair<EOperationModes, ESymmetricCipherKeySize>, const EVP_CIPHER*(*)()>` (header @1911772)
is filled by the file's static initializer, which is inlined into `__wasm_call_ctors` (func 14478). It has 9 entries:
`{CBC, ECB, CTR} × {128, 192, 256}` → `EVP_aes_<bits>_<mode>` (funcs 7413–7422). The constructor
copies the `IRng`, looks up `(mode, keySize)`, and stores `m_cipher` and `m_blockSize = EVP_CIPHER_get_block_size`.
If the pair is not in the table, it throws **1330 "Chave AES de tamanho inválido."**. The same message is used when
the *mode* is the problem.

### 2.2 `SetKey` (func 9494, line 70)

`if (m_cipher->key_len != key.key.size()) throw 1331 "Chave AES de tamanho inválido."`, then `m_key = key`
(defaulted member-wise `operator=`: `vector::operator=` = func 1681, `__assign_with_size<uebyte*, uebyte*>`).
**The IV length is not checked.** This does not cause an out-of-bounds read: `Encrypt`/`Decrypt` copy the key IV
into a local vector that already holds 16 zero bytes, and the assign reuses that capacity. A shorter IV is therefore
silently **zero-padded** to 16 bytes, and a longer one is **truncated** to its first 16 bytes.

### 2.3 `Encrypt` (func 9493, lines 79–105)

```
ctx = EVP_CIPHER_CTX_new()                      -> 1332 "Erro ao alocar contexto de cifra OpenSSL."
buffer = make_unique<uebyte[]>(in.size() + blockSize)   (operator new[] = func 7750, memset 0)
iv = vector<uebyte>(16)                                  (operator new = func 137, zero-filled)
if key.iv.empty():  m_rng->Fill(iv, 16)          (IRng slot 5 = CTrng::vf5; status IGNORED)
else:               iv.assign(key.iv.cbegin(), key.iv.cend())   (func 1158, const-iterator assign)
EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv)   -> 1333 "EncryptInit_ex falhou."
EVP_EncryptUpdate(...)  fail && ERR_get_error() -> 1334 ERR_error_string(e, NULL)
EVP_EncryptFinal_ex(...) fail && ERR_get_error() -> 1335 ERR_error_string(e, NULL)
out = buffer[0 .. len1+len2]
if key.iv.empty():  out += iv                     (the random IV is APPENDED)
EVP_CIPHER_CTX_free(ctx)                          (RAII: invoked with a terminate landing pad)
```

### 2.4 `Decrypt` (func 9491, lines 119–150)

This is the mirror image. Without a key IV, `iv = in[end-16 .. end)` and `ciphertext = in[0 .. end-16)`.
Nothing checks that the input has at least 16 bytes (`CBlockCipher` only rejects an empty one). With 1–15 bytes
the 16-byte IV copy reads memory before `in.data()`, then the ciphertext assign gets a negative length and throws
`std::length_error("vector")` (func 1158 → func 158), not a `CBaseError`. Error codes
1336–1339 match 1332–1335, with the message "DecryptInit_ex falhou." at 1337. OpenSSL padding is left on
(PKCS#7), so a wrong key normally fails in `EVP_DecryptFinal_ex` with an OpenSSL error string (1339).

### 2.5 `CBlockCipher<CAesCipher, CTrng>::Encrypt/Decrypt` (funcs 9487/9485 → 6154)

```cpp
CAesCipher algoritmo(m_mode, m_key.keySize, std::make_shared<CTrng>());   // new objects on every call
if (in.empty()) throw 1344 "Sem dados para cifrar." / 1345 "Sem dados para decifrar."
algoritmo.SetKey(m_key);
algoritmo.Encrypt(in, out);   /  algoritmo.Decrypt(in, out);
```

The two methods had identical bodies except for constants. wasm-opt's *merge-similar-functions* turned them
into **one** body (func 6154), which takes the member function as a table slot (6284/6285), plus the srcloc, the
error code and the message. 9487 and 9485 are 26-byte thunks.

### 2.6 Who creates ciphers (`CSymmetricCipherFactory`, not in u01)

| factory entry | key | IV | users |
|---|---|---|---|
| vf2 `Create(const std::string& segredo)` (func 1884) | `SHA-512(segredo)[0..32)`, 256 bits | **empty** → random IV appended | key files: `vota::CGeraBU::StartState` (via `CKeyLoader`), `CGravadorBU::LeChavePublica`, `CGravadorRCSecao::LeChavePublica`, `CControlaArmazenamentoDeImagens::LeChavePublica`, `CConversorBiometriaEleitorCifrada`. Each passes bytes from `api::IKernelHSM` (vtable slot 3) as `segredo` |
| vf3 `Create(const CAesKey&)` (func 9489) | given | given | `comum::(anon)::GetCifradorCryptoTable` (crdv.cpp:60–78, inlined into func 7787): the **RDV** |

Both create `CBlockCipher<CAesCipher, CTrng>` with **mode 1 = CBC** (func 9489 stores 1 at +32). So every AES
use in VOTA is **AES-CBC**, 256-bit in practice. This settles the question left open in `docs/libraries/openssl.md` §11
("… would select `EVP_aes_256_ecb`"): func 1884 does not choose the mode. The `CBlockCipher` object that
func 9489 builds sets it.

---

## 3. Data written and read

### 3.1 RDV file (`/dsk/{fi,fe}/dinamico/trab1/rdv.dat`), confirmed at runtime

`votaInit` (func 7840) → func 6737 (`CGeraDadosDinamicos`) → funcs 5737/5736 → func 2767. Func 2767 is
really `api::CEncryptedFile::Save` (srcloc `cencryptedfile.cpp:88`; `CFile::Flush` is inlined into it). The chain continues
→ `CBlockCipher::Encrypt` → `CAesCipher::Encrypt`. The key comes from
`GetCifradorCryptoTable`: HKDF-SHA512 (`CHKDFSeed`, info `"RDV"`) produces a 128-byte seed, and
**key = seed[0..32), IV = seed[32..48)**. The IV is therefore **fixed**, and nothing is appended.

I checked this by running the simulator with a copy of `tools/run/headless.mjs` that logs the arguments of
`invoke_iiiiii(6260 = EVP_EncryptInit_ex, …)`. The copy was made in a temporary directory; `tools/` was not modified:

```
AESINIT enc cipher=1641408 (= aes_256_cbc) key=c4f9eff4…495a4952 iv=568d9298776ea63bc8a58367190a4d51   (twice)
$ openssl enc -d -aes-256-cbc -nopad -K <key> -iv <iv> -in rdv.dat
30 38 02 02 09 6a …  (58-byte BER RDV)  06 06 06 06 06 06  10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
```

* The same scenario gives a **byte-identical** `rdv.dat` in every run (3 runs, plus the snapshot in
  `analysis/runtime/memfs-after-*`). A different scenario gives a different file.
* The plaintext is **padded twice**. `CEncryptedFile::Save` adds PKCS#7 padding itself (`16 - size%16` bytes of
  that value), then `EVP_EncryptFinal_ex` adds a second PKCS#7 block (16 × `0x10`). An RDV decoder must strip both.
* In the web build, the 128-byte "hardware" table that seeds the HKDF comes from `api::teste::CUrnaMock::vf6`
  (func 8131), which is `memset(out, 0x03, 128)`. The RDV key can therefore be derived from public data.
  That is expected for a simulator.

About 70 of the 83 profiler samples in `CAesCipher::Encrypt` are in `EVP_EncryptInit_ex`. Every call builds a
new `CTrng`, a new `CAesCipher` (map lookup), and a new `EVP_CIPHER_CTX` with an implicit provider fetch.

### 3.2 Key envelopes (`/dsk/fi/estatico/chave/*`)

`CKeyLoader::DecipherKeyIfNeeded<ModuloEnvelopeChave::EntidadeChave>` (func 9473):

```
EntidadeChave ::= SEQUENCE { tagChaves NumericString(SIZE(12)), descritor DescritorChave,
                             cifrado BOOLEAN, tipo TipoChave, chave OCTET STRING }
bytes = copy of chave
if !cifrado            -> return bytes
if !m_cifrador         -> throw 1469 "Um TSharedSymmetricCipher é necessário para decifrar chaves."  (line 78)
m_cifrador->Decrypt(bytes, out) ; return out
```

These key files use factory vf2 (random IV appended), so a ciphered `chave` is laid out as `AES-256-CBC(key) ‖ IV(16)`.
The other readers (`…::LeChavePublica`) do the same thing inline, but they call `Decrypt` unconditionally.

### 3.3 Base64 "Dados" code (func 3703)

`EncodeBase64` (base64.cpp:247/267) uses the classic public-domain encoder: the alphabet @1112848,
`mod_table = {0,2,1}` @1112912, `output_length = 4*((n+2)/3)`, `'='` padding. It throws **1325** "Vetor de dados vazio."
on empty input and **1327** "Nao foi converter para base64." (sic) on failure. The buffer size it
allocates is `4*(n/3 + n%3)`. This is 4 bytes too large when `n%3 == 2`, which is harmless.

Func 3703 is **not** a standalone `EncodeBase64`. It is a caller with `EncodeBase64` inlined. Its argument is
the `CEstadoGeral` (all callers pass `comum::GetEstado<CEstadoGeral>()`). It encodes the vector at **+168**,
which is `EstadoGeralUrna.hashVersoesPacotes` (func 11397 writes that ASN.1 field from +168), and
returns only the **first 8 characters** (`min(size, 8)` copied into an SSO string). Callers:
`CGeradorRelVersaoPacoteDados::vf4` (the report line "Dados:", right-aligned to 38 columns), func 3059
("Versão: {}" / "Dados: {}") and func 7787. The 8 characters are 48 bits of the hash of the package versions.

### 3.4 Observed at runtime

Only the RDV path of §3.1 runs in the recorded sessions (6154, 9487, 9493, 9494, 9495 during `votaInit`).
The web build does not persist votes to the RDV (`CSincronismoVotoEleitorWeb`), so the encrypted file is written
at init only. Decrypt, CKeyLoader, Base64 and CEPESC did not run.

---

## 4. CEPESC data objects

| class / func | checks (all throw `CBaseError<ESecurityError>`) |
|---|---|
| `CInfoSalt(salt, info)` 5169 | 1513 "salt vazio." (27), 1514 "tamanho minimo do salt deve ser 16 bytes." (31), 1515 "conteúdo de info vazio." (35) |
| `CPlainText(tipoArquivo, idCriptografia, zona?, seção?, tabela, numeroAleatorio, chave, conteudo, infoSalt)` 9465 | 1517 tipo ≥ 3 (89), 1518 id ≥ 4 (92), 1519 "tabela de criptografia da UE vazia." (95), 1520 "número aleatório da UE vazio." (98), 1521 "chave vazia." (101), 1522 "conteúdo vazio." (104) |
| `CCipheredOut(chave, conteudo)` 9467 | 1504 "chave vazia." (26), 1505 "conteúdo vazio." (29) |
| `CCipheredOut(chave, conteudo, const CInfoSalt&)` 9466 | same checks, codes 1506/1507 (44/47). It stores `shared_ptr<CInfoSalt>(new CInfoSalt(copy))` |

`CInfoSalt` and both `CCipheredOut` constructors copy the members first and then test the **parameters**;
`CPlainText` moves its by-value arguments in first and then tests the **members** (same result).

`CPlainText` has a second, 6-argument entry point, func 5168 (no srcloc, outside u01). It has the shape of a
**delegating constructor**: `(this, ueword, ueword, 4 vectors by value)` → `CPlainText(0, 1, w1, w2, …, nullptr)`
(table slot 6328 = func 9465), and it returns `this`. Its callers are `CGravadorBU::GravaResultado` (func 11629) and
`CControlaArmazenamentoDeImagens::CifrarWsq` (srcloc ccontrolaarmazenamentodeimagens.cpp:149/152, inlined into
func 2725, which the database names after another inlined function, `LeChavePublica`). `CGravadorRCSecao`
(func 11616) calls the 9-argument constructor directly, with a `CInfoSalt`.

Mapping to ASN.1 (`src/asn1/`):

* `CPlainText.tipoArquivo`/`idCriptografia` → `ModuloTiposEleitorais.Seguranca { idTipoArquivo INTEGER (0..2),
  idCriptografia INTEGER (1..3), idArquivoCD, idArquivoChave OCTET STRING }` through `comum::md::CSeguranca(uebyte, uebyte,
  const vector&)`. The third argument is `CCipheredOut.chave`. Note that `CPlainText` accepts `idCriptografia = 0`,
  which the ASN.1 range rejects.
* `CInfoSalt` ↔ `DadosCifracao { chave, salt, informacaoAdicional }` (attendance file,
  `CGravadorRCSecao`) and `BiometriaEleitorCifrada { …, conteudo, salt }`.
* The two `ueword`s: both callers pass two 16-bit fields that they also wrap as `CBaseType<0,9999>`. These are
  probably zona and seção (medium confidence).

### 4.1 The web build does not run CEPESC

`CCepescCipher::vf2` (func 2681, *Encrypt*) builds a **32-byte all-zero vector** as the key and returns
`CCipheredOut(zeroKey, plaintext.conteudo[, *plaintext.infoSalt])`. The content is returned **unchanged**, and the
table, random number and public key are ignored. `CCepescCipher::vf3` (func 5171, *Decrypt*) returns
`CCipheredIn.conteudo` unchanged. Every "ciphered" CEPESC output of the simulator therefore holds plaintext plus a
zero key. Encrypt callers: `CGravadorBU` (BU envelope), `CGravadorRCSecao` (attendance, `DadosComparecimentoCifrado`)
and `CControlaArmazenamentoDeImagens`. The biometrics reader `CConversorBiometriaEleitorCifrada` (func 11419)
uses the identity Decrypt.

---

## 5. BOLETIM DE URNA (BU): where this unit is involved

This unit does not generate the BU. It supplies three pieces that the BU code (other units: `vota::CGeraBU`,
`comum::CGravadorBU`, `CGeradorBUBase`) uses:

1. **Key of the *código verificador*** (`vota::CGeraBU::StartState`, func 12110, inlined
   `comum::util::(anon)::LeChave`, util.cpp:37):
   1. path `/dsk/fi/estatico/chave/` + `cv.ber.pri` (func 1948 gives the directory);
   2. `segredo = CPolySingletonList::instance<api::IKernelHSM>().<slot 3>()`;
   3. a **local** `CSymmetricCipherFactory` object → `Create(string(segredo))` → AES-256-CBC with key
      `SHA-512(segredo)[0..32)`;
   4. `CKeyLoader loader(cifrador)`; `DeserializeFromBuffer<EntidadeChave>` (func 2280, BER; errors "Falha ao
      fazer decode BER." and "Foi lido conteúdo inválido.");
   5. `bytes = loader.DecipherKeyIfNeeded(entidade)` (func 9473);
   6. `CKey(descritor.nomeUsuario, descritor.serial, tipo → {0 secreta, 1 pública, else −1}, bytes, tagChaves)`,
      then `bytes` is wiped with zeros. `CKeyData` rejects empty bytes with 1411 "Bytes da chave vazio." (ckey.cpp:31);
   7. the first 16 bytes become the key of `comum::CCalculaCV`. `ccalculacv.cpp:74` rejects keys shorter than 16 bytes.
      This is the "código verificador" of the BU and the reports. `docs/libraries/openssl.md` §9 says those reports use SipHash.
   **In the web build this cannot work.** No class implements `api::IKernelHSM`: there is no typeinfo object
   (only the name string `N3api10IKernelHSME` @495784, used as the lookup key by `CPolySingletonList::instance`,
   func 2279) and no `CPolySingletonList::push` srcloc for it. This is a static conclusion: BU generation is not
   reached in the recorded sessions. `/dsk/fi/estatico/chave/` is not shipped either. If the path is
   reached, `instance<IKernelHSM>` throws "PolySingleton - solicitada uma instancia nao criada".
2. **CEPESC protection of the BU file** (`comum::CGravadorBU::GravaResultado`, func 11629):
   `LeChavePublica` reads an `EntidadeChave`, deciphers its `chave` with the HSM-derived AES cipher ("O arquivo {} não
   existe" / "está vazio" otherwise), then builds
   `CPlainText(zona?, seção?, tabela[1024], numeroAleatorio, chavePublica, conteudo)` (func 5168, which delegates to
   `CPlainText(0, 1, …, nullptr)`, §4) →
   `CCepescCipher::Encrypt` → `CSeguranca(idTipoArquivo 0, idCriptografia 1, idArquivoChave = out.chave)`.
   `comum::md::CEnvelopeGenerico` (func 3821) then receives that `CSeguranca` and `out.conteudo`, and the result is
   written as `ModuloEnvelopeGenerico::EntidadeEnvelopeGenerico` (`CFileASN::CodeObjectFunction`, func 2724,
   with fields `seguranca` and `conteudo`). In the simulator, `idArquivoChave` = 32 zero bytes and
   `conteudo` = plaintext (§4.1). The 1024-byte table and the rest of the envelope header are documented by the
   CGravadorBU unit.
3. **RDV**: the file is AES-256-CBC as described in §3.1.

---

## 6. Web-build specifics (summary)

* `CCepescCipher` is an identity "cipher" (§4.1).
* No `api::IKernelHSM` exists, and no key files are shipped: key loading (§5.1) and BU/RC/image/biometrics public-key reading fail if they are reached.
* `IUrna` is `api::teste::CUrnaMock`. The 128-byte crypto table that feeds the RDV HKDF is all `0x03`.
* `CTrng` uses `std::random_device` → WASI `random_get` → `crypto.getRandomValues`. OpenSSL's DRBG is not used.
  `CBlockCipher` always creates its own `CTrng`, so random AES IVs really are random.
* The application-wide `IRng` singleton is something else. `main` (func 10307) registers
  `ecourna::api::security::CPrng` built with the literal seed **0** (`invoke_iii(114 = CPrng::CPrng, obj, 0)`,
  then push `IRng`). Every `CPrng` method (funcs 9502/9500/9499/5178) re-seeds a **local** MT19937
  (multiplier 1812433253, 624 words on the stack) with that seed before drawing. So every call returns the
  **same** values. The engine is **`boost::random::mt19937`**, not `std::mt19937`: after the seeding loop
  the code runs Boost's `normalize_state()` (x[0] rebuilt from `x[396] ^ x[623]` with the constant 839999935 =
  `(0x9908B0DF << 1) | 1`, then the "all words zero → x[0] = 0x80000000" fix-up), which libc++'s `seed()` does not have.
  vf2 and vf5 read only the low byte of the stored seed (`i32.load8_u` at +4); vf3 reads all 32 bits.
  vf5 keeps the top 8 bits of each tempered output (`>> 24`). Users: `CGravadorBU::GravaResultado` (the 32-byte CEPESC `numeroAleatorio`, cgravadorbu.cpp:485),
  `CGravadorRCSecao` (32 bytes, cgravadorrcsecao.cpp:110), `CControlaArmazenamentoDeImagens`, and
  `CPoliticaExecucaoEleitor::LimpaBufferInput` (func 13564: `n = r%4+3` rounds of `emscripten_sleep(r%100+50)`).
  In this build these "random" values are constants. These functions are outside u01.

## 7. WebAssembly / Emscripten observations

* **merge-similar-functions**: 6154 is `Encrypt`+`Decrypt` of `CBlockCipher` in one body; the member-function
  target is passed as a table slot. 9505 is one of many 18-byte thunks (`CBaseError<E>` constructor = func 1011
  with a vtable argument).
* **Inlining hosts**: func 3703 carries `EncodeBase64`'s srclocs but is its caller (§3.3). `CKeyLoader`'s
  constructor only exists as the ICF body func 5172, which is shared with an unrelated converter.
* **trivial_abi**: libc++ ABI v2 makes `shared_ptr` trivially relocatable. In the `CAesCipher(…, shared_ptr<IRng>)`
  constructor, the callee decrements the parameter's refcount (the Itanium caller-destroys rule does not apply).
* **`invoke_*` around OpenSSL**: most EVP calls go through `invoke_iiiiii`/`invoke_iiii` because RAII objects
  are alive. That is why the JS hook in §3.1 could see `EVP_EncryptInit_ex`'s arguments.
* **Constructors return `this`**, but `~CAesCipher` (9486) returns void: DAE removed the unused result.
* `EVP_CIPHER_get_block_size`/`get_key_length` were inlined (LTO) as loads at `+4`/`+8` of the `EVP_CIPHER`.

## 8. Error codes used by this unit (`ESecurityError`, limits 1325–1725)

| code | where | message |
|---|---|---|
| 1325 | EncodeBase64 :247 | Vetor de dados vazio. |
| 1327 | EncodeBase64 :267 | Nao foi converter para base64. |
| 1330 / 1331 | CAesCipher ctor :54 / SetKey :70 | Chave AES de tamanho inválido. |
| 1332–1335 | Encrypt :79/:96/:100/:105 | Erro ao alocar contexto… / EncryptInit_ex falhou. / OpenSSL text / OpenSSL text |
| 1336–1339 | Decrypt :119/:141/:145/:150 | Erro ao alocar contexto… / DecryptInit_ex falhou. / OpenSSL text / OpenSSL text |
| 1344 / 1345 | CBlockCipher :57 / :71 | Sem dados para cifrar. / Sem dados para decifrar. |
| 1469 | CKeyLoader :78 | Um TSharedSymmetricCipher é necessário para decifrar chaves. |
| 1504–1507 | CCipheredOut :26/:29/:44/:47 | chave vazia. / conteúdo vazio. |
| 1513–1515 | CInfoSalt :27/:31/:35 | salt vazio. / tamanho minimo do salt deve ser 16 bytes. / conteúdo de info vazio. |
| 1517–1522 | CPlainText :89…:104 | see §4 |

Other nearby codes are outside u01: 1397–1403 (HKDF, chkdfseed.cpp), 1411 (CKeyData) and 1445 (ISymmetricCipher).
Code 1326 does not appear; it is probably the Base64 decoder, which is not linked.

## 9. Mapping table (all 17 functions of u01)

| func | size | ran | reconstructed symbol | source file |
|---|---|---|---|---|
| 1681 | 216 | | `std::vector<uebyte>::__assign_with_size<uebyte*, uebyte*>(first, last, n)`: the raw-pointer instance behind `vector::operator=` (SetKey's `m_key = key`) and `out.assign(buffer, buffer + n)` | library helper (libc++ template instance, also used outside u01). The const-iterator instance is func 1158 (not in u01) |
| 3703 | 1126 | | caller of `EncodeBase64` with the body inlined: `EncodeBase64(CEstadoGeral.hashVersoesPacotes).substr(0, 8)` | `EncodeBase64` → `src/ecourna/api/security/base64.cpp`; host function: name and file unknown (described at the end of base64.cpp) |
| 5169 | 1197 | | `ecourna::api::cepesc::CInfoSalt::CInfoSalt(const vector&, const vector&)` | `src/ecourna/api/security/cepesc/cinfosalt.cpp` |
| 6154 | 672 | ✓ | `CBlockCipher<CAesCipher,CTrng>::Encrypt` [merged with `Decrypt`] (merge-similar body) | `src/ecourna/api/security/cblockcipher.hpp` |
| 7750 | 7 | | `operator new[](size_t)` (libc++ `new.cpp`: `return ::operator new(n)`). Used for value-initialised arrays: the `make_unique<uebyte[]>` output buffer of Encrypt/Decrypt, `OSSL_PARAM[4]` (80 bytes) in `CSiphashMac::DoMac`, a 1024-byte read buffer in `CZip::DoAdd`. `vector<uebyte>(n)` allocations in the same functions call `operator new` (func 137) directly | library helper (libc++abi/libc++ `new.cpp`) |
| 9465 | 1769 | | `ecourna::api::cepesc::CPlainText::CPlainText(...)` | `src/ecourna/api/security/cepesc/cplaintext.cpp` |
| 9466 | 1198 | | `ecourna::api::cepesc::CCipheredOut::CCipheredOut(chave, conteudo, const CInfoSalt&)` | `src/ecourna/api/security/cepesc/ccipheredout.cpp` |
| 9467 | 987 | | `ecourna::api::cepesc::CCipheredOut::CCipheredOut(chave, conteudo)` | `src/ecourna/api/security/cepesc/ccipheredout.cpp` |
| 9473 | 866 | | `ecourna::api::security::CKeyLoader::DecipherKeyIfNeeded<ModuloEnvelopeChave::EntidadeChave>` | `src/ecourna/api/security/asn1/ckeyloader.cpp` |
| 9485 | 26 | | `CBlockCipher<CAesCipher,CTrng>::Decrypt` (thunk → 6154) | `src/ecourna/api/security/cblockcipher.hpp` |
| 9486 | 110 | | `ecourna::api::security::CAesCipher::~CAesCipher()` (implicit) | `src/ecourna/api/security/caescipher.hpp` (defaulted) |
| 9487 | 26 | ✓ | `CBlockCipher<CAesCipher,CTrng>::Encrypt` (thunk → 6154) | `src/ecourna/api/security/cblockcipher.hpp` |
| 9491 | 2265 | | `ecourna::api::security::CAesCipher::Decrypt` | `src/ecourna/api/security/caescipher.cpp` |
| 9493 | 2005 | ✓ | `ecourna::api::security::CAesCipher::Encrypt` | `src/ecourna/api/security/caescipher.cpp` |
| 9494 | 359 | ✓ | `ecourna::api::security::CAesCipher::SetKey` | `src/ecourna/api/security/caescipher.cpp` |
| 9495 | 632 | ✓ | `ecourna::api::security::CAesCipher::CAesCipher(mode, keySize, shared_ptr<IRng>)` | `src/ecourna/api/security/caescipher.cpp` |
| 9505 | 18 | | `exception::CBaseError<security::ESecurityError, exception::SErrorLimits{1325, 1725}>::CBaseError(code, std::string, const source_location&)` (merge-similar thunk → 1011 with vtable @1112932) | library/inlined helper (ecourna exception header, not reconstructed) |

Related functions outside u01, used in this doc: 1884/9489 (factory), 9474 (ISymmetricCipher ctor), 9484/9488
(dtors), 5173 (`CTrng::vf5`), 2681/5171 (`CCepescCipher`), 9470 (`CInfoSalt` copy ctor), 9492 (`vector::insert`),
1158 (`vector<uebyte>::__assign_with_size` for const iterators: `assign(v.cbegin(), v.cend())`), 986 (`string::assign`), 5172 (ICF `CKeyLoader` ctor), 2767 (`api::CEncryptedFile::Save`).

## 10. Open questions

* The real name and file of func 3703's host function (a `CEstadoGeral` accessor or a comum helper).
* The names of the `ESecurityError` enumerators and of `IRng`'s methods (slot 5 is used here as `Fill(vector&, n)`).
* The two `ueword` fields of `CPlainText` (zona/seção is inferred from the callers' `CBaseType<0,9999>` wrappers).
* Whether the real urna links a real `CCepescCipher`. The stub here clearly does not encrypt, but it could also be the
  non-urna build of ecourna.
* Whether func 5168 really is a delegating `CPlainText` constructor (shape and `this` return say so; no srcloc).

## 11. Fidelity review (2026-09-23)

Checked against the pseudo-code and the raw wat of all 17 functions, plus 1158, 2681, 5168, 5171, 5173, 5178,
9489, 9502/9500/9499, 9503 and `main`. Corrections made in this pass:

* The Encrypt/Decrypt output buffer is `make_unique<uebyte[]>` (func 7750 = `operator new[]`), not a
  `std::vector`. Func 7750 was listed as `std::allocator<unsigned char>::allocate`.
* The key-IV copy is an iterator-range assign with const iterators (func 1158). It is not `iv = m_key.iv`, which
  would have compiled to func 1681, as it does in `SetKey`.
* An IV of the wrong length is zero-padded or truncated to 16 bytes, with no out-of-bounds read (§2.2).
* `CPrng` is Boost's MT19937, not `std::mt19937` (§6).
* The caller of `CPlainText` is `CifrarWsq`, not `LeChavePublica`. Func 5168 is described in §4.

Re-checked at runtime by the reviewer: the RDV key/IV hook of §3.1 gives the same key, IV and 64-byte padded
plaintext, and a `Final` block of 16 × `0x10`. The resulting `rdv.dat` is byte-identical to
`analysis/runtime/memfs-after-init`. The following claims rest only on static analysis: the IKernelHSM failure
(§5.1), the constant `CPrng` output (§6) and the identity CEPESC (§4.1). None of these paths runs in the
recorded sessions.
