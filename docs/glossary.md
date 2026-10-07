# Glossário: termos eleitorais em português e siglas do TSE

As strings, os nomes de classes e os nomes de arquivos em `vota_web_wasm.wasm` estão em português do Brasil, e
muitos são siglas do TSE (o TSE é o tribunal eleitoral federal). Esta página dá a cada termo um significado
de uma linha e, quando o termo aparece no binário, mostra onde. Cada endereço, índice de função
e contagem desta página foi produzido pelos comandos mostrados aqui (principalmente os da §0).

Os outros capítulos já explicam a maior parte desses assuntos em profundidade. Esta página apenas define as
palavras e aponta para esses capítulos:
00-provenance, MAP, [os esquemas ASN.1](data-model/asn1-schemas.md),
os capítulos do BU, os [capítulos de módulos](modules/) e os
capítulos de bibliotecas.

**Como ler a coluna "onde"**

| notação | significado | como consultar |
|---|---|---|
| `@123634` | uma string terminada em NUL no endereço 123634 (decimal) da memória linear, coluna 1 de `analysis/strings.tsv`. Algumas strings curtas (`"uf"` @168637, `"ee"` @187015, `"te"` @176379, `"o"` @142797, `"s"` @81614) são o **final de uma string mais longa** (o linker permite que `"uf"` compartilhe os bytes de `"EC_POINT_point2buf"`), por isso não estão na coluna 1; `q.py s` ainda assim as decodifica | `python3 tools/wasmmap/q.py s 123634` |
| `func 12074` | índice de função wasm, o `N` em `(func (;N;) …)` do `.wat` | `q.py f 12074` (pseudocódigo), `q.py wat 12074` (texto wasm) |
| `vota::CGeraBU` | um nome de classe C++, vindo do RTTI (`analysis/classes.tsv`) ou de uma assinatura `std::source_location` | `q.py cls 'vota::CGeraBU$'` |
| `ModuloX.Tipo` | um tipo ASN.1 reconstruído a partir do binário, em [`src/asn1/`](../src/asn1/) | abra o arquivo `.asn` |
| `uenux2/src/app/…` | um caminho de fonte original (`analysis/srcloc.tsv`, `analysis/files.tsv`) | `q.py file 'fimvotacao'` |
| `t02400ac-pu.dat` | um arquivo do sistema de arquivos do simulador (`upstream/fs/<scenario>/dsk/…`) ou um que o programa grava | `ls`, ou `analysis/runtime/memfs-*` |

`q.py` é `python3 tools/wasmmap/q.py`, executado a partir da raiz do projeto.

---

## 0. Encontrando uma palavra por conta própria

```sh
python3 tools/wasmmap/q.py grep 'zer.sima'          # regex over all strings, with the functions that use them
grep -i 'boletim' analysis/strings.tsv               # the same table, plain grep
cut -f3 analysis/classes.tsv | grep -i mesario       # class names (only classes that have RTTI)
cut -f2 analysis/files.tsv | grep -i justificativa   # original source files
node tools/run/headless.mjs --scenario municipal-t1 --keys "91001  C  12  C  "   # watch the words at run time
```

No último comando, a pausa (dois espaços) depois de `91001` é importante: logo após o quinto dígito
o programa fica em um estado transitório curto (`CConfereVotoEmCargo<…>`), em que um CONFIRMA é
rejeitado como *tecla indevida* (§6). Digitado como `"91001C  12C  "`, o primeiro `C` se perde, o
`1`/`2` destinado a Prefeito cai na tela de confirmação de Vereador, e a execução para no
pedido de Prefeito.

Três armadilhas:

1. **As strings da aplicação são Latin-1, não UTF-8.** Um padrão UTF-8 encontra **0** ocorrências:
   `LC_ALL=C grep -a -o 'zerésima' vota_web_wasm.wasm | wc -l`. O padrão `'zer.sima'` encontra **23**
   (mesmo comando), porque o `é` é o byte único `0xE9`. No binário bruto, uma letra minúscula
   + um byte `0xE0–0xFA` + uma letra minúscula ocorre 1.340 vezes; o equivalente em UTF-8
   (letra minúscula + `0xC3` + byte `0xA0–0xBA` + letra minúscula) ocorre uma vez, em @367628
   `'Turno invÃ¡lido.'` (é assim que `strings.tsv` mostra um `á` UTF-8 decodificado como Latin-1).
   As duas contagens vêm de
   `python3 -c "import re;d=open('vota_web_wasm.wasm','rb').read();print(len(re.findall(rb'[a-z][\xe0-\xfa][a-z]',d)),len(re.findall(rb'[a-z]\xc3[\xa0-\xba][a-z]',d)))"`
   (imprime `1340 1`; ocorrências sem sobreposição).
   `strings.tsv` e `q.py` decodificam Latin-1 para você, então use-os. O log de execução `logd.dat` também é
   Latin-1 (README de runtime).
2. **Endereços pequenos geram referências cruzadas falsas.** `q.py s 1544`
   (`'Código Verificador: {}.{}.{}.{}'`) lista o usuário real, func 11182 `comum::CRelUtil::DSCodigoVerificador`. Ele também
   lista as funcs 3197, 4413 e 7243 do OpenSSL e o `unixOpen` do SQLite (12912), porque nessas
   funções `1544` é apenas um inteiro (nas do OpenSSL, o número da linha do fonte passado a
   `ERR_raise`). `q.py s 8704` (`'Seção agregada: {:04}'`) lista sete funções. Seis são ocorrências
   falsas, cinco no SQLite (`sqlite3RunParser`, `resolveAlias`, …, onde 8704 é uma palavra de flags) e
   o `do_i2b` do OpenSSL. A sétima, func 12105 `vota::CGeraRelatorios::StartState`, é um usuário
   real. Um uso real de uma string de formato normalmente tem esta cara: o código guarda o início
   **e** o fim do texto, como um `std::string_view` para `std::format`: `i32.const 8704` seguido de
   `i32.const 8725` na func 12105 (8725 − 8704 = 21 caracteres), e `1544` seguido de `1575` na func 11182
   (31 caracteres). Quando um endereço estiver abaixo de cerca de 10.000, leia o código antes de confiar na
   referência.
3. **A func 7787 não é de fato `CHKDFSeed::GetSeed`.** Muitas strings (por exemplo
   @129510 `'[1] para imprimir a zerésima e seu resumo'`) são referenciadas pela func 7787, que as
   ferramentas antes mostravam como `ecourna::api::security::CHKDFSeed::GetSeed`. A func 7787 é a
   função de inicialização de 148.912 bytes; seu único chamador é `votaInit` (7840). Seu cartão `q.py f 7787`
   lista 113 registros `source_location` de 27 arquivos-fonte diferentes, porque a otimização
   em tempo de link inlinou nela cerca de vinte funções de outros arquivos (`CHKDFSeed::GetSeed` é
   uma delas). As ferramentas primeiro a nomearam a partir dos registros de `chkdfseed.cpp` no topo dessa
   lista, com `name_confidence low`. O banco de dados agora a chama de `vota::CInformacaoEleitor::Inicializar`
   (nome inferido, `medium`), de modo que `q.py s 129510` imprime `ref by 7787: vota::CInformacaoEleitor::Inicializar`.
   Veja [u02](modules/u02-ecourna-lib-ecourna-api-security.md).

### Exemplo prático: de uma sigla até instruções wasm (BEHB)

`BEHB` aparece em nomes de arquivos (`behb.dat` @60700, `behb.vsu` @17985) e na classe
`vota::CImprimindoBEHB`. O binário nunca escreve a sigla por extenso. Em vez disso, procure as palavras que você
esperaria encontrar nela:

```
$ python3 tools/wasmmap/q.py grep 'habilitados biograficamente'
  174639 'Gerando relatório de eleitores habilitados biograficamente'  <- 12105:vota::CGeraRelatorios::StartState
  376538 'Ocorreu um erro durante a geração do relatório de eleitores habilitados biograficamente na MI.'  <- 12105:…
  485744 'boletim de eleitores\nhabilitados biograficamente'  <- 12074:vota::CImprimindoBEHB::StartState
```

Portanto BEHB = **B**oletim de **E**leitores **H**abilitados **B**iograficamente, a lista impressa de
eleitores que foram admitidos pelo ano de nascimento em vez da impressão digital. O título do relatório é a terceira
string. É assim que a func 12074 a usa (`q.py wat 12074`, linhas 178–212):

```wasm
local.get 1
i32.const 56
call 137                          ;; operator new(56): heap buffer for a long std::string
local.tee 3
i32.store offset=12               ;; string.data = buffer
local.get 1
i64.const -9223371796336607184    ;; = 0x80000038_00000030: size 48; cap word 56 (bytes allocated) | long-mode bit
i64.store offset=16 align=4       ;; string.size and the cap word in one 8-byte store
local.get 3
i32.const 485784                  ;; copy the text 8 bytes at a time, last chunk first
i64.load
i64.store offset=40 align=1
…                                 ;; 485776, 485768, 485760, 485752
local.get 3
i32.const 485744                  ;; the address q.py printed
i64.load
i64.store align=1
local.get 3
i32.const 0
i32.store8 offset=48              ;; NUL terminator after the 48 characters
```

A string não é passada por ponteiro. O compilador inlinou o construtor de `std::string`, de modo que um
literal de 48 caracteres virou seis pares `i64.load`/`i64.store` (o título tem 20 + 1 + 27 = 48
caracteres, contando o `\n`). As palavras de tamanho e de capacidade seguem o layout da libc++
descrito em libcxx-core §2: a palavra de capacidade guarda o
tamanho da alocação (56 = capacidade 55 + 1 para o NUL) com o bit de sinal ligado para indicar "string longa". Quando você procurar os usuários de um
literal longo, procure também os endereços dentro dele (início + 8, + 16, …): o primeiro bloco
nem sempre é o que o código carrega primeiro.

---

## 1. Siglas em resumo

"Não escrita por extenso" significa que nem o binário nem os documentos do TSE do projeto escrevem a
expansão. Uma expansão em *itálico* também não está no binário. Ela vem do documento de formato de arquivos publicado
pelo TSE
([`bu-rdv-format-v2/README.md`](../upstream/tse-docs/bu-rdv-format-v2/README.md)), de outro
capítulo, ou é o nome padrão de um termo do setor. As demais estão escritas no binário,
ou foram montadas a partir de strings dele (o endereço é indicado).

| sigla | expansão | em uma linha | § |
|---|---|---|---|
| ADH | não escrita por extenso | ferramenta usada para acertar a data e a hora da urna (@363656 "utilize o ADH para ajustar o horário desta urna") | 11 |
| ATUE | não escrita por extenso | um tipo de aplicativo da urna (`TipoAplicativo atue(7)`); `EstadoGeralGap.identificadoATUE` | 11 |
| BEHB | Boletim de Eleitores Habilitados Biograficamente (título @485744) | lista impressa dos eleitores admitidos pelo ano de nascimento | 10 |
| BIM | Boletim de Identificação de Mesários (@66858) | lista impressa dos mesários que se registraram | 10 |
| BU | Boletim de Urna (@137222 `'Boletim de Urna foi atingido'`) | a apuração de uma urna, impressa e gravada na mídia de resultado | 10 |
| BUJ, BJust | Boletim de Justificativa (@155797 "Boletim de Justificativa Eleitoral") | lista impressa das justificativas de ausência | 10 |
| CEPESC | *Centro de Pesquisa e Desenvolvimento para a Segurança das Comunicações* ([u01](modules/u01-ecourna-lib-ecourna-api-security.md)) | o centro governamental de criptografia cujo algoritmo protege os arquivos do TSE; `ecourna::api::cepesc` | 10 |
| CV | Código Verificador (@1544; classe `comum::CCalculaCV`) | o número de verificação impresso sob cada bloco do BU | 10 |
| EG | Estado Geral (`EstadoGeralUrna`; @328306 `'EG Geral MI'`) | os arquivos de estado persistente da urna (`eg.bin`, `vota.bin`, `gap.bin`, `sa.bin`) | 11 |
| FC / FV | flash de carga (@230523 `serialFlashCarga`) / *flash de votação* | o cartão de carga e o cartão de votação; `ModuloInformacaoMidia.TipoMidia { mr(1), fc(2), fv(3) }` | 8 |
| FI / FE | *flash interna / flash externa* (o código tem `EFlashOrigem` e `LogaCopiandoArqResParaFI`) | memória flash interna e externa, `/dsk/fi` e `/dsk/fe` | 8 |
| GAP | não escrita por extenso | estado da seleção de aplicativo da urna (`gap.bin`, `ModuloEstadoGeralGap`) | 11 |
| GEDAI-UE | não escrita por extenso (veja §11) | o programa Windows do TSE que prepara as mídias da urna; `Sistema.gedai(6)` | 11 |
| HSF | Hot Swap Flash (o título `<h2>` `HOT SWAP FLASH - 02/09/2026` de `pc1hsfg.html`, cujos arquivos ficam em `D:\Aplic\SEVIN\HSF\`) | uma ferramenta Windows do TSE; não está no binário | 11 |
| HSM | *Hardware Security Module* | o chip criptográfico da urna; `api::IKernelHSM`, @7762 | 8 |
| HV | horário de verão | o horário de verão; `comum::CHV` | 12 |
| JE | Justiça Eleitoral (@155716) | o sistema da justiça eleitoral; nomes de tipos `DataHoraJE`, `DataJE` | 2 |
| MC | mídia de carga (@367431) | a mídia de carga (@8376 "Código identificação MC", @367431) | 8 |
| MI / ME | mídia interna / mídia externa (@373900, @373852) | o mesmo que FI / FE, nas mensagens (@331065, @334922) | 8 |
| MR | mídia de resultado (@137869) | a mídia removível que leva os resultados para fora da urna | 8 |
| MV | mídia de votação (@362075) | o cartão que guarda os votos; ele é transferido para uma urna de contingência (@370017) | 8 |
| PADA-UE | não escrita por extenso | sistema do TSE que produz os pacotes de dados da eleição (`Sistema.padaUE(14)`) | 11 |
| PE | processo eleitoral (@8457) | a eleição como um todo (por exemplo, as eleições gerais de 2026); `comum::CPE`, `"pe": 2400` | 4 |
| PU | parametrização da urna (inferido: o pacote `-pu` contém `EntidadeParametrizacaoUrna`) | pacote de parâmetros da urna (`t02400ac-pu.dat`); `vota::CImpressaoPU` | 10 |
| RDV | Registro Digital do Voto (módulo `ModuloRegistroDigitalVoto`) | a lista de todos os votos dados, sem vínculo com o eleitor: mantida ordenada por (tipo de voto, dígitos digitados), de modo que a ordem de votação se perde (`comum::CRdvPosicionadorVota::Posiciona`, func 11496) | 10 |
| RED | Recuperador de Dados (@7981 "Sistema Recuperador de Dados") | recupera os resultados de uma urna que falhou | 11 |
| RZE / ZE | resumo da zerésima / zerésima | `rze.dat`, `imgze.dat` (@328640 "Resumo da Zerésima MI") | 9 |
| SA | *Sistema de Apuração* (documento do TSE) | sistema de apuração para cédulas de papel ou mídias ilegíveis | 11 |
| SAVD | não escrita por extenso | a interface do serviço de assinatura/validação; `comum::IInterfaceSavd` | 11 |
| SCUE | não escrita por extenso | um nome de chave e o arquivo de configuração da carga `scueconf-t1.dat` | 11 |
| SEVIN, SEINT, SECAD, SECINP | unidades do TSE; *Seção de Voto Informatizado* para SEVIN (00-provenance) | nomes das chaves/signatários dos pacotes de dados (func 5900) | 2 |
| SIECO | não escrita por extenso | `REG AUTENTICAÇÕES SIECO` @326941, `uesieco-…-aut.dat` @60410 | 11 |
| STE | não escrita por extenso | um tipo de aplicativo (`TipoAplicativo ste(5)`). O sufixo de arquivo `-ste` é outra coisa, veja §10 | 11 |
| TPM | *Trusted Platform Module* | o chip de segurança cujo certificado identifica o gerador de mídias | 8 |
| TRE | *Tribunal Regional Eleitoral* | tribunal eleitoral estadual (@475104) | 2 |
| TSE | *Tribunal Superior Eleitoral* | o tribunal eleitoral federal; em minúsculas nos caminhos de fonte `/home/rubio/tse/…`, em maiúsculas apenas no fim de três nomes de tipo de pacote (`pacoteCandidatosTSE`…) | 2 |
| TTE | Transferência Temporária *de Eleitores* (@228762 `'TTE: Transferência Temporária'`) | eleitores transferidos para outra seção no dia da eleição | 7 |
| UE | urna eletrônica (@369448) | a própria urna (@8309 "Código identificação UE") | 8 |
| UENUX | não escrita por extenso (provavelmente UE + Linux) | o sistema Linux da urna; árvore de fontes `uenux2` | 11 |
| UF | *unidade da federação* (documento do TSE) | um estado (`ac`, `df`), `br` para nacional, `zz` para o exterior | 3 |
| VOTA | Software de Votação (@123634) | o aplicativo de votação do dia da eleição, ou seja, este programa | 11 |
| VPE | não escrita por extenso | arquivo de mídia esperado `NNNueNN.vpe` (@181199); `vpe99_*.jez` no GEDAI-UE | 11 |
| VPP | não escrita por extenso | um tipo de aplicativo (`TipoAplicativo vpp(4)`) | 11 |
| WSQ | *Wavelet Scalar Quantization* | formato de imagem de impressão digital do FBI; `wsqbio.jez`, `wsqmes.jez`, `wsqman.jez` | 10 |

---

## 2. Pessoas e instituições

| termo | significado | onde neste binário |
|---|---|---|
| **TSE** (Tribunal Superior Eleitoral) | tribunal eleitoral federal; é ele que escreve o software da urna | Nunca é uma string por si só. Em minúsculas, 1.639 dos 1.939 registros `std::source_location` apontam para baixo de `/home/rubio/tse/uenux2/…` (`grep -c '/home/rubio/tse/' analysis/srcloc.tsv`). Em maiúsculas, apenas termina três nomes de valores ASN.1 de `ModuloTiposPacotes.TipoPacote`: `pacoteCandidatosTSE(31)` @330535, `pacoteFotosCandidatosTSE(32)` @330510, `pacoteAlteracaoCandidaturaTSE(33)` @330555 (`grep -P '\t[^\t]*TSE' analysis/strings.tsv`) |
| **TRE** (Tribunal Regional Eleitoral) | um tribunal eleitoral por estado; seus técnicos dão manutenção às urnas | @475104 `'Por favor, solicite ajuda dos técnicos do TRE'` (func 11812 `vota::testeteclado::CEnviarManutencao::ProcessInput`, fluxo de teste do teclado) |
| **Justiça Eleitoral** (JE) | o ramo eleitoral do Judiciário (TSE + TREs + zonas) | @155716 `'eleitor convocado pela Justiça Eleitoral'`; ASN.1 `DataHoraJE` (`"YYYYMMDDThhmmss"`) em `ModuloTiposEleitorais` |
| **cartório eleitoral** | escritório eleitoral local de uma zona | @155402 `'Eleitor deve procurar cartório eleitoral'` |
| **Junta Eleitoral**, **junta / turma apuradora** | junta eleitoral; a junta/equipe de apuração que opera o SA | @369448 `'…envie-a para a Junta Eleitoral.'`; @225832 `juntaApuradora`, @225847 `turmaApuradora` |
| **eleitor** | quem vota | `vota::CEleitorVotando` (a sessão do eleitor), `comum::md::CEleitor` (`…/dados/md/eleitor/celeitor.cpp`), `ModuloEleitores` |
| **mesário** | quem trabalha na seção | `uenux2/src/app/comum/comparecimentomesario/`: o binário nomeia 18 arquivos `.cpp` só em `estados/` (`grep -o 'comparecimentomesario/estados/[a-z]*\.cpp' analysis/strings.tsv \| sort -u \| wc -l`; 15 deles possuem funções em `analysis/files.tsv`); tabela SQLite `comparecimento_mesario` em `uenux.db` |
| **Mesa Receptora** | a equipe de mesários da seção | @225817 `'Mesa Receptora'` |
| **operador** | a pessoa no terminal do mesário, nos termos do código | `uenux2/src/app/vota/operador/…`, `vota::CThreadOperador`; @123127 `'Operador cancelou o encerramento da votação'` |
| **candidato** | quem disputa um cargo | veja §5 |
| **SEVIN, SEINT, SECAD, SECINP** | unidades do TSE (*seções*) que assinam os pacotes de dados. SEVIN é a Seção de Voto Informatizado (00-provenance); o binário não expande as outras três | Nomes de chaves retornados por `comum::CPacoteArquivos::RetornaNomeChave(ESavdChaveValidar)` (func 5900), ao lado de `CLOGI`, `CLOGI_CERT`, `CLOGI_UPDATE` e das constantes inline `'SCUE'`, `'UE'`, `'PU'`. Os signatários dos pacotes estão listados em [asn1-schemas §6](data-model/asn1-schemas.md) |
| **CEPESC** | centro governamental de P&D em criptografia; a cifragem de arquivos do TSE leva o seu nome | `ecourna::api::cepesc::CCepescCipher` (typeinfo 1115020). **Neste build ela não cifra**, veja [u01 §4.1](modules/u01-ecourna-lib-ecourna-api-security.md) |

## 3. Lugares: onde um voto acontece

Os arquivos de cenário codificam o lugar em seus nomes: `t02400ac0000100010001-el.dat` é fase `t`,
processo eleitoral `02400`, UF `ac`, município `00001`, zona `0001`, seção `0001`, e depois o tipo
de arquivo (`-el` = eleitores). Veja [asn1-schemas §6](data-model/asn1-schemas.md).

| termo | significado | onde neste binário |
|---|---|---|
| **UF** (unidade da federação) | um estado; duas letras | `"uf": "ac"` em `upstream/fs/municipal-t1/cenario.json`; `df` e `zz` (votação no exterior, cenário `geral-zz-t1` "Votação no exterior") nos demais; `br` em `t00000br-pu.dat` (pacote nacional). `votaInit` (func 7840) lê a chave JSON `uf` @168637 com valor padrão `"ee"` @187015, que também é o marcador que a especificação do TSE usa para uma UF |
| **município** | município | @5122 `'Município inválido: {}'`; `…/dados/md/municipiozona/cmunicipio.cpp` |
| **zona (eleitoral)** | zona eleitoral, um grupo de seções sob um cartório | @6530 `'Zona inválida: {}'`; `ModuloTiposEleitorais::MunicipioZona` |
| **seção (eleitoral)** | seção eleitoral: uma urna, uma lista de eleitores, um BU | @6431 `'Seção inválida: {}'`; `ModuloLocal::SecaoEleitoral`; `comum::CGravadorRCSecao` |
| **seção agregada** | uma seção incorporada à urna de outra | @8704 `'Seção agregada: {:04}'`, usada pela func 12105 `vota::CGeraRelatorios::StartState`; suas outras seis referências são falsas (armadilha 2); `ModuloLocal.SecaoEleitoral.agregadas` |
| **local de votação** | local de votação (uma escola, por exemplo) que abriga várias seções | @4776 `'Tipo de local de votação inválido: {}'`; `0000100010001-lo.dat` (`ModuloLocal.Local`, nome montado a partir de @60623 `"{:05}{:04}{:04}-lo.dat"`) |
| **tipo de local** | tipo de local de votação | `ModuloTiposCadastro.TipoLocalVotacao { normal(1), emTransito(2), presoProvisorio(3), temporario(4) }` |
| **abrangência** | alcance de uma eleição ou de um pacote: municipal, estadual ou federal | `ModuloTiposEleitorais.TipoAbrangencia { municipal(0), estadual(1), federal(2) }`; @5508 `'Tipo de abrangência inválido: {}'` |
| **exterior** | fora do país (eleitores brasileiros que estão no exterior) | UF `zz`; @85932 `EhExterior` ("é exterior") |
| **cabina** | cabine de votação | @226226 `'Inspecione cabina e urna'` |

## 4. A eleição: processo, pleito, eleição, turno, fase

| termo | significado | onde neste binário |
|---|---|---|
| **processo eleitoral** (PE) | o evento eleitoral inteiro (por exemplo, as eleições gerais de 2026), com um id numérico | `"pe": 2400` (municipal) ou `2500` (geral) em `cenario.json`; @8457 `'Processo eleitoral: {:05}'`; `comum::CPE`, `…/md/processoeleitoral/cprocessoeleitoral.cpp` |
| **pleito** | um dia de votação de um processo; o 1º e o 2º turnos são dois pleitos | @8443 `'Pleito: {:05}'`; `…/md/processoeleitoral/cpleito.cpp`; em `municipal-t1`, pleito = 2410 |
| **eleição** | uma eleição dentro de um pleito (por exemplo, a eleição municipal de uma UF) | 2411 em `municipal-t1` (`t02411ac-ce.dat` = `ModuloEleicao.EntidadeEleicao`); `ModuloTiposEleitorais.TipoEleicao { ordinaria(0), extraordinaria(1), consultaPopular(2) }` |
| **eleição ordinária / extraordinária** | eleição regular / eleição especial (por exemplo, depois de uma anulada) | `TipoEleicao` acima |
| **eleições gerais / municipais** | eleições gerais (presidente, governador, Congresso) / eleições municipais (prefeito, câmara municipal) | rótulos de cenário `"Eleições Gerais - 1º turno"`, `"Eleições Municipais - 1º turno"` |
| **turno** | rodada de votação. O 2º turno (*segundo turno*) é uma disputa final para os cargos majoritários | `ModuloTiposEleitorais.Turno { semTurno(0), turno1(1), turno2(2) }`; @7554 `'Urna em 2º turno sem dados de pleito 2: {}'`; pacote `turno2.jez` @9375 |
| **fase** | "fase" de um conjunto inteiro de dados: `oficial` (eleição real), `simulado` (ensaio), `treinamento` (treinamento) | ASN.1 `Fase { simulado(1), oficial(2), treinamento(3) }`. Os nomes de arquivos começam com `o`, `s` ou `t`. Strings de faixa @337308 `'==============TREINAMENTO============='`, @337386 `'…SIMULADO…'`. O simulador só traz dados `t`. Veja a nota abaixo da tabela |
| **origem da configuração: oficial / comunitária** | se a configuração é para uma eleição oficial ou para uma *comunitária* (não oficial) | `ModuloProcessoEleitoral.OrigemConfiguracao { oficial(1), comunitaria(2) }`; impresso como `ORLC:LEG` / `ORLC:COM` no QR code do BU (build-a-bu) |
| **consulta popular** | referendo / plebiscito. O código trata a sua pergunta como um *cargo* | `TipoEleicao.consultaPopular(2)`, `TipoCargoConsulta.consulta(3)`; @222527 `telaVotoNuloConsulta` |
| **totalização** | soma de todos os BUs de forma centralizada (fora da urna) | @142002 `pacoteResultadoTotalizacao`; `Sistema.sistot(11)` provavelmente é o sistema de totalização |

**Nota: `fase` no wasm.** Observe dois lugares:

* `votaInit` (func 7840) lê a chave JSON `fase` @177525, com `"te"` @176379 como valor padrão.
  Ele compara o valor com `"oficial"` @157370 ou `"o"` @142797 (resultado 49 = `'1'`), depois
  com `"simulado"` @139917 ou `"s"` @81614 (resultado 50 = `'2'`). Qualquer outro valor dá 51 = `'3'`.
  Assim, o `"te"` do site significa treinamento por padrão, não porque o programa conheça `"te"`.
* `FormataFase(EUrnaFase)` (func 2828, `nomearquivo/cnomearquivo.cpp:38`) transforma `'1'..'3'` na
  letra do nome de arquivo com uma pequena tabela de consulta compactada em uma constante `i32`:

```wasm
local.get 1          ;; EUrnaFase, a char '1'..'3'
i32.const 49
i32.sub              ;; index = fase - '1'
local.tee 1
i32.const 3
i32.ge_u
if                   ;; index >= 3 -> throw "Fase inválida: {}" (@6512)
…
local.get 0          ;; the std::string being returned (through a pointer)
i32.const 7631727    ;; 0x0074736F: the bytes 'o','s','t' in little-endian order
local.get 1
i32.const 3
i32.shl              ;; index * 8
i32.const 16777208   ;; 0xFFFFF8
i32.and
i32.shr_u            ;; shift the wanted letter into the low byte
i32.store8           ;; keep that byte: 'o' (oficial), 's' (simulado) or 't' (treinamento)
```

O `EUrnaFase` do C++ (`'1'` = oficial) é numerado de forma diferente do `Fase` do ASN.1 (`1` = simulado).
Dois enums com os mesmos nomes de valores não precisam compartilhar os números, então confira cada um.

## 5. Cargos, candidatos, partidos

| termo | significado | onde neste binário |
|---|---|---|
| **cargo** | cargo em disputa (prefeito, vereador…) | `ModuloTiposEleitorais.CargoConstitucional { presidente(1) … vereador(13) }`; `…/md/processoeleitoral/ccargo.cpp`; @1810 `'Cargo {} não encontrado para eleição {}'` |
| **presidente, governador, senador, prefeito** | presidente, governador de estado, senador, prefeito | os cargos *majoritários* dos cenários |
| **deputado federal / estadual / distrital, vereador** | deputado federal / deputado estadual / deputado do Distrito Federal / vereador | os cargos *proporcionais* |
| **vice** | companheiro de chapa (vice-presidente, vice-governador, vice-prefeito) | `CargoConstitucional.vicePresidente(2)`, `viceGovernador(4)`, `vicePrefeito(12)` |
| **suplente** | substituto; os senadores são eleitos com dois | `primeiroSuplenteSenador(9)`, `segundoSuplenteSenador(10)`; @5873 `'Suplente inexistente: {}'`; @73414 `'Sem suporte a mais de 2 suplentes'`; @8210 `{candidato-com-suplentes}` |
| **majoritário / proporcional** | cargo em que o vencedor leva tudo / cargo repartido em proporção aos votos dos partidos | `ModuloTiposEleitorais.TipoCargoConsulta { majoritario(1), proporcional(2), consulta(3) }`; os estados `vota::CPedeMajoritario`, `vota::CPedeProporcional` |
| **número de dígitos** | quantos dígitos o eleitor digita | Observado em tempo de execução: Presidente 2, Governador 2, Senador 3, Deputado Federal 4, Deputado Estadual 5 (`geral-t1`); Prefeito 2, Vereador 5 (`municipal-t1`). O JSON de estado os informa como `"digits"` |
| **candidato / candidatura** | candidato / uma candidatura (um candidato a um cargo) | `ModuloCandidatos`; `…/md/candidatura/ccandidatura.cpp`; @1379 `'Candidato não encontrado: {}/{}'` |
| **apto / inapto** | (candidatura) válida / não válida; um voto digitado para um candidato *inapto* não é armazenado como voto nominal | `SituacaoCandidatura { …, inapto(3), …, apto(12), … }` (17 valores); `"apt": true` no JSON de estado (`--full`; todos os candidatos dos cenários fornecidos são aptos). O número ganha uma tela própria, @124633 `'Tela de Candidato Inapto'`, classe `vota::CCandidatoInapto` (nome RTTI `CConfereVotoEmCargo<CCandidatoInapto, (ETelaVotacao)14>`), e a verificação do RDV @222091 `'…voto nominal ({}) para candidatura inapta'` (func 11514) trata um voto nominal para uma candidatura inapta como erro |
| **partido** | partido político; seu número são os dois primeiros dígitos dos números de seus candidatos | `ModuloPartidos`, @67438 `EntidadePartidos`. Em `municipal-t1`, o candidato `91001` "Golfe" tem `"party": 91` (`node tools/run/headless.mjs --scenario municipal-t1 --full --keys "9"`) |
| **federação (partidária)** | federação de partidos que atuam como um só na eleição | `ModuloFederacoes`; @235226 `'o partido {} fazia parte das federações [{}] e [{}]'` |
| **coligação** | coligação eleitoral | **0 ocorrências** em `strings.tsv` e `classes.tsv` |
| **legenda** | o número/rótulo de um partido como alvo de voto (veja *voto de legenda*) | `"legendaDigits": 2` para Vereador (JSON de estado `--full`); @335222 `'VOTO DE LEGENDA'` |
| **votável** | qualquer coisa que possa receber um voto: um número de candidato ou um número de partido | @154709 `identificacaoVotavel`, um membro do BU do tipo `IdentificacaoVotavel { partido, codigo }`; @154695 `NumeroVotavel`, o nome de tipo ASN.1 que a especificação do TSE dá a `codigo` (referenciado a partir de `__wasm_call_ctors`, que monta as tabelas ASN.1) |

## 6. Dando um voto

Você pode observar cada tipo de voto no programa real:

```sh
node tools/run/headless.mjs --scenario municipal-t1 --keys "91C  C  B  C  "
```

Isso imprime o seguinte (resumido):

```
state: …"substate":"vota::CPedeProporcional"…"cargo":"Vereador","digits":5
key 1 -> …"vota::CPedeNominal"…"voteMode":"legendaOuNominal","legendaValida":true
key C -> …"vota::CConfereVotoEmCargo<vota::CConfirmaVotoLegenda, (vota::ETelaVotacao)10>"…"voteMode":"legenda"
pause -> …"vota::CConfirmaVotoLegenda"…
key C -> …"vota::CPedeMajoritario"…"cargo":"Prefeito","digits":2
key B -> …"vota::CConfereVotoEmCargo<vota::CMajoritarioBranco, (vota::ETelaVotacao)4>"…"voteMode":"branco"
pause -> …"vota::CMajoritarioBranco"…
key C -> {"state":"vota::CSincronismoEleitor",…"voteMode":"gravando"…}
pause -> {"state":"vota::CAguardaMensagem","done":true,…"voteMode":"fim"…}
```

Depois de digitar `91`, o programa já sabe que 91 é um partido válido (`legendaValida: true`). Ele
espera ou mais três dígitos (um voto nominal) ou CONFIRMA (um voto só no partido).

| termo | significado | onde neste binário |
|---|---|---|
| **voto nominal** | voto em um candidato, digitando o seu número completo | `vota::CPedeNominal`, `vota::CConfirmaVotoNominal`; @156611 `VotoNominal`; @441748 `'Voto nominal para o cargo '` |
| **voto de legenda** | voto só no partido: digitar o número de 2 dígitos do partido e confirmar (apenas cargos proporcionais) | `vota::CConfirmaVotoLegenda`; @156594 `legendaOuNominal`; @230592 `'    Votos de legenda'` (linha do BU); a verificação do RDV @156380 `'…voto de legenda ({}) com cargo que não é proporcional'` |
| **voto em branco** | voto em branco (a tecla BRANCO) | `vota::CMajoritarioBranco`, `vota::CProporcionalBranco`; @140660 `'Tela de Voto Branco'` |
| **voto nulo** | voto nulo: confirmar um número que não pertence a nenhum candidato nem partido. Não existe tecla NULO | `vota::CPedeNulo` (`votaproporcional/cpedenulo.cpp`); em `geral-t1`, se os dois primeiros dígitos para Deputado Estadual não correspondem a nenhum partido (`12`, `22` ou `02`), o estado muda para `CPedeNulo`, `voteMode "nulo"`, assim que o segundo dígito é digitado: `--scenario geral-t1 --keys "91C  C  12  "` |
| **cargo sem candidato** | um cargo sem nenhum candidato válido; o voto nele é registrado como nulo | BU `TipoVoto.cargoSemCandidato(5)`; RDV `nuloCargoSemCandidato(8)`; @127526 |
| **tipo de voto** (BU / RDV) | as categorias de voto nos arquivos de resultado | BU `TipoVoto { nominal(1), branco(2), nulo(3), legenda(4), cargoSemCandidato(5) }`; o `TipoVoto` do RDV tem 9 valores, acrescentando `brancoAposSuspensao`, `nuloAposSuspensao`, `nuloPorRepeticao` e os casos de "cargo sem candidato" ([src/asn1/ModuloRegistroDigitalVoto.asn](../src/asn1/ModuloRegistroDigitalVoto.asn)) |
| **CONFIRMA / CORRIGE / BRANCO** | as três teclas de comando da urna: confirmar (verde), corrigir (laranja), branco (branca); a página as desenha como `.btn-con` `#4da94f`, `.btn-cor` `#fe5100`, `.btn-bra` `#f7f7f7` em `upstream/site/css/vota-wasm.css` | teclas do headless `C`, `D`, `B`; @91180 `'CONFIRMA: habilitar'`, @91157 `'CORRIGE: não habilitar'` |
| **tecla indevida** | tecla errada (pressionada quando não é aceita) | @232379 `'Tecla indevida pressionada'` (func 1455 `vota::CVotacaoStateAudio::PlayKey`); também gravada em `logd.dat` |
| **tela** | tela. `ETelaVotacao` numera as telas de votação | observado em tempo de execução em `CConfereVotoEmCargo<…, N>`: 2, 4, 7 e 10 são as telas de *conferência* de um voto nominal, branco, nulo (`CProporcionalNulo` para Vereador `12345`, `CMajoritarioNulo` para Prefeito `12`) e de legenda (`vota::NomeTela`, func 6718: "Tela de Conferência de Voto Nominal / Branco / Nulo / Legenda"); as telas seguintes são 1, 3, 6 e 9 (08 §11 item 6) |
| **suspensão** | o mesário interrompe um eleitor que saiu sem terminar | @124993 `'Eleitor foi suspenso e não confirmou nenhum voto'`; `ModuloParametrizacaoUrna.FormaSuspenderComVoto { tornaOutrosBranco(1), tornaOutrosNulo(2), tornaNaoVotouParcial(3) }` |
| **voto confirmado** | voto confirmado (linha de log) | @235278 `'Voto confirmado para [{}]'` (func 4537) |
| **colinha (eleitoral)** | a lista em papel, do próprio eleitor, com os números dos candidatos | **apenas na página web**, não no wasm: `upstream/site/eleitor-escolhas.html` ("Sua Colinha Eleitoral"), guardada na chave `escolhas` do `localStorage` por `upstream/site/js/eleitor-escolhas.js` |

Depois de um voto, o único arquivo que muda é o log `dsk/fi/dinamico/log/logd.dat`, com linhas
Latin-1 como `1|1|Voto confirmado para [Vereador]` (README de runtime).

## 7. Identificação do eleitor, comparecimento, justificativa

| termo | significado | onde neste binário |
|---|---|---|
| **título de eleitor** | documento de inscrição do eleitor; o seu número (*número de inscrição eleitoral*, 12 dígitos) identifica o eleitor | @82819 `'Título de eleitor'`; `ecourna-lib/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp`; `numero_titulo BIGINT` em `uenux.db`; exemplo `XXXXXXXXXXXX` de `geral-t1` ([u03](modules/u03-uenux2-src-app-comum-dados.md)) |
| **identificador do eleitor** | qual número identifica um eleitor | `ModuloTiposEleitorais.TipoIdentificadorEleitor { numeroInscricaoEleitoral(1), numeroCPF(2), numeroLivre(3) }`; o `CHECK(tipo_identificador IN (1,2,3))` em `uenux.db` |
| **CPF** | o número nacional de contribuinte, um id alternativo do eleitor | `ecourna-lib/ecourna/app/dados/cnumerocpf.cpp` |
| **habilitação / habilitar** | o mesário "habilita" a urna para um eleitor depois de identificá-lo | @138039 `'Eleitor foi habilitado'` (func 7481 `vota::CAguardaMensagem::ProcessMessage`, também em `logd.dat`); @123014 `'Eleitor não habilitado para votação'` |
| **habilitação biométrica** | habilitado por correspondência da impressão digital | @6831 `'Habilitação biométrica: {}'`; campo do QR `HBBM` @440139 |
| **habilitação biográfica** | impressão digital não reconhecida, por isso o eleitor é habilitado depois de informar o ano de nascimento | @6858 `'Habilitação biográfica: {}'`; @338785 `'Digite o ANO de nascimento:'`; campos do QR `HBBG` @440211 e `HBSB` @440310 (qrcode) |
| **biometria / digital / dedo** | biometria / impressão digital / dedo | @3799 `'Mais de dez dedos na biometria do eleitor: {}'`; `ModuloEleitores.ComposicaoBiometria { foto(1), dedos(2), fotoDedos(3) }`; `TipoDedo` (10 dedos, mais `naoIdentificado(0)`). O leitor é simulado por `simulador::CFingerPrepareSimulador` |
| **terminal do mesário / terminal do eleitor** | a unidade com teclado do mesário / a unidade do eleitor (tela e teclado) | @365472 `'Siga as instruções no terminal do mesário.'`; @82666 `'Instruções no terminal do eleitor'`; threads `vota::CThreadOperador` e `vota::CThreadEleitor` |
| **comparecimento** | comparecimento / presença | `ModuloResultadoUrnaCadastro.SituacaoComparecimentoEleitor { faltou(1), naoVotou(2), votou(3), semCargoParaVotar(4) }` |
| **eleitores aptos** | eleitores registrados na seção | @445790 `'Eleitores aptos                   {:04}\n'`; campo do QR `APTO` |
| **faltosos** | eleitores que não compareceram | @66128 `'Eleitores faltosos'` (func 12110 `vota::CGeraBU::StartState`) |
| **impedido / impedimento** | eleitor impedido de votar aqui, e o motivo | `ModuloImpedidos.TipoImpedimento` (15 valores, por exemplo `suspenso`, `semIdadeMinima`, `militarEmServico`); @122762 `'está impedido de votar nesta seção'`; arquivo `-imp.dat` |
| **justificativa (eleitoral)** | um eleitor fora do seu domicílio declara por que não pode votar, em qualquer urna | @155797 `'Boletim de Justificativa Eleitoral'`; `comum::CJustificador::Justifica`; tabela SQLite `registro_justificativa (numero_titulo, ano_nascimento)` |
| **TTE** (transferência temporária de eleitores) | eleitor transferido para outra seção nesta eleição | @228762 `'TTE: Transferência Temporária'` (func 11908 `vota::CImpressaoListaEleitores::StartState`); `TipoTransferenciaTemporaria` (9 motivos); arquivo `-tte.dat` |
| **voto em trânsito** | votar fora da sua seção de origem, mediante pedido antecipado | @126224 `'Voto em Trânsito'`; `TipoTransferenciaTemporaria.votoEmTransito(1)` |
| **preso provisório** | detento ainda não condenado, que continua votando | @130651 `'Preso provisório'`; `TipoLocalVotacao.presoProvisorio(3)` |
| **acessibilidade / áudio** | votação acessível; a urna lê as telas em voz alta pelos fones de ouvido | @192094 `'eleitor pediu acessibilidade'`; `ModuloEleitores.NecessidadeEspecial.necessitaAudio(2)`; a voz Letícia-F123 do RHVoice (rhvoice) |
| **caderno de votação** | a lista impressa de eleitores que os eleitores assinam | @192272 `'Assinar o caderno de votação antes de'`; @122981 `'Não assinar o caderno de votação'` |
| **registro de mesários** | os mesários se identificam (título + impressão digital) na abertura e no encerramento | EstadoVota `registromesarioinicial(6)` / `registromesariofinal(9)`; `comum::CEncerraRegistroMesarios` |

## 8. A máquina, suas memórias e suas mídias

| termo | significado | onde neste binário |
|---|---|---|
| **urna (eletrônica)**, **UE** | a máquina eletrônica de votação | @8309 `'Código identificação UE       {:08}'` (funcs 1942, 5591); `ModuloTiposResultadosEcoUrna::Urna` |
| **modelo** | modelo de hardware, por ano | `ModuloTiposEcoUrna.ModeloEquipamento { tpm20(2), ue2013(13), ue2015(15), ue2020(20), ue2022(22) }` |
| **flash interna (FI) / mídia interna (MI)** | a memória flash embutida da urna | caminho `/dsk/fi/` @358629. `comum::CLogComum::LogaCopiandoArqResParaFI` (func 5878) registra @235731 `'Copiando arquivo de resultado para MI: [{}]'`, o que mostra que MI = FI. @334922 `'FALHA NA MI: SUBSTITUA A URNA'` (troca-se a urna, não a memória) |
| **flash externa (FE) / mídia externa (ME)** | o cartão flash removível da urna | caminho `/dsk/fe/` @358702. `LogaResultadoCopiadoResFE` (func 3828) registra @235775 `'…para ME: [{}]'`. @331065 `'FALHA NA ME: SUBSTITUA A ME'` |
| **mídia de votação (MV)** | o cartão que guarda os dados dos votos. Em caso de falha, ele é levado para uma urna de contingência | @362075 `'…substitua a mídia de votação.'`; @370017 `'Desligue a urna, insira uma mídia de votação usada de outra urna que apresentou problema…'`; `CUrna::ValidaCriacao` (func 5862) verifica @319987 `'Serial da MV inválido ['`. A thread de monitoramento informa a MV ao lado da MI (@235442/@235494, func 10226), então a MV muito provavelmente é o cartão externo visto pelo seu papel |
| **flash de votação (FV)** | o cartão de votação, como o tipo de mídia ASN.1 o chama (as mensagens dizem MV) | `TipoMidia.fv(3)`; a descrição da mídia do cenário é `infomidia-fv-1-t.dat` (`ModuloInformacaoMidia.InformacaoMidia`) |
| **flash de carga (FC) / mídia de carga (MC)** | o cartão usado para carregar os dados da eleição nas urnas | `TipoMidia.fc(2)`; @8376 `'Código identificação MC       {:.8}'`; @367431 `'Serial da mídia de carga inválido.'` (func 11423 `comum::asn::CConversorDadosDisponiveisCarga::DoDesconverte`); @320011 `'Serial da MC inválido ['` (func 5670 `comum::md::CCarga::ValidaCriacao`) |
| **mídia de resultado (MR)** | mídia removível que leva os resultados até o ponto de transmissão | `TipoMidia.mr(1)`; caminho `/dsk/mr/` @358550; @137869 `'Retire a mídia de resultado'`; estado `vota::CRetirarMR`; `mr.ver` @86764 |
| **mídia de aplicação** | mídia com o software do aplicativo | @362203 `'…substitua a mídia de aplicação.'` |
| **estático / dinâmico** | dados estáticos (pacotes da eleição carregados) / dados dinâmicos (estado gravado durante o dia) | `/dsk/fi/estatico/…` (arquivos do cenário), `/dsk/fi/dinamico/…` (`eg.bin`, `log/logd.dat`); `/dsk/fi/estatico/chave/` @358678 guarda chaves |
| **trab1 / trab2** | dois diretórios de "trabalho" para os arquivos de estado | `analysis/runtime/memfs-after-init/dsk/fi/dinamico/trab1/` contém `gap.bin`, `sa.bin`, `vota.bin`, `rdv.dat`, `uenux.db` e os seus `.vsu`; depois de `votaInit`, `trab2/` contém cópias dos três arquivos `.bin` e dos arquivos `.vsu`, mas não `rdv.dat` nem `uenux.db`. `eg.bin` fica um nível acima, em `dinamico/` |
| **serial** | número de série de uma mídia | `serialv.dat` @60380, lido por `votaInit`; no simulador ele contém `ABCDDCBA` |
| **bateria interna / externa** | bateria interna / bateria externa | @136713 `'Tempo limite de uso de bateria interna inválido'` (`vota::CAjusteInicial::ValidaTemposDesligamento`, inlinada na func 7160 `vota::CAjusteInicial::StartState`); `comum::util::CMonitoraAlimentacao` (`comum/util/cmonitoraalimentacao.cpp`) |
| **urna de contingência** | urna reserva que substitui uma quebrada na mesma seção | @229763 `'Urna de contingência'` (func 1543); `ModuloTiposResultadosEcoUrna.TipoUrna { secao(1), contingencia(3), contingenciaSecao(4), contingenciaEncerrandoSecao(6) }`; `EstadoGeralUrna.TipoUrnaOperacao` |
| **HSM** (hardware security module) | chip criptográfico que guarda os segredos da urna | @7762 `'Falha ao enviar ação ao HSM. {}'` (func 5889 `comum::EnviarAcaoHSM`); interface `api::IKernelHSM`. A chave do código verificador é envolvida com um segredo do HSM (codigo-verificador) |
| **TPM** (trusted platform module) | chip de segurança na máquina que gera as mídias | @327351 `serialCertificadoTPM`, um membro de `ModuloTiposEcoUrna.IdentificadorGeradorMidia { nome, serialCertificadoTPM, serialInstalacao }`; no `infomidia` do simulador o serial do TPM é todo zeros ([asn1-schemas §7](data-model/asn1-schemas.md)) |

## 9. O dia da eleição, estado por estado

As próprias enumerações da urna dão a linha do tempo ([src/asn1/](../src/asn1/)):

* `ModuloEstadoGeralUrna.EstadoUrna { carregando(1), carregada(2), testada(3) }`: sendo carregada,
  carregada, testada.
* `ModuloEstadoGeralVota.EstadoVota`: `inicial(0)`, `gerabasedinamica(1)`, `aguardahorazeresima(2)`,
  `gerarze(3)`, `zeresimagerada(4)`, `zeresimaimpressa(5)`, `registromesarioinicial(6)`, `votar(7)`,
  `fimaquisicaovotos(8)`, `registromesariofinal(9)`, `gerarbu(10)`, `gerarrelatorios(11)`,
  `imprimirbu(12)`, `gravarresultados(13)`, `copiaresultadosmr(14)`, `encerrada(15)`,
  `exibealertadesligamento(16)`.
* `ModuloEstadoGeralVota.EstadoEncerramento`: `inicial(0)`, `imprimirobrigatoriabu(1)`, `retirarmr(2)`,
  `fimdostrabalhos(3)`.

| termo | significado | onde neste binário |
|---|---|---|
| **carga** | carregamento dos dados da eleição em uma urna, dias antes da eleição | `ModuloTiposEcoUrna::Carga`; `…/md/correspondencia/ccarga.cpp`; @8983 `'Data da carga               {:.10}'`; `dadoscarga.dat` @60735 |
| **código (de identificação) da carga** | id de uma carga. Ele é impresso nos relatórios e tem 24 dígitos no QR code (`IDCA:`) | @230358 `'Código de identificação da carga'`; @230329 `'Histórico de código de carga'` |
| **correspondência** | o vínculo entre uma urna e a sua seção depois da carga | `…/dados/md/correspondencia/`; `tabcorr.dat` @60601 |
| **inspeção** | um mesário inspeciona a cabina e a urna; o terminal do mesário a inicia e espera que ela termine | @226226 `'Inspecione cabina e urna'` (func 10676 `vota::CPedeIdentidade::ProcessTick`); estados `vota::CInspecionaUrna`, `vota::CAguardaInspecao`, `vota::CConfirmaInspecionada`; @232454 `'Inspeção da urna terminada'` |
| **zerésima** | relatório impresso antes do início da votação, mostrando todos os candidatos com zero votos | @336236 `'Quer imprimir a zerésima?'`; `vota::CImprimindoZeresima`, `vota::CGeraZeresima`; envelope `envelopeZeresimaImpressa(6)`; arquivos `imgze.dat` @60670 e `rze.dat` (o *resumo*) @60662 |
| **abertura / início da votação** | abertura da votação | `…/vota/eleitor/iniciovotacao/` |
| **fim da aquisição de votos** | o ponto a partir do qual nenhum voto é mais aceito | EstadoVota `fimaquisicaovotos(8)` |
| **encerramento (da votação)** | encerramento da votação | @123171 `'Encerramento da votação'`; @77708 `'Encerramento só pode ser solicitado após as {} horas'`; `…/vota/eleitor/fimvotacao/` |
| **fim dos trabalhos** | fim do trabalho da seção (depois que a MR é retirada) | `EstadoEncerramento.fimdostrabalhos(3)` |
| **emissão / reemissão** | impressão / reimpressão (de um relatório) | @227344 `'Emissão de zerésima'`, @227364 `'Reemissão da zerésima'` |
| **simulado / treinamento** | modos de ensaio / de treinamento. O id do aplicativo da urna os codifica | `ModuloEstadoGeralGap.UrnaAplicativo { apvotatreinaeleitor1(0), apvotatreinamesario1(2), apvotasimula1(4), apvotaoficial1(6), apapuracaotreina1(8), … }` |

## 10. Saída: relatórios, arquivos de resultado e a sua proteção

A tabela do próprio TSE com os arquivos de saída (sufixo, conteúdo, qual sistema o grava) está em
[`upstream/tse-docs/bu-rdv-format-v2/README.md`](../upstream/tse-docs/bu-rdv-format-v2/README.md).
Os nomes dos arquivos de saída seguem `fpppppuuMMMMMZZZZSSSS-suf.ext`. O wasm os monta a partir de
`'{:c}{:05}{}{:05}{:04}{:04}-'` @378043 (funcs 1164, 3798), e pega os sufixos (`rdv.dat`,
`imgbu.dat`, `rdvred.dat`, `imgze.dat`…) da tabela em `comum::CArquivosResultado::operator[]`
(func 347).

| termo | significado | onde neste binário |
|---|---|---|
| **BU** (boletim de urna) | a apuração da urna para a sua seção, impressa várias vezes e gravada na MR | `ModuloBoletimUrna.EntidadeBoletimUrna`; `vota::CGeraBU`, `comum::CGravadorBU`; `bu.dat`, `imgbu.dat` @60400 (imagem do BU impresso); `busa.dat`, `imgbusa.dat` @60709 quando o SA o grava. Veja build-a-bu |
| **via / cópia do BU** | cópia impressa; os parâmetros definem quantas são obrigatórias (*obrigatórios*) e quantas extras (*adicionais*) podem ser impressas | @226157 `'Emitir mais cópias do boletim de urna'`; `vota::CVerificaQtdBUsAdicionais`; @66511 `'BUs do RED obrigatórios'`, @72090 `'BUs do RED adicionais'` |
| **QR code do BU** | os dados do BU como QR codes no rodapé da impressão | @322396 `'Imprimir QR Code no BU'`; `comum::CGeradorBUQRCode`; veja qrcode |
| **código verificador** (CV) | número de verificação impresso sob cada bloco do BU; um MAC SipHash-4-6 com chave | @1544 `'Código Verificador: {}.{}.{}.{}'` (func 11182); MAC na func 9498 `CSiphashMac::DoMac`; veja codigo-verificador |
| **RDV** (registro digital do voto) | todos os votos, mantidos ordenados por (tipo de voto, dígitos digitados) para que a ordem de votação se perca e nenhum voto possa ser ligado a um eleitor | `ModuloRegistroDigitalVoto`; `comum::CRdv`, `comum::CRdvPosicionadorVota` (*posicionador* = a parte que escolhe a posição: uma busca binária, `upper_bound`, func 11496); `rdv.dat` @60392 (80 bytes depois de `votaInit`), `rdvred.dat` @60689 quando o RED o grava |
| **BIM** | boletim de identificação de mesários: lista dos mesários registrados | @66858 (func 12105); `vota::CImprimindoBim`; `bim.dat` @60646 |
| **BEHB** | boletim de eleitores habilitados biograficamente | @485744 (func 12074); `behb.dat` @60700 (exemplo prático na §0) |
| **BUJ / BJust** | boletim de justificativa | `comum::CGeradorBUJ::GeraBUJ`, `vota::CImprimirBJust`; `buj.dat` @60654 |
| **PU** (parametrização da urna) | pacote de parâmetros da urna (quantas cópias do BU, quando a zerésima pode ser impressa…) | `t00000br-pu.dat`, `t02400ac-pu.dat` (`ModuloParametrizacaoUrna.EntidadeParametrizacaoUrna`); `vota::CImpressaoPU::StartState` (func 11902) imprime parâmetros como @66511 `'BUs do RED obrigatórios'` |
| **relatório** | relatório | `vota::CGeraRelatorios`, `uenux2/src/app/comum/relatorios/` |
| **log** | o log de eventos da urna | `logd.dat` @60680 (texto Latin-1), empacotado como `log.jez` @9209; `comum::CLogComum`, `vota::CLogVota` |
| **jufa.dat** | TSE: "registro de comparecimento de eleitores e mesários" | `jufa.pk1` @353405 é a chave pública carregada por `comum::CGravadorRCSecao::LeChavePublica` (inlinada na func 11616, `CGravadorRCSecao::GravaResultado`); essa classe grava os dados de comparecimento envolvidos em CEPESC ([u01](modules/u01-ecourna-lib-ecourna-api-security.md)) |
| **wsqbio / wsqman / wsqmes** | impressões digitais de eleitores habilitados por biometria / habilitados manualmente / de mesários (imagens WSQ) | @9187, @9198, @9176; `comum::CGravadorWSQ`; veja compression §2 |
| **hash / hashes** | resumos dos arquivos gravados junto com os resultados | `comum::CGravadorHashes`; @65529 `hashesArquivos`; `hash.dat` na tabela do TSE |
| **mr.ver** | versões dos pacotes ASN.1 na MR | @86764 |
| **assinatura** | assinatura digital | `comum::CAssinador::AssinaArquivosResultado`; assinaturas de pacotes `*.vsc` (algoritmo `cepesc`); assinaturas de resultados `vota.vsc`, `red.vsc` @207089, `sa.vsc` @207097 |
| **.vsu** | assinatura de um arquivo de estado dentro da urna | `eg.vsu`, `vota.vsu`, `rdv.vsu`… (@17876–@18012). Neste build cada uma contém `'assinatura simulada para vota_web_wasm'` @149594 (func 11733) |
| **.vst** | arquivos de assinatura ao lado dos pacotes de software na lista do GEDAI-UE | `vota_apl.vst`… em `pc1geda.html`; stub de 0 bytes `upstream/fs/vota_web_wasm/etc/avetc.vst` |
| **.jez** | um arquivo ZIP | veja compression §2 |
| **.pid / pacote** | cabeçalho de pacote / pacote: os dados da eleição são entregues como pacotes assinados | `ModuloTiposEleitorais.CabecalhoPacote`; `ModuloTiposPacotes.TipoPacote` (`pacoteEleitores(1)`, `pacoteCandidatosTRE(7)`…); decodificado à mão em [asn1-schemas §6.1](data-model/asn1-schemas.md) |
| **-ste.dat** | *situações das eleições*: quais eleições estão ativas, suspensas ou canceladas (não é o aplicativo STE) | `t02410ac-ste.dat` = `ModuloSituacoesEleicoes.EntidadeSituacoesEleicoes`; `TipoSituacaoEleicao { ativa(0), suspensa(1), cancelada(2), excluida(3) }` |
| **envelope** | invólucro assinado genérico em torno de um arquivo de resultado | `ModuloEnvelopeGenerico.TipoEnvelope { envelopeBoletimUrna(1), envelopeRegistroDigitalVoto(2), envelopeBoletimUrnaImpresso(4), envelopeImagemBiometria(5), envelopeZeresimaImpressa(6) }` |
| **CEPESC** | a cifragem do TSE para os arquivos de resultado | `ecourna::api::cepesc::{CCepescCipher, CPlainText, CCipheredOut, CInfoSalt}`; uma função identidade neste build ([u01 §4](modules/u01-ecourna-lib-ecourna-api-security.md)) |
| **chave** | chave | `/dsk/fi/estatico/chave/` @358678; `ModuloEnvelopeChave.EntidadeChave`; `cv.ber.pri` (codigo-verificador) |

## 11. Software e sistemas

| termo | significado | onde neste binário |
|---|---|---|
| **VOTA** (Software de Votação) | o aplicativo de votação: o que roda no dia da eleição em cada urna | `main` (func 10307) chama `api::CApplication::InitApplication` (func 11159) com `"VOTA Web"` @221349, `"Software de Votação"` @123634 e `"10.23.0.1 - DESENVOLVIMENTO"` @326597. `'VOTA'` @517095 é o valor `ORIG:` do QR code do BU. `q.py s 517095` não mostra nenhum usuário, porque a func 11242 `comum::CGeradorBUQRCodeVota::PreencheCabecalho` o passa como uma constante `std::string_view` compactada, `17180386279` = `4 << 32 \| 517095` (tamanho 4, endereço), ao lado de `"RED"` @517100 (qrcode). `TipoAplicativo.vota(1)` |
| **versão** | versão do software; as versões reais têm um nome | este build `10.23.0.1 - DESENVOLVIMENTO`; os BUs de exemplo de 2024 dizem `9.29.0.1 - Kayapó` (00-provenance), uma versão de pré-lançamento usada na fase simulada (FASE:S, junho de 2024); as urnas da eleição de outubro de 2024 rodaram `9.30.0.0 - Tupiniquim` (dados das urnas de 2026: [investigation/LEIAME.md](../investigation/LEIAME.md), achado E10) |
| **SA** (Sistema de Apuração) | sistema de apuração usado com cédulas de papel ou quando a MR não pode ser lida | `TipoAplicativo.sa(2)`; `sa.bin` = `ModuloEstadoGeralSA` (estados como `apurarmajoritaria`, `buproporcionalpedepartido`); `TipoApuracao { totalmenteManual(1), totalmenteEletronica(2), mistaBU(3), mistaMR(4) }`; @322654 `'BU SEÇÃO SA INT'` |
| **cédula** | cédula de papel | @228360 `cedula`; `ModuloRegistroDigitalVoto.OrigemVotosSA { cedula(1), rdv(2), bu(3) }` |
| **apuração** | contagem (de uma seção, pelo SA) | `ModuloTiposResultadosEcoUrna.MotivoApuracao*` (motivos como `urnaComDefeito`, `indisponibilidadeFlashContingencia`) |
| **RED** (Recuperador de Dados) | sistema que recupera os resultados da memória de uma urna que falhou | @7981 `'Sistema Recuperador de Dados\n{}'` (func 12105); `TipoAplicativo.red(3)`; `ORIG:RED` nos QR codes; `red.vsu` @17977 |
| **GAP** | (não escrita por extenso) o estado de seleção de aplicativo da urna: qual aplicativo roda agora e qual rodou antes | `gap.bin` @147746 = `ModuloEstadoGeralGap.EstadoGeralGap { appId, appAnteriorId, executadoRED, identificadoATUE, audio, … }`; @122347 `'assinatura EG Gap'` |
| **EG** (estado geral) | estado geral da urna | `eg.bin` @147766 = `ModuloEstadoGeralUrna`; `vota.bin`, `sa.bin`, `gap.bin` são as partes por aplicativo; @328306 `'EG Geral MI'` |
| **VPP, ATUE, ADH, STE** | outros aplicativos da urna. O binário só tem os seus nomes curtos | `ModuloInformacaoMidia.TipoAplicativo { vota(1), sa(2), red(3), vpp(4), ste(5), adh(6), atue(7) }`; strings `'vpp'` @92414 e `'adh'` @161567 (func 1164), `'atue'` @172328. ADH é a ferramenta que acerta o relógio (@363656) |
| **GEDAI-UE** | programa Windows/Qt 6 do TSE que prepara as mídias da urna (o seu nome completo não está no binário) | `Sistema.gedai(6)`, string `'gedai'` @159427; lista de hashes `pc1geda.html` (00-provenance) |
| **PADA-UE** | sistema do TSE que exporta os pacotes de dados que a urna carrega | `Sistema.padaUE(14)`, a `origem` de todo `.pid`; a descrição do cenário padrão da página diz "Base de treinamento gerada a partir dos arquivos Pada-UE." (`upstream/site/index.html`, linha 46) |
| **Sistema** (enum ASN.1) | os sistemas do TSE que podem produzir um arquivo | `ModuloTiposEleitorais.Sistema { configurador(1), intercad(2), candidaturas(3), gedai(6), urnaEletronica(7), simulador(8), parametrizador(9), geradorDeBases(10), sistot(11), seweb(12), simon(13), padaUE(14) }` |
| **UENUX / uenux2** | o sistema Linux da urna; `uenux2` é a árvore de fontes dos seus aplicativos | `/home/rubio/tse/uenux2/src/app/{vota,comum}/…`, `src/api/…`; @320989 `versaoUENUX`; `uenux.db` @221385 (SQLite, sqlite) |
| **ecourna** | biblioteca C++ do TSE compartilhada pelos aplicativos da urna (dados ASN.1, criptografia, E/S) | compilada como pacote Conan: 300 registros srcloc sob `/home/rubio/.conan2/…/ecourna/…`; namespaces `ecourna::api::…`, `ecourna::app::dados::…` |
| **SAVD** | (não escrita por extenso) o serviço ao qual o aplicativo pede para assinar e validar arquivos e para enviar ações ao HSM | @8013 `'Falha ao validar assinatura UE … Erro SAVD: ({})'`; `comum::IInterfaceSavd`; o build web instala `(anonymous namespace)::CWasmSavd` (vtable 1526688, armazenada por `main`) |
| **SCUE** | (não escrita por extenso) aparece como nome de chave e na configuração da carga | constante inline `'SCUE'` na func 5900; `scueconf-t1.dat` (lista `"pacoteunico.sh"`, [asn1-schemas §6](data-model/asn1-schemas.md)) |
| **SIECO** | (não escrita por extenso) um registro de autenticações e certificações nas mídias | @326941 `'REG AUTENTICAÇÕES SIECO'`, @326965 `'REG CERTIFICAÇÕES SIECO'`, `sieco-dados/` @358506 |
| **VPE** | (não escrita por extenso) um tipo de arquivo que o validador de mídias espera | `'#regex#[0-9]{3}ue[0-9]{2}\.vpe'` @181199 em `comum::impl::(anonymous)::ValidaConteudo` (func 5572, `validamidia/cvalidamidia.cpp`); `vpe99_{ofi,sim,tre}.jez` em `pc1geda.html` |
| **HSF** (Hot Swap Flash) | ferramenta Windows do TSE, listada em `pc1hsfg.html` | não está no binário |
| **simulador** | o simulador de treinamento; a camada de mock substitui o hardware | `uenux2/mock/app/simulador/wasm/`; `simulador::CWasmThread`, `simulador::CFingerPrepareSimulador`; gerador de mídias `"simulador-votacao-ng"` |
| **manutenção** | manutenção (pelos técnicos do TRE) | `vota::testeteclado::CEnviarManutencao` |

## 12. Lendo identificadores do TSE

Os nomes de classes são locuções verbais ou nominais em português, prefixadas com `C` (classe) ou `I`
(interface). Se você consegue ler estas palavras, consegue ler os nomes:

| palavra em um nome | significado | exemplos (`cut -f3 analysis/classes.tsv`) |
|---|---|---|
| `Pede…` | pede (entrada) | `vota::CPedeNominal`, `CPedeProporcional`, `CPedeMajoritario`, `CPedeNulo`, `CPedeIdentidade`, `CPedeDigital` |
| `Confere…` | confere (uma tela transitória) | `vota::CConfereVotoEmCargo<CConfirmaVotoNominal, (ETelaVotacao)2>` |
| `Confirma…` | confirma | `vota::CConfirmaVotoLegenda`, `CConfirmaImpressaoZeresima` |
| `Gera… / Gerador…` | gera | `vota::CGeraBU`, `vota::CGeraRelatorios`, `comum::CGeradorBU` |
| `Grava… / Gravador…` | grava no armazenamento | `comum::CGravadorBU`, `CGravadorHashes`, `CGravadorRCSecao`, `CGravadorWSQ` |
| `Imprime… / Imprimindo… / Impressao…` | imprime / imprimindo / impressão | `vota::CImprimindoBU`, `CImprimindoBEHB`, `CImpressaoPU` |
| `Conversor…`, `Converte / Desconverte` | conversor entre o objeto C++ e a sua forma ASN.1, um sentido cada | 93 classes `CConversor*` que não são templates em `comum::`/`ecourna::`, por exemplo `ecourna::app::dados::asn::CConversorMunicipioZona` |
| `Aguarda…` | aguarda | `vota::CAguardaMensagem`, `CAguardaInspecao`, `CAguardaEleitoresVotarem` |
| `Quer…` | "quer…?" (uma pergunta de sim/não) | `vota::CQuerImprimirBU`, `CQuerReimprimirZeresima` |
| `Mostra…` | mostra | `vota::CMostraQRCodeBU`, `CMostraEleitorVotando` |
| `Verifica…` | verifica | `vota::CVerificaHorarioZeresima`, `CVerificaQtdBUsAdicionais` |
| `Sincronismo…` | sincronização (entre a thread do eleitor e a do mesário) | `vota::CSincronismoEleitor`, `CSincronismoOperador` |
| `Retirar…` | retirar | `vota::CRetirarMR` |
| `Encerra…` | encerra | `comum::CEncerraRegistroMesarios` |
| `Tela`, `Telas…` | tela(s) | `vota::CTelasVota`, `CTelasCargo`; strings `telaVotoLegenda` @230613 |
| `Estado…` | estado (tanto "estado da máquina de estados" quanto "estado persistente") | `comum::CAppState` é a classe base de classes de tela/estado como `vota::CImprimindoBEHB` e `vota::CRetirarMR` (`q.py cls`); `EstadoGeral*` |
| `dados`, `md` | dados; o namespace dos objetos de domínio (provavelmente *modelo de dados*) | `uenux2/src/app/comum/dados/md/…`, `comum::md::CCargo` |
| `dao` | objeto de acesso a dados (SQLite) | `comum::dao::CJustificadorDAO`, `ccomparecimentomesariodao.cpp` |
| `DS…` | fonte de dados (para um campo de relatório) | `comum::CRelUtil::DSCodigoVerificador`, `api::CDataText<comum::CCandidaturasDSNumero>` |
| `eleitor/`, `operador/`, `monitor/` | a thread do eleitor, a thread do mesário, a thread de monitoramento | `uenux2/src/app/vota/…`; threads `vota::CThreadEleitor`, `CThreadOperador`, `CThreadMonitor` |
| `iniciovotacao/`, `fimvotacao/` | início da votação (zerésima, testes) / fim da votação (BU, relatórios, MR) | `…/vota/eleitor/iniciovotacao/`, `…/vota/eleitor/fimvotacao/` |
| `votamajoritario/`, `votaproporcional/` | votação para um cargo majoritário / proporcional | `cpedemajoritario.cpp`, `cpedeproporcional.cpp`, `cpedenulo.cpp` |
| `CHV`, `HorarioVerao` | horário de verão por município | `comum::CHV::VerificaEntraEmHorarioVerao`; @1985 `'Arquivo de HV ({}) não contém munícipio {}'` |
| `CPE` | detentor dos dados do processo eleitoral | `comum::CPE::CreateInst` |
| `Ue…`, `uebyte`, `ueword` | prefixo "urna eletrônica" dos tipos do TSE | `ueword ecourna::api::util::CBitArray::GetValor(const ueqword, const uebyte) const` @26248 |

## 13. Palavras que você não vai encontrar no binário

| palavra | por que não |
|---|---|
| **colinha** | ela pertence à página web (§6), não à urna |
| **coligação** | 0 ocorrências; o modelo de dados tem **federação** no lugar |
| **TSE** como string | apenas nos caminhos de fonte (`tse`, em minúsculas) e como sufixo de três nomes de valores de `TipoPacote` (§2) |
| **lacração** (a *Cerimônia de Assinatura Digital e Lacração dos Sistemas*, pública, em que o TSE assina e lacra o software) | um evento, não código; 0 ocorrências de `lacra` ou `cerim` em `strings.tsv` |
| **FC / FV** em maiúsculas | apenas os nomes de enum ASN.1 `fc`/`fv` e o nome de arquivo `infomidia-fv-1-t.dat`; as mensagens usam MC / MV / MI / ME / MR |
| **"Boletim de Eleitores Habilitados Biograficamente"** como uma única frase | apenas a sigla BEHB e o título em minúsculas (§0) |
