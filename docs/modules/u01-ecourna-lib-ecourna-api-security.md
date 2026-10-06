# u01: `ecourna::api::security` (cifra de bloco AES, Base64, carregador de chaves, objetos de dados CEPESC)

A unidade u01 cobre 17 funções wasm da biblioteca do TSE **ecourna** (`ecourna-lib/ecourna/api/security/…`,
compilada a partir do cache do Conan `/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/…`). Elas formam a
camada de criptografia simétrica de uso geral da urna:

* **`CAesCipher`** e **`CBlockCipher<CAesCipher, CTrng>`**: AES por meio de `EVP_*` do OpenSSL 3.0.17. A
  aplicação o usa para cifrar o **RDV** (*Registro Digital do Voto*, o arquivo com o registro anonimizado
  de cada voto) e para decifrar arquivos de chave.
* **`EncodeBase64`**: texto Base64 para exibição. Neste binário ele só sobrevive dentro de um chamador que imprime
  os 8 primeiros caracteres do hash dos pacotes de dados ("Dados: …").
* **`CKeyLoader::DecipherKeyIfNeeded`**: desembrulha um envelope de chave (`ModuloEnvelopeChave::EntidadeChave`) lido
  de `/dsk/fi/estatico/chave/`. O código do BU o usa para carregar a chave do *código verificador*.
* **Objetos de dados CEPESC** `CPlainText`, `CCipheredOut` e `CInfoSalt`. CEPESC é a cifração do TSE para
  arquivos de resultado, cujo nome vem do *Centro de Pesquisa e Desenvolvimento para a Segurança das Comunicações*.
  Essas classes são a entrada e a saída de `CCepescCipher`. **No build web, `CCepescCipher` não
  cifra** (§4.1).

Arquivos-fonte reconstruídos: `src/ecourna/api/security/{caescipher,cblockcipher,base64}.*`,
`src/ecourna/api/security/asn1/ckeyloader.*`, `src/ecourna/api/security/cepesc/{cinfosalt,cplaintext,ccipheredout}.*`.

Cinco das 17 funções executaram durante os votos gravados (`analysis/runtime/*.functions.tsv`): 6154, 9487,
9493, 9494 e 9495. Todas as cinco executaram durante `votaInit`, quando o RDV é salvo (§3.4).

---

## 1. Classes e como se relacionam

RTTI (`q.py cls`) e os layouts recuperados dos construtores:

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

| classe | layout (wasm32) |
|---|---|
| `CAesKey` | `+0 vector<uebyte> key`, `+12 ESymmetricCipherKeySize keySize` (padrão **256**), `+16 vector<uebyte> iv` |
| `ISymmetricCipher` | `+0 vptr`, `+4 CAesKey m_key` |
| `CBlockCipher<A,R>` | `ISymmetricCipher`, `+32 EOperationModes m_mode` (36 bytes) |
| `CAesCipher` | `+0 const EVP_CIPHER*`, `+4 mode`, `+8 shared_ptr<IRng>`, `+16 CAesKey`, `+44 int blockSize` |
| `CKeyLoader` | `+0 TSharedSymmetricCipher` (= `std::shared_ptr<ISymmetricCipher>`) |
| `CInfoSalt` | `+0 vector salt`, `+12 vector info` |
| `CPlainText` | `+0 uebyte tipoArquivo`, `+1 uebyte idCriptografia`, `+2 ueword zona?`, `+4 ueword seção?`, `+8 tabelaCriptografia`, `+20 numeroAleatorio`, `+32 chave`, `+44 conteudo`, `+56 shared_ptr<CInfoSalt>` |
| `CCipheredOut` | `+0 vector chave`, `+12 vector conteudo`, `+24 shared_ptr<CInfoSalt>` |

Enumerações (valores tirados do código; os nomes são inferidos):

* `EOperationModes`: ECB = 0, CBC = 1, CTR = 4. Os valores 2 e 3 não estão na tabela de cifras.
* `ESymmetricCipherKeySize`: o tamanho da chave em **bits**: 128, 192 ou 256.

---

## 2. `CAesCipher` e `CBlockCipher`: fluxo de controle

### 2.1 Escolha do algoritmo (func 9495, `caescipher.cpp:54`)

Um `std::map<pair<EOperationModes, ESymmetricCipherKeySize>, const EVP_CIPHER*(*)()>` global (header @1911772)
é preenchido pelo inicializador estático do arquivo, que está inlinado em `__wasm_call_ctors` (func 14478). Ele tem 9 entradas:
`{CBC, ECB, CTR} × {128, 192, 256}` → `EVP_aes_<bits>_<mode>` (funcs 7413–7422). O construtor
copia o `IRng`, procura `(mode, keySize)` e armazena `m_cipher` e `m_blockSize = EVP_CIPHER_get_block_size`.
Se o par não está na tabela, ele lança **1330 "Chave AES de tamanho inválido."**. A mesma mensagem é usada quando
o problema é o *modo*.

### 2.2 `SetKey` (func 9494, linha 70)

`if (m_cipher->key_len != key.key.size()) throw 1331 "Chave AES de tamanho inválido."`, depois `m_key = key`
(`operator=` padrão membro a membro: `vector::operator=` = func 1681, `__assign_with_size<uebyte*, uebyte*>`).
**O comprimento do IV não é verificado.** Isso não causa leitura fora dos limites: `Encrypt`/`Decrypt` copiam o IV da chave
para um vetor local que já contém 16 bytes zero, e o assign reaproveita essa capacidade. Um IV mais curto é, portanto,
**completado com zeros** silenciosamente até 16 bytes, e um mais longo é **truncado** nos seus 16 primeiros bytes.

### 2.3 `Encrypt` (func 9493, linhas 79–105)

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

### 2.4 `Decrypt` (func 9491, linhas 119–150)

É a imagem espelhada. Sem IV na chave, `iv = in[end-16 .. end)` e `ciphertext = in[0 .. end-16)`.
Nada verifica se a entrada tem pelo menos 16 bytes (`CBlockCipher` só rejeita uma entrada vazia). Com 1–15 bytes,
a cópia do IV de 16 bytes lê memória antes de `in.data()`, e então o assign do texto cifrado recebe um comprimento negativo e lança
`std::length_error("vector")` (func 1158 → func 158), e não um `CBaseError`. Os códigos de erro
1336–1339 correspondem a 1332–1335, com a mensagem "DecryptInit_ex falhou." em 1337. O padding do OpenSSL fica ativado
(PKCS#7), de modo que uma chave errada normalmente falha em `EVP_DecryptFinal_ex` com uma string de erro do OpenSSL (1339).

### 2.5 `CBlockCipher<CAesCipher, CTrng>::Encrypt/Decrypt` (funcs 9487/9485 → 6154)

```cpp
CAesCipher algoritmo(m_mode, m_key.keySize, std::make_shared<CTrng>());   // new objects on every call
if (in.empty()) throw 1344 "Sem dados para cifrar." / 1345 "Sem dados para decifrar."
algoritmo.SetKey(m_key);
algoritmo.Encrypt(in, out);   /  algoritmo.Decrypt(in, out);
```

Os dois métodos tinham corpos idênticos, exceto pelas constantes. O *merge-similar-functions* do wasm-opt os transformou
em **um único** corpo (func 6154), que recebe a função-membro como slot da tabela (6284/6285), mais o srcloc, o
código de erro e a mensagem. 9487 e 9485 são thunks de 26 bytes.

### 2.6 Quem cria as cifras (`CSymmetricCipherFactory`, fora da u01)

| entrada da factory | chave | IV | usuários |
|---|---|---|---|
| vf2 `Create(const std::string& segredo)` (func 1884) | `SHA-512(segredo)[0..32)`, 256 bits | **vazio** → IV aleatório anexado | arquivos de chave: `vota::CGeraBU::StartState` (via `CKeyLoader`), `CGravadorBU::LeChavePublica`, `CGravadorRCSecao::LeChavePublica`, `CControlaArmazenamentoDeImagens::LeChavePublica`, `CConversorBiometriaEleitorCifrada`. Cada um passa bytes de `api::IKernelHSM` (slot 3 da vtable) como `segredo` |
| vf3 `Create(const CAesKey&)` (func 9489) | fornecida | fornecido | `comum::(anon)::GetCifradorCryptoTable` (crdv.cpp:60–78, inlinada na func 7787): o **RDV** |

Ambas criam `CBlockCipher<CAesCipher, CTrng>` com **modo 1 = CBC** (a func 9489 armazena 1 em +32). Portanto, todo uso de AES
no VOTA é **AES-CBC**, de 256 bits na prática. Isso resolve a questão deixada em aberto em `docs/libraries/openssl.md` §11
("… would select `EVP_aes_256_ecb`"): a func 1884 não escolhe o modo. Quem o define é o objeto `CBlockCipher` que
a func 9489 constrói.

---

## 3. Dados gravados e lidos

### 3.1 Arquivo RDV (`/dsk/{fi,fe}/dinamico/trab1/rdv.dat`), confirmado em tempo de execução

`votaInit` (func 7840) → func 6737 (`CGeraDadosDinamicos`) → funcs 5737/5736 → func 2767. A func 2767 é
na verdade `api::CEncryptedFile::Save` (srcloc `cencryptedfile.cpp:88`; `CFile::Flush` está inlinada nela). A cadeia continua
→ `CBlockCipher::Encrypt` → `CAesCipher::Encrypt`. A chave vem de
`GetCifradorCryptoTable`: HKDF-SHA512 (`CHKDFSeed`, info `"RDV"`) produz uma semente de 128 bytes, e
**chave = seed[0..32), IV = seed[32..48)**. O IV é, portanto, **fixo**, e nada é anexado.

Verifiquei isso executando o simulador com uma cópia de `tools/run/headless.mjs` que registra os argumentos de
`invoke_iiiiii(6260 = EVP_EncryptInit_ex, …)`. A cópia foi feita em um diretório temporário; `tools/` não foi modificado:

```
AESINIT enc cipher=1641408 (= aes_256_cbc) key=c4f9eff4…495a4952 iv=568d9298776ea63bc8a58367190a4d51   (twice)
$ openssl enc -d -aes-256-cbc -nopad -K <key> -iv <iv> -in rdv.dat
30 38 02 02 09 6a …  (58-byte BER RDV)  06 06 06 06 06 06  10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
```

* O mesmo cenário gera um `rdv.dat` **idêntico byte a byte** em todas as execuções (3 execuções, mais o snapshot em
  `analysis/runtime/memfs-after-*`). Um cenário diferente gera um arquivo diferente.
* O texto claro recebe **padding duas vezes**. `CEncryptedFile::Save` adiciona ele mesmo o padding PKCS#7 (`16 - size%16` bytes com
  esse valor), e depois `EVP_EncryptFinal_ex` adiciona um segundo bloco PKCS#7 (16 × `0x10`). Um decodificador de RDV precisa remover ambos.
* No build web, a tabela de "hardware" de 128 bytes que semeia o HKDF vem de `api::teste::CUrnaMock::vf6`
  (func 8131), que é `memset(out, 0x03, 128)`. A chave do RDV pode, portanto, ser derivada de dados públicos.
  Isso é esperado em um simulador.

Cerca de 70 das 83 amostras do profiler em `CAesCipher::Encrypt` estão em `EVP_EncryptInit_ex`. Cada chamada constrói um
novo `CTrng`, um novo `CAesCipher` (busca no map) e um novo `EVP_CIPHER_CTX` com um fetch implícito de provider.

### 3.2 Envelopes de chave (`/dsk/fi/estatico/chave/*`)

`CKeyLoader::DecipherKeyIfNeeded<ModuloEnvelopeChave::EntidadeChave>` (func 9473):

```
EntidadeChave ::= SEQUENCE { tagChaves NumericString(SIZE(12)), descritor DescritorChave,
                             cifrado BOOLEAN, tipo TipoChave, chave OCTET STRING }
bytes = copy of chave
if !cifrado            -> return bytes
if !m_cifrador         -> throw 1469 "Um TSharedSymmetricCipher é necessário para decifrar chaves."  (line 78)
m_cifrador->Decrypt(bytes, out) ; return out
```

Esses arquivos de chave usam a vf2 da factory (IV aleatório anexado), então uma `chave` cifrada tem o formato `AES-256-CBC(key) ‖ IV(16)`.
Os outros leitores (`…::LeChavePublica`) fazem o mesmo inline, mas chamam `Decrypt` incondicionalmente.

### 3.3 Código Base64 "Dados" (func 3703)

`EncodeBase64` (base64.cpp:247/267) usa o codificador clássico de domínio público: o alfabeto @1112848,
`mod_table = {0,2,1}` @1112912, `output_length = 4*((n+2)/3)`, padding com `'='`. Ele lança **1325** "Vetor de dados vazio."
para entrada vazia e **1327** "Nao foi converter para base64." (sic) em caso de falha. O tamanho do buffer que ele
aloca é `4*(n/3 + n%3)`. Isso é 4 bytes maior que o necessário quando `n%3 == 2`, o que é inofensivo.

A func 3703 **não** é um `EncodeBase64` isolado. É um chamador com `EncodeBase64` inlinada. Seu argumento é
o `CEstadoGeral` (todos os chamadores passam `comum::GetEstado<CEstadoGeral>()`). Ela codifica o vetor em **+168**,
que é `EstadoGeralUrna.hashVersoesPacotes` (a func 11397 grava esse campo ASN.1 a partir de +168), e
retorna apenas os **8 primeiros caracteres** (`min(size, 8)` copiados para uma string SSO). Chamadores:
`CGeradorRelVersaoPacoteDados::vf4` (a linha do relatório "Dados:", alinhada à direita em 38 colunas), func 3059
("Versão: {}" / "Dados: {}") e func 7787. Os 8 caracteres são 48 bits do hash das versões dos pacotes.

### 3.4 Observado em tempo de execução

Apenas o caminho do RDV de §3.1 executa nas sessões gravadas (6154, 9487, 9493, 9494, 9495 durante `votaInit`).
O build web não persiste votos no RDV (`CSincronismoVotoEleitorWeb`), então o arquivo cifrado é gravado
apenas na inicialização. Decrypt, CKeyLoader, Base64 e CEPESC não executaram.

---

## 4. Objetos de dados CEPESC

| classe / func | verificações (todas lançam `CBaseError<ESecurityError>`) |
|---|---|
| `CInfoSalt(salt, info)` 5169 | 1513 "salt vazio." (27), 1514 "tamanho minimo do salt deve ser 16 bytes." (31), 1515 "conteúdo de info vazio." (35) |
| `CPlainText(tipoArquivo, idCriptografia, zona?, seção?, tabela, numeroAleatorio, chave, conteudo, infoSalt)` 9465 | 1517 tipo ≥ 3 (89), 1518 id ≥ 4 (92), 1519 "tabela de criptografia da UE vazia." (95), 1520 "número aleatório da UE vazio." (98), 1521 "chave vazia." (101), 1522 "conteúdo vazio." (104) |
| `CCipheredOut(chave, conteudo)` 9467 | 1504 "chave vazia." (26), 1505 "conteúdo vazio." (29) |
| `CCipheredOut(chave, conteudo, const CInfoSalt&)` 9466 | mesmas verificações, códigos 1506/1507 (44/47). Armazena `shared_ptr<CInfoSalt>(new CInfoSalt(copy))` |

`CInfoSalt` e os dois construtores de `CCipheredOut` primeiro copiam os membros e depois testam os **parâmetros**;
`CPlainText` primeiro move seus argumentos passados por valor e depois testa os **membros** (mesmo resultado).

`CPlainText` tem um segundo ponto de entrada, com 6 argumentos, a func 5168 (sem srcloc, fora da u01). Ela tem a forma de um
**construtor delegante**: `(this, ueword, ueword, 4 vectors by value)` → `CPlainText(0, 1, w1, w2, …, nullptr)`
(slot da tabela 6328 = func 9465), e retorna `this`. Seus chamadores são `CGravadorBU::GravaResultado` (func 11629) e
`CControlaArmazenamentoDeImagens::CifrarWsq` (srcloc ccontrolaarmazenamentodeimagens.cpp:149/152, inlinada na
func 2725, que o banco de dados nomeia segundo outra função inlinada, `LeChavePublica`). `CGravadorRCSecao`
(func 11616) chama diretamente o construtor de 9 argumentos, com um `CInfoSalt`.

Mapeamento para ASN.1 (`src/asn1/`):

* `CPlainText.tipoArquivo`/`idCriptografia` → `ModuloTiposEleitorais.Seguranca { idTipoArquivo INTEGER (0..2),
  idCriptografia INTEGER (1..3), idArquivoCD, idArquivoChave OCTET STRING }` por meio de `comum::md::CSeguranca(uebyte, uebyte,
  const vector&)`. O terceiro argumento é `CCipheredOut.chave`. Observe que `CPlainText` aceita `idCriptografia = 0`,
  que a faixa ASN.1 rejeita.
* `CInfoSalt` ↔ `DadosCifracao { chave, salt, informacaoAdicional }` (arquivo de comparecimento,
  `CGravadorRCSecao`) e `BiometriaEleitorCifrada { …, conteudo, salt }`.
* Os dois `ueword`s: ambos os chamadores passam dois campos de 16 bits que também embrulham como `CBaseType<0,9999>`. Provavelmente
  são zona e seção (confiança média).

### 4.1 O build web não executa o CEPESC

`CCepescCipher::vf2` (func 2681, *Encrypt*) constrói um **vetor de 32 bytes todo zerado** como chave e retorna
`CCipheredOut(zeroKey, plaintext.conteudo[, *plaintext.infoSalt])`. O conteúdo é retornado **inalterado**, e a
tabela, o número aleatório e a chave pública são ignorados. `CCepescCipher::vf3` (func 5171, *Decrypt*) retorna
`CCipheredIn.conteudo` inalterado. Toda saída CEPESC "cifrada" do simulador contém, portanto, texto claro mais uma
chave zerada. Chamadores de Encrypt: `CGravadorBU` (envelope do BU), `CGravadorRCSecao` (comparecimento, `DadosComparecimentoCifrado`)
e `CControlaArmazenamentoDeImagens`. O leitor de biometria `CConversorBiometriaEleitorCifrada` (func 11419)
usa o Decrypt identidade.

---

## 5. BOLETIM DE URNA (BU): onde esta unidade está envolvida

Esta unidade não gera o BU. Ela fornece três peças que o código do BU (outras unidades: `vota::CGeraBU`,
`comum::CGravadorBU`, `CGeradorBUBase`) usa:

1. **Chave do *código verificador*** (`vota::CGeraBU::StartState`, func 12110, com
   `comum::util::(anon)::LeChave` inlinada, util.cpp:37):
   1. caminho `/dsk/fi/estatico/chave/` + `cv.ber.pri` (a func 1948 fornece o diretório);
   2. `segredo = CPolySingletonList::instance<api::IKernelHSM>().<slot 3>()`;
   3. um objeto `CSymmetricCipherFactory` **local** → `Create(string(segredo))` → AES-256-CBC com chave
      `SHA-512(segredo)[0..32)`;
   4. `CKeyLoader loader(cifrador)`; `DeserializeFromBuffer<EntidadeChave>` (func 2280, BER; erros "Falha ao
      fazer decode BER." e "Foi lido conteúdo inválido.");
   5. `bytes = loader.DecipherKeyIfNeeded(entidade)` (func 9473);
   6. `CKey(descritor.nomeUsuario, descritor.serial, tipo → {0 secreta, 1 pública, else −1}, bytes, tagChaves)`,
      e depois `bytes` é apagado com zeros. `CKeyData` rejeita bytes vazios com 1411 "Bytes da chave vazio." (ckey.cpp:31);
   7. os 16 primeiros bytes se tornam a chave de `comum::CCalculaCV`. `ccalculacv.cpp:74` rejeita chaves com menos de 16 bytes.
      Este é o "código verificador" do BU e dos relatórios. `docs/libraries/openssl.md` §9 diz que esses relatórios usam SipHash.
   **No build web isso não pode funcionar.** Nenhuma classe implementa `api::IKernelHSM`: não há objeto typeinfo
   (apenas a string de nome `N3api10IKernelHSME` @495784, usada como chave de busca por `CPolySingletonList::instance`,
   func 2279) e não há srcloc de `CPolySingletonList::push` para ela. Esta é uma conclusão estática: a geração do BU não é
   alcançada nas sessões gravadas. `/dsk/fi/estatico/chave/` também não é distribuído. Se o caminho for
   alcançado, `instance<IKernelHSM>` lança "PolySingleton - solicitada uma instancia nao criada".
2. **Proteção CEPESC do arquivo do BU** (`comum::CGravadorBU::GravaResultado`, func 11629):
   `LeChavePublica` lê uma `EntidadeChave`, decifra sua `chave` com a cifra AES derivada do HSM ("O arquivo {} não
   existe" / "está vazio" caso contrário), e então constrói
   `CPlainText(zona?, seção?, tabela[1024], numeroAleatorio, chavePublica, conteudo)` (func 5168, que delega para
   `CPlainText(0, 1, …, nullptr)`, §4) →
   `CCepescCipher::Encrypt` → `CSeguranca(idTipoArquivo 0, idCriptografia 1, idArquivoChave = out.chave)`.
   `comum::md::CEnvelopeGenerico` (func 3821) recebe então esse `CSeguranca` e `out.conteudo`, e o resultado é
   gravado como `ModuloEnvelopeGenerico::EntidadeEnvelopeGenerico` (`CFileASN::CodeObjectFunction`, func 2724,
   com os campos `seguranca` e `conteudo`). No simulador, `idArquivoChave` = 32 bytes zero e
   `conteudo` = texto claro (§4.1). A tabela de 1024 bytes e o resto do cabeçalho do envelope são documentados pela
   unidade do CGravadorBU.
3. **RDV**: o arquivo é AES-256-CBC, como descrito em §3.1.

---

## 6. Particularidades do build web (resumo)

* `CCepescCipher` é uma "cifra" identidade (§4.1).
* Não existe `api::IKernelHSM`, e nenhum arquivo de chave é distribuído: o carregamento de chaves (§5.1) e a leitura de chaves públicas do BU/RC/imagens/biometria falham se forem alcançados.
* `IUrna` é `api::teste::CUrnaMock`. A tabela de criptografia de 128 bytes que alimenta o HKDF do RDV é toda `0x03`.
* `CTrng` usa `std::random_device` → `random_get` do WASI → `crypto.getRandomValues`. O DRBG do OpenSSL não é usado.
  `CBlockCipher` sempre cria seu próprio `CTrng`, então os IVs aleatórios do AES são de fato aleatórios.
* O singleton `IRng` de toda a aplicação é outra coisa. `main` (func 10307) registra
  `ecourna::api::security::CPrng` construído com a semente literal **0** (`invoke_iii(114 = CPrng::CPrng, obj, 0)`,
  depois push de `IRng`). Todo método de `CPrng` (funcs 9502/9500/9499/5178) ressemeia um MT19937 **local**
  (multiplicador 1812433253, 624 palavras na pilha) com essa semente antes de sortear. Assim, toda chamada retorna os
  **mesmos** valores. O motor é **`boost::random::mt19937`**, e não `std::mt19937`: depois do laço de semeadura,
  o código executa o `normalize_state()` do Boost (x[0] reconstruído a partir de `x[396] ^ x[623]` com a constante 839999935 =
  `(0x9908B0DF << 1) | 1`, e depois a correção "todas as palavras zero → x[0] = 0x80000000"), que o `seed()` da libc++ não tem.
  vf2 e vf5 leem apenas o byte baixo da semente armazenada (`i32.load8_u` em +4); vf3 lê os 32 bits.
  vf5 mantém os 8 bits superiores de cada saída temperada (`>> 24`). Usuários: `CGravadorBU::GravaResultado` (o `numeroAleatorio` CEPESC de 32 bytes, cgravadorbu.cpp:485),
  `CGravadorRCSecao` (32 bytes, cgravadorrcsecao.cpp:110), `CControlaArmazenamentoDeImagens` e
  `CPoliticaExecucaoEleitor::LimpaBufferInput` (func 13564: `n = r%4+3` rodadas de `emscripten_sleep(r%100+50)`).
  Neste build esses valores "aleatórios" são constantes. Essas funções estão fora da u01.

## 7. Observações sobre WebAssembly / Emscripten

* **merge-similar-functions**: 6154 é `Encrypt`+`Decrypt` de `CBlockCipher` em um único corpo; o alvo da função-membro
  é passado como slot da tabela. 9505 é um dos muitos thunks de 18 bytes (construtor de `CBaseError<E>` = func 1011
  com um argumento de vtable).
* **Hospedeiros de inlining**: a func 3703 carrega os srclocs de `EncodeBase64`, mas é seu chamador (§3.3). O construtor
  de `CKeyLoader` só existe como o corpo ICF func 5172, que é compartilhado com um conversor não relacionado.
* **trivial_abi**: a ABI v2 da libc++ torna `shared_ptr` trivialmente realocável. No construtor `CAesCipher(…, shared_ptr<IRng>)`,
  a chamada decrementa o refcount do parâmetro (a regra do Itanium de que o chamador destrói não se aplica).
* **`invoke_*` em torno do OpenSSL**: a maioria das chamadas EVP passa por `invoke_iiiiii`/`invoke_iiii` porque há objetos RAII
  vivos. Foi por isso que o hook em JS de §3.1 conseguiu ver os argumentos de `EVP_EncryptInit_ex`.
* **Construtores retornam `this`**, mas `~CAesCipher` (9486) retorna void: o DAE removeu o resultado não usado.
* `EVP_CIPHER_get_block_size`/`get_key_length` foram inlinadas (LTO) como loads em `+4`/`+8` do `EVP_CIPHER`.

## 8. Códigos de erro usados por esta unidade (`ESecurityError`, limites 1325–1725)

| código | onde | mensagem |
|---|---|---|
| 1325 | EncodeBase64 :247 | Vetor de dados vazio. |
| 1327 | EncodeBase64 :267 | Nao foi converter para base64. |
| 1330 / 1331 | ctor de CAesCipher :54 / SetKey :70 | Chave AES de tamanho inválido. |
| 1332–1335 | Encrypt :79/:96/:100/:105 | Erro ao alocar contexto… / EncryptInit_ex falhou. / texto do OpenSSL / texto do OpenSSL |
| 1336–1339 | Decrypt :119/:141/:145/:150 | Erro ao alocar contexto… / DecryptInit_ex falhou. / texto do OpenSSL / texto do OpenSSL |
| 1344 / 1345 | CBlockCipher :57 / :71 | Sem dados para cifrar. / Sem dados para decifrar. |
| 1469 | CKeyLoader :78 | Um TSharedSymmetricCipher é necessário para decifrar chaves. |
| 1504–1507 | CCipheredOut :26/:29/:44/:47 | chave vazia. / conteúdo vazio. |
| 1513–1515 | CInfoSalt :27/:31/:35 | salt vazio. / tamanho minimo do salt deve ser 16 bytes. / conteúdo de info vazio. |
| 1517–1522 | CPlainText :89…:104 | ver §4 |

Outros códigos próximos estão fora da u01: 1397–1403 (HKDF, chkdfseed.cpp), 1411 (CKeyData) e 1445 (ISymmetricCipher).
O código 1326 não aparece; provavelmente é o decodificador Base64, que não está linkado.

## 9. Tabela de mapeamento (todas as 17 funções da u01)

| func | tamanho | executou | símbolo reconstruído | arquivo-fonte |
|---|---|---|---|---|
| 1681 | 216 | | `std::vector<uebyte>::__assign_with_size<uebyte*, uebyte*>(first, last, n)`: a instância com ponteiros crus por trás de `vector::operator=` (o `m_key = key` de SetKey) e de `out.assign(buffer, buffer + n)` | helper de biblioteca (instância de template da libc++, também usada fora da u01). A instância com const iterators é a func 1158 (fora da u01) |
| 3703 | 1126 | | chamador de `EncodeBase64` com o corpo inlinado: `EncodeBase64(CEstadoGeral.hashVersoesPacotes).substr(0, 8)` | `EncodeBase64` → `src/ecourna/api/security/base64.cpp`; função hospedeira: nome e arquivo desconhecidos (descrita no final de base64.cpp) |
| 5169 | 1197 | | `ecourna::api::cepesc::CInfoSalt::CInfoSalt(const vector&, const vector&)` | `src/ecourna/api/security/cepesc/cinfosalt.cpp` |
| 6154 | 672 | ✓ | `CBlockCipher<CAesCipher,CTrng>::Encrypt` [mesclada com `Decrypt`] (corpo merge-similar) | `src/ecourna/api/security/cblockcipher.hpp` |
| 7750 | 7 | | `operator new[](size_t)` (`new.cpp` da libc++: `return ::operator new(n)`). Usado para arrays inicializados por valor: o buffer de saída `make_unique<uebyte[]>` de Encrypt/Decrypt, `OSSL_PARAM[4]` (80 bytes) em `CSiphashMac::DoMac`, um buffer de leitura de 1024 bytes em `CZip::DoAdd`. As alocações `vector<uebyte>(n)` nas mesmas funções chamam `operator new` (func 137) diretamente | helper de biblioteca (`new.cpp` da libc++abi/libc++) |
| 9465 | 1769 | | `ecourna::api::cepesc::CPlainText::CPlainText(...)` | `src/ecourna/api/security/cepesc/cplaintext.cpp` |
| 9466 | 1198 | | `ecourna::api::cepesc::CCipheredOut::CCipheredOut(chave, conteudo, const CInfoSalt&)` | `src/ecourna/api/security/cepesc/ccipheredout.cpp` |
| 9467 | 987 | | `ecourna::api::cepesc::CCipheredOut::CCipheredOut(chave, conteudo)` | `src/ecourna/api/security/cepesc/ccipheredout.cpp` |
| 9473 | 866 | | `ecourna::api::security::CKeyLoader::DecipherKeyIfNeeded<ModuloEnvelopeChave::EntidadeChave>` | `src/ecourna/api/security/asn1/ckeyloader.cpp` |
| 9485 | 26 | | `CBlockCipher<CAesCipher,CTrng>::Decrypt` (thunk → 6154) | `src/ecourna/api/security/cblockcipher.hpp` |
| 9486 | 110 | | `ecourna::api::security::CAesCipher::~CAesCipher()` (implícito) | `src/ecourna/api/security/caescipher.hpp` (padrão) |
| 9487 | 26 | ✓ | `CBlockCipher<CAesCipher,CTrng>::Encrypt` (thunk → 6154) | `src/ecourna/api/security/cblockcipher.hpp` |
| 9491 | 2265 | | `ecourna::api::security::CAesCipher::Decrypt` | `src/ecourna/api/security/caescipher.cpp` |
| 9493 | 2005 | ✓ | `ecourna::api::security::CAesCipher::Encrypt` | `src/ecourna/api/security/caescipher.cpp` |
| 9494 | 359 | ✓ | `ecourna::api::security::CAesCipher::SetKey` | `src/ecourna/api/security/caescipher.cpp` |
| 9495 | 632 | ✓ | `ecourna::api::security::CAesCipher::CAesCipher(mode, keySize, shared_ptr<IRng>)` | `src/ecourna/api/security/caescipher.cpp` |
| 9505 | 18 | | `exception::CBaseError<security::ESecurityError, exception::SErrorLimits{1325, 1725}>::CBaseError(code, std::string, const source_location&)` (thunk merge-similar → 1011 com vtable @1112932) | helper de biblioteca/inlinado (header de exceção do ecourna, não reconstruído) |

Funções relacionadas fora da u01, usadas neste documento: 1884/9489 (factory), 9474 (ctor de ISymmetricCipher), 9484/9488
(dtors), 5173 (`CTrng::vf5`), 2681/5171 (`CCepescCipher`), 9470 (ctor de cópia de `CInfoSalt`), 9492 (`vector::insert`),
1158 (`vector<uebyte>::__assign_with_size` para const iterators: `assign(v.cbegin(), v.cend())`), 986 (`string::assign`), 5172 (ctor ICF de `CKeyLoader`), 2767 (`api::CEncryptedFile::Save`).

## 10. Questões em aberto

* O nome e o arquivo reais da função hospedeira da func 3703 (um acessor de `CEstadoGeral` ou um helper do comum).
* Os nomes dos enumeradores de `ESecurityError` e dos métodos de `IRng` (o slot 5 é usado aqui como `Fill(vector&, n)`).
* Os dois campos `ueword` de `CPlainText` (zona/seção é inferido a partir dos wrappers `CBaseType<0,9999>` dos chamadores).
* Se a urna real linka um `CCepescCipher` real. O stub aqui claramente não cifra, mas também pode ser o
  build do ecourna para fora da urna.
* Se a func 5168 é de fato um construtor delegante de `CPlainText` (a forma e o retorno de `this` indicam isso; não há srcloc).

## 11. Revisão de fidelidade (2026-09-23)

Verificado contra o pseudocódigo e o wat cru de todas as 17 funções, mais 1158, 2681, 5168, 5171, 5173, 5178,
9489, 9502/9500/9499, 9503 e `main`. Correções feitas nesta passada:

* O buffer de saída de Encrypt/Decrypt é `make_unique<uebyte[]>` (func 7750 = `operator new[]`), e não um
  `std::vector`. A func 7750 estava listada como `std::allocator<unsigned char>::allocate`.
* A cópia do IV da chave é um assign de intervalo de iteradores com const iterators (func 1158). Não é `iv = m_key.iv`, que
  teria compilado para a func 1681, como acontece em `SetKey`.
* Um IV de comprimento errado é completado com zeros ou truncado para 16 bytes, sem leitura fora dos limites (§2.2).
* `CPrng` é o MT19937 do Boost, e não `std::mt19937` (§6).
* O chamador de `CPlainText` é `CifrarWsq`, e não `LeChavePublica`. A func 5168 é descrita em §4.

Reverificado em tempo de execução pelo revisor: o hook de chave/IV do RDV de §3.1 dá a mesma chave, o mesmo IV e o mesmo texto claro
de 64 bytes com padding, e um bloco `Final` de 16 × `0x10`. O `rdv.dat` resultante é idêntico byte a byte a
`analysis/runtime/memfs-after-init`. As seguintes afirmações se apoiam apenas em análise estática: a falha do IKernelHSM
(§5.1), a saída constante de `CPrng` (§6) e o CEPESC identidade (§4.1). Nenhum desses caminhos executa nas
sessões gravadas.
