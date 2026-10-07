# Investigação (outubro de 2026): o VOTA reconstruído frente aos dados reais das urnas de 2026 e aos documentos públicos do TSE

*[English version](README.md)*

Este relatório compara a reconstrução deste repositório com três tipos de evidência pública sobre a
eleição geral de 2026 (1º turno, 4 de outubro de 2026): os arquivos que as urnas reais gravaram e que
o TSE publica (BU, RDV, logs, arquivos de assinatura), as tabelas de dados abertos do TSE e a
documentação oficial dos seus campos, e os documentos públicos do TSE sobre formatos e procedimentos.
Foi escrito em 2026-10-07. Cada resultado novo e cada divergência foi refeito de forma independente
por um segundo revisor, e uma amostra das confirmações foi conferida por amostragem; quando a segunda
passada corrigiu um número da primeira, o número corrigido é o que aparece aqui.

Nada aqui vem do TSE além do que o TSE publica. Nenhum arquivo bruto do TSE está incluído no
repositório (ver §9).

## Resposta curta

* **O modelo de dados recuperado é exato para todas as variantes de arquivo de resultado de 2026 que o
  TSE publicou.** 135/135 arquivos de BU reais (`bu.dat`/`busa.dat`) e 135/135 `rdv.dat` são
  decodificados e recodificados byte a byte com `src/asn1`, incluindo arquivos recuperados pelo RED, de
  SA (votação em cédula) e de contingência. O próprio `bu.asn1` de 2026 do TSE é igual a `src/asn1` em
  41/41 tipos estruturados (B1, B2, B22, H6, H7).
* **Todos os vínculos de integridade que o código implica valem nos dados reais.** A cadeia de hash
  dos votos de 260 eleições (15.611 tuplas) é recalculada com as strings de formato do código; cada
  linha de voto do BU é uma função pura do RDV (635 listas de cargo); 260/260 assinaturas de BU e
  405/405 assinaturas de arquivo são verificadas sobre SHA-512(hash) com certificados de hardware
  próprios de cada urna: Ed521 nas UE2020/2022, ECDSA P-521 nas UE2013/2015 (B4, B5, B8, B9, H2).
* **As listas de hash de 2026 publicadas pelo TSE estão ligadas ao código deste repositório.** Cada
  "HASH GERAL" (10 listas de PC, 112 valores de urna por UF, lidos de uma cópia de terceiros da página
  do TSE) é a cadeia calculada por `CMontadorHash::CalculaHashGeral`, sobre os arquivos na ordem de
  diretório de `CriaHashesDiretorio` e no escopo de `CGravadorHashes`. Um único binário do VOTA atende
  os quatro modelos de urna, e 42 arquivos de dados do RHVoice da urna são idênticos byte a byte aos do
  simulador (A1, A2, A5).
* **15 achados contradizem afirmações de documentos públicos do TSE**, pelo código, por arquivos
  reais de 2026 ou por outro documento do próprio TSE: cinco sobre o documento de formato de log de
  2022, as enumerações desatualizadas do `rdv.asn1` de 2026, a especificação do arquivo de assinatura
  de 2024, quatro sobre a documentação dos campos dos dados abertos (um deles, D2, abrange dois
  leiames), o tempo limite de 15 s da digital no manual do mesário e três dispositivos da Res.-TSE
  23.751/2026 que a urna não implementa como estão escritos (§1). Um décimo sexto, o limite de
  caracteres do QR, está pendente (H5).
* **O QR code de 2026 bate com o código até onde o material público permite.** O gerador reconstrói
  byte a byte o exemplo resolvido do manual do QR de 2026 (a partir de uma transcrição de terceiros),
  e a divisão do QR do certificado dá 2 códigos QRCE para cada um dos 110 certificados reais de urna. A
  reserva de 1.100 − 277 caracteres do código é pequena demais para as assinaturas de 2026:
  reconstruídos com as regras do código, 13/110 BUs reais imprimiriam um último QR de 1.102–1.239
  caracteres (H1, H3, H5, G10).
* **Os logs reais mostram onde a reconstrução está incompleta.** 183 dos 191 modelos de mensagem de
  log do VOTA são produzidos por `src/`; faltam 8 (driver da impressora, "Votação suspensa", um erro
  de exibição do terminal do mesário); falta o caminho de desligamento pela chave; duas chamadas de
  log têm a severidade errada (C1, C2, C3, C18).
* **A contagem dos votos segue a Res.-TSE 23.751/2026 em todos os pontos conferidos**: ordem dos
  painéis, legenda/nulo/inapto, senador repetido, abrangência do voto em trânsito, suspensão,
  horários. O terminal do mesário impõe menos do que o procedimento escrito: a liberação "pela digital
  do mesário" aceita qualquer dedo (550 de 1.605 liberações reais não puderam ser atribuídas a um
  mesário cadastrado), a suspensão e o encerramento aceitam qualquer título com dígitos verificadores
  válidos, e o cadastro dos mesários pode ser pulado (4/110 seções abertas sem ninguém cadastrado)
  (F1–F3, F6, F10–F17).
* **Os arquivos de chave secreta são por UF, não por urna.** Os 17 arquivos de chave de uma UF são
  idênticos nos quatro modelos de urna, e o segredo que cifra as chaves funciona entre modelos dentro
  de uma UF (confiança média) (A4, G4).
* **Os logs de urna publicados pelo TSE trazem os números de título dos mesários**: 440 títulos
  distintos em 110/110 seções da amostra. Testes de teclado que falharam são registrados como
  "Sucesso" (pelo menos 27 vezes, em 21/110 seções) (G1, G2).
* **Armadilhas dos dados abertos.** As colunas numéricas `CD_CARGA_URNA_*` perdem os zeros à esquerda
  em 9,3% dos códigos do AC, o que quebra junções e conferências da cadeia de hash; os conjuntos de
  dados misturam hora local do município e hora de Brasília sem avisar (D3, D5).
* **A cadeia de preparação de mídias GEDAI-UE → urna → BU se confirma em 62 urnas do AC.** O BU
  registra o PC que gerou a mídia de carga, não a mídia de votação, e o código de carga é criado pela
  urna (E3, E8, E9).
* **A ordem do RDV não revela nada** (os votos são ordenados por tipo e pelos dígitos digitados), mas
  3.410 votos em 108/110 seções da S110 têm um par (tipo de voto, dígitos digitados) que é único na
  sua seção e cargo (B10, G12).

## Fontes e procedência

Todos os downloads e leituras foram feitos em 2026-10-07.

| tipo | o quê | onde | peso |
|---|---|---|---|
| Dados oficiais do TSE, baixados diretamente | Arquivos de urna por seção do 1º turno de 2026 (pleito 3220): índice `aux.json`, `-bu.dat`/`-busa.dat`, `-rdv.dat`, `-log.jez`/`-logsa.jez`, `-vota.vsc`/`-sa.vsc`; listas de seções por UF | `https://resultados.tse.jus.br/oficial/ele2026/arquivo-urna/3220/dados/<uf>/<mun>/<zona>/<seção>/…` e `…/3220/config/<uf>/<uf>-p003220-cs.json` | Máximo. Dados primários; os arquivos de BU, RDV e log levam assinaturas da urna, e todas foram verificadas (B5). |
| Dados abertos oficiais do TSE, baixados diretamente | BU na Web (`bweb_1t_<UF>_…`), correspondências efetivadas (`CEFT_1t_<UF>_…`), correspondências esperadas por seção e de contingência (`CESP_1t_<UF>_…`, arquivos `csec_…`/`ccont_…`), logs do GEDAI-UE (`log_gedai_1t_<UF>_…`); cada zip com seu `.sha512` e seu `leiame` oficial em PDF | `https://cdn.tse.jus.br/estatistica/sead/eleicoes/eleicoes2026/{buweb,correspefet,correspesp,logsgedai}/…`; os logs do GEDAI de 2024 em `…/eleicoes2024/logsgedai/`; localizados pela API CKAN de `https://dadosabertos.tse.jus.br` | Máximo para os dados. Os leiames são a documentação oficial dos campos feita pelo TSE e são citados literalmente (a partir do `pdftotext`; os números de linha se referem a esse texto). |
| Projetos de software originais (upstream) | Configuração do RHVoice 1.14.0, dados da língua português brasileiro, voz Letícia-F123 | `https://github.com/RHVoice/RHVoice`, `https://github.com/RHVoice/Brazilian-Portuguese`, `https://github.com/RHVoice/leticia-f123-pt-br` | Alto (só comparação de bytes). |
| Documento do TSE, cópia oficial em site de TRE | Manual do Mesário 2022 (TSE) | `https://static.tre-al.jus.br/portal/eleitor/mesarios/tre-al-manual-do-mesario-tse-versao-web-2022.pdf` | Alto para o texto de 2022, que é o único texto de manual do mesário conferido palavra por palavra. |
| Documentos do TSE, cópias de terceiros | Pacote "Formato dos arquivos de BU, RDV e assinatura digital" de 2026: README datado de 2026-09-10, `bu.asn1`, `rdv.asn1`, `assinatura.asn1`, diagramas, scripts e um verificador de QR com dados de demonstração | `https://github.com/fernandysson/resultados-eleicoes-2026` (commit 80d8f36) | Médio-alto. Suas especificações decodificam os arquivos reais byte a byte (H6, H7); a autoria do verificador de QR é desconhecida, mas os resultados que o usam se apoiam em verificações criptográficas independentes. |
| | Manual do QR de 2026 ("manual do QR code no boletim de urna", ago. 2026, 36 p.): anotações de um terceiro e uma transcrição do seu pequeno exemplo resolvido | `https://github.com/fabricioluna/apuracaojuntos` (`tests/fixtures/bu-2026.json`, commit 664a111) | Médio. A cadeia de hash interna da transcrição se verifica, o que descarta erros de transcrição no texto que entra no hash; as anotações são paráfrase. |
| | Pacote de formatos de 2024 v2 (`bu`/`rdv`/`assinatura.asn1`, scripts, certificados de CA em `mr_util.py`), manual do QR de 2024, "Formato dos arquivos de log" de 2022 | `https://github.com/doccaz/urnas-br` (commit a87fbee6) | Médio-alto. O pacote de 2024 é igual ao zip do TSE guardado no mesmo repositório. |
| | Listas "Resumos digitais (hashes) das Eleições 2026": sistemas de urna para UE2013/2015/2020/2022 (`path1u13/u15/u20/u22`), aplicativos de PC (`pc1*`), outros sistemas (`itnm1*`), mídias de verificação (`hash_tse/oab/mp/confea`) | Uma cópia em HTML e texto, feita por terceiros, da página do TSE; o endereço da cópia não foi registrado. A página do TSE está listada abaixo, em "Não pôde ser lido". | Médio-alto. Todas as listas são consistentes internamente (cada HASH GERAL é recalculado, A1), e suas contagens e nomes são iguais aos que a análise anterior leu diretamente do TSE (A16). Os resumos individuais não foram cruzados com o TSE. |
| | Res.-TSE 23.751/2026 (atos gerais do processo eleitoral, Eleições 2026), 283 artigos | `https://jurishand.com/resolucao-tse-23751-de-26-fevereiro-2026`; a página oficial é `tse.jus.br/legislacao/compilada/res/2026/resolucao-no-23-751-de-26-de-fevereiro-de-2026` (encontrada por busca, não legível) | Médio. Duas frases foram conferidas palavra por palavra por busca de frase exata. |
| Trechos de buscadores | Manual do Mesário 2026, Guia Rápido e o curso "Mesários Brasil"; afirmações atribuídas ao manual do QR de 2026 (limite de 1.100 caracteres, EdDSA/ECDSA); descrições do TSE do AVPART e do SAVP; frases da Res. 23.759/2026; a descrição do SipHash no documento de log de 2022 | vários | Baixo. Sempre marcados como "trecho de busca (redação não verificada)". Nenhuma contradição da §1 se apoia só em um trecho de busca. |
| Não pôde ser lido | `www.tse.jus.br` responde 403 (Akamai) à máquina da análise, e as ferramentas dela informam que o host está bloqueado: o "Log do ecossistema" de 2026 (`…/eleicoes/eleicoes-2026-content/arquivos/log-do-ecossistema/@@display-file/file/log-do-ecossistema.pdf`), o próprio PDF do manual do QR de 2026 (`…/eleicoes/eleicoes-2026-content/arquivos/manual-do-qr-code-no-boletim-de-urna/@@display-file/file/manual-do-qr-code-no-boletim-de-urna.pdf`), a página de hashes de 2026 (`…/eleicoes/urna-eletronica/seguranca-da-urna/hash/resumos-digitais-hashes-das-eleicoes-2026-1o-e-2o-turnos`), `qrcodenobu.tse.jus.br`, `justicaeleitoral.jus.br` e páginas de TREs com o material de mesário de 2026; o `web.archive.org` estava limitando a taxa de requisições | — | Nenhum. |
| Código | A reconstrução em `src/`. O próprio binário wasm não estava na máquina da análise. | este repositório | Tão bom quanto a reconstrução (ver README_EN.md, "What this is not"). |

**Amostras.** Os mesmos rótulos são usados em todo o texto.

| rótulo | conteúdo |
|---|---|
| S110 | 110 seções do 1º turno, cerca de 4 por UF incluindo ZZ (exterior): BU, RDV, log e arquivo de assinatura de cada uma. 106 vêm de um sorteio aleatório com semente de 4 seções em cada uma das 28 UFs (6 dos 112 sorteios não têm arquivos publicados, todos de seções agregadas); 4 foram acrescentadas pelo nome (3 no exterior, 1 no AC; ver §9). Os seus 110 logs têm 622.567 linhas. |
| S135 | S110 mais 25 seções escolhidas de propósito nas tabelas CEFT do TSE, em várias UFs: 7 BUs recuperados pelo RED (um deles em urna de contingência), 6 BUs de SA (votação em cédula) no exterior, 6 urnas de contingência e 6 urnas substituídas no dia da eleição. 135 arquivos de BU, 135 arquivos de RDV, 135 arquivos de assinatura. |
| S129 | S110 mais as 19 seções não SA do conjunto escolhido de propósito (usada onde `bu.dat` é necessário). |
| dados abertos do AC | Todas as 2.270 seções do AC no bweb, CEFT, CSEC, CCONT e `aux.json`; os 22 logs oficiais do GEDAI-UE do AC (30.023 linhas, 454 sessões); arquivos de urna de 62 urnas do AC (5 da S110, 57 baixadas) para a cadeia de preparação, e das 10 seções de contingência do AC. |
| outros dados abertos | CEFT de AM, DF, MT, PE, RR e ZZ (49.930 linhas em 7 UFs com o AC); bweb do ZZ (1.309 seções); arquivos de urna de 2 seções SA do ZZ e de 1 seção com RED de PE; os pacotes de logs do GEDAI-UE de RR 2026 e de AC 2024; 3 urnas do AC de 2024. |

**Quanto peso tem cada tipo de fonte.** Uma afirmação apoiada em dados baixados do TSE e no código é
dada como fato. Uma afirmação que depende de um documento lido por meio de uma cópia de terceiros diz
isso; quando a cópia decodifica arquivos reais byte a byte (o pacote de formatos de 2026) ou se
verifica criptograficamente (o exemplo do QR), ela é tratada como confiável no conteúdo, mas a sua
redação exata deve ser conferida de novo com o TSE quando `www.tse.jus.br` estiver acessível. Uma
afirmação apoiada em um trecho de busca é marcada como "trecho de busca" e só é usada como contexto.
Inferências são identificadas como tais.

## Como ler

* **Ids.** A1…H18 remetem ao apêndice. A letra é a linha de trabalho: A sistema de arquivos e chaves
  (listas de hash), B arquivos de resultado e ASN.1, C logs reais, D tabelas de dados abertos e sua
  documentação, E preparação de mídias pelo GEDAI-UE, F regras eleitorais frente ao código, G
  inferências feitas no código e testadas nos dados, H documentos públicos do TSE de 2026.
* **Tipos.** *confirmação* (uma previsão do código que os dados confirmam), *nova inferência* (algo
  aprendido que o código ou a documentação anterior não dizia), *divergência* (duas fontes
  discordam), *questão em aberto*.
* **Veredictos.** Cada nova inferência, divergência e questão em aberto foi refeita de forma
  independente. *confirmado*: reproduzido como enunciado. *confirmado com correções*: reproduzido, com
  números ou redação corrigidos; a versão corrigida é a deste relatório. *refutado*: a afirmação não
  se sustenta. *não verificável*: uma previsão que nenhum artefato público consegue testar ainda. As
  confirmações foram conferidas por amostragem em vez de refeitas uma a uma: *conferido por
  amostragem* (refeito, concorda), *conferido por amostragem, corrigido* (refeito, uma contagem mudou)
  ou *não reconferido*. *Corrigido na revisão* marca a única confirmação (A12) que a conferência
  cruzada final de todos os achados encontrou parcialmente errada.
* **Referências ao código** são caminhos do repositório, `src/…:line`. `func N` é um índice de função
  wasm (ver README_EN.md).
* **Privacidade.** Não aparece aqui nenhum título, CPF, matrícula funcional, nome de pessoa, nome de
  computador, número de série de TPM ou de instalação, número de série de urna ou de mídia. Os nomes
  de titular (subject) dos certificados são mascarados como `UEAO########`. As seções só são
  identificadas quando a conduta de ninguém está em questão; as falhas de procedimento da §5 e da §6
  são dadas como contagens.
## 1. Documentos públicos que divergem do código ou dos dados reais

### 1.1 Tabela-resumo

| documento e seção | o que diz (citação) | o que mostram o código ou os dados reais de 2026 | ids |
|---|---|---|---|
| TSE, "Formato dos arquivos de log", set. 2022, p. 1 (cópia de terceiros) | "Ao ser extraído da urna, ele é empacotado num arquivo em formato 7z" | ZIP. O código empacota o log com `CZip` (`src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp:120-121`). 110/110 `-log.jez` de urna, 22/22 `.jez` externos e 22/22 internos do GEDAI começam com a assinatura ZIP `50 4b 03 04`. | H9 |
| mesmo documento, layout da linha | "(campos delimitados por TAB): [Data] [Hora] [Severidade do Evento] [ID_UE] [Aplicativo] [Mensagem] [MAC]", "Data – … AAAAMMDD", "Hora – … HHMMSS" | 622.567/622.567 linhas reais têm 6 campos separados por TAB; o primeiro é `DD/MM/YYYY HH:MM:SS`, com um espaço no meio. A própria expressão regular do documento casa com todas as linhas, e seus exemplos imprimem a mesma forma de data e hora; só o texto descreve sete campos. | H10 |
| mesmo documento, "Categorias de severidade do evento" | INFO, ALERTA, EXTERNO, ERRO | Um quinto valor, `WHAT`, em 3 linhas do GAP. O próprio VOTA emite só INFO, ALERTA e ERRO. | H11 |
| mesmo documento, logs desktop | "O log dos aplicativos deskotp possui o mesmo formato utilizado pela urna" | Os logs do GEDAI-UE de 2026 têm 4 campos TAB (sem id da urna, sem aplicativo) e fim de linha CRLF; 0 de 30.023 linhas casam com a expressão regular do documento. | H12 |
| mesmo documento, nome do arquivo de log da mídia de resultado | "[fase][ppppp]-[MMMMM][ZZZZ][SSSS].[ext]", extensão `logjez` | O código e todos os 110 arquivos reais usam `<fase><pleito:5><uf><mun:5><zona:4><seção:4>-log.jez`, por exemplo `o03220ac0107400040042-log.jez` (`src/uenux2/src/app/comum/carquivossavd.u22.cpp:44`, `:191`). O README de 2026 do TSE concorda com o código. | H13 |
| pacote de formatos de 2026 do TSE, `rdv.asn1` (cópia de terceiros) | `urnaChegouAposInicioVotacao (5)`, `reservaSecao (4)`, `reservaEncerrandoSecao (6)` (arquivo idêntico byte a byte ao de 2024) | O `bu.asn1` de 2026 do mesmo pacote, e `src/asn1/ModuloTiposResultadosEcoUrna.asn:98`, `:135-136`: `urnaEncerradaComEleitoresNaFila (5)`, `contingenciaSecao (4)`, `contingenciaEncerrandoSecao (6)`. Mesmos números, significado diferente para o 5. | H8 |
| pacote de formatos de 2024 v2 do TSE, `assinatura.asn1` (cópia de terceiros) | `EntidadeAssinaturaEcourna ::= SEQUENCE { modeloEquipamento ModeloEquipamento, assinaturaSW EntidadeAssinatura, assinaturaHW EntidadeAssinatura }`; campos da chave sem tag | Falha em 110/110 arquivos de assinatura reais de 2026. Layout real: `SEQUENCE { SW, HW, SEQUENCE { model, algorithm } }` com tags `[0]`/`[1]`. O `assinatura.asn1` de 2026 do TSE descreve esse layout e decodifica 110/110. | B3, H6 |
| leiame do BU na Web, l. 205-208 | `CD_CARGA_1_URNA_EFETIVADA` "O conjunto dos primeiros 19 caracteres…"; `CD_CARGA_2_URNA_EFETIVADA` "O conjunto dos últimos 6 caracteres…" | 24 e 7 caracteres em todas as 258.052 linhas do AC e em todas as 12.035 linhas do ZZ: `GetIDCargaFormatado` cortado depois do caractere 24 (`src/uenux2/src/app/comum/relatorios/crelutil.cpp:191-198`). | D2 |
| leiame da correspondência efetivada, l. 73-75 | `CD_CARGA_2_URNA_EFETIVADA` "…dos 06 últimos dígitos do código da carga original da urna esperada…" | Montado a partir do código da urna efetivada em 303/303 linhas em que os dois códigos diferem (7 UFs). | D2 |
| leiames de CEFT, CSEC, CCONT (definições das colunas e a divisão em 18 + 6 dígitos) | "Código da carga original da urna …, enviado pelo GEDAI" | Gravados como números sem aspas, perdendo os zeros à esquerda: 210/2.270 códigos do AC (9,3%); MT e PE têm códigos com só 19 dígitos. A urna exige exatamente 24 dígitos (`src/uenux2/src/app/comum/dados/md/correspondencia/ccarga.cpp:29-33`) e a cadeia de hash do BU usa os 24. | D3 |
| preâmbulo de todos os leiames de 2026 | "Os campos estão entre aspas e separados por ponto e vírgula, inclusive os campos numéricos"; "Campos preenchidos com #NULO … O correspondente para #NULO nos campos numéricos é -1" | Colunas sem aspas: bweb 19, CEFT 11, CSEC 7, CCONT 5. Marcadores de nulo de fato usados: `#NULO#`, strings vazias e `"-1"` entre aspas. | D21 |
| leiame dos logs do GEDAI, l. 16 | "UF: unidade federativa onde ocorreu a eleição (composto por 02 dígitos)" | Duas letras (`ac`) em todos os 22 nomes de arquivo. | D22 |
| Manual do Mesário 2022, l. 729-730 (extrato de busca: mesma frase em 2026) | "ELEITOR NÃO RECONHECIDO. Essa mensagem também é mostrada caso a pessoa demore mais de 15 segundos para posicionar o dedo no sensor." | 30 s na primeira tentativa, 15 s nas seguintes (`src/uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp:84-85`). Timeouts reais: primeira tentativa n=152 em 30–37 s; tentativas seguintes n=476 em 15–22 s. | F4 |
| Res.-TSE 23.751/2026 art. 127 VII | o relatório "Eleitores Não Reconhecidos Biometricamente", "contendo a quantidade e o título eleitoral das eleitoras e dos eleitores que não foram habilitados por biometria" | A urna emite "Eleitores com habilitação biográfica" (BEHB, `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp:130-150`). Na leitura literal, ele deixa de fora os eleitores sem biometria cadastrada liberados pelo ano de nascimento (1.424 nos 106 BUs biométricos da S110). | F5 |
| Res.-TSE 23.751/2026 art. 123 | "Emitida a Zerésima e, antes do início da votação, a presença das mesárias e dos mesários será registrada no Terminal do Mesário." | O código deixa o registro terminar sem ninguém registrado (`src/uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp:45-47`, `:76-77`); 4/110 seções abriram sem nenhum. | F6 |
| Res.-TSE 23.751/2026 art. 210 XII | os Boletins de Urna contêm o número de eleitores "a) habilitados por identificação biométrica; b) habilitados por identificação biográfica; e c) sem biometria cadastrada" | Urnas não biométricas omitem as contagens (`src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp:179-181`): 4/110 BUs, todos no exterior. | F7 |
| manual do QR code do TSE de 2024 (formato 1.5), verificado; o manual de 2026 só por extrato de busca | "Cada QR Code está limitado a 1.100 caracteres, incluindo todas as três seções." | O código reserva 277 caracteres (`src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp:277`); a cauda do último QR de 2026 precisa de 422–436. Reconstruídos com as regras do código, 13/110 BUs reais dão um último QR de 1.102–1.239 caracteres. **Pendente**: só é contradição se o manual de 2026 mantiver o limite. | H5 |

### 1.2 As evidências, item por item

**Formato do pacote de log (H9).** O documento de 2022 (cabeçalho "Tribunal Superior Eleitoral /
Secretaria de Tecnologia da Informação / Seção de Voto Informatizado") diz que o log da urna "é
empacotado num arquivo em formato 7z", e o mesmo para os aplicativos desktop. O código grava o pacote
de log da urna com `CZip` (minizip): `temp.jez` é gravado e depois renomeado para o nome de resultado
`-log.jez`. O gravador de 7z está linkado no binário, mas não é usado. Todos os 110 arquivos
`-log.jez` de urna da S110 são ZIP (108 contêm só `logd.dat`, 2 também um `.jez` arquivado), e o mesmo
vale para os 22 pacotes externos e os 22 internos do GEDAI-UE. A edição de 2022 foi escrita quando os
logs de resultado se chamavam `.logjez`; uma edição de 2026 ("Log do ecossistema") consta da página de
2026 do TSE, mas não pôde ser lida.

**Layout da linha (H10).** O texto de 2022 define sete campos separados por TAB, com `AAAAMMDD` e
`HHMMSS`. Todas as 622.567 linhas da S110 têm seis: `DD/MM/YYYY HH:MM:SS` (um campo), severidade, um
id de urna de 8 dígitos completado com zeros à esquerda, programa, mensagem e um código hexadecimal de
16 caracteres em maiúsculas. A própria expressão regular do documento junta data e hora com `[\s]`,
que aceita um espaço, e por isso casa com 100% das linhas; seus exemplos também imprimem `DD/MM/YYYY
HH:MM:SS`, mas trazem ainda um id de urna de 7 dígitos onde a expressão regular exige 8. O VOTA não
resolve a questão do layout: ele passa só (aplicativo, severidade, mensagem), e o daemon que
acrescenta os outros campos (`logd`) não está no binário. Linhas por programa: VOTA 566.963, GAP
33.538, ATUE 10.260, SCUE 9.329, LOGD 2.214, INITJE 263.

**Severidades (H11).** O documento de 2022 lista quatro categorias. Por severidade, as linhas da S110
são INFO 620.331, ALERTA 2.218, ERRO 13, EXTERNO 2 (LOGD, como documentado) e WHAT 3. As três linhas
WHAT vêm do GAP, cada uma logo depois de uma linha ERRO do GAP, e imprimem o texto de uma exceção C++
com a assinatura da função e a linha do código-fonte (C15). Em `src/`, o VOTA e o código comum só
registram log com as severidades 1, 2 e 3.

**Logs desktop (H12).** Os 22 logs oficiais do GEDAI-UE do AC (30.023 linhas) têm quatro campos TAB
(data e hora, severidade, mensagem, código de 16 hex), sem id de urna e sem campo de aplicativo, fim
de linha CRLF, e 0 linhas que casam com a expressão regular de 2022. Os nomes dos pacotes diferem da
regra de 2022, mas essa parte está documentada no próprio leiame do GEDAI de 2026 do TSE
(`gedai-ue-UF-PLEITO-oficial-NM_COMPUTADOR-SERIAL_TPM-SERIAL_INSTALACAO-log.extensão`), então, quanto
aos nomes, o texto de 2022 foi superado, e não contradito.

**Nome do log na mídia de resultado (H13).** A regra de 2022
`[fase][ppppp]-[MMMMM][ZZZZ][SSSS].logjez` correspondia aos arquivos de 2022. O prefixo do código
`{:c}{:05}{}{:05}{:04}{:04}-` (fase, pleito, UF, município, zona, seção) mais o sufixo `log.jez` é o
que seguem todos os 110 nomes reais de 2026, e o README de 2026 do TSE
(`fpppppuuMMMMMZZZZSSSS-suf.ext`; "log.jez … Arquivo compactado de LOG em formato texto") concorda com
o código. Os logs arquivados dentro de um pacote ainda seguem a regra de 2022 para arquivos arquivados
(`<urna id><DDMMAAAAHHMMSS>-01.jez`). Trata-se de uma edição desatualizada, e não de um erro.

**`rdv.asn1` desatualizado (H8).** O `rdv.asn1` do pacote de 2026 é idêntico byte a byte ao arquivo v2
de 2024 (SHA-256 `282dcde0c7ba1f1b…a465` para os dois) e mantém `urnaChegouAposInicioVotacao (5)`,
`reservaSecao (4)` e `reservaEncerrandoSecao (6)`. O `bu.asn1` de 2026 do mesmo pacote tem
`urnaEncerradaComEleitoresNaFila (5)`, `contingenciaSecao (4)` e `contingenciaEncerrandoSecao (6)`, os
nomes que `src/asn1` recuperou da informação de tipos do binário. Os dois cabeçalhos listam
`tiposresultadosecourna.asn1` entre as fontes agregadas, e o diagrama de classes do RDV de 2026 também
mantém os nomes antigos, enquanto o diagrama do BU foi atualizado. A decodificação não é afetada
(mesmos números), mas `MotivoApuracaoMistaComBU = 5` quer dizer "chegou depois do início da votação"
em um arquivo do TSE e "encerrada com eleitores ainda na fila" no outro. O pacote foi lido de uma
cópia de terceiros; o diagrama inalterado torna improvável que seja um arquivo trocado por engano.

**Especificação de assinatura de 2024 (B3, H6).** O `assinatura.asn1` de 2024 publicado falha em todos
os 110 arquivos de assinatura da S110 em `EntidadeAssinaturaEcourna.modeloEquipamento`. Os arquivos
reais de 2026 são `SEQUENCE { SW signature, HW signature, SEQUENCE { model, x } }`; a assinatura SW
termina em um campo `[0]` (a tag do conjunto de chaves) e a assinatura HW em um campo `[1]` (o
certificado), em 135/135 arquivos. Os pares (model, x) na S135 são (22, 4) 45, (20, 4) 55, (15, 2) 29
e (13, 2) 6. O `assinatura.asn1` de 2026 do TSE chama o trecho final de `OrigemAssinaturaHardware {
modeloEquipamento, algoritmoAssinatura }` e o campo da chave de `InfoChave ::= CHOICE { tagChaves [0],
certificadoDigital [1] }`, e recodifica 110/110 arquivos. Assim, o texto de 2024 foi superado, mas não
está errado para o seu ano; a mudança de layout em si já tinha sido registrada no capítulo de análise
12-real-urna-2026 §3. A leitura de que x é o algoritmo da assinatura de hardware (2 ecdsa, 4 eddsa) se
encaixa em todos os arquivos, mas nesta amostra x também é totalmente determinado pela família do
modelo, então os dados não permitem distinguir as duas leituras.

**Tamanhos de CD_CARGA e um erro de copiar e colar (D2).** O leiame do BU na Web (que vem dentro de
`bweb_1t_<UF>_051020261403.zip`) dá 19 e 6 caracteres. Todas as linhas do AC e do ZZ têm 24 (18
dígitos e 6 pontos, `ddd.` seis vezes) e 7 (`ddd.ddd`). Juntas, as duas partes são a string
`GetIDCargaFormatado` impressa pela urna (31 caracteres) cortada depois do caractere 24; a segunda
parte é o "RESUMO DA CORRESPONDÊNCIA" impresso pela urna (D1). Os números 19/6 não batem nem com
caracteres (24/7) nem com dígitos (18/6). O leiame da CEFT acerta os 18 e 6 dígitos, mas descreve
`CD_CARGA_2_URNA_EFETIVADA` como vindo da "urna esperada"; em todas as linhas em que os códigos
esperado e efetivado diferem (AC 10, AM 51, DF 28, MT 35, PE 145, RR 4, ZZ 30), a coluna vem do código
da urna efetivada.

**Códigos que perdem os zeros à esquerda (D3).** Os arquivos CEFT, CSEC e CCONT gravam
`CD_CARGA_URNA_*` sem aspas, como números. Contagem de dígitos no AC (22/23/24): CEFT efetivada
20/190/2.060, CEFT esperada 20/191/2.059, CSEC 20/191/2.059, CCONT 11/55/510; ou seja, 210 de 2.270
códigos de urna efetivada (9,3%) perderam pelo menos um zero. Alguns códigos de MT e PE têm só 19
dígitos. Os logs do GEDAI imprimem 24 dígitos em todas as linhas de correspondência, o `CD_CARGA_1`
formatado do bweb mantém o zero, e a urna rejeita qualquer coisa que não sejam 24 dígitos decimais
(erros 8021/8022). A semente da cadeia de hash do BU formata o código como `{:24}`
(`src/uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp:62`), então, nos dois BUs
decodificados do AC cujo código começa com 0, o hash da primeira tupla das duas eleições (4/4) só é
recalculado a partir do valor da CEFT depois de completá-lo com zeros até 24 dígitos. Quem usa os
dados abertos precisa completar com zeros antes de cruzar ou verificar os dados.

**Convenções de aspas e de nulo (D21).** O preâmbulo promete campos entre aspas "inclusive os campos
numéricos" e `#NULO` / `-1` para dados ausentes. Os arquivos têm 19 colunas sem aspas no bweb (por
exemplo `CD_MUNICIPIO`, `QT_*`, `NR_URNA_EFETIVADA`), 11 na CEFT (incluindo `CD_CARGA_URNA_*`), 7 na
CSEC e 5 na CCONT. Valores ausentes aparecem como `#NULO#`, com um `#` no final (por exemplo
`DS_SECOES_AGREGADAS` em 243.808 linhas do AC), como strings vazias (`TP_DIVERGENCIA` da CEFT em 2.260
linhas do AC, horários de abertura e encerramento do ZZ em 111 linhas, campos da urna esperada do ZZ
em 28 linhas) e como a string `"-1"` entre aspas (junta e turma). O próprio leiame do bweb usa
`#NULO#` uma vez. Impacto baixo, mas é a causa raiz de D3.

**UF em "dígitos" (D22).** O leiame dos logs do GEDAI (cuja entrada no PDF tem data de 2024-10-05, ou
seja, foi reaproveitado) diz que a parte UF do nome do arquivo é "composto por 02 dígitos". São duas
letras em todos os 22 nomes. O "PLEITO … composto por 05 dígitos" do mesmo leiame está certo
(`03220`), como no `{:05}` do código.

**Timeout da impressão digital (F4).** O código arma 30.000 ms para a primeira tentativa e 15.000 ms
para as seguintes (`src/uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp:84-85`,
comentário em `:177`). Nos logs da S110, os timeouts da primeira tentativa (`Timeout de reconhecimento
do dedo. Tentativa [1]`) vêm 30–37 s depois do pedido (mediana 32, n=152), e os das tentativas
seguintes, 15–22 s depois (mediana 17, n=476). O manual de 2022 verificado diz 15 s; um extrato de
busca atribui a mesma frase ao manual de 2026. Não prejudica o eleitor, mas o procedimento publicado
não descreve a primeira tentativa.

**Nome e escopo do relatório biométrico (F5).** O art. 127 VII dá ao relatório o nome "Eleitores Não
Reconhecidos Biometricamente" e o define como os eleitores "que não foram habilitados por biometria".
O relatório da urna tem o título "Eleitores com habilitação biográfica" (`behb.dat`, impresso em "via
única"), e o material dos mesários de 2026 (extrato de busca) também o chama de BEHB. Ele é gerado em
106/110 seções; as outras 4 são as urnas não biométricas do exterior. Suas linhas são os eleitores
liberados pelo ano de nascimento mais a digital do mesário (habilitação tipo 2: 1.605 eleitores, igual
ao `qtdEleitoresHabilitadosPorBiografia` dos BUs); que as linhas sejam só do tipo 2 se apoia apenas em
um comentário da reconstrução (`src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp:192-200`).
Pela redação literal da resolução, os 1.424 eleitores sem biometria cadastrada liberados pelo ano de
nascimento (a contagem deles no BU) também deveriam constar nele. Na leitura "não reconhecidos", o
escopo coincide e só o nome difere.

**Registro dos mesários (F6).** A pergunta "Registrar mesário?" aceita CORRIGE ("Cancelar") e segue
adiante sem nada que impeça a abertura da votação. Mesários registrados entre a zerésima do dia da
eleição e "Urna pronta para receber votos", por seção da S110: 4 em 82 seções, 3 em 17, 2 em 5, 1 em
1, 5 em 1, nenhum em 4. Nas 4 seções sem nenhum, houve registro de mesários mais tarde, durante a
votação; 27 seções registraram alguém durante a votação. Algum membro da mesa estava necessariamente
presente para imprimir a zerésima, então nessas 4 seções o passo do art. 123 não foi feito antes da
votação, o que o código permite. O parágrafo único permite que quem chega atrasado se registre durante
a votação.

**Contagens de habilitação no exterior (F7).** `CGravadorBU` preenche `detalhamentoComparecimento` só
`if (m_urnaBiometrica)`; o campo é `[1] … OPTIONAL` em `src/asn1/ModuloBoletimUrna.asn:37`, e o BU
impresso, da mesma forma, só imprime as três contagens em urnas biométricas
(`src/uenux2/src/app/comum/relatorios/cgeradorbu.cpp:66-70`). 106 BUs da S110 têm o campo e 4 não têm:
exatamente as 4 seções do exterior, cujos 1.036 eleitores foram todos liberados pelo ano de
nascimento. É só uma lacuna formal: para essas urnas os valores seriam (0, 0, comparecimento).

**O limite de 1.100 caracteres do QR (H5), pendente.** O código reserva 277 caracteres para a parte de
cada QR que não é conteúdo: `limite = tamanhoMaximo − 277` com `tamanhoMaximo = 1100`
(`src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp:277`;
`src/uenux2/src/app/comum/relatorios/cgeradorbu.cpp:135`). No formato 6.0, a parte fixa do último QR é
`QRBU:i:n VRQR:6.0 ` (18) + ` HASH:` e 128 hex (134) + ` ASSI:` e a assinatura em hex (270 para uma
assinatura Ed521 de 132 bytes, 284 para uma ECDSA de 139 bytes), 422–436 caracteres. O último QR pode,
portanto, chegar a 823 + 422 = 1.245 caracteres (1.259 com ECDSA) e passa de 1.100 sempre que sua
fatia de conteúdo tiver mais de cerca de 678 (664) caracteres. Mesmo o formato 1.5 de 2024 precisava
de 300 > 277, embora nenhum QR de 2024 tenha passado de 1.100 (o mais longo tinha 1.073).
Reconstruindo os 110 BUs da S110 com as regras do código, há 13 últimos QRs acima de 1.100
(1.102–1.239). A contagem depende da largura desconhecida do campo `PROC` (12 com 1–3 dígitos, 13 com
4–5, 14 com 6) e omite `AGRE`, que não está no `bu.dat`; 11 dos 13 passam do limite em 30 caracteres
ou mais. Nada no código rejeita um QR longo: a verificação da imagem só exige largura + 4 ≤ 400
módulos. O limite verificado está no manual de 2024, que descreve o formato 1.5. A redação do manual
de 2026 só é conhecida por extratos de busca, então isto vira contradição se, e somente se, o manual
de 2026 mantiver o limite. Nenhum BU impresso de 2026 estava disponível para consulta.

### 1.3 Documentação dos dados abertos: convenções não documentadas, não contradições

Estes pontos não contradizem nenhuma frase dos leiames, mas quem seguir só os leiames vai interpretar
mal os dados.

| tema | o que o leiame diz | o que os dados e o código mostram | ids |
|---|---|---|---|
| `CD_TIPO_URNA` (bweb) | "1: Apurada; 2: Não apurada; 3: Anulada e apurada em separado; 4: Anulada; e 5: Não instalada" | Um status de apuração, sem relação com o `TipoUrna {secao 1, contingencia 3, contingenciaSecao 4, contingenciaEncerrandoSecao 6}` do BU. Todas as 2.270 seções do AC e as 1.309 do ZZ são 1, inclusive 10 seções de contingência e 28 de SA. O número 3 significa coisas diferentes nos dois. O tipo da própria urna só aparece indiretamente (`TP_DIVERGENCIA` 'C' da CEFT mais `CD_ORIGEM_VOTO`). | D4 |
| fusos horários | um fuso só é dado para `HH_GERACAO` ("com base no horário de Brasília") | Hora local do município: `DT_ABERTURA`, `DT_ENCERRAMENTO`, `DT_EMISSAO_BU`, `DT_CARGA_*`, `DT_RECBTO_URNA_EFETIVADA` da CEFT. Horário de Brasília: `DT_BU_RECEBIDO` do bweb, `DT_RECBTO_URNA_ESPERADA` da CEFT. As duas colunas de recebimento de um mesmo registro da CEFT diferem em exatamente +2 h em 2.260/2.260 linhas do AC, +1 h em MT e RR, 0 no DF, −1 h em Fernando de Noronha, +1 ou +2 h no AM, e pelo fuso de cada cidade no exterior (Tóquio −12 h, Wellington −15 h). No ZZ, 515/1.309 seções parecem "recebidas antes da emissão". | D5 |
| `QT_ELEI_BIOM_SEM_HABILITACAO` | "Quantidade de eleitoras e eleitores com biometria, mas que não foram habilitados por meio dela" | Igual ao `qtdEleitoresHabilitadosPorBiografia` do BU (15/15 BUs do AC; nunca ao `…SemBiometria`). Onde o BU não tem detalhamento (todos no exterior), o bweb imprime 0, então "sem dado" e "0 eleitores" ficam iguais. | D10, B13 |
| `QT_APTOS`, `QT_COMPARECIMENTO` | "aptos a votar", "que compareceram às eleições" | Por eleição, como no BU: em 57/2.270 seções do AC a eleição presidencial tem 1–55 aptos a mais (eleitores em trânsito vindos de outro estado), e o comparecimento difere em 48. | D18, B11 |
| `DS_SECOES_AGREGADAS` | uma lista separada por "/" | Simplesmente não está no `bu.dat`; a urna imprime a lista `{:04}` separada por espaços e a põe no QR como `AGRE:` com pontos. O bweb usa um terceiro formato. | D19 |
| `CD_ORIGEM_VOTO`, `TP_DIVERGENCIA` | "Código da origem dos votos" (sem lista de valores); D, C, O | U = `votacaoUE`; R ("BU gerado por RED em urna de seção") = `votacaoRED`; C ("Sistema de apuração") = `saManual`. Só '' e 'C' ocorrem em 49.930 linhas da CEFT de 7 UFs; 'C' cobre tanto urnas de contingência quanto BUs do SA. | D12 |
| "urna efetivada" para BUs do SA | "a urna que foi utilizada na eleição, podendo ser a urna original ou uma outra urna de contingência" | Para BUs do SA, é a urna em que o sistema de apuração rodou: as 29 linhas do ZZ com origem C compartilham uma urna, uma carga e uma mídia de carga. A CEFT grava `NM_MUNICIPIO` "BUENOS AIRES" nas 29, seja qual for o `CD_MUNICIPIO`, e o bweb lista 28 das 29. | D13 |
| correspondências de contingência | sem colunas de seção | Como em `IdentificacaoContingencia`. Urnas de contingência atenderam seções fora do município (6/10) ou da zona (1/10) para os quais foram preparadas, o que os leiames não mencionam. | D14 |
| uma urna esperada e uma efetivada por seção (CEFT) | — | O modelo não consegue representar urnas de contingência intermediárias; elas só aparecem no histórico de cargas do BU/RDV (uma seção do AC tem 3 cargas de 3 urnas). | D6 |
| "código da carga original da urna" do bweb | — | É a carga da própria urna efetivada, que é o que o leiame diz ("original" qualifica a carga, não a urna). A afirmação da primeira passada de que isso induz a erro foi **refutada**. | D7 |
| README de formatos de 2026, fase | "f é a fase (s para simulado e o para oficial)" | O código também usa `t` (treinamento), que o próprio documento de logs de 2022 do TSE lista. | H16 |
| manual do QR de 2024, assinaturas | Ed25519, "a biblioteca OpenSSL", um par de chaves por UF | Superado, e não contradito: o README de 2026 do TSE diz Ed521 (certificado PEM) para UE2020/2022 e ECDSA (certificado ASN.1) para os outros modelos; os dados concordam e mostram um certificado por urna. | H2 |
## 2. O que os dados reais de 2026 confirmam sobre a reconstrução

A reconstrução foi feita a partir de uma build de desenvolvimento do simulador (`10.23.0.1 -
DESENVOLVIMENTO`). As urnas de 2026 rodaram `10.23.0.0 - Praia da Barra do Cahy`. As verificações
abaixo testam previsões do código reconstruído contra o que essas urnas gravaram.

| o que o código prevê | o que os dados reais mostram | ids |
|---|---|---|
| Os módulos ASN.1 de `src/asn1` descrevem exatamente os arquivos de resultado | 135/135 arquivos de BU (o envelope e a `EntidadeBoletimUrna` interna) e 135/135 arquivos de RDV da S135 são decodificados e recodificados byte a byte, incluindo as variantes RED, SA e de contingência; toda alternativa de CHOICE de `IdentificacaoUrna`, `DadosSecaoSA` e `Eleicoes` aparece em algum arquivo real que volta idêntico depois de decodificado e recodificado | B1 |
| `rdv.dat` é gravado sem envelope (`CGravadorRDV`, func 11587) | 0/135 arquivos de RDV decodificam como `EntidadeEnvelopeGenerico`; todos os 135 decodificam como uma `EntidadeResultadoRDV` sem envelope | B2 |
| As especificações de 2026 do TSE e `src/asn1` concordam | `bu.asn1` de 2026 = `src/asn1` em 41/41 tipos estruturados, incluindo as três mudanças de enum posteriores a 2024 que a análise anterior previu a partir do binário (`envelopeZeresimaImpressa (6)`, `urnaEncerradaComEleitoresNaFila (5)`, `contingenciaSecao (4)`); o `assinatura.asn1` de 2026 tem o `ModeloEquipamento {tpm20, ue2013, ue2015, ue2020, ue2022}` do binário | H7 |
| Nenhum valor codificado fora das enumerações do código | Nenhum encontrado na S135. Definidos, mas não vistos: `TipoUrna` 6, `TipoArquivo` 3/4/6, tipos de voto do RDV 5/8/9, `cargoSemCandidato`, consultas, cargos municipais, fases simulado/treinamento, `tpm20` | B22 |
| A cadeia de hash dos votos de `src/uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp:62-72` (três strings de formato) | Recalculada para todas as 260 eleições da S135 (15.611 tuplas); `ultimoHashVotosVotavel` é igual ao hash da última tupla em todas elas | B8 |
| As linhas do BU são calculadas a partir do RDV com a tabela de tipos em `@546352` e as regras de `CGravadorBU` (sem linhas zeradas; nominal, branco, nulo e depois legenda; numeração por cargo) | 635/635 listas de cargo recalculadas exatamente (534 na S110, 101 no conjunto escolhido); nenhuma linha zerada entre 15.611; comparecimento = votos do primeiro cargo / número de escolhas em todos os casos; partido do voto nominal = dois primeiros dígitos em 11.007/11.007 linhas | B9 |
| `aptos` indexado por abrangência, comparecimento por eleição | A divisão por eleição é necessária: em 2 seções da S110 e 2 das seções escolhidas, a eleição presidencial tem mais aptos e mais votantes que a estadual | B11 |
| O BU e o RDV de uma urna de contingência identificam coisas diferentes (`src/uenux2/src/app/comum/gravadores/asn/cconversorcorrespresultado.cpp:20-22` e `src/uenux2/src/app/comum/u18-foreign-fragments.cpp:144-146`) | 10/10 BUs de contingência trazem `identificacaoContingencia` com o município e a zona da carga de contingência, enquanto o RDV traz a seção; o local da correspondência é a constante 1 em 119/119 identificações de seção de BU e em 129/129 de RDV | B14, G9 |
| Um único `agora` para todos os arquivos de resultado (`src/uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp:115`) | Os cabeçalhos do envelope, do BU e do RDV são iguais em 110/110; o horário de geração do BU é igual ao da linha `[bu.dat] + [Início]` do log em 108/110 (2 são 1 s mais cedo) | B23 |
| A ordem dos arquivos assinados é a ordem dos gravadores de `CGravaResultado`, com arquivos WSQ só em urnas biométricas | 106 arquivos de assinatura listam 11 arquivos exatamente nessa ordem, 4 (exterior) listam 8, sem arquivos WSQ; `log.jez` é o último, embora o seu id SAVD seja menor | B6 |
| O log é empacotado antes de os arquivos de resultado serem assinados e copiados | 110/110 logs publicados terminam com `Gerando arquivo de resultado [log.jez] + [Início]`; as linhas da sessão de assinatura (`Inicia uma sessão no MSE/MSD`) nunca aparecem | B7, G13 |
| `CGravadorLog` empacota `logd.dat` mais todos os logs arquivados na ordem do `std::map` | 135/135 pacotes: membros arquivados ordenados por nome, `logd.dat` por último; todos os membros com deflate | B19, C16 |
| Verificação de assinaturas como descrevem os scripts do TSE | 135/135 certificados de hardware encadeiam até os certificados de AC do TSE; 260/260 assinaturas de eleição do BU e 405/405 assinaturas de arquivo se verificam sobre o SHA-512 do hash; 0/260 sobre o hash bruto | B5 |
| O gerador de QR de 2026 (VRQR 6.0, sem VRCH, ordem do cabeçalho, cadeia de hash, fatias de 823 caracteres) | O pequeno exemplo resolvido do manual de 2026 (transcrição de terceiros) é reconstruído byte a byte; os dois campos HASH batem quando recalculados | H1 |
| Divisão do QR do certificado `ceil(2·len/1082)` (`src/uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp:256`) | Bate exatamente com os dados do QR de demonstração do TSE; dá 2 códigos QRCE para todos os 110 certificados reais de urna | H3 |
| Ordem dos cargos no QR por `ordemImpressao`, `IDEL` a cada mudança de eleição; `TOTC` = total de votos | `IDEL 6 7 5 3 IDEL 1` em 102/110 BUs (DF: `6 8 5 3`, exterior: `1`); `TOTC = 2 × COMP` para Senador em 106/106 | H14, H15 |
| A cadeia do HASH GERAL e o percurso de diretórios de `CMontadorHash`/`CGravadorHashes` | Reproduz todos os HASH GERAL das listas de hashes de 2026: 10/10 listas de PC, 112/112 valores de urna por UF | A1 |
| Uma build para quatro modelos de urna, com o modelo decidido em tempo de execução (`IUrna::GetModelo()`) | Um único digest de `vota.of` nas quatro listas de urna; MSD nas UE2013/2015 e MSE nas UE2020/2022 nos logs de autoteste de hardware (19/19 e 91/91) | A2, A12 |
| Textos de log do VOTA | 183 dos 191 modelos do VOTA (agrupamento grosso) são produzidos por `src/` (86 exatos, 92 `std::format`, 5 compostos); a severidade no ponto de chamada (1 → INFO, 2 → ALERTA) é igual ao nível observado para 136 modelos | C1, C3 |
| Constantes do código do monitor, da biometria, das inspeções, dos horários e da energia | Monitor a cada 1.800 s (2.200/2.200 intervalos); tensão registrada só nos modelos ≥ 2020; alerta de inatividade em 45 s (1.074/1.074); 4 tentativas de digital com timeouts de 30/15 s; inspeções a cada U(60, 90) minutos, nunca dentro de uma sessão de eleitor; abertura estritamente depois do horário de início; desligamento por bateria em 1.800 s; limite de 20 eventos de energia; limite de 6 mesários; limiar de pontuação da digital 20 (720 correspondências ≥ 20, 2.828 não correspondências ≤ 19) | C6–C11, G5–G7 |
| Lógica de suspensão (`src/uenux2/src/app/vota/eleitor/celeitorvotando.cpp:329-370`) | As linhas `Voto confirmado` por cargo são iguais aos votos confirmados do RDV; os votos nulos depois de suspensão (11 Presidente, 10 Governador, 7 Senador) são iguais às linhas do log; o primeiro cargo nunca recebe um; 11/11 suspensões parciais preenchem todas as escolhas restantes, incluindo a segunda vaga do Senado | B12, C12, F15 |
| Tipos de habilitação | Reconstruídos a partir do log, são iguais ao detalhamento do BU em 101/106 seções no Brasil; as 5 diferenças são 3 eleitores suspensos antes de confirmar qualquer voto (habilitados, mas não contados como comparecimento) e 2 artefatos do parser numa inversão do log entre threads (ali as contagens brutas de linhas batem) | C13, B13 |
| A string de versão é uma constante da build | `10.23.0.0 - Praia da Barra do Cahy` nos arquivos do VOTA, do RED e do SA e nos logs do SCUE, do GAP, do ATUE e do VOTA | B20 |
| As regras de votação (§5) | Ordem dos painéis em 24.523 + 1.116 (DF) sessões completas; 736 nulos de senador repetido; eleitores em trânsito fora da sua UF recebem só Presidente; nenhuma liberação antes das 08:00 de Brasília; encerramento recusado antes das 17:00 | F10–F12, F17 |

## 3. Onde a reconstrução está incompleta ou errada

| # | problema | evidência | onde corrigir | ids |
|---|---|---|---|---|
| 1 | Duas chamadas de log têm a severidade errada. `Título {} é inválido/válido para suspender a votação` é registrado com o `Loga` de um argumento (severidade 1, INFO); a linha 123 contradiz o seu próprio comentário (`api_f1398` é `LogaAviso`, `src/uenux2/src/app/vota/log/clogvota.h:51`). | As 29/29 linhas reais (15 "inválido", 14 "válido", 13 seções) são ALERTA. | `src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp:123` e `:130`: usar `LogaAviso` (severidade 2). | C3 |
| 2 | Falta a continuação do desligamento pela chave. `SaiPorVotacaoSuspensa` registra a linha da chave, para as threads, inicia `std::async`, e o comentário diz que o resto não está no binário. | Das 250 linhas de desligamento pela chave do VOTA, 249 são seguidas por `Desligando a urna` → `Finalização de aplicativo`, sempre nessa ordem (a outra, por uma linha de impressora atolada enquanto o BU era impresso). `Votação suspensa` vem antes nos 3 desligamentos pela chave entre a zerésima do dia da eleição e o encerramento, e em nenhum dos 246 anteriores à zerésima. | `src/uenux2/src/app/vota/monitor/cthreadmonitor.cpp:194-207` (comentário em `:203-205`); `MostraMensagemDesligamento` (`:287-291`) está vazio. A ordem inversa em `src/uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp:53-54` é outro caminho (relógio errado) e não é um erro. | C2 |
| 3 | 8 modelos de log do VOTA (919 linhas) não têm literal em nenhum lugar de `src/`: `Início de impressão` (452), `Fim de impressão` (446), `Impressora atolada. Aguardando ação do operador` (5), `Operador confirmou que concluiu a manutenção na impressora` (4), `Detectada impressora com serial …` (4), `Impressora OK` (4), `Votação suspensa` (3) e um ERRO `Não foi possível exibir imagem no LCD do terminal do mesário: Erro (12) exibindo imagem no microterminal.` | A única impressora em `src/` é a impressora nula da web (`src/uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp`); `CInfoMTLCD::MostrarImagemNoLCD` não tem caminho de erro. A comparação é com `src/`, não com o wasm; que os 6 modelos novos também faltem no wasm é inferido. | Documentá-los: a seção "What this is not" do README_EN.md, o capítulo u22 (`src/uenux2/src/app/comum/cinfomtlcd.cpp:40-49`) e os capítulos u08/u26, para impressão e desligamento. Não existe ponto reconstruído a corrigir. | C1 |
| 4 | Os corpos de método de algumas das linhas mais frequentes não foram reconstruídos, então a sua severidade e os seus argumentos não podem ser lidos em `src/`: `Voto confirmado para [{}]` (154.959 linhas), `Atribuido voto nulo por suspensão [{}]` (28), `Aguardando confirmação para emissão da zerésima` (111), `Áudio desativado pelo fim da votação` (67). | Todas as 155.165 linhas são INFO. `Gerando relatório [{}] [{}]` e `Gerando arquivo de resultado [{}] + [{}]` estão reconstruídos (`src/uenux2/src/app/comum/log/ieventoslog.u35.cpp:20`, `src/uenux2/src/app/comum/log/clogcomum.cpp:75-76`). | `src/uenux2/src/app/vota/eleitor/celeitorvotando.cpp:438`, `:440` (funcs 4537, 4541); `src/uenux2/src/app/vota/eleitor/iniciovotacao/estadosiniciovotacao.u34.h:46`; `src/uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.u34.cpp:39` e `src/uenux2/src/app/vota/operador/comum/ccancelahabilitacaoeleitor.cpp:20`. | C18 |
| 5 | O id nos nomes dos arquivos de dados se chama `pleito`, mas é o processo eleitoral (`idPE`). | O GEDAI registra `Processo eleitoral registrado: 01219`, `Pleito corrente: 03220`; os pacotes `-el`, `-imp`, `-se`, `-tte`, `-mz`, `-mme`, `-cp` trazem 01219 (2024: 00439); `-cm`/`-ste` trazem o pleito; 323 nomes de pacote em três lotes se encaixam na gramática. | `src/uenux2/src/app/comum/nomearquivo/cnomearquivo.h:36` (`uedword pleito`); comentário em `src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp:120-132`. | E11 |
| 6 | `-pu` é descrito como "partidos". É a parametrização da urna (`EntidadeParametrizacaoUrna`); os partidos são `-pa`. | A própria decodificação do repositório (`docs_en/data-model/asn1-schemas.md`); as mídias reais trazem um único `o00000br-pu` nacional. | `src/uenux2/src/app/comum/nomearquivo/cnomearquivo.h:7`, `src/uenux2/src/app/comum/nomearquivo/cnomearquivo.cpp:80`, `docs_en/modules/u24-uenux2-src-app-comum-justificativa-uenux2-src-app-comum-log-.md:176`. | E12 |
| 7 | O comentário diz que a func 7787 lê um `-pu.dat` de UF; não existe nenhum real. | Só `o00000br-pu.jez` em AC 2026, RR 2026 e AC 2024; as 62 urnas do AC verificam só `o00000br-pu.vsc`. | `src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp:7`; `docs_en/modules/u24-uenux2-src-app-comum-justificativa-uenux2-src-app-comum-log-.md:175-176`; `docs_en/modules/u11-ecourna-lib-ecourna-api-asn.md:354`. Questão em aberto: como a 7787 trata a falta do arquivo. | E13 |
| 8 | A variável do certificado se chama `der`, mas nas UE2020/2022 o token devolve um texto PEM de 1.034 bytes. | Os certificados reais das UE2020/2022 são PEM (91/91), os das UE2013/2015 são DER (19/19); o QR de demonstração do TSE imprime metades em PEM. | `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp:252`. | H3 |
| 9 | O comentário diz que a reserva de 277 caracteres cobre o cabeçalho, o HASH e o ASSI; no formato 6.0 ela não cobre (são necessários 422–436). | §1.2, H5. | `src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp:276`; as frases dos capítulos listadas em §7(a). | H5 |
| 10 | Comentários ligam o gerador `simulador-votacao-ng` / 64 × '0' ao `eg.bin`. A correspondência do `eg.bin` (e, portanto, um BU do simulador) traz `nome_maquina` / `12345678` / `99999999`; `simulador-votacao-ng` só está nas fixtures `infomidia-fv-*-t.dat`, das quais o VOTA lê só o serial. | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp:76-82` (func 9952), `src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp:246-251`. | `src/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.h:17-21`; `docs_en/modules/u40-lib-ecourna-classes-without-known-file.md:335`. | E4 |
| 11 | O nome de chave `wsq.pk1` está marcado como inferido (`?`). | `/dsk/fi/estatico/chave/wsq.pk1` existe para toda UF nas quatro listas de urna (apoio forte; `bio.sk1` é o único outro nome de chave com 7 caracteres). | `src/uenux2/src/app/comum/reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp:71-75`; `docs_en/modules/u24-uenux2-src-app-comum-justificativa-uenux2-src-app-comum-log-.md:356`. | A3 |
| 12 | A tabela de pacotes do SAVD tem `dadoscarga.vsu` em `/dsk/fe`; toda execução real do GAP verifica `/dsk/fi/estatico/dadoscarga.vsu`. | 494/494 execuções do GAP. A tabela foi lida do objeto vivo, então é isso que o wasm contém; a tabela também lista a cópia na MV do `-lo.vsc`, que o GAP verifica nas duas mídias. | Acrescentar uma nota em `src/uenux2/src/app/comum/carquivossavd.u22.cpp:103`; fica em aberto se é um deslize ou se nomeia a cópia da MV. | A10 |
| 13 | `MDUE` é lido de um deslocamento marcado `+44 (?)`; que seja o ano do modelo com 4 dígitos (2020, e não o enum 20) é uma inferência. | Os valores de `EUrnaModelo` são anos (`src/uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocarga.h:8`, `src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocarga.cpp:64-67`); não havia nenhum BU impresso de 2026 disponível. | `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp:266`. | H4 |
| 14 | Que o relatório BEHB liste só as liberações do tipo 2 se apoia num comentário da reconstrução. | 106/110 seções o geram; a sua condição é `QtdHabilitados(2)`. | `src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp:192-200`. | F5 |
| 15 | Os leiautes e tamanhos de objetos nos capítulos (por exemplo, uma `std::string` de 12 bytes) são os da build wasm32 com libc++; a urna real roda x86 de 32 bits com glibc e a libstdc++ do GCC, e liga a `libecourna.so` dinamicamente. | Nomes de arquivo nas listas de urna de 2026 (A15). | Uma nota no README_EN.md ou em `docs_en/modules/README.md`. | A15 |
| 16 | Os ramos para os modelos 2009–2011 estão mortos em 2026: as listas de urna existem só para UE2013/2015/2020/2022. | A2. | Uma nota no capítulo u23 (item 14 da sua tabela). | A2 |

## 4. O que se aprendeu de novo sobre a urna real

### 4.1 Imagem do sistema e as listas oficiais de hashes

O TSE publica, para cada eleição, digests SHA-512 (Base64) dos arquivos instalados de cada sistema,
e cada lista termina num "HASH GERAL". Para 2026 há uma lista por modelo de urna (`path1u13`,
`path1u15`, `path1u20`, `path1u22`), com 135/136/140/140 arquivos de base mais 17 arquivos de chave
para cada uma das 28 UFs (27 estados e ZZ).

* **O HASH GERAL é a própria cadeia de hash do código (A1).** `CMontadorHash::CalculaHashGeral`
  (`src/uenux2/src/api/hash/cmontadorhash.cpp:88-101`) calcula G1 = H1 e Gk =
  Base64(SHA-512(G(k−1) ‖ Hk)), concatenando os textos Base64. Isso reproduz as 10 listas de PC e
  outras, na ordem listada, e 112/112 valores de urna por UF, quando a cadeia percorre os arquivos de
  base na ordem listada e depois os 17 arquivos de chave da UF. A ordem da base é exatamente o
  percurso de `CriaHashesDiretorio` (em cada diretório, primeiro os arquivos, em ordem de bytes,
  depois os subdiretórios; `:144-145`, recursão em `:161`); ordenar simplesmente pelo caminho dá
  0/112. O escopo é o de `CGravadorHashes` (`src/uenux2/src/app/comum/gravadores/cgravadorhashes.cpp:62-73`):
  a raiz sem `/dsk`, depois `/dsk/fi/estatico/chave/`. Os arquivos por urna `uenux.cfg` e
  `avbootcfg.vsu` ficam de fora, como faz `foraDoHashGeral`
  (`src/uenux2/src/api/hash/cmontadorhash.cpp:148`). Ressalva: nesta build o valor acumulado nunca é
  lido e não é gravado em nenhum arquivo, então o próprio `hash.dat` da urna guarda só as entradas
  por arquivo; que o seu escopo seja igual ao da lista publicada é inferido.
* **Um único binário do VOTA para os quatro modelos (A2).** A união dos caminhos de base é 173; 123
  são idênticos nas quatro listas, incluindo `/uenux/bin/vota.of`, todos os programas em
  `/uenux/bin`, `libecourna.so`, o kernel e a biblioteca C. Os 50 que diferem são módulos do kernel
  (7/8/11/11 arquivos `.ko` por modelo), a camada de hardware `libapihwil*`, a biblioteca do MSD
  (UE2013/2015) contra as bibliotecas do terminal do mesário `libapimtpos`/`libapimtss`
  (UE2020/2022), e os arquivos de assinatura que os cobrem. Os ramos do código que dependem do modelo
  são ramos em tempo de execução, como na reconstrução.
* **Espaço de usuário (A15).** Pelos nomes de arquivo: `/lib/ld-linux.so.2` (o carregador da glibc
  x86 de 32 bits; nenhum carregador de 64 bits em nenhuma lista), a glibc, `libgcc_s` e a
  `libstdc++` do GCC, mais bibliotecas compartilhadas do TSE (`libecourna.so`, `libapigeneric`,
  `libapiio_resources`, `libapisdk`, `libapiui` e as `libapihwil*` de cada modelo). O wasm, ao
  contrário, liga libc++/musl e a ecourna estaticamente.
* **Gráficos e recursos (A14).** Não há `/uenux/app`, `/resource`, arquivo de fonte, Qt6Svg nem
  plugin SVG instalados. O Qt6 roda sobre `linuxfb` só com os plugins de imagem GIF e JPEG, que são
  exatamente os formatos que o código pede (12 GIFs, 23 JPEGs, um PNG, que o QtGui lê nativamente, e
  um WAV). Os ícones SVG do simulador e a sua fonte variável existem só na web. Que os recursos
  fiquem em `libapiio_resources.so` é inferido pelo nome.
* **RHVoice (A5).** Os 20 arquivos de idioma de português do Brasil na urna são iguais aos do commit
  `910c889` do upstream (2023-10-29; nenhum outro commit bate com os 20); os 21 arquivos da voz
  Letícia-F123 (`voice.info`, `voice.params`, `16000/*`) são iguais aos do upstream da voz;
  `/etc/RHVoice/RHVoice.conf` é igual à configuração da tag 1.14.0 do RHVoice; a biblioteca principal
  se chama `libRHVoice_core.so.1.14.0`. A análise anterior tinha associado os mesmos arquivos do
  upstream ao pacote do simulador, então 42 arquivos de dados da urna são idênticos aos do simulador.
  Os 8 arquivos provisórios do simulador não estão na urna: existem só na web.
* **Verificações de assinatura na inicialização (A18).** O GAP verifica uma série fixa de arquivos
  de assinatura a cada inicialização. Em 110 logs há 494 execuções com 32 (6 execuções), 33 (119),
  34 (354) ou 35 (15) passos, e todas as 16.680 verificações dão "SUCESSO". Os passos 1–24 são fixos:
  `avusrbin.vst` duas vezes, `avboot.vst`, `avbootcfg.vsu`, `avbin.vst`, `avetc.vst`, cinco arquivos
  do RHVoice, `avpart90/91/97/99.vst`, `avmod.vst`, `avmodue.vst`, `avlib.vst` e três arquivos de
  plugin do Qt, `avusrlib`/`avusrlibue.vst` e a assinatura de chaves da UF `avusrchave.vst`. Depois
  vêm o `-lo.vsc` da seção na mídia interna, o `infomidia.vsc` e o `-lo.vsc` da mídia de votação
  (0–2 verificações), `dadoscarga.vsu`, `-cp`, `-pu`, dois pacotes `-ste` e de um a três pacotes
  `-ce`. Número de passos = 30 + verificações na mídia de votação + número de pacotes `-ce`.
* **As quatro entidades signatárias (A6).** `/uenux/bin/avpart90.vst`, `91`, `97` e `99` estão em
  todos os modelos. As listas das mídias de verificação nomeiam uma chave cada: `90.pub` (OAB),
  `91.pub` (MP), `97.pub` (CONFEA), `99.pub` (TSE), então 90/91/97/99 são os códigos dessas
  entidades. Que cada `avpart9N.vst` seja a assinatura da entidade N sobre o software da urna é
  inferido pelos números. O GAP verifica os quatro numa ordem que é constante em cada urna (110/110
  logs com várias execuções), mas varia entre urnas (ocorrem 23 das 24 permutações). O log do
  GEDAI-UE chama o aplicativo correspondente da mídia de resultado de "Sistema Externo de Auditoria e
  Verificação - AVPart"; outras expansões de AVPART encontradas por busca são redações não
  verificadas.
* **Um arquivo de assinatura não nomeado (A7).** `/uenux/bin/avusrbinof.vst` está nas quatro listas,
  ao lado de `avusrbin.vst`, mas nenhuma linha de nenhum programa o nomeia: os passos 1 e 2 do GAP
  nomeiam ambos `avusrbin.vst` (494/494). Ou o passo 2 verifica `avusrbinof.vst` com o rótulo errado,
  ou o arquivo não é verificado; os logs não permitem dizer qual. A sua ligação com os binários da
  fase oficial `vota.of`/`sa.of` é inferida pelo nome.
* **Pacotes de instalação (A8, A9, A17, A20).** Uma instalação oficial real verifica
  `jez/avgm.vsc`, `jez/vota_apl.vst`, `jez/vota_ofi.vst` e `jez/avpart_ofi.vst` na mídia de carga
  (110/110 logs; os números dos passos variam: 10–12 em 102 logs, 12–14 em 4, 6–8 em 4), então
  `vota_apl` é instalado junto com o pacote de fase `vota_ofi`, e não como uma variante de fase. O
  contêiner `.jez` é um ZIP simples (nenhum membro dos 110 logs de resultado é cifrado); se os pacotes
  de aplicativo do GEDAI também são cifrados não dá para saber pelos digests. Durante a instalação,
  `/dsk/fe` é a mídia de carga, que tem o seu próprio diretório `chave/`; depois, é a mídia de
  votação. O SCUE, o instalador, roda a partir da mídia de carga e não está nas listas de urna. Os
  programas instalados batem com o vocabulário de aplicativos do código (`vota.of`, `sa.of`, `red`,
  `vpp`, `adh`, `atue`, `ste`, `gap`, `savd`, `logd`); só as variantes `.of` são instaladas numa
  carga oficial.
* **Código compartilhado com outros programas da urna (A13, C4).** Os sete rótulos de assinatura do
  SCUE nos logs ("EG Geral MI", "EG GAP 1 MI", "EG VOTA MI", "EG SA MI", "EG RED MI", "Tabela de
  correspondência", "UENUX CFG") são entradas da tabela de nomes SAVD do VOTA
  (`src/uenux2/src/app/comum/iinterfacesavd.u36.cpp`). O SCUE e o ATUE compartilham com o código
  reconstruído as linhas de início (`Iniciando aplicação - {}`, `Versão da aplicação: {}`), e o ATUE
  também duas linhas de `IInterfaceInit` (`Tamanho da MR: {}`, `Urna desligada a pedido do
  aplicativo`). As suas mensagens de energia usam uma redação diferente da de `CLogComum` do VOTA
  ("Urna operando com rede elétrica" e "Bateria interna com carga plena" no SCUE e no GAP, "Operando
  com a rede elétrica" no ATUE), então essa classe é do lado do VOTA (inferido pela redação).
* **O aplicativo Windows "Votação" (A11).** A lista de PC `pc1vota` é um aplicativo desktop em Qt6
  (`votacao.exe`, carregador do WebView2, sem plugin de rede nem de TLS) e irmão do GEDAI-UE e do
  Sorteio. Resultados de busca descrevem um "SAVP-Votação" usado no teste de integridade (redação não
  verificada). Não é uma build do simulador web. Os seus únicos arquivos iguais a arquivos da urna
  são `dependencias.properties` e `versoes.properties`, o catálogo de contratos ASN.1. O Transportador
  de 2026 traz jars de contrato com a versão `20260601173148`, o `TAG_CONTRATOS` compilado nesta build
  do VOTA (`src/uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp:86`). Que o arquivo da
  própria urna traga essa tag é inferido; só o seu digest é público.
* **O que as listas não cobrem (E19).** O GEDAI-UE verifica `gedai-ue.vst` e `app.ini.vsc` a cada
  inicialização (454/454 sessões) e baixa
  `https://config-integracao.tse.jus.br:443/eleitoral/producao/oficial-ele2026.uris.properties`
  (444 sessões). A lista do GEDAI contém `gedai-ue.vst`, mas nenhuma lista contém `app.ini.vsc` nem
  o arquivo oficial de URIs. Os dois provavelmente são configuração, e não software; não podem ser
  auditados a partir das listas publicadas.
* **A cópia das listas é consistente (A16).** A cópia tem as contagens e os nomes que a análise
  anterior leu diretamente do TSE (HotSwapFlash 8 digests, GEDAI-UE 70, 75 distintos juntos, os
  mesmos títulos), e as suas formas em HTML e em texto concordam.

### 4.2 Material de chaves

* **Nomes dos arquivos de chave (A3).** O diretório de chaves `/dsk/fi/estatico/chave/` e os cinco
  arquivos de chave que o VOTA lê (`cv.ber.pri`, `bu.pk1`, `jufa.pk1`, `wsq.pk1`, `bio.sk1`) estão em
  todo bloco de UF de toda lista, byte a byte. Isso fecha a questão em aberto sobre `wsq.pk1`, com uma
  ressalva: `bio.sk1` é o único outro nome de 7 caracteres que a consulta reconstruída poderia conter.
* **Chaves por UF e nacionais (A4).** Cada UF tem os mesmos 17 arquivos de chave, e cada bloco de UF
  é idêntico nas quatro listas de modelo. Onze arquivos diferem por UF: cinco chaves privadas
  (`bio.sk1`, `cv.ber.pri`, `log.ber.pri`, `sc.ber.pri`, `ue.ber.pri`), três chaves de cifração do
  CEPESC guardadas encapsuladas (`bu.pk1`, `jufa.pk1`, `wsq.pk1`), duas chaves públicas
  (`tse.ber.pub`, `ue.ber.pub`) e o arquivo de assinatura `avusrchave.vst`. Seis chaves públicas
  (`pu`, `sc`, `secad`, `secinp`, `seint`, `sevin` `.ber.pub`) são nacionais e iguais às
  `chaves\legal\o<name>.ber.pub` do Holocron. Os nomes `.ber` correspondem ao enum
  `ESavdChaveValidar` do código (`src/uenux2/src/app/comum/cpacotearquivos.h:24-26`); `sc` = SCUE é
  inferido. Todos os 110 arquivos de assinatura da S110 trazem a mesma tag de conjunto de chaves, de
  12 dígitos, `202607021654`.
* **O segredo de cifração das chaves é compartilhado entre os modelos de uma mesma UF (G4, confiança
  média).** O código sempre decifra `wsq.pk1`, `bio.sk1`, `bu.pk1` e `jufa.pk1` com AES-256-CBC,
  chave = SHA-512(segredo do IKernelHSM)[16..48), IV = os últimos 16 bytes
  (`src/ecourna/api/security/csymmetriccipherfactory.cpp:26-37`); o flag `cifrado` é ignorado;
  `cv.ber.pri` só é decifrado se estiver marcado. Nos logs, o caminho do `wsq.pk1` devolveu um id de
  imagem na UE2013 (1 seção), na UE2015 (13), na UE2020 (46) e na UE2022 (39), e eleitores foram
  habilitados por digital (o que exige `bio.sk1`) em UE2013 2/5, UE2015 13/14, UE2020 47/47 e UE2022
  44/44 seções; 22 UFs (wsq) e 24 UFs (bio) têm dois ou mais modelos com sucesso. O mesmo texto
  cifrado, portanto, é decifrado nos quatro modelos, então o segredo não está ligado nem ao
  dispositivo nem ao modelo; é por mídia da UF ou nacional. A evidência do `wsq.pk1` supõe que um id
  de imagem registrado significa que a chave foi decifrada. Consequência: as chaves de assinatura são
  por urna, mas a chave do Código Verificador (`cv.ber.pri`) e a chave da biometria dos eleitores são
  uma por UF. Decifrar `bio.sk1` dá a chave secreta do CEPESC; os templates biométricos dos eleitores
  também exigem uma chave de sessão por registro e o algoritmo CEPESC.
* **O código das linhas de log (C5, A19).** O sexto campo de cada linha de log tem 16 dígitos
  hexadecimais maiúsculos. Todos os 622.567 são distintos, e 2.767 linhas que repetem uma linha
  anterior em todos os outros campos (na maioria "Tecla indevida pressionada" no mesmo segundo)
  trazem códigos diferentes, então o código depende de estado oculto (uma cadeia, um contador ou um
  nonce). 1.240 construções sem chave ou com chave trivial (MD5, SHA-1/2/3, BLAKE2, CRC-64, xxHash64,
  SipHash com chaves adivinháveis…) nunca batem nas primeiras linhas de 3 logs de urna e 2 do GEDAI
  (só evidência negativa fraca). Cada UF tem um `log.ber.pri`, que o VOTA nunca usa; o Holocron traz
  chaves `<app>.log.ber.pri` para os logs de desktop. A leitura "uma cadeia SipHash com a chave
  `log.ber.pri`" se encaixa na descrição de 2022 do TSE (extrato de busca) e na única primitiva de MAC
  da biblioteca, `CSiphashMac` (SipHash-4-6, chave de 16 bytes,
  `src/ecourna/api/security/csiphashmac.cpp:26-27,38-39`), mas é inferência. Sem a chave, terceiros
  não podem conferir linhas isoladas; o log publicado é protegido como um todo pelo arquivo de
  assinatura (B5).
* **O caminho do BU cifrado estava desligado (G8).** `CGravadorBU` grava um envelope cifrado pelo
  CEPESC só se `m_permiteCifrar && GetCriptografarBU()`
  (`src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp:268`). Nenhum dos 129 `bu.dat` da S129 tem o
  campo `seguranca`, e o `bu.dat` publicado é exatamente o arquivo que a urna assinou, então
  `criptografarBU` estava falso no 1º turno, embora `bu.pk1` seja enviado a toda urna.

### 4.3 Assinaturas, certificados e o QR code de 2026

* **O arquivo de assinatura (B3, H6, E16).** Veja §1.2 para o leiaute. A assinatura de software (SW)
  é uniforme: signatário `SEVIN`, serial 20260708, conjunto de chaves `202607021654`, CEPESC de 256
  bits sobre SHA-512, assinaturas de 4.765 bytes. Os arquivos de assinatura dos logs do GEDAI-UE usam
  o mesmo conjunto de chaves SW, mais uma assinatura de hardware por `chaveAssinaturaDesktop`
  (RSA-2048) com um certificado por PC emitido por `CN=Coruscant_Assinatura` (validade de 3 anos, com
  início entre 10 e 23 de setembro de 2026; os seus nomes comuns terminam num byte NUL, que parsers
  estritos rejeitam) e o trailer `{tpm20, 1}`. Em 189 arquivos (167 de urna, 22 do GEDAI), o segundo
  valor do trailer é igual ao número do algoritmo de hardware (RSA 1, ECDSA 2, EdDSA 4), mas só
  ocorrem três combinações, cada uma ligada a uma geração de equipamento.
* **Assinaturas e certificados de hardware (B4, A12, H17).** A S110 tem 47 urnas UE2020, 44 UE2022,
  14 UE2015 e 5 UE2013; as 4 seções no exterior usam UE2013 (3) e UE2015 (1). As UE2020/2022 assinam
  com Ed521 (assinaturas de 132 bytes); o seu certificado é um PEM de 1.034 bytes (721–723 bytes como
  DER) com OID de chave `1.3.6.1.4.1.44588.2.1`, emitido pela "AC UE2020" ou pela "AC UE2022", com
  sujeito `UEAO########`. As UE2013/2015 assinam com ECDSA P-521 (assinaturas DER de 139 bytes) e
  trazem um certificado DER de 944–946 bytes (ecdsa-with-SHA512) emitido pela "AC URNA", com validade
  2013–2027 (UE2013) ou 2016–2030 (UE2015) e sujeito `ueao########`. *Correção:* a afirmação da
  primeira passada de que os arquivos de assinatura das UE2013/2015 não trazem certificado (A12)
  estava errada: o seu script procurava só PEM; 19/19 trazem o certificado DER. Em 5 dos 19, um ou
  dois bytes `0x00` vêm depois do DER. Os dígitos do sujeito são iguais ao número interno da urna no
  BU em 110/110 da S110 e em 22 das 25 seções escolhidas; as 3 exceções são as seções em que o RED
  rodou em outra urna (B18): o certificado identifica a urna que assinou, que nem sempre é a urna que
  o BU nomeia.
* **O que é assinado (B5, H2).** Os 135 certificados da S135 se verificam contra os certificados de
  AC do `mr_util.py` do TSE (AC URNA 35, AC UE2020 55, AC UE2022 45). Todas as 260 assinaturas de
  eleição (`assinaturaUltimoHashVotosVotavel`: UE2013 9, UE2015 51, UE2020 110, UE2022 90) se
  verificam sobre o SHA-512 do último hash, de 64 bytes, e 0/260 sobre o hash bruto; as 405
  assinaturas de arquivo (bu 129, busa 6, rdv 135, log 129, logsa 6) e as 135 autoassinaturas se
  verificam do mesmo jeito. O código passa o hash bruto para `IPkcs11::Assina`
  (`src/uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp:83`,
  `src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp:317-323`), e não há
  implementação de `IPkcs11` no wasm, então o passo SHA-512 muito provavelmente acontece abaixo dessa
  interface (inferência: os dados mostram o que é assinado, e não qual camada calcula o hash). Para
  ECDSA isso é o ecdsa-with-SHA512 comum; só o Ed521 tem um pré-hash a mais. O `ASSI` do QR segue a
  mesma regra: nos dados do QR de demonstração do TSE, uma assinatura Ed521 de 132 bytes (certificado
  UE2020) e uma assinatura ECDSA de 139 bytes (certificado UE2015) se verificam sobre o SHA-512 dos
  bytes do HASH, e não sobre o hash bruto nem sobre o seu texto hex.
* **Os certificados de AC.** A AC URNA é emitida pela "AC RAIZ UE", e a AC UE2020/AC UE2022 pela "AC
  URNA v2"; os certificados do `mr_util.py` são intermediários, e não raízes autoassinadas. O
  certificado da AC URNA distribuído ali tem `notAfter` 2026-07-18, antes da eleição, enquanto os
  certificados das UE2013/2015 que ela emitiu valem até 2027/2030: um validador que confira as datas
  da cadeia inteira os rejeitaria (uma observação do revisor, não parte de um achado).
* **O formato do QR de 2026 (H1).** A transcrição feita por terceiros do pequeno exemplo resolvido do
  manual de 2026 (`QRBU:1:2 VRQR:6.0 ORIG:VOTA …`) é reconstruída byte a byte pelo gerador do código:
  sem `VRCH`, `AGRE` logo depois de `SECA`, a mesma ordem de cabeçalho, os dois campos HASH batendo
  quando recalculados, fatias de 815 + 102 caracteres, textos de QR de 967 e 524. Limites de divisão
  de 816 a 825 reproduzem o corte publicado; com a faixa anterior, de 2024 (823–829), isso dá
  823–825, e o código diz 823. O `VERS:10.17.1.0` do exemplo mostra que ele foi feito com uma build
  10.x anterior. A análise anterior tinha VRQR 6.0 e "sem VRCH" só como previsões do código.
* **QR codes de certificado (H3, H4, G10).** `MontaQRCodesCertificado` divide o texto hex do
  certificado em `ceil(2·len/1082)` partes iguais, cada uma impressa como
  `QRCE:i:n IDUE:<urna> MDUE:<model> CERT:<hex>` depois dos códigos do BU, com numeração própria.
  Para os certificados atuais isso dá 2 códigos (todos os 110 da S110). O texto de cada código tem
  37 + 1.034 = 1.071 caracteres para um certificado PEM (UE2020/2022) e 983 para um DER
  (UE2013/2015). "Dois" é uma consequência do tamanho: um certificado com mais de 1.082 bytes daria 3,
  e a constante 1.082 não deixa espaço para o prefixo de 37 caracteres (uma parte poderia chegar a
  1.119 caracteres). O QR do certificado na tela é um código único com o certificado inteiro, com
  cerca de 2.100 caracteres para um certificado PEM. `MDUE` muito provavelmente é o ano do modelo
  com 4 dígitos (confiança média). Previsto, mas não observado: o `ASSI` impresso tem 264 caracteres
  hex (Ed521) ou 278 (ECDSA; as 34 assinaturas DER usam r e s de largura fixa, 66 bytes, então o
  comprimento é estável).
* **Ordem dos cargos e totais do Senado (H14, H15).** Ordenar os totais dos BUs reais por
  `ordemImpressao` põe a eleição estadual primeiro e Presidente no seu próprio bloco `IDEL`, como no
  exemplo do manual; para Senador, `TOTC = 2 × COMP` (duas vagas).
* **Nomes de certificado nos dados de demonstração (H17).** Os certificados dos QR de demonstração
  do TSE têm sujeitos `UEAD…`/`uead…`, e os reais de 2026, `UEAO…`/`ueao…` (maiúsculas nas
  UE2020/2022, minúsculas nas UE2013/2015). Os dados de demonstração provavelmente vêm de urnas de
  teste. Se uma urna oficial imprime nos seus códigos QRCE o mesmo certificado do seu arquivo de
  assinatura só pode ser conferido num BU impresso de 2026.
* **Ainda não visto (H18).** Os rótulos impressos que o código grava
  (`============= BU DIGITAL =============`, `======== CERTIFICADO DIGITAL =========`,
  `ASSINATURA BU DIGITAL:`, `-------------- 01 / 02 ---------------`), a tabela completa de campos do
  manual de 2026 (campos de SA/RED) e o seu exemplo grande não puderam ser conferidos.

### 4.4 Arquivos de resultado: contingência, RED, SA e seções agregadas

* **Comparecimento por eleição (B11, D18).** Em 106 BUs da S110 com as duas eleições, a eleição 6257
  contém Presidente e a 6259, os cargos estaduais. `qtdEleitoresAptosSecao` é igual nas duas, e a
  diferença de aptos é igual à diferença de eleitores em trânsito: só a eleição presidencial pode ter
  mais, o que se encaixa na regra de que eleitores em trânsito fora da sua UF votam só para
  Presidente (art. 31). Exemplo AL 27855/0002/0235: 475 aptos (50 em trânsito) e 379 votantes para
  Presidente, contra 436 (11) e 341. `qtdEleitoresCompareceram` é o número presidencial e é igual à
  contagem de `O voto do eleitor foi computado` (110/110). No AC inteiro, 57 de 2.270 seções têm mais
  aptos para Presidente.
* **Significado dos contadores de comparecimento (B13).** Em 5 seções do AC comparadas com o BU na
  Web, `QT_ELEI_BIOM_SEM_HABILITACAO` é igual a `qtdEleitoresHabilitadosPorBiografia` (valores 7, 6,
  9, 9, 9) e nunca a `…SemBiometria` (2, 5, 13, 4, 36). Em 106 BUs biométricos, os três contadores
  somam o número de votantes. Um eleitor habilitado, mas depois suspenso sem nenhum voto, não é
  contado. Esses significados concordam com os comentários do próprio `bu.asn1` de 2024 do TSE.
* **Urnas de contingência (B14, B15, D6, G9).** Para os 10 BUs de contingência do conjunto escolhido,
  e separadamente para os 10 do AC, a correspondência do BU é `identificacaoContingencia` com o
  município e a zona para os quais a mídia de contingência foi preparada, enquanto o RDV nomeia a
  seção, exatamente como fazem os dois conversores. O histórico de cargas `historicoCodigosCarga`
  começa com a carga da urna original da seção (CEFT "esperada", 10/10) e termina com a carga da
  própria urna de contingência (CEFT "efetivada", 10/10); uma seção do AC e uma do RJ têm 3 entradas;
  na do AC, a entrada do meio é uma urna de contingência intermediária que o CEFT nunca lista. As
  seções cuja urna substituta foi recarregada como urna de seção em 03/10 têm uma única entrada.
  Todos os 110 BUs da S110 têm uma entrada. Então o histórico é o da votação da seção (estado do
  gap), e não "todo código de carga que esta urna teve".
* **Regra do envelope para o SA (B16).** Os 6 envelopes de SA usam identificação de contingência (o
  seu `TipoUrna` é `contingencia`), embora a correspondência seja a seção apurada, o que o conversor
  de envelope compartilhado prevê
  (`src/uenux2/src/app/comum/gravadores/asn/cconversorenvelopegenerico.cpp:53-66`).
* **BUs de SA (cédula de papel) no exterior (B17, D13).** Os 6 BUs de SA do conjunto escolhido têm
  `tipoUrna contingencia`, `tipoArquivo saManual`, `motivoUtilizacaoSA` "totalmente manual / outros",
  `dadosSA {junta 1, turma 1}` sem urna de origem, `qtdEleitoresCompareceram` 0 enquanto cada cargo
  tem 11–49 votos, nenhum detalhamento, serial da mídia de votação `00000000` (o padrão do código em
  `src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp:226`) e um RDV com `eleicoesSA`. Os 6 foram
  apurados numa única urna UE2015. No CEFT, 31 linhas de todo o país têm origem C ("Sistema de
  apuração"), 29 delas no exterior.
* **BUs recuperados pelo RED (B18).** Os 7 têm `tipoArquivo votacaoRED`, a mesma string de versão e
  os mesmos nomes de arquivo que o VOTA, e a sua própria ordem de arquivos assinados
  (`bu, jufa, rdv, …` em vez do `bu, rdv, jufa, …` do VOTA); o BU é gerado 0,79–5,29 h depois do
  horário original de emissão. Em 3 dos 7, o RED rodou em outra urna: o log identifica "uma urna de
  contingência", o `logd.dat` traz o id de outra urna, e o certificado de hardware, o serial do
  signatário e todas as assinaturas são dessa urna, enquanto o BU e o RDV mantêm a identidade de urna
  da correspondência original.
* **O índice do site de resultados (B21).** No `aux.json`, o `hash` de cada pacote transmitido é o
  hex de um texto ASCII: o Base64 padrão do SHA-256 do arquivo `.vsc` do pacote, com `/` escrito como
  `-` (135/135; `+` e `=` são mantidos). O TSE, portanto, indexa cada pacote pelo seu arquivo de
  assinatura, que contém o hash de todos os outros arquivos.
* **Seções agregadas (B24, D19).** O BU nomeia só a seção principal; o site de resultados não serve
  arquivos para as agregadas (as 6 falhas de download da amostra aleatória foram todas de seções
  agregadas). O BU não tem lista de seções agregadas; a lista do bweb vem de outro lugar.
* **Logs de urnas substituídas (B19).** O log de uma urna substituída chega ao TSE dentro do
  `log.jez` da sua sucessora, como membro arquivado (por exemplo, o log do VOTA de uma urna original,
  que termina em 04/10, dentro do pacote de uma urna de contingência).
* **Valores codificados e horários (B20, B22, B23).** `10.23.0.0 - Praia da Barra do Cahy` é uma
  string de toda a build (arquivos do VOTA, do RED e do SA; logs do SCUE, do GAP, do ATUE e do VOTA).
  O nome do binário, `contingenciaSecao (4)`, é o que o TSE adotou em 2026 (`bu.asn1`); o texto de
  2024 tinha `reservaSecao`. Os horários de carga, abertura, encerramento, emissão e geração estão
  nessa ordem em 129/129 BUs.
* **Uma seção atrasada (B25).** O único status fora do padrão na S110, uma seção do PA recebida dois
  dias atrasada com um `imgbu.dat` a mais na sua lista de transmissão (que responde 404), tem
  arquivos de urna comuns, que se verificam. Nem os arquivos de urna nem o código explicam o atraso;
  presumivelmente é uma questão de transmissão ou do lado do TSE.
* **Transmissão única (D24).** Todas as 2.270 seções do AC e todas as 135 da S135 têm exatamente um
  hash transmitido, então o significado das seções com vários hashes não pôde ser estudado.
### 4.5 Cadeia de preparação: GEDAI-UE → urna → BU

* **O fluxo de trabalho do AC em 2026 (E1).** Dos 22 logs oficiais do GEDAI-UE do AC (30.023 linhas,
  454 sessões): toda sessão começa com "Abertura do GEDAI-UE - Versão 8.23.0.0 - Praia da Barra do
  Cahy", a identidade do computador, a fase OFICIAL, a UF AC, o usuário do Windows, o Holocron 4.20.0.0
  e uma verificação de assinatura de `gedai-ue.vst`, depois `app.ini.vsc`. 104 pacotes foram
  importados de 16 a 24 de set.; 2.850 mídias de votação (FV) foram geradas de 17 a 20 de set. em 13
  PCs da zona 001; 88 mídias de carga (FC), de 18 a 20 de set., em só dois deles, 44 cada, com 10–32
  seções cada; as FCs listam as 2.270 seções do AC exatamente uma vez (2.141 seções simples e 129
  principais; as 141 seções agregadas não são listadas). 133 execuções de mídias de resultado: VOTA
  108; SA, RED, VPP e ATUE 13; ADH 9 (123 seriais); AVPart 3. 5.924 linhas de correspondência (24 de
  set. a 3 de out.) vieram 4,97–14,89 dias depois de a FC ser gerada. Dois PCs de zona só importaram
  pacotes; os dois PCs centrais também trataram correspondências. Os logs são enviados para
  `https://gedai-produtor.tse.jus.br:443/gedai/rest/jms/enviarLogGedai/`.
* **A identidade do gerador (E2, D9, E15).** A linha do GEDAI "Computador | Serial TPM | Serial
  instalação" é exatamente `IdentificadorGeradorMidia {nome, serialCertificadoTPM, serialInstalacao}`
  e a trinca `*_GERACAO_MIDIA` dos CSVs (o leiame define o campo TPM como "Número de série do
  certificado EK do TPM"). Em 62/62 urnas do AC, a trinca do BU é igual à trinca "Mídia de carga gerada
  pelo computador" do SCUE e à trinca do nome de arquivo do PC do GEDAI que registrou a mídia de carga
  do BU. Formatos: nome `Z<UF><zona:3>STD<2–3 digits>`; serial TPM de 20, 32 ou 40 caracteres hex nos
  22 PCs do AC (no país, 44/110 BUs da S110 trazem um serial TPM de 8 hex); serial de instalação de 8
  hex. O serial TPM é estável enquanto os nomes mudam: dois PCs foram renomeados, e um TPM aparece em
  2024 e em 2026 com nomes diferentes, então uma máquina é identificada pelo serial TPM, não pelo nome.
* **Qual PC o BU registra (E3).** O BU registra o PC que gerou a mídia de carga (FC). A trinca "Mídia
  de votação gerada pelo computador" do GAP é o PC que gerou a mídia de votação: ela é igual à trinca
  do BU em 0/62 urnas. As mídias de votação dessas 62 urnas vieram de 11 PCs; as mídias de carga, de
  2.
* **O campo de usuário (E6).** O usuário que o GAP registra para a mídia de votação corresponde ao
  logon do Windows da sessão do GEDAI que a gerou (62/62 urnas), e não ao login do Odin (52/62; as 10
  falhas são exatamente as sessões em que os dois diferem). Portanto `DadosGeracaoMidia.usuario` guarda
  o logon do Windows (a ligação entre a linha do GAP e esse campo é inferida). Nenhum identificador é
  reproduzido aqui.
* **Seriais das mídias (E7, D8).** Todos os 3.061 seriais de mídia (2.850 FV, 88 FC, 123 ADH) têm 8
  dígitos hex em maiúsculas, todos distintos, de acordo com as regras do código
  (`src/ecourna/app/dados/cserialmidia.cpp:25`,
  `src/uenux2/src/app/comum/dados/md/correspondencia/ccarga.cpp:25-28`). Nove eventos "Sobrescrevendo
  mídia de votação" mostram que uma mídia gerada de novo recebe um serial novo: o serial pertence a uma
  geração do conteúdo, não ao cartão. O "flash card" do bweb é o serial da mídia de carga
  (`Carga.numeroSerieFC`, "Serial da MC"), nunca o da mídia de votação, que não é publicado. 88 FCs
  levam as 2.270 seções do AC, 10–32 seções cada.
* **O código de carga (E8, D1, D23).** Ele aparece no GEDAI só nas correspondências lidas de volta das
  mídias de carga, nunca quando uma FC é gerada; junto com a linha do SCUE "Código de carga … gravado
  na tabela de correspondência", isso mostra que a urna o cria na carga. Todos os 5.924 têm 24 dígitos;
  284 dos 2.877 códigos distintos começam com 0. As duas colunas do bweb `CD_CARGA_1` + `CD_CARGA_2`
  são a forma impressa pela urna (`GetIDCargaFormatado`), e `CD_CARGA_2` é o "RESUMO DA
  CORRESPONDÊNCIA" impresso por ela (4.540/4.540 valores). O código não é uniformemente aleatório: em
  2.846 códigos do AC há 1.278 prefixos distintos de 6 dígitos (2.842 esperados se fosse aleatório),
  todo prefixo compartilhado fica dentro de uma zona, o dígito 6 é determinado pelos dígitos 1–5 nesta
  amostra (não é um dígito de Luhn nem de mod-10/11 ponderado), e a estrutura se estende até uns 8–9
  dígitos. O VOTA só verifica "24 dígitos decimais".
* **A cadeia inteira (E9).** Para 62 urnas do AC (5 da S110 e 57 baixadas), 33 verificações por urna
  ligam os logs do GEDAI, as linhas do SCUE e do GAP da urna e o BU: seriais, trincas do gerador,
  carga, datas e ordem. 31 passam em 62/62, e as 2 que envolvem o usuário do Odin passam em 52/62
  (E6). O horário de geração da FC registrado pela urna fica 168–259 s depois do "Início da geração de
  mídia de carga" do GEDAI; as urnas foram carregadas de 23 a 27 de set., 3,9–9,1 dias depois de a sua
  mídia de votação ser gerada.
* **Versões (E10).** 2026: programas da urna `10.23.0.0`, GEDAI-UE `8.23.0.0`, Holocron `4.20.0.0`,
  todos "Praia da Barra do Cahy"; HotSwapFlash `8.12.0.0`. AC 2024: urna `9.30.0.0 - Tupiniquim` (nos
  logs e nos BUs), GEDAI `7.29.0.0`, Holocron `3.20.0.0`. Os componentes compartilham o nome da versão
  dentro de um ciclo, o número principal sobe um por ciclo, e o `.23` comum de 2026 não é uma regra.
  Os arquivos de exemplo de 2024 com `9.29.0.1 - Kayapó` são da fase simulada (junho de 2024), não da
  eleição.
* **Biblioteca compartilhada (E14).** Todas as 109 linhas ERRO do GEDAI têm a forma do `CError` da
  ecourna (`function:line:code - message`, `src/ecourna/api/exception/cerror.cpp:31-33`), todas com o
  código 3400, e uma retorna `ecourna::app::dados::CCorrespondenciasUrna`, então o GEDAI-UE linka o
  mesmo namespace de modelo de dados que contém `CInformacaoMidia`, `CDadosGeracaoMidia` e
  `CIdentificadorGeradorMidia`.
* **Quem assina a mídia de votação (E17, inferência).** O GAP verifica o `infomidia.vsc` da mídia de
  votação (o wasm nunca faz isso: `src/uenux2/src/app/comum/validamidia/cvalidamidia.cpp:46-47`); uma
  urna registrou "Falha ao validar assinatura SEVIN | pacote: (/dsk/fe/estatico/infomidia.vsc)" duas
  vezes. O `sevin.ber.pub` da urna é nacional e igual ao `osevin.ber.pub` do Holocron; o Holocron
  também traz `osevin.ber.pri` e roda em toda sessão do GEDAI. Então a mídia de votação muito
  provavelmente é assinada no PC do GEDAI com a chave SEVIN mantida pelo Holocron.
* **Mídias de resultado (E18).** Os nomes de mídias de resultado do GEDAI (VOTA, SA, RED, VPP, ATUE,
  ADH) correspondem um a um ao `TipoAplicativo` do código; AVPart não tem valor no enum, o que combina
  com a sua mídia não ter `infomidia`
  (`src/uenux2/src/app/comum/validamidia/cfabricaconteudomidiastart.cpp`, case 20).
* **Fixtures do simulador (E4, E5).** Um BU do simulador traz o gerador `nome_maquina` / `12345678`
  / `99999999` (de `eg.bin`), e não `simulador-votacao-ng`. Nenhum dos nomes das fixtures tem a forma
  real `Z<UF><zona:3>STD<2–3 digits>`; o serial TPM de 64 zeros da fixture de `infomidia` é mais longo
  que qualquer serial real e que o limite de 20 octetos da RFC 5280 para seriais X.509; e o horário de
  carga da fixture, `20201231T235958`, tem segundos, enquanto todos os 62 `dataHoraCarga` reais
  terminam em `00`. As fixtures do serial de instalação seguem o formato real de 8 hex. Os
  conversores não verificam nada disso.
* **Larguras dos números (D25).** A urna e o GEDAI completam os identificadores com zeros à esquerda
  (carga 24, urna 8, município 5, zona 4, seção 4 dígitos); os CSVs tiram esses zeros de forma
  inconsistente (`CD_MUNICIPIO` do bweb sem zeros, CEFT com zeros), então cruzamentos exigem
  normalização.

### 4.6 Logs e parâmetros de 2026 recuperados

Os 110 logs da S110 têm 622.567 linhas em Latin-1 (VOTA 566.963, GAP 33.538, ATUE 10.260, SCUE 9.329,
LOGD 2.214, INITJE 263). O arquivo de parâmetros de 2026 não é publicado, então estes valores foram
recuperados do código junto com os logs; a maioria é igual aos valores dos cenários do simulador.

| parâmetro ou comportamento | valor nas urnas reais | código | ids |
|---|---|---|---|
| período do monitor | exatamente 1.800 s (2.200/2.200 intervalos) | `INTERVALO_LOG_S = 1800`, `src/uenux2/src/app/vota/monitor/cthreadmonitor.h:65` | C6 |
| tensão na linha do monitor | só nos modelos ≥ 2020 (UE2020 1.105/1.105 ciclos, UE2022 1.015/1.015; UE2013/2015 0/454) | `src/uenux2/src/app/vota/monitor/cthreadmonitor.cpp:265` | C6 |
| alerta de inatividade | 45 s (1.074/1.074 linhas) | `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp:359-360` | C7 |
| tentativas de digital | 4 (37.444/37.444 linhas "de [4]"); sucesso na tentativa 1/2/3/4: 17.142/3.552/1.352/628; identificação manual só depois de 4/4 | `src/uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp:134` | C8 |
| timeouts da digital | 30 s na primeira, 15 s nas seguintes | `src/uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp:84-85` | C8, F4 |
| novas tentativas do ano de nascimento | no máximo 2 (nunca 3 entradas erradas) | `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp:446-470` | C8, F16 |
| limiar de correspondência da digital do mesário | pontuação ≥ 20 (720 correspondências com 20–271; 2.828 não correspondências com 0–19) | `src/uenux2/src/app/comum/comparecimentomesario/ccontroladorreconhecimetomesario.h:29` | G5 |
| inspeção da cabine | a cada U(60, 90) minutos (634 intervalos: 60,0 a 93,9, mediana 74,5), só enquanto o terminal do mesário espera um identificador; 0 de 747 dentro de uma sessão de eleitor | `src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp:36` | C9 |
| abertura | estritamente depois do horário de início (95 urnas abriram às hh:00:01 ou hh:00:02, nunca às hh:00:00); hora de início 06 no AC, 07 em AM/MS/MT/RO/RR, 08 nos demais e no exterior (relógios locais) | `src/uenux2/src/app/vota/u26-foreign-fragments.cpp:107` | C10, F17 |
| encerramento | recusado antes do horário local de término (115/115 pedidos coerentes) | `src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp:207-214` | C10 |
| desligamento automático na bateria | 1.800 s (8/8 eventos em 1.801–1.802 s, todos em dias de preparação) | `src/uenux2/src/app/vota/eleitor/cestadocomdesligamentoautomatico.cpp:49-84`, `src/uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp:34-51` | C11, G7 |
| limite de eventos de energia | 20 (2/2 linhas de suspensão, cada uma depois de exatamente 20 trocas de fonte) | `src/uenux2/src/app/comum/util/cmonitoraalimentacao.h:50` | C11, G6 |
| limite de mesários registrados | 6 | `src/uenux2/src/app/vota/u26-foreign-fragments.cpp:256` | C11 |
| monitor de energia | uma linha "Carga" depois de cada troca para a bateria (165/165), só para a bateria em uso (181/181) | `src/uenux2/src/app/comum/util/cmonitoraalimentacao.cpp:58-96` | G6 |
| corte rígido depois de `terminoVotacao` | nunca acionado; 48 seções receberam 638 votos depois das 17:00 de Brasília, o último 104 minutos depois | `src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp:170-185` | F8 |

Outros resultados dos logs:

* **Suspensão parcial (C12, B12).** Em 11/11 suspensões parciais, as linhas de voto confirmado e de
  nulo somam as seis escolhas; quando só a primeira vaga do Senado foi confirmada, é registrado um nulo
  para Senador. Três suspensões sem nenhum voto registram "Eleitor foi suspenso e não confirmou nenhum
  voto" e não contam como comparecimento.
* **Ordem enviar-depois-registrar (C14).** "Eleitor foi habilitado" (thread do eleitor) vem logo
  depois de "Tipo de habilitação do eleitor [biométrica]" (thread do operador) 22.611 vezes e vem antes
  dela duas vezes, durante rajadas de log do monitor. Uma ordem registrar-depois-enviar tornaria a
  inversão impossível, então os dados apoiam a ordem reconstruída em
  `src/uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp:66-72`.
* **Exceções do GAP (C15).** As três linhas WHAT são textos de exceção do GAP exatamente no formato
  `CError` da ecourna; uma informa o erro 9342, um número que o VOTA usa para outra coisa
  (`ERRO_TELA_BRANCO_CONSULTA`, dentro da faixa 9300–9500 do VOTA), então os códigos de erro não são
  únicos entre aplicativos.
* **Teclas recusadas (C17).** 71.318 linhas "Tecla indevida pressionada": 2,67 por voto computado, 0
  fora de sessões de eleitor, 1,53–4,30 por voto nas seções do Brasil e 0,03–0,24 no exterior (só
  Presidente). 29,0% são seguidas de uma confirmação em até 1 s (linha de base 16,5%). Isso é compatível
  com uma tecla antecipada durante a tela de verificação de 1 s, mas igualmente com outras recusas que
  acontecem logo antes do CONFIRMA; horários com resolução de 1 s não permitem distinguir os dois
  casos. Nenhum dos 128.222 pares consecutivos de confirmações cai dentro de um segundo.
* **Instruções nunca vistas (C19).** 43 das 163 instruções de log em `src/` nunca aparecem na S110.
  Elas se dividem em linhas posteriores ao empacotamento (14, nunca em um log publicado), uma linha só
  de desenvolvimento, duas bloqueadas por configuração, uma só de treinamento, umas 10 escolhas raras
  do operador e uns 12 caminhos de erro.
* **O log publicado termina no empacotamento (G13, B7).** Em 110/110 logs, o último registro é
  `Gerando arquivo de resultado [log.jez] + [Início]`; a cópia para as mídias interna e externa e a
  assinatura dos arquivos de resultado
  (`src/uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp:320-365`) acontecem depois dele,
  então só existem no log corrente da urna.

### 4.7 Aleatoriedade e sigilo do voto

* **O gerador aleatório da urna não é constante (G3).** A build web registra `CPrng(0)`
  (`src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp:449-450`), que refaz a semente a cada chamada e
  sempre devolve 209652397, então todo id de imagem de digital seria 652606 e uma segunda imagem
  entraria em laço infinito
  (`src/uenux2/src/app/comum/reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp:47-61`).
  Os logs reais imprimem 274 ids de imagem distintos (2.878–999.190) em 99 seções, nunca 652606,
  nenhum compartilhado entre seções, com 2–5 ids distintos dentro de uma mesma execução em 84 seções e
  um qui-quadrado por decis de 6,58 (9 graus de liberdade). Isso descarta um gerador constante e uma
  semente compartilhada entre urnas; não mede a qualidade criptográfica. Que o mesmo gerador também
  alimente os bytes aleatórios do CEPESC é uma suposição.
* **Ordem do RDV (B10).** Em toda lista de cargo (534 na S110, 101 no conjunto escolhido, SA incluído),
  os votos são ordenados por (tipo de voto, dígitos digitados como string de bytes), e não
  numericamente, sem posições vazias ou fictícias: o número de votos é igual a comparecimento ×
  escolhas. A ordem lexicográfica e a numérica diferem em 223 grupos (cargo, tipo) da S110 (183
  legenda, 40 nulo) e em 38 do conjunto escolhido, e os arquivos sempre seguem a ordem lexicográfica
  (`src/uenux2/src/app/comum/dados/crdvposicionadorvota.cpp:13-22`). A ordem de votação não pode ser
  recuperada, e as duas escolhas para o Senado de um mesmo eleitor não podem ser pareadas.
* **Votos guardados como digitados (F13).** Um voto de partido (legenda) mantém os dígitos digitados.
  Dos votos de legenda da S110, 869/1.317 para Deputado Estadual, 146/714 para Deputado Federal e 18/43
  para Deputado Distrital têm mais que os 2 dígitos do partido, mas menos que o número completo;
  contando as entradas de 2 dígitos, 1.061/1.317 e 365/714 são mais curtos que um número completo. Há
  462 (Estadual), 308 (Federal) e 27 (Distrital) strings distintas entre os votos de legenda, até 71
  para um mesmo partido. Isso corresponde ao art. 207 e ao art. 204 §1 ("cada voto, como digitado").
* **Strings digitadas únicas (G12).** 3.410 votos em 108/110 seções têm um par (tipo, dígitos) que é
  único em sua seção e cargo: 2.083 nulos, 1.163 votos de legenda com três ou mais dígitos e 164 nulos
  de senador repetido (por exemplo, 1.068 de 3.594 nulos para Senador; 698 de 1.125 votos de legenda
  para Deputado Estadual). Contando só a string de dígitos, em todos os tipos juntos, dá 3.246. Ver §6.
* **A chave do RDV na flash da urna (G11).** `GetCifradorCryptoTable`
  (`src/uenux2/src/app/comum/dados/crdv.cpp:45-91`) deriva a chave AES-256 e o IV do RDV gravado a
  partir do SHA-512 dos códigos de cargo ordenados (o salt) e de 32 bytes tirados de uma tabela de
  hardware de 128 bytes (slot 6 de `IUrna`, `src/uenux2/src/api/hwil/iurna.h:53-54`), por HKDF-SHA512.
  O 1º turno de 2026 tem 3 listas de cargos: (1,3,5,6,7) em 102 seções (estados), (1,3,5,6,8) em 4 (DF)
  e (1) em 4 (exterior), portanto 3 salts, que leem 28, 30 e 28 posições distintas da tabela. Numa urna
  cuja tabela não muda, a chave e o IV são função pura da lista de cargos e se repetem em toda eleição
  com os mesmos cargos; um 2º turno só com Presidente reutilizaria o salt de (1). Não se sabe se a
  tabela é por dispositivo.

## 5. Regras eleitorais (Res.-TSE 23.751/2026 e procedimentos dos mesários de 2026) frente ao código

O texto que rege é a Res.-TSE 23.751, de 26 de fev. de 2026 (atos gerais do processo eleitoral para as
Eleições 2026), lida de uma cópia literal de terceiros; a Res. 23.736/2024 é a equivalente para as
eleições municipais de 2024, e a Res. 23.760/2026, o calendário (F24). O Manual do Mesário de 2022 é o
único texto de manual verificado palavra por palavra; o manual e o curso de 2026 são conhecidos por
extratos de busca. As colunas de dados reais se referem à S110 (26.740 sessões de eleitor).

| regra | o que diz | código | dados reais de 2026 | resultado | ids |
|---|---|---|---|---|---|
| art. 135 §1 II; Manual 2022 l. 749-751 | depois que o ano de nascimento é aceito, o eleitor "será habilitada(o) a votar mediante a leitura da digital da mesária ou do mesário"; o presidente "posiciona o próprio dedo … para atestar o procedimento" | `src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp:285-297`: o dedo é comparado só para o log; o comentário diz "the voter is released in every case" | 1.605 liberações desse tipo; 1.055 atribuídas a um mesário registrado, 321 só correspondem a uma captura anterior não registrada, 229 não correspondem a nenhuma digital guardada (mesários registrados ou as 6 capturas desconhecidas mais recentes): 550 (34%) não atribuíveis, 6,0% de todas as sessões | lacuna de fiscalização: a regra descreve um procedimento humano; a urna registra o dedo, mas não verifica de quem é | F1 |
| arts. 143-144; manual de 2026 (extrato de busca) "digite o número do título do(a) Presidente" | a suspensão é feita por "a(o) Presidente da Mesa" | `src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp:120-121`: qualquer título com dígitos verificadores válidos que seja diferente do identificador do próprio eleitor | 29 títulos de suspensão digitados: 14 aceitos (11 de um mesário registrado, 3 não), 15 recusados; só 3 dos 15 falham nos dígitos verificadores, e a causa das outras 12 recusas está em aberto | lacuna de fiscalização | F2 |
| art. 127 caput e I; manual de 2026 (extrato de busca) | o encerramento cabe "à(ao) Presidente … ou a quem ela(e) designar, dentre os componentes da Mesa" | `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp:835-848`: qualquer entrada só de dígitos que seja válida como título depois de completada com zeros até 12 | 112 títulos de encerramento: depois do mesmo preenchimento com zeros, 111 são de um mesário registrado e 1 não é | lacuna de fiscalização, pequena; a própria resolução permite um membro designado | F3 |
| Manual 2022 l. 729-730 | mensagem depois de 15 s sem dedo | `src/uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp:84-85` | 30 s na primeira tentativa, 15 s nas seguintes | **divergência** (§1) | F4 |
| art. 127 VII | relatório "Eleitores Não Reconhecidos Biometricamente" | `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp:130-150` | "Eleitores com habilitação biográfica" em 106/110 | **divergência** de nome; o escopo depende da leitura (§1) | F5 |
| art. 123 | mesários registrados depois da zerésima e antes da votação | `src/uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp:45-47`, `:76-77` | 4/110 seções abriram sem nenhum registrado | **divergência** (procedimento não seguido, o código permite) | F6 |
| art. 210 XII | os BUs trazem as contagens de biométricos, biográficos e sem biometria | `src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp:179-181` | ausentes nos 4 BUs de urnas não biométricas (exterior) | **divergência** (formal) | F7 |
| art. 160 §§2-3 | eleitores na fila votam "até que a última eleitora ou o último eleitor vote"; sem limite de horário | `src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp:170-185`: depois de `terminoVotacao`, a identificação é bloqueada quando ninguém vota por 5 minutos | nunca acionado; 638 votos depois das 17:00 em 48 seções, até 104 minutos depois | um mecanismo que as regras não mencionam; seu valor de 2026 não é público | F8 |
| códigos do Manual 2022 (555555555555 etc.); menu de 2026 (extrato de busca) | 2026: CORRIGE → "outras opções" → 1 áudio, 2 encerrar | `src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp:63-66`, `:170-205` | escolhas de menu registradas: áudio 2, encerramento 115, registrar mesários 38, contadores 6; não existe código de 12 dígitos em `src/` | confere com 2026; os códigos de 2022 sumiram | F9 |
| art. 142 §1 | painéis: Dep. Federal, Dep. Estadual/Distrital, Senador 1ª e 2ª vaga, Governador, Presidente | ordem tirada dos dados da eleição (`src/uenux2/src/app/comum/u30-foreign-fragments.cpp:90-97`) | 24.523 sessões completas nessa ordem, 1.116 com Distrital | confere | F10 |
| art. 31 | eleitores em trânsito fora de sua UF votam só para Presidente | `src/uenux2/src/app/comum/dados/celeitores.cpp:37-44`, `src/uenux2/src/app/comum/dados/ccargos.cpp:142-170` | 38 e 13 sessões só com Presidente nas duas seções com esses eleitores; 1.036 no exterior | confere | F11 |
| art. 206 §2 | mesmo senador para as duas vagas: o segundo voto é nulo | `src/uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp:67-80` | 736 nulos desse tipo, cada um com o voto nominal correspondente | confere | F12 |
| arts. 207, 204 §1 | votos de legenda; o RDV guarda cada voto "como digitado" | `src/uenux2/src/app/comum/dados/u04-foreign-fragments.cpp:370-384` | números parciais guardados como digitados (§4.7) | confere | F13 |
| arts. 94 V, 208 II, 206 | só os inaptos proporcionais são carregados; seus votos são nulos; números majoritários desconhecidos são nulos | `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp:240-250`, `:413-421`; `src/uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp:52` | não testável (sem tabelas de candidatos na amostra) | confere pela leitura do código | F14 |
| arts. 143, 144 §3 | os votos não confirmados de um eleitor que abandona a votação são nulos; sem nada confirmado, o eleitor pode voltar | `src/uenux2/src/app/vota/eleitor/celeitorvotando.cpp:329-370` (parametrizado) | nulos depois de suspensão: Presidente 11, Governador 10, Senador 7; nenhum branco depois de suspensão; 3 suspensões sem voto descartadas | confere (parâmetros de 2026 "descartar" e "nulo") | F15 |
| arts. 133 §2, 134, 135 | até 4 leituras biométricas; depois o ano de nascimento, com mais uma tentativa; depois o Caderno | `src/uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp:89-93`; `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp:446-476`; `src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp:321-331` | N = 4 em todas as linhas; erros de ano de nascimento nunca 3 seguidos; ano de nascimento pedido também a eleitores sem biometria (novidade de 2026) | confere, exceto F1 e F4 | F16 |
| arts. 129, 160 | votação das 08:00 às 17:00, horário de Brasília | `src/uenux2/src/app/vota/u26-foreign-fragments.cpp:90-112`; `src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp:206-215` | nenhuma liberação antes das 08:00 de Brasília; um pedido de encerramento antecipado recusado (13:27 local, "após as 16:00:00 horas"); eleitores na fila depois das 17:00 | confere (nos dados, os horários são locais por município) | F17 |
| arts. 121, 122 | às 07:00 de Brasília a mesa verifica a urna; o presidente então imprime a Zerésima e o seu Resumo | `src/uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp:23-35`; `src/uenux2/src/app/vota/u20-foreign-fragments.cpp:208-216` | zerésima 0–62 minutos depois das 07:00 (mediana 30) em 110/110; uma zerésima com mais de 3 h de atraso exige que o mesário confirme o relógio (só no código) | confere; a confirmação do relógio é um extra | F18 |
| art. 121 III, art. 126 II | teste de teclado obrigatório | `src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp:28-33`, `:64-69` | 101 testes pulados, todos em dias de preparação; 1 no dia da eleição, depois de um reinício com a zerésima já impressa | confere | F19 |
| art. 162; curso de 2026 (extrato de busca) | 5 vias obrigatórias e até 5 vias adicionais do BU; vias obrigatórias, retirar a mídia de resultado, vias adicionais | `src/uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp:54-87`, `src/uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp:62-90`, `src/uenux2/src/app/vota/eleitor/fimvotacao/cretirarmr.cpp:26-69` | "qualidade OK" na primeira via em 110/110; as quantidades de vias não estão no log publicado | confere com 2026; difere do manual de 2022 | F20 |
| Manual 2022 l. 1134-1136; art. 147 | um voto interrompido por falta de energia não é gravado e o eleitor pode voltar | a cédula fica na memória até o último cargo; reinício só com CONFIRMA (`src/uenux2/src/app/vota/eleitor/iniciovotacao/creiniciovotacao.u07.cpp:56`) | 3 reinícios, nenhum com sessão de eleitor aberta; 8 desligamentos por bateria, todos em dias de preparação | confere (o caso no meio do voto é só código) | F21 |
| arts. 142 §§3-5, 133 III-IV, 168 §2 IV, 140 §4 III | mensagem de cargo sem candidato; o terminal do mesário mostra o cargo; CPF ou título e foto; justificativa na urna; áudio | `src/uenux2/src/app/vota/eleitor/celeitorvotando.cpp:165-166`; `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp:145`; `src/uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp:86` | identificações por CPF 6.125; foto mostrada 23.801 vezes; 673 justificativas | confere | F22 |
| descrições das teclas do TSE (extrato de busca) | "BRANCO e depois CONFIRMA"; CORRIGE "apaga aquela escolha" | `src/uenux2/src/api/gui/iinputfield.u17.h:28-63` com as flags de campo de `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp:70`, `:137` | coerente com todos os testes de teclas registrados na análise anterior | confere (significado das flags inferido dos offsets dos membros) | F23 |

## 6. Observações relevantes para segurança e privacidade

Estes são fatos medidos e inferências identificadas como tais. Nenhum deles mostra alteração de votos;
vários mostram o que os arquivos publicados podem e não podem provar.

**Observações**

* **Os números de título dos mesários estão nos logs de urna publicados pelo TSE (G2).** Doze modelos
  de linha de log do VOTA imprimem um número de título, por exemplo "Mesário <N> registrado" (814
  linhas, 110 seções), "Mesário <N> habilitou o eleitor" (1.055 linhas), "Biometria coletada não é do
  mesário <N> (<id>)" (2.828 linhas) e "Título digitado para encerramento: <N>" (112 linhas). Todo
  número nos modelos de mesário é um título válido pela própria regra da urna (as entradas de
  suspensão recusadas são a exceção: 12 de 15 são). Na S110 há 440 títulos distintos de mesários, 2–7
  por seção, em 110/110 seções, e o log liga cada um ao id de uma imagem de digital guardada no
  `wsqmes.jez`, que não é publicado. Linhas do GAP e do SCUE também imprimem o número de 12 dígitos do
  usuário que gerou cada mídia (602 e 110 linhas). O identificador do eleitor é registrado só pelo tipo
  ("Identificador digitado pelo mesário foi: (Título de eleitor)"). Uma ressalva: se as 12 entradas de
  suspensão recusadas de F2 foram recusadas por serem iguais ao identificador do próprio eleitor (a
  única outra condição de recusa do código), essas 12 linhas publicadas trariam títulos de eleitores;
  isso não foi apurado. Este relatório dá só contagens, e os resultados da análise não contêm nenhum
  título.
* **Testes de teclado que falham são registrados como sucesso (G1).** Numa tecla errada,
  `CTesteTeclado` registra "Fim do teste de Teclado do TE - Sucesso" e depois mostra "Teste Falhou"
  (`src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp:126-138`). Nos logs
  reais, "Sucesso" é seguido de um novo "Início do teste" 7–48 s depois (o código espera pelo menos
  5 s) 27 vezes, em 21 de 110 seções (19%): pelo menos 27 testes que falharam aparecem como "Sucesso",
  e as teclas esperada e pressionada se perdem. A urna ainda obriga a repetir o teste; o defeito só
  afeta o registro de auditoria.
* **O terminal do mesário impõe menos que o procedimento (F1, F2, F3, F6).** A liberação pela
  "digital do mesário" aceita qualquer dedo: 550 de 1.605 liberações não puderam ser atribuídas a um
  mesário registrado. A suspensão aceita qualquer título com dígitos verificadores válidos que não seja
  o do eleitor: 3 de 14 títulos de suspensão aceitos não eram de um mesário registrado naquela urna. O
  encerramento aceita qualquer título válido: 1 de 112 não era. O registro dos mesários pode ser
  pulado: 4 de 110 seções abriram sem nenhum. Os dados não podem mostrar que alguém além de um mesário
  agiu (um mesário pode ter se registrado sem uma digital utilizável, ou um presidente pode não ter se
  registrado), e há responsabilização a posteriori pelas imagens de digital guardadas.
* **Um canal de marcação no RDV publicado (G12).** O RDV guarda os dígitos de cada voto como
  digitados, por regra (art. 204 §1) e pelo esquema do TSE ("Digitação como feita pelo eleitor na
  urna"). Um eleitor pode, portanto, produzir uma string que dificilmente aparece em outro lugar da
  seção, por exemplo um voto nulo com um número incomum. Como os RDVs são publicados, essa string
  poderia, em princípio, ser usada para mostrar a um terceiro que uma dada cédula foi votada, o que
  importa em casos de coação ou compra de votos. Os dados mostram que essas strings únicas existem em
  108/110 seções (3.410 votos); a maioria presumivelmente são erros de digitação comuns e números
  inválidos. Isto é a medição de uma propriedade conhecida por projeto, não um defeito.
* **Granularidade das chaves (G4, A4).** As chaves de assinatura são por urna, mas a chave do Código
  Verificador (`cv.ber.pri`), a chave da biometria dos eleitores (`bio.sk1`) e a chave do log
  (`log.ber.pri`, inferida) são uma por UF, decifradas com um segredo que é o mesmo nos quatro modelos
  de urna da UF (confiança média). Quem extraísse esse segredo de uma urna poderia decifrar os arquivos
  de chave daquela UF; para os templates biométricos, ainda seriam necessárias outras chaves e o
  algoritmo CEPESC.
* **O RDV gravado (G11).** A chave e o IV que protegem o RDV na flash da urna dependem só da lista de
  cargos e de 28–30 bytes de uma tabela de hardware de 128 bytes; houve 3 listas de cargos no 1º
  turno, e a mesma chave se repete em eleições com os mesmos cargos. A análise anterior (capítulo de
  análise 11-weird-code-audit, U2) descobriu que as cópias cifradas do RDV que foram substituídas não
  são sobrescritas; juntando as duas coisas, a confidencialidade dos instantâneos intermediários do RDV
  depende dessa tabela, cuja granularidade (por dispositivo ou não) não é conhecida.
* **O certificado identifica a urna que assinou (B4, B18).** Quando o RED roda em outra urna, o BU
  mantém a identidade da urna original, mas é assinado pela chave de hardware da urna do RED.
  Ferramentas que ligam um BU a uma urna pelo certificado precisam contar com isso.
* **Um certificado de autoridade certificadora vencido (H2, observação do revisor).** O certificado da
  AC URNA no script de verificação do TSE venceu em 2026-07-18; os certificados de urna UE2013/2015
  que ele emitiu valem até 2027/2030. Um validador que verifique as datas na cadeia inteira rejeitaria
  assinaturas de 2026 de urnas UE2013/2015.
* **O que o log publicado pode mostrar (G13, C5).** O log publicado para quando o `log.jez` é
  empacotado, então a assinatura e a cópia dos arquivos de resultado nunca aparecem nele. Cada linha
  traz um código com chave que terceiros não conseguem verificar; o log é protegido como um todo pelo
  arquivo de assinatura, que foi verificado em todos os logs publicados (B5).
* **Lacunas no que as listas de hash cobrem (E19, A7).** O `app.ini.vsc` e o arquivo de URIs oficiais
  que o GEDAI-UE carrega em toda inicialização não estão em nenhuma lista publicada, e
  `/uenux/bin/avusrbinof.vst` está listado, mas nunca é citado em nenhum log.
* **QR codes longos demais (H5).** Pelas regras do código, uns 13/110 BUs imprimem um último QR com
  mais de 1.100 caracteres; isso só contraria o manual de 2026 se ele mantiver o limite. Esse QR ainda
  cabe no padrão QR, impresso mais denso.
* **Arquivos não publicados.** O `jufa.dat` guarda o comparecimento e o resultado da habilitação de
  cada eleitor (`src/uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp:125-198`); ele está listado
  nos arquivos de assinatura, mas não é publicado, e não dá para saber se o parâmetro
  `criptografarJUFA` estava ligado. A opção de BU cifrado estava desligada (G8); de qualquer forma, os
  BUs são públicos.
* **Textos de exceção em logs publicados (H11).** As linhas WHAT do GAP imprimem assinaturas internas
  de funções C++ e números de linha do código-fonte. Menor.

**Não é problema**

* **A ordem do RDV não vaza nada (B10, G12).** Os votos são ordenados pelo conteúdo, então nem a
  ordem de votação nem o pareamento das duas escolhas para o Senado de um mesmo eleitor podem ser
  recuperados de um RDV publicado.
* **O gerador aleatório da urna não é o gerador constante do simulador (G3).**
* **As verificações de integridade passam em todo lugar em que podem ser feitas (B1, B5, B8, B9).**
  Todas as cadeias de hash, todos os certificados e todas as assinaturas da S135 são verificados; toda
  linha do BU decorre do seu RDV; nenhum valor de código desconhecido aparece.
* **A diferença de identidade BU/RDV das urnas de contingência é intencional (G9, B14).**
* **A suspensão, os horários e o teste de teclado obrigatório se comportam como as regras dizem (F15,
  F17, F19).**
* **Os parâmetros de 2026 recuperados dos logs batem com o código e, onde os cenários do simulador os
  definem, com os valores dos cenários (C7, C11, G7).**
## 7. Correções à documentação existente

### 7(a) Arquivos deste repositório

Toda mudança em `docs_en/` precisa da mesma mudança na tradução em português em `docs/` (mesmos
nomes de arquivo), e toda mudança no README_EN.md, no README.md.

| documento | seção | diz | deveria dizer | ids |
|---|---|---|---|---|
| README_EN.md | "What this is not", l. 47-48 | "The result files and logs they wrote are consistent with this code, which suggests the same source tree but does not prove the binaries are the same." | Manter, e acrescentar: 8 modelos de linha de log reais do VOTA (driver da impressora, um erro de exibição do terminal do mesário, "Votação suspensa") e o caminho de desligamento pela chave estão ausentes da reconstrução; duas chamadas de log têm a severidade errada; o log publicado termina quando o `log.jez` é empacotado. | C1, C2, C3, G13 |
| docs_en/glossary.md | l. 448, "versão" | "the real 2024 BUs say `9.29.0.1 - Kayapó`" | Os arquivos de exemplo de 2024 são arquivos da fase simulada (FASE:S, junho de 2024) da versão de pré-lançamento `9.29.0.1 - Kayapó`; as urnas da eleição de outubro de 2024 rodaram `9.30.0.0 - Tupiniquim`. | E10 |
| docs_en/data-model/asn1-schemas.md | §5, l. 420-431 | compara o binário só com as especificações v2 (2024) publicadas | Acrescentar o pacote de 2026: o `bu.asn1` é igual a `src/asn1` em 41/41 tipos; o `assinatura.asn1` de 2026 tem o `ModeloEquipamento` do binário e os novos `OrigemAssinaturaHardware`/`InfoChave`; o `rdv.asn1` de 2026 é idêntico byte a byte ao de 2024 e ainda diz `reservaSecao`/`urnaChegouAposInicioVotacao`. | H6, H7, H8, B22 |
| docs_en/modules/u23-uenux2-src-app-comum-gravadores-uenux2-src-app-comum-iinterf.md | l. 114 e l. 196 | `CEstadoGeralGap` "(all loads of this urna)"; `historicoCodigosCarga` "every load code" | O histórico de cargas da votação da seção: numa urna de contingência, ele começa com a carga da urna original da seção e pode incluir urnas de contingência intermediárias; uma nova carga o reinicia. | D6, B15 |
| docs_en/modules/u24-uenux2-src-app-comum-justificativa-uenux2-src-app-comum-log-.md | l. 356 | questão em aberto: "whether it really is `wsq.pk1`" | `/dsk/fi/estatico/chave/wsq.pk1` existe para toda UF nas quatro listas de urna de 2026 (forte indício; `bio.sk1` é o único outro nome de chave com 7 caracteres). O mesmo vale para o `?` em `src/uenux2/src/app/comum/reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp:71-75`. | A3 |
| docs_en/modules/u24-uenux2-src-app-comum-justificativa-uenux2-src-app-comum-log-.md e docs_en/modules/u11-ecourna-lib-ecourna-api-asn.md | u24 l. 175-176; u11 l. 354 | `t02400ac-pu.dat`, `t00000ac-pu.dat`, `t00000br-pu.dat` "(partidos of the pleito, of the UF, national)"; "`-pu.dat` (national + UF)" | `-pu` é a parametrização da urna (os partidos são `-pa`), e 02400 é o processo eleitoral do simulador; as mídias reais de 2026 e de 2024 trazem só o `o00000br-pu` nacional, e a forma como a func 7787 trata a falta do arquivo da UF está em aberto. | E12, E13 |
| docs_en/modules/u27-uenux2-src-app-vota-operador.md | l. 141 e l. 166 | `CPedeTituloEncerramento` "presidente's título"; "título typed by the presidente to close the vote" | Qualquer título que seja válido depois de completado com zeros até 12; nada identifica o presidente ou um mesário registrado (o art. 127 também permite um membro da mesa designado); 111/112 títulos de encerramento reais eram de um mesário registrado. | F3 |
| docs_en/modules/u40-lib-ecourna-classes-without-known-file.md | l. 335 | "simulator data: 'simulador-votacao-ng', 64 x '0'; 9224 observed through `CConversorDadoCorrespondencia` (11400, eg.bin)" | A correspondência de `eg.bin` traz `nome_maquina` / `12345678` / `99999999`; `simulador-votacao-ng` / 64 × '0' / `A1B2C3DA` é a fixture `infomidia-fv-*-t.dat`. Valores reais: um nome `Z<UF>###STD##`, um serial de certificado EK de 8/20/32/40 hex e um serial de instalação de 8 hex. O mesmo vale para os comentários em `src/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.h:17-21`. | E4, E5 |
| docs_en/modules/u16-uenux2-src-api-gui-cprogressbar-cpp-uenux2-src-api-gui-cqrco.md, docs_en/modules/u36-app-comum-classes-without-known-file.md, docs_en/modules/u19-uenux2-src-api-pattern-cpolysingletonlist-h.md | u16 l. 241; u36 l. 388; u19 l. 397 | every "BU DIGITAL" part "≤ 1100 characters"; "parts of ≤ 1100 characters; on-screen … ≤ 2500"; "at most 2500 characters each" | Todas as partes, menos a última, são limitadas; a última pode chegar a 1.245 (Ed521) ou 1.259 (ECDSA) caracteres impressos e a cerca de 2.659 na tela, porque a reserva de 277 caracteres é menor que o final da versão 6.0. O mesmo vale para o comentário em `src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp:276`. | H5 |
| src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | l. 123 e l. 130 | `CLogVota::GetInst().Loga(std::format("Título {} é inválido/válido para suspender a votação", …))` | `LogaAviso` (severidade 2, ALERTA), como o comentário de `api_f1398` já diz. | C3 |
| src/uenux2/src/app/comum/nomearquivo/cnomearquivo.h, cnomearquivo.cpp, src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | cnomearquivo.h l. 36 e l. 7; cnomearquivo.cpp l. 80; cdadosestaticos.u02.cpp l. 120-132 | `uedword pleito; // +4 CEstadoGeral +4`; "t02400ac-pu.dat (pleito 2400, UF AC, "pu" = partidos)" | O campo é o id do processo eleitoral (`idPE`, por exemplo 01219 nos nomes `-el`/`-tte`/`-imp`), não o pleito (03220, usado nos nomes dos arquivos de resultado); "pu" é a parametrização da urna. | E11, E12 |
| src/uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp | l. 252 | `const std::vector<uebyte> der = …RecuperarCertificado();` | O token devolve um texto PEM de 1.034 bytes nas UE2020/2022 e DER só nas UE2013/2015; o nome engana. | H3 |
| src/uenux2/src/app/vota/monitor/cthreadmonitor.cpp | l. 203-205 (comentário) | "whatever followed here is not in the binary. ?" | Acrescentar o que as urnas reais registram em seguida: "Votação suspensa" (só se a zerésima foi impressa naquele dia), depois "Desligando a urna" e "Finalização de aplicativo". | C2 |

### 7(b) Capítulos do projeto de análise original (fora deste repositório)

| documento | seção | diz | deveria dizer | ids |
|---|---|---|---|---|
| capítulo de análise 00-provenance | §1 (listas do HotSwapFlash e do GEDAI) | "Even with them, no hash could ever match" | Nenhum digest de binário pode bater, mas arquivos de dados podem: 42 arquivos do RHVoice da urna de 2026 são iguais aos do upstream e aos do pacote do simulador; e todo HASH GERAL publicado é reproduzido por `CMontadorHash::CalculaHashGeral`. | A5, A1 |
| capítulo de análise 00-provenance | §1, lista de pacotes do GEDAI | `vota_apl.jez`, `vota_ofi.jez`, `vota_sim.jez`, `vota_tre.jez`: "the VOTA application in its oficial / simulado / treinamento variants" | `vota_apl` é instalado junto com o pacote da fase; uma carga oficial real verifica `vota_apl.vst`, `vota_ofi.vst` e `avpart_ofi.vst` (110/110 instalações). Só `_ofi`/`_sim`/`_tre` são variantes de fase. | A8 |
| capítulo de análise 00-provenance | §3, linha da tabela sobre `eg.bin` | o "generator" da mídia registrado nos dados é `simulador-votacao-ng`, com um serial TPM todo zerado (funcs 10268/9952) | O gerador copiado para um BU do simulador (func 9952) é `nome_maquina` / `12345678` / `99999999`; `simulador-votacao-ng` está só na fixture `infomidia-fv-*-t.dat`. | E4 |
| capítulo de análise 00-provenance | §5, linhas do GAP | "a series of 34 steps" | 32–35 passos (494 execuções: 32 × 6, 33 × 119, 34 × 354, 35 × 15): 24 pacotes fixos de sistema e de chaves, depois os pacotes da seção; o número varia com as verificações da mídia de votação e com o número de pacotes `-ce`. | A18, A6, A10 |
| capítulo de análise 00-provenance | §5, "The media generator" | os dados por trás de "gerada pelo computador (nome - tpm - instalação)" fazem parte do registro da carga | Só a trinca "Mídia de carga gerada pelo computador" do SCUE é igual ao `Carga.identificadorGeradorMidia` do BU (62/62); a trinca "Mídia de votação gerada pelo computador" do GAP é o PC da mídia de votação, que difere em 62/62 e não é registrado no BU. | E3, E2, D9 |
| capítulo de análise 00-provenance §2 e glossário; também asn1-schemas, bu/qrcode e 10-boletim-de-urna, onde chamam os exemplos de 2024 de "real" | referências a 2024 | "the real 2024 BUs say `9.29.0.1 - Kayapó`" | Os exemplos de 2024 são arquivos da fase simulada de uma versão de pré-lançamento; a eleição de 2024 rodou `9.30.0.0 - Tupiniquim` (GEDAI `7.29.0.0`, Holocron `3.20.0.0`). | E10 |
| capítulo de análise libraries/boost-fmt-and-small-libs | item sobre o GEDAI | os pacotes da urna "in their encrypted `.jez` form" | `.jez` é um contêiner ZIP (110/110 logs de urna abrem sem senha, e os 22 + 22 pacotes de log do GEDAI também são ZIP); não se sabe se os pacotes de aplicativo do GEDAI são cifrados. Manter o ".jez packages are ZIP files" do 00-provenance. | A9, H9 |
| capítulo de análise libraries/rhvoice | §9 | "Nothing in the public files tells which RHVoice build or voice revision is on the machines." | As listas de urna de 2026 mostram `libRHVoice_core.so.1.14.0`; os arquivos de idioma (upstream `910c889`), a voz Letícia-F123 de 16 kHz e o `RHVoice.conf` (tag 1.14.0) são idênticos byte a byte aos do upstream e aos do pacote do simulador; os 8 placeholders do simulador existem só na versão web. | A5 |
| capítulo de análise data-model/asn1-schemas | §5 | compara só com as especificações v2 de 2024 | A mesma correção de `docs_en/data-model/asn1-schemas.md` §5, acima. | H6, H7, H8 |
| capítulo de análise 12-real-urna-2026 | §1, resultados | as 2 mensagens que faltam no wasm são "Início de impressão" e "Fim de impressão" | Em 110 urnas, 8 modelos de linha de log do VOTA (919 linhas) estão ausentes de `src/` (§3, linha 3); 183/191 são produzidos por ele. | C1 |
| capítulo de análise 12-real-urna-2026 | §2, "Same code, probably" | "Every kind of event the real VOTA logged is something the simulator's code can log, with the same text." | Quase todo tipo. Exceções: os 8 modelos, a sequência de desligamento pela chave e a severidade das duas linhas de título de suspensão (ALERTA nas urnas, INFO em `src/`). | C1, C2, C3 |
| capítulo de análise 12-real-urna-2026 | §2, "Tecla indevida pressionada" | "1.3 to 3.6 refused keys per computed vote" | Em 110 urnas: 2,67 no total, 1,53–4,30 por seção no Brasil, 0,03–0,24 no exterior; nunca fora de uma sessão de eleitor. | C17 |
| capítulo de análise 12-real-urna-2026 | §3, o arquivo de assinatura de 2026 | um bloco final com `ue2022` (22) e um segundo valor 4; os dois campos opcionais levam `[0]`/`[1]` | O bloco final é o `OrigemAssinaturaHardware {modeloEquipamento, algoritmoAssinatura}` do TSE: 4 (EdDSA, Ed521) nas UE2020/2022 e 2 (ECDSA P-521) nas UE2013/2015. `[0] tagChaves` e `[1] certificadoDigital` são as duas alternativas de um único CHOICE obrigatório `InfoChave`; o certificado é PEM nas UE2020/2022 e DER nas UE2013/2015. | B3, H6, E16, B4 |
| capítulo de análise 10-boletim-de-urna | §3.3, linha 10 da tabela | `historicoCodigosCarga`: "every carga code this urna had" | O histórico de cargas da votação da seção (ver a correção do u23, acima). | D6, B15 |
| capítulo de análise 10-boletim-de-urna | §4.2, último parágrafo, e §7 item 2 | o módulo PKCS#11 precisa tratar de forma diferente as assinaturas do arquivo e do QR; o algoritmo não é visível | Nos dados de 2026, as duas são verificadas sobre o SHA-512 do último hash bruto, com a mesma chave de hardware por urna (Ed521 nas UE2020/2022, ECDSA P-521 nas UE2013/2015); o cálculo do hash muito provavelmente acontece abaixo da interface `IPkcs11` (inferido). A regra de 2024 do QR sobre o hash bruto pertencia ao esquema Ed25519 por UF. | B5, H2 |
| capítulo de análise 10-boletim-de-urna | §4.3 e §7 item 4 | o QR do certificado sugere um certificado por urna (inferência); VRQR 6.0 / ausência de VRCH vistos só no harness | Os certificados por urna estão confirmados em 135/135 arquivos de assinatura reais (autoridades certificadoras AC UE2020, AC UE2022, AC URNA); o código reconstrói o exemplo do manual de 2026; o BU impresso de 2026 ainda não foi conferido. | B4, H1, H2, H3 |
| capítulo de análise bu/qrcode | §8, linha da assinatura, frase sobre o MDUE e parágrafo final | "IPkcs11 slot 6 over the raw last hash; the algorithm is not visible"; MDUE "probably the urna model"; uma chave por urna ou por módulo é uma inferência | Ed521 (UE2020/2022) ou ECDSA P-521 (UE2013/2015) sobre o SHA-512 do último hash bruto; ASSI com 264 ou 278 caracteres hex; chave num certificado por urna impresso como códigos QRCE (2 por BU para todos os 110 certificados reais; 1.071 caracteres cada para PEM, 983 para DER); MDUE muito provavelmente é o ano do modelo, com 4 dígitos. As §6.2–§6.3 (chaves Ed25519 por UF) descrevem só 2024. | H2, H3, H4, G10 |
| capítulo de análise bu/qrcode | §4, final | "Every QR in the files is ≤ 1,100 characters (the longest … has 1,073)" | Verdadeiro para os arquivos de 2024. Com as assinaturas da versão 6.0, o último QR pode passar de 1.100 (13/110 BUs de 2026 reconstruídos: 1.102–1.239). | H5, G10 |
| capítulo de análise bu/codigo-verificador | §4 | o arquivo de chave "presumably comes from the key packages that GEDAI-UE installs" (inferência) | `/dsk/fi/estatico/chave/cv.ber.pri` está nas listas oficiais de urna de 2026, um por UF (28 digests distintos), idêntico nos quatro modelos; a chave do CV é por UF (confiança média). | A3, A4, G4 |
| capítulo de análise 11-weird-code-audit | §2.2 U6 e §6 | "not verified: the TSE pages returned 403" | Confirmado: os logs publicados pelo TSE trazem 440 títulos distintos de mesários em 110/110 seções da amostra (12 modelos de linha de log), ligados a ids de imagens de digital; as linhas do GAP e do SCUE trazem o número de 12 dígitos do usuário que gerou cada mídia. | G2, E6 |
| capítulo de análise 11-weird-code-audit | §2.2 U7, impacto | um gerador constante na urna é "improbable" | Excluído pelos dados (274 ids de imagem distintos e uniformemente distribuídos; nunca 652606). | G3 |
| capítulo de análise 11-weird-code-audit | §2.2 U1 e U9, impacto | U1 e U9 apresentados como inferências | U1 observado: pelo menos 27 testes de teclado com falha registraram "Sucesso" em 21/110 seções. U9 medido: 869/1.317 votos de legenda para Dep. Estadual e 146/714 para Dep. Federal têm números parciais; 3.410 votos em 108/110 RDVs trazem uma string única na sua seção e no seu cargo. | G1, F13, G12 |
| capítulo de análise 11-weird-code-audit | §6, afirmações externas | a regra do voto repetido para o Senado veio de notícias da imprensa | Res.-TSE 23.751/2026 art. 206 §2; 736 desses nulos nos RDVs reais. | F12, F24 |
| capítulo de análise 08-voting-flow | §11 item 3 | "The meaning of each flag was not worked out." | Inferido dos offsets dos membros (confiança média): CORRIGE apaga o campo inteiro, BRANCO só age com o campo vazio, CONFIRMA só finaliza no campo dos dígitos restantes; isso explica todas as execuções registradas. | F23 |

## 8. Questões em aberto e o que não pôde ser verificado

**Documentos do TSE não lidos diretamente**

* O manual do QR de 2026, o "Log do ecossistema" de 2026, o Manual do Mesário, o Guia Rápido e as
  páginas de curso de 2026, e a página de hashes de 2026 não puderam ser lidos da máquina de análise.
  A Res.-TSE 23.751/2026 foi lida numa cópia de terceiros. Textos e números tirados dessas fontes
  devem ser verificados de novo em `www.tse.jus.br`. Em particular: se o manual do QR de 2026 mantém o
  limite de 1.100 caracteres (H5), se o documento de log de 2026 ainda fala em 7z e em sete campos (H9,
  H10), e se o `rdv.asn1` desatualizado de 2026 também está no pacote do próprio TSE (H8).
* As listas de hash foram lidas numa cópia de terceiros. Elas são coerentes entre si, mas os digests
  dos arquivos de chave em que A4 e G4 se apoiam não foram comparados com a página do TSE.
* Se os `verifica_qrcode*.py` da cópia do pacote de 2026 são do TSE (o README não os lista). As
  conclusões tiradas deles se apoiam em verificações criptográficas independentes.

**Artefatos que não são públicos**

* Um BU de 2026 impresso, ou a sua imagem `imgbu.dat` (HTTP 404): necessário para ver no papel os
  rótulos dos QR, `ASSI` e `CERT`, o valor de `MDUE`, o nome do certificado no QRCE (`UEAO`, como no
  arquivo de assinatura, ou `UEAD`, como nos dados de demonstração do TSE), o limite real de divisão e
  o último QR longo demais (G10, H4, H5, H17, H18).
* `jufa.dat`, `imgze.dat`, `hash.dat`, `mr.ver` e os pacotes `wsq*.jez` estão listados nos arquivos
  de assinatura, mas não são publicados. Por isso `criptografarJUFA`, o `local = 1` de `hash.dat`, a
  tag de contrato em `mr.ver` (espera-se `20260601173148`) e a assinatura SAVD de cada voto no RDV
  gravado continuam sendo só código. Não é possível fechar a questão de se o `hash.dat` da urna chega
  ao HASH GERAL por UF publicado (A1).
* Os valores de 2026 dos parâmetros que os logs não revelam: número de vias obrigatórias e adicionais
  do BU, `numTentativasVerificacao` (limitado a 2 pelos dados), `terminoVotacao`, o tempo do aviso de
  bateria, `permitirHabManualAudio`, o conjunto de justificativas de trânsito.

**Chaves e segredos**

* A construção e a chave do código de log de 16 hex (C5, A19): SipHash-2-4 ou 4-6, o arranjo da
  cadeia, e se a cadeia recomeça a cada início do `logd`.
* Se o segredo do IKernelHSM (G4) e a tabela de 128 bytes do RDV (G11) são por dispositivo, por UF ou
  nacionais. Só está estabelecido "nem por dispositivo nem por modelo, para a KEK", e isso se apoia na
  suposição de que um id de imagem registrado no log significa que `wsq.pk1` foi decifrado.
* Qual chave o SAVD usa para assinar os arquivos `.vsu` (um `ue.ber.pri` por UF ou uma chave de
  hardware), e se as chaves privadas do Holocron são protegidas (wrapped) pelo TPM (E17).
* Se os pacotes de aplicativo do GEDAI (`.jez`) são cifrados (A9).

**Questões de código sem acompanhamento**

* F2: por que 12 das 15 entradas de suspensão recusadas foram recusadas, embora passem nos dígitos
  verificadores do título. A única outra condição de recusa do código é "igual ao identificador do
  próprio eleitor"; não ficou resolvido se `CValidadorIdentidade::EhValida` verifica mais do que os
  dígitos verificadores. A resposta importa para a §6 (significaria que essas linhas publicadas trazem
  títulos de eleitores).
* A7: o GAP verifica `/uenux/bin/avusrbinof.vst` com o rótulo errado no passo 2, ou não verifica?
* A10: o `dadoscarga.vsu` em `/dsk/fe` na tabela SAVD do VOTA é um deslize, ou é a cópia da mídia de
  votação?
* E13: como a func 7787 trata a ausência de um `-pu.dat` no nível da UF.
* Os significados de `ste`, `vpe` e `avgm` (o log do GEDAI-UE dá os nomes `VPP` "Verificador Pré e
  Pós-Eleição" e `ADH` "Software de Ajuste de Data/Hora"); o texto oficial das definições de AVPART e
  SAVP (A6, A11, E18).
* O que `/etc/ld.so.preload` carrega, o que `ld.so.cache.sig` e `ld.so.preload.sig` protegem, e qual
  componente verifica `/boot/boot/avboot.vst` antes de o kernel rodar.
* `docs_en/modules/u14` (l. 359) chama os arquivos `infomidia-fv-{1,2}-t.dat` de "FC media"; neste
  relatório FC é a mídia de carga e FV a mídia de votação. Talvez seja um deslize; não foi testado.
* As 43 instruções de log de `src/` nunca vistas na S110 (C19), na maioria caminhos de erro e escolhas
  raras do operador.
* O truncamento de `size_t` de 32 bits do wasm em `FormataTamanho`: nenhum valor real chega a 4 GiB.

**Cobertura dos dados**

* A análise do GEDAI cobre só o AC (mais os preâmbulos de RR 2026 e AC 2024). Os seriais TPM de 8 hex
  de 44/110 BUs de outras UFs não foram rastreados até logs do GEDAI; as correspondências de 5 seções
  do AC faltam nos 22 logs do GEDAI do AC publicados (2.265 de 2.270 encontradas), por motivos não
  estabelecidos.
* O CEFT foi lido para 7 UFs, e o bweb só para AC e ZZ. Os fusos horários do AM em D5 não foram
  comparados com uma lista oficial por município.
* Não existe seção com vários hashes na S135 nem entre as 2.270 seções do AC, então o significado de
  uma retransmissão é desconhecido. Não há explicação para uma seção do PA ter sido recebida dois dias
  depois, com um `imgbu.dat` a mais (B25).
* RED e SA não estão no binário; o comportamento deles (B17, B18, D13) é descrito só a partir dos
  dados. Valores nunca vistos no 1º turno: `TipoUrna` 6, `TipoArquivo` 3/4/6, tipos de voto 5/8/9 do
  RDV, consultas, cargos municipais.
* O 2º turno (fim de outubro de 2026) ainda não tinha acontecido. A previsão de G11 de que um turno só
  com Presidente reutiliza o salt `(1)`, e todas as verificações de arquivos do 2º turno, não foram
  testadas.
* A contagem de H5 se apoia em placeholders de largura fixa para `PROC` e `VERC` e deixa `AGRE` de
  fora; ela pode mudar em uma ou duas seções.
* Nenhum voto em inapto ou em legenda de federação pôde ser identificado (não há tabelas de candidatos
  de 2026 na amostra), não houve queda de energia no meio de um voto, e o tamanho da janela de Libras
  não pôde ser comparado.
* Se os 3 títulos de suspensão e o 1 título de encerramento que não correspondem a nenhum mesário
  registrado pertenciam a um presidente não registrado: os logs não permitem dizer (F2, F3).
* A árvore em português `docs/` não foi revisada; toda correção da §7(a) vale para ela também.

## 9. Como reproduzir

Os scripts estão em `investigation/scripts/`; `investigation/scripts/README.md` explica cada um, e
`investigation/scripts/MANIFEST.json` lista as entradas, as saídas e os resultados que eles
reproduzem. Nenhum arquivo bruto do TSE está no repositório: os logs das urnas contêm identificadores
de mesários em texto claro, e cabe ao TSE publicar os outros arquivos. `fetch_sections.py` e
`fetch_open_data.py` baixam os arquivos das urnas e os dados abertos; as páginas HTML (ou os zips) da
página "Resumos digitais (hashes)" de 2026 do TSE precisam ser salvas à mão. Guarde os dados baixados
fora do repositório. As saídas contêm só contagens e modelos de linha de log: `log_templates.py` nunca
imprime linhas brutas e mascara sequências de 10–12 dígitos e os valores de computador e de usuário da
geração de mídias, e `vsc2026.py` mascara os dígitos dos nomes dos signatários de hardware. Revise
qualquer saída antes de publicá-la.

Requisitos: Python 3.9 ou mais recente com `asn1tools` e `cryptography` (`hashlists.py`,
`log_templates.py` e os dois scripts de download precisam só da biblioteca padrão); rode todo script
com `python3 -I`. A verificação Ed521 está implementada em Python puro em `sigtools.py`, então não é
preciso nenhuma outra biblioteca EdDSA. As funções auxiliares compartilhadas estão em `kitlib.py`; a
raiz do repositório é encontrada a partir da localização dos scripts, ou passada com `--repo`.

| passo | script | o que faz | resultados que reproduz |
|---|---|---|---|
| 1 | `fetch_sections.py` | baixa os arquivos de urna de 2026 por seção (`aux.json`, BU, RDV, log, arquivo de assinatura) de `resultados.tse.jus.br`: uma amostra aleatória com semente por UF (`sample`), seções indicadas pelo nome (`sections`), ou o número de seções por UF (`count`); seções sem arquivos publicados são informadas e puladas | dados para os passos abaixo |
| 2 | `fetch_open_data.py` | busca, lista e baixa recursos de dados abertos do TSE pela API CKAN de `dadosabertos.tse.jus.br`, conferindo cada download com o seu `.sha512` | dados para a §1 e a §4.5 (o kit não os analisa) |
| 3 | `hashlists.py` | lê as páginas HTML (ou os zips) dos "Resumos digitais (hashes)" do TSE, recalcula todo HASH GERAL, verifica a ordem de percurso dos diretórios, imprime o inventário por modelo e a variação das chaves por UF | A1, A2, A4 |
| 4 | `roundtrip_asn1.py` | decodifica e recodifica `bu.dat` e `rdv.dat` com `src/asn1` e compara os bytes | B1, B2 |
| 5 | `bu_vs_rdv.py` | recalcula a cadeia de hash dos votos e cada linha do BU a partir do RDV, com as regras do código de contagem, de ordem e de omissão das linhas zeradas | B8, B9 |
| 6 | `vsc2026.py` (com `sigtools.py` e `assinatura2026_inferred.asn`, o layout inferido por esta investigação, não um documento do TSE) | decodifica e recodifica os arquivos de assinatura de 2026, confere os digests e os tamanhos dos arquivos publicados, e verifica as assinaturas de hardware (arquivos, listas, eleições do BU) com o certificado de cada urna; com `--roots`, também verifica as cadeias de certificados contra os certificados AC UE2020, AC UE2022 e AC URNA, que não vêm junto e precisam ser tirados do pacote de verificação do TSE. A assinatura de software da CEPESC não pode ser verificada (não há implementação pública). | B3, B4, B5 |
| 7 | `log_templates.py` | lê `logd.dat` em memória, monta modelos de linha de log por programa e encontra os modelos do VOTA em `src/` | C1 (e as contagens de linhas da §4.6) |
| 8 | `qr_rebuild.py` | reconstrói os textos dos QR do BU de 2026 e os QR codes do certificado a partir de `bu.dat` e do arquivo de assinatura, com as regras do código (`PROC`, `VERC`, `DTPL` e `TURN` como placeholders de largura fixa, `AGRE` deixado de fora), e os mede | H3, H4, H5, G10 (tamanhos) |

`PYTHON=python3 sh investigation/scripts/run_all.sh SECTIONS_DIR HASH_HTML_DIR [OUT_DIR]` roda de novo
os passos 3–8 offline sobre os dados baixados e imprime um resumo (defina `REPO=…` se os scripts não
estiverem dentro de `<repo>/investigation/scripts`; o padrão de `OUT_DIR` é `./investigation-out`).

**Tamanhos das amostras.** `fetch_sections.py … sample --per-uf 4 --seed 2026` nas 28 UFs seleciona
112 seções; 106 delas são da S110, e as outras 6 não têm arquivos publicados. As 4 seções restantes da
S110 estão listadas em `MANIFEST.json` e são baixadas com o modo `sections`. As 25 seções escolhidas
da S135 não estão listadas no kit. Na S110 o kit imprime: 110/110 idas e voltas de BU e RDV (passo 4);
216 eleições, 13.088 tuplas e 534 listas de cargo (passo 5; os 260, 15.611 e 635 deste relatório
incluem as seções escolhidas); 110/110 arquivos de assinatura, modelos 47/44/14/5, e assinaturas de
hardware verificadas sobre SHA-512 para 1.001 arquivos listados Ed521 e 197 ECDSA (todo arquivo de cada
lista, publicado ou não), 110 listas e 216 eleições do BU, com 0/216 sobre o hash bruto (passo 6; as
405 assinaturas de arquivo deste relatório contam só os arquivos publicados da S135, e as suas 260
assinaturas de BU incluem as seções escolhidas); 622.567 linhas e 191 modelos do VOTA, 86/92/5/8
(passo 7); 13/110 últimos QRs acima de 1.100 caracteres, o mais longo com 1.239 (passo 8). Os outros
números deste relatório (as seções escolhidas, os cruzamentos com os dados abertos, a cadeia do GEDAI,
as verificações das regras e as estatísticas de log da §4.6 além das contagens de linhas) vêm de
scripts de trabalho da análise que não fazem parte do kit; eles podem ser obtidos de novo a partir dos
mesmos dados públicos.
## Apêndice: índice dos achados

Veredictos: confirmado; confirmado c/ corr. (confirmado com correções); refutado; não verificável; para
achados do tipo confirmação: conferido por amostragem; conferido por amostragem, corrigido; não reconferido; corrigido pela
revisão. O enunciado é o verificado, resumido.

| id | tipo | veredicto | enunciado |
|---|---|---|---|
| A1 | nova inferência | confirmado | Todo HASH GERAL de 2026 (10 listas de PC, 112 valores de urna por UF) é a cadeia de `CMontadorHash::CalculaHashGeral` na ordem de percurso de `CriaHashesDiretorio` e no escopo de `CGravadorHashes`. |
| A2 | confirmação | conferido por amostragem, corrigido | Um único binário VOTA e um único sistema para os quatro modelos de urna; diferenças só nos módulos de kernel (7/8/11/11 `.ko`), na camada de hardware e nas bibliotecas de MSD/terminal do mesário. |
| A3 | confirmação | conferido por amostragem | O diretório de chaves e os cinco arquivos de chave que o VOTA lê existem na urna real; `wsq.pk1` tem forte respaldo. |
| A4 | nova inferência | confirmado | 17 arquivos de chave por UF, idênticos nos quatro modelos; 11 diferem por UF, 6 chaves públicas nacionais são iguais às chaves legais do Holocron; uma única tag de conjunto de chaves em todos os 110 arquivos de assinatura. |
| A5 | divergência | confirmado | 42 arquivos de dados do RHVoice na urna são idênticos byte a byte aos do upstream e aos do pacote do simulador; corrige duas afirmações da análise anterior. |
| A6 | nova inferência | confirmado c/ corr. | `avpart90/91/97/99.vst` pertencem a OAB/MP/CONFEA/TSE; o GAP os verifica numa ordem própria de cada urna; no log do GEDAI, AVPart é "Sistema Externo de Auditoria e Verificação". |
| A7 | questão em aberto | confirmado | `avusrbinof.vst` está nas quatro listas, mas nenhum log o cita; os passos 1 e 2 do GAP citam ambos `avusrbin.vst`. |
| A8 | divergência | confirmado c/ corr. | Uma carga oficial real instala `vota_apl` junto com `vota_ofi` e `avpart_ofi` (110/110); `vota_apl` não é uma variante de fase. |
| A9 | divergência | confirmado | `.jez` é um contêiner ZIP comum; a análise anterior se contradizia ("criptografado" vs ZIP). |
| A10 | questão em aberto | confirmado | A tabela SAVD do VOTA põe `dadoscarga.vsu` em `/dsk/fe`; toda execução real do GAP o verifica em `/dsk/fi`. |
| A11 | nova inferência | confirmado c/ corr. | A lista "Votação" de Windows é de um aplicativo de apoio ao teste de integridade (provavelmente o SAVP-Votação); ela tem em comum com a urna só o catálogo de contratos ASN.1; a versão de contratos `20260601173148` = o `TAG_CONTRATOS` do código. |
| A12 | confirmação | corrigido pela revisão | A divisão MSD/MSE no modelo 2020 bate com os arquivos da urna e com os logs de autoteste. Correção: os arquivos de assinatura de UE2013/2015 trazem, sim, um certificado DER P-521 da AC URNA (19/19); os valores de algoritmo 2 vs 4 estão certos. |
| A13 | confirmação | conferido por amostragem | Os sete rótulos de assinatura do SCUE são entradas da tabela de nomes SAVD do VOTA. |
| A14 | nova inferência | confirmado c/ corr. | Não há diretório de recursos nem suporte a SVG na urna; Qt6 `linuxfb` só com os plugins GIF e JPEG, o que bate com os 12 recursos GIF e 23 JPEG do código. |
| A15 | nova inferência | confirmado c/ corr. | O espaço de usuário da urna é x86 de 32 bits com glibc e libstdc++ e bibliotecas compartilhadas do TSE; as bibliotecas da camada de hardware diferem por modelo. |
| A16 | confirmação | conferido por amostragem | A cópia das listas de hash tem as contagens e os nomes que a análise anterior leu do TSE. |
| A17 | confirmação | conferido por amostragem | As raízes das mídias batem com o código: `/dsk/fi` interna, `/dsk/fe` mídia de carga durante a instalação e mídia de votação depois. |
| A18 | nova inferência | confirmado c/ corr. | O GAP executa de 32 a 35 passos de assinatura (494 execuções); 24 fixos, depois os pacotes da seção; a contagem varia com as verificações da mídia de votação e com os pacotes `-ce`. |
| A19 | questão em aberto | confirmado | `log.ber.pri` (por UF) não é usado pelo VOTA; provavelmente é a chave MAC do daemon de log (confiança baixa). |
| A20 | confirmação | conferido por amostragem | Os identificadores de aplicativo do código são iguais aos programas instalados; o SCUE roda a partir da mídia de carga. |
| B1 | confirmação | conferido por amostragem | 135/135 arquivos de BU e 135/135 de RDV decodificam e recodificam byte a byte com `src/asn1`, incluindo RED, SA e contingência. |
| B2 | confirmação | conferido por amostragem | `rdv.dat` é uma `EntidadeResultadoRDV` pura, sem envelope. |
| B3 | nova inferência | confirmado c/ corr. | Layout dos arquivos de assinatura de 2026 reconstruído (135/135); a especificação publicada de 2024 falha; o segundo valor do trailer bate com o algoritmo de HW (2/4), confundido com a família do modelo. |
| B4 | nova inferência | confirmado c/ corr. | Modelos UE2020 47, UE2022 44, UE2015 14, UE2013 5; um único conjunto nacional de chaves de SW; certificados de HW por urna (exceto que o RED em outra urna assina com a chave dessa urna). |
| B5 | nova inferência | confirmado c/ corr. | 135 certificados, 260 assinaturas de BU e 405 assinaturas de arquivo verificam sobre o SHA-512 do hash; a camada que calcula o hash é inferida. |
| B6 | confirmação | conferido por amostragem | Ordem dos arquivos assinados = ordem de gravação do `CGravaResultado`; arquivos WSQ só em urnas biométricas; as ordens de RED e SA são diferentes. |
| B7 | confirmação | conferido por amostragem | Todo log publicado termina em `[log.jez] + [Início]` (110/110). |
| B8 | confirmação | conferido por amostragem | Cadeia de hash dos votos recalculada para 260 eleições (15.611 tuplas). |
| B9 | confirmação | conferido por amostragem | Toda linha do BU é recalculada a partir do RDV (635 listas de cargo). |
| B10 | confirmação | conferido por amostragem, corrigido | RDV ordenado por (tipo, dígitos como bytes), sem posições vazias; lexicográfico ≠ numérico em 223 grupos de S110 e em 38 do conjunto-alvo. |
| B11 | nova inferência | confirmado | Aptos e comparecimento são por eleição; eleitores em trânsito de fora da UF contam só para Presidente. |
| B12 | confirmação | não reconferido | Os nulos por suspensão e o registro dos votos no log batem com o código; os tipos 5/8/9 do RDV nunca ocorrem. |
| B13 | nova inferência | confirmado c/ corr. | `QT_ELEI_BIOM_SEM_HABILITACAO` = `qtdEleitoresHabilitadosPorBiografia` (5 seções do AC); os significados concordam com os comentários da especificação de 2024 do TSE. |
| B14 | confirmação | conferido por amostragem | BUs de contingência trazem `identificacaoContingencia`, enquanto os RDVs deles trazem a seção (10/10). |
| B15 | nova inferência | confirmado | O histórico de carga de uma urna de contingência começa com a carga da urna original e termina com a própria; uma urna de seção que recebeu nova carga o reinicia. |
| B16 | confirmação | não reconferido | Os envelopes de SA usam a identificação de contingência (6/6). |
| B17 | nova inferência | confirmado | BUs de SA no exterior: `saManual`, comparecimento 0, número de série da mídia de votação `00000000`, seis seções numa única urna UE2015. |
| B18 | nova inferência | confirmado c/ corr. | BUs de RED: `votacaoRED`, ordem de arquivos própria; em 3 de 7 o RED rodou em outra urna, que assinou com a própria chave. |
| B19 | confirmação | conferido por amostragem | A ordem dos membros de `log.jez` bate com o empacotamento por `std::map` do `CGravadorLog` (135/135). |
| B20 | confirmação | não reconferido | `10.23.0.0 - Praia da Barra do Cahy` vale para a build inteira (VOTA, RED, SA e outros programas). |
| B21 | nova inferência | confirmado | Hash do `aux.json` = hex do Base64(SHA-256 do `.vsc`), com `/` escrito como `-` (135/135). |
| B22 | confirmação | não reconferido | Nenhum valor codificado desconhecido do código; `contingenciaSecao (4)` é o nome do binário e o do TSE em 2026. |
| B23 | confirmação | não reconferido | Um único `agora` para todos os arquivos de resultado; os horários batem com o log com diferença de até 1 s. |
| B24 | confirmação | não reconferido | O BU cita só a seção principal; não são servidos arquivos para as seções agregadas. |
| B25 | questão em aberto | confirmado | A única seção atrasada do PA tem arquivos de urna comuns e que verificam; os arquivos da urna e o código não explicam o atraso. |
| C1 | divergência | confirmado c/ corr. | 8 modelos de linha de log do VOTA (919 linhas) não têm literal em `src/`; 6 são novos em relação à análise anterior. |
| C2 | divergência | confirmado c/ corr. | Falta a continuação do desligamento pela chave; "Votação suspensa" aparece só depois da zerésima do dia da eleição. |
| C3 | divergência | confirmado | "Título … (in)válido para suspender a votação" é ALERTA nas urnas (29/29) e INFO em `src/`. |
| C4 | nova inferência | confirmado c/ corr. | SCUE/ATUE têm em comum com a reconstrução as linhas de inicialização (o ATUE também duas linhas de `IInterfaceInit`); as mensagens de energia deles são outro código. |
| C5 | nova inferência | confirmado c/ corr. | O campo de 16 hex do log é um código com estado e com chave (622.567 distintos; linhas repetidas diferem); não verificável sem a chave. |
| C6 | confirmação | conferido por amostragem | Monitor a cada 1.800 s; tensão só nos modelos ≥ 2020. |
| C7 | confirmação | conferido por amostragem | Alerta de inatividade em 45 s (1.074/1.074). |
| C8 | confirmação | não reconferido | 4 tentativas de digital, timeouts de 30/15 s, manual só depois de 4/4, no máximo 2 tentativas de ano de nascimento. |
| C9 | confirmação | conferido por amostragem | Inspeção da cabine a cada U(60, 90) minutos, nunca dentro da sessão de um eleitor. |
| C10 | confirmação | não reconferido | Abertura estritamente depois do horário de início; encerramento recusado antes do horário local de término; horários locais por município. |
| C11 | confirmação | conferido por amostragem | Desligamento por bateria em 1.800 s, limite de 20 eventos de energia, limite de 6 mesários. |
| C12 | confirmação | não reconferido | Uma suspensão parcial preenche todas as escolhas restantes, inclusive a segunda vaga do Senado. |
| C13 | confirmação | não reconferido | Os tipos de habilitação do log são iguais ao detalhamento do BU (101/106; diferenças explicadas). |
| C14 | confirmação | conferido por amostragem | Duas inversões entre threads apoiam a ordem enviar-e-depois-registrar em `CDigitalReconhecida`. |
| C15 | nova inferência | confirmado | As linhas WHAT do GAP usam o formato de `CError::what()` da ecourna; os números de erro se sobrepõem entre aplicativos. |
| C16 | confirmação | não reconferido | O contêiner `-log.jez` bate com o `CGravadorLog` (logs arquivados primeiro, deflate). |
| C17 | nova inferência | confirmado c/ corr. | 71.318 teclas recusadas: 2,67 por voto, nenhuma fora de sessões, menos no exterior, sobrerrepresentadas antes das confirmações. |
| C18 | questão em aberto | confirmado c/ corr. | Quatro mensagens frequentes existem em `src/` só como comentários (155.165 linhas, todas INFO). |
| C19 | confirmação | não reconferido | As instruções de log nunca vistas se explicam pela ordem de empacotamento, pela configuração ou por caminhos raros. |
| D1 | confirmação | conferido por amostragem | `CD_CARGA_1`/`CD_CARGA_2` são as representações impressas do código de carga pela urna (4.540/4.540). |
| D2 | divergência | confirmado | O leiame do bweb diz 19/6 caracteres para `CD_CARGA_1/2` (24/7 nos dados); o leiame do CEFT diz "urna esperada" para uma coluna da urna efetivada. |
| D3 | divergência | confirmado | As colunas numéricas `CD_CARGA_URNA_*` perdem os zeros à esquerda (9,3% dos códigos do AC); é preciso completar com zeros até 24 para junções e para a cadeia de hash. |
| D4 | divergência | confirmado c/ corr. | `CD_TIPO_URNA` do bweb é um status da apuração, não o `TipoUrna` do BU; uma coincidência de nomes, não uma contradição. |
| D5 | nova inferência | confirmado | Os conjuntos de dados misturam horário local do município e horário de Brasília; as duas colunas de recepção do CEFT diferem pelo deslocamento em relação ao UTC. |
| D6 | divergência | confirmado | `historicoCodigosCarga` é o histórico de carga da seção, não "todo código de carga que esta urna teve" (corrige a documentação anterior). |
| D7 | divergência | **refutado** | Alegava-se redação enganosa no "código da carga original da urna" do bweb; o leiame descreve corretamente a carga da urna efetivada. |
| D8 | confirmação | conferido por amostragem, corrigido | "Flash card" = número de série da mídia de carga; o da mídia de votação não é publicado; 88 FCs trazem de 10 a 32 seções cada (não 34). |
| D9 | confirmação | conferido por amostragem | A tripla do gerador no BU = a tripla `*_GERACAO_MIDIA` do CSV = a tripla do nome de arquivo do log do GEDAI (15/15). |
| D10 | divergência | confirmado c/ corr. | `QT_ELEI_BIOM_SEM_HABILITACAO` = habilitações biográficas; um detalhamento ausente é publicado como 0 (convenção não documentada). |
| D11 | confirmação | não reconferido | O CHOICE `DadosSecaoSA` aparece no bweb como datas ou como junta/turma. |
| D12 | confirmação | conferido por amostragem | Valores não documentados de `CD_ORIGEM_VOTO` correspondem a `TipoArquivo`; `TP_DIVERGENCIA` 'C' cobre contingência e SA. |
| D13 | nova inferência | confirmado c/ corr. | Para BUs de SA, a "urna efetivada" é a urna da apuração, compartilhada por muitas seções; o nome do município no CEFT está errado nessas linhas. |
| D14 | confirmação | não reconferido | As correspondências de contingência não têm seção; urnas de contingência atenderam seções fora do município ou da zona para os quais foram preparadas. |
| D15 | confirmação | não reconferido | `CD_TIPO_ELEICAO` confirma o `TipoEleicao` do ASN.1, que tinha nomes provisórios. |
| D16 | confirmação | conferido por amostragem, corrigido | `CD_TIPO_VOTAVEL` usa os números de `TipoVoto` do BU; 95/96/97 são convenções do banco de dados (121 BUs decodificados, não 120). |
| D17 | confirmação | conferido por amostragem, corrigido | `CD_CARGO_PERGUNTA` = números de `CargoConstitucional`. |
| D18 | nova inferência | confirmado | `QT_APTOS`/`QT_COMPARECIMENTO` são por eleição (57/2.270 seções do AC diferem). |
| D19 | nova inferência | confirmado c/ corr. | As seções agregadas não estão no `bu.dat`; a urna e o bweb apresentam a lista em três formatos. |
| D20 | confirmação | conferido por amostragem, corrigido | A precisão de minuto do `DT_CARGA` vem da urna (segundos `00` em 121/121 BUs). |
| D21 | divergência | confirmado | As convenções de aspas e de `#NULO`/`-1` do preâmbulo do leiame não valem nos arquivos. |
| D22 | divergência | confirmado | O leiame do GEDAI diz que a UF tem "02 dígitos"; são duas letras. |
| D23 | nova inferência | confirmado c/ corr. | O código de carga tem estrutura nos seus primeiros 8 a 9 dígitos (prefixos comuns por zona); o VOTA o trata como opaco. |
| D24 | confirmação | conferido por amostragem | Não há seções com vários hashes no AC; `DT_BU_RECEBIDO` = horário de recepção no site de resultados. |
| D25 | confirmação | não reconferido | A urna e o GEDAI completam identificadores com zeros; os CSVs perdem esse preenchimento de forma inconsistente. |
| E1 | nova inferência | confirmado c/ corr. | Fluxo de trabalho do GEDAI-UE no AC em 2026 reconstruído a partir de 22 logs oficiais (454 sessões, 2.850 FV, 88 FC, 133 execuções de mídia de resultado). |
| E2 | confirmação | conferido por amostragem | A tripla de identidade do GEDAI é `IdentificadorGeradorMidia`; o BU a traz byte a byte (62/62). |
| E3 | divergência | confirmado | O gerador da "mídia de votação" no GAP não é o gerador do registro de carga do BU (0/62); corrige uma frase da análise anterior. |
| E4 | divergência | confirmado c/ corr. | O gerador do BU do simulador é `nome_maquina`, não `simulador-votacao-ng`; corrige a documentação anterior e o u40. |
| E5 | divergência | confirmado c/ corr. | Os dados fixos de identidade do simulador não têm os formatos reais (exceto os números de série de instalação). |
| E6 | nova inferência | confirmado | `DadosGeracaoMidia.usuario` corresponde ao logon do Windows da sessão do GEDAI, não ao usuário do Odin (62/62 contra 52/62). |
| E7 | nova inferência | confirmado | Os números de série das mídias são atribuídos a cada geração; todos os 3.061 cumprem as regras de 8 hex do código. |
| E8 | confirmação | conferido por amostragem | O código de carga é criado pela urna na carga e só lido de volta pelo GEDAI. |
| E9 | confirmação | conferido por amostragem, corrigido | A cadeia GEDAI → SCUE/GAP → BU vale para 62 urnas do AC (33 verificações: 31 passam em 62/62, 2 relacionadas ao Odin em 52/62). |
| E10 | nova inferência | confirmado | Os nomes de versão são compartilhados por ciclo e o número principal sobe de um em um; a eleição de 2024 rodou `9.30.0.0 - Tupiniquim`. |
| E11 | divergência | confirmado | A reconstrução chama o id do processo eleitoral de `pleito` nos nomes de arquivos de dados. |
| E12 | divergência | confirmado | `-pu` é a parametrização da urna, não "partidos". |
| E13 | questão em aberto | confirmado | Não existe nenhum `-pu.dat` real no nível de UF, embora o comentário do código diga que a func 7787 lê um. |
| E14 | nova inferência | confirmado c/ corr. | O GEDAI-UE usa o formato de erro e o namespace do modelo de dados da ecourna. |
| E15 | nova inferência | confirmado | O número de série do TPM de um PC do GEDAI é estável enquanto o nome dele muda. |
| E16 | nova inferência | confirmado c/ corr. | O trailer da assinatura é {modelo, algoritmo de HW} em 189/189 arquivos (só 3 combinações); assinaturas do log do GEDAI descritas. |
| E17 | nova inferência | confirmado | O `infomidia.vsc` da mídia de votação provavelmente é assinado no PC do GEDAI com a chave SEVIN do Holocron. |
| E18 | confirmação | conferido por amostragem | Os aplicativos de mídia de resultado do GEDAI correspondem a `TipoAplicativo`; o AVPart não precisa de `infomidia`. |
| E19 | questão em aberto | confirmado | Nenhuma lista de hash publicada cobre `app.ini.vsc` nem o arquivo oficial de URIs que o GEDAI-UE carrega. |
| F1 | divergência | confirmado c/ corr. | A liberação "pela digital do mesário" aceita qualquer dedo; 550/1.605 liberações reais não são atribuíveis (lacuna na aplicação da regra). |
| F2 | divergência | confirmado c/ corr. | A suspensão aceita qualquer título com dígito verificador válido diferente do do eleitor; 3/14 aceitos não eram de um mesário cadastrado; a causa de 12/15 recusas está em aberto. |
| F3 | divergência | confirmado c/ corr. | O encerramento aceita qualquer título válido; 1/112 não era de um mesário cadastrado (não 6, depois de completar com zeros); o "título do presidente" de u27 é a intenção. |
| F4 | divergência | confirmado | Manual: timeout de 15 s para a digital; código e dados: 30 s na primeira tentativa. |
| F5 | divergência | confirmado c/ corr. | O nome do relatório no art. 127 VII difere do BEHB da urna; o alcance depende da leitura. |
| F6 | divergência | confirmado c/ corr. | O cadastro de mesários pode terminar sem ninguém cadastrado; 4/110 seções abriram sem nenhum. |
| F7 | divergência | confirmado | BUs de urnas não biométricas (exterior) omitem as contagens do art. 210 XII (4/110). |
| F8 | nova inferência | confirmado c/ corr. | Existe no código um corte por `terminoVotacao` que nenhuma regra pública menciona; não foi acionado em 2026. |
| F9 | confirmação | conferido por amostragem | O código segue o menu do terminal do mesário de 2026; os códigos de 12 dígitos de 2022 sumiram. |
| F10 | confirmação | conferido por amostragem | A ordem dos painéis (art. 142 §1) vem dos dados da eleição e é a legal. |
| F11 | confirmação | conferido por amostragem | Eleitores em trânsito de fora da UF só recebem Presidente (art. 31). |
| F12 | confirmação | conferido por amostragem | Mesmo senador duas vezes → segundo voto nulo (art. 206 §2): 736 votos assim. |
| F13 | nova inferência | confirmado c/ corr. | Votos de legenda são gravados como digitados: 869/1.317 votos de legenda para Dep. Estadual têm um número parcial. |
| F14 | nova inferência | confirmado | Só os inaptos proporcionais são carregados; o lado majoritário não tem tela de inapto (arts. 94 V, 208 II). |
| F15 | confirmação | conferido por amostragem | Parâmetros de suspensão de 2026: descarte sem voto, nulo para os cargos restantes (arts. 143-144). |
| F16 | confirmação | conferido por amostragem | 4 tentativas biométricas, depois ano de nascimento com uma nova tentativa (arts. 133-135). |
| F17 | confirmação | conferido por amostragem | Nenhuma liberação antes das 08:00 de Brasília; encerramento recusado antes das 17:00; eleitores na fila votam depois das 17:00. |
| F18 | confirmação | não reconferido | Zerésima nunca antes das 07:00 de Brasília; uma zerésima atrasada exige confirmar o relógio. |
| F19 | confirmação | não reconferido | O teste do teclado só pode ser pulado antes da véspera da zerésima ou depois de um reinício pós-zerésima. |
| F20 | confirmação | não reconferido | A ordem do fim do dia bate com o curso de 2026, não com o manual de 2022. |
| F21 | confirmação | não reconferido | Um voto interrompido por falta de energia não é gravado; o reinício só precisa de CONFIRMA. |
| F22 | confirmação | não reconferido | Mensagem de ausência de candidatos, cargo no terminal do mesário, CPF e foto, justificativa e áudio batem com as regras de 2026. |
| F23 | nova inferência | confirmado | CORRIGE apaga o número inteiro; BRANCO só funciona com o campo vazio (flags inferidas a partir de offsets). |
| F24 | questão em aberto | confirmado | A Res. 23.751/2026 rege 2026 (não a 23.736/2024); a 23.760 é o calendário. |
| G1 | confirmação | conferido por amostragem | Testes de teclado com falha são registrados como "Sucesso": pelo menos 27 em 21/110 seções. |
| G2 | confirmação | conferido por amostragem | Os títulos dos mesários estão nos logs publicados pelo TSE: 440 distintos em 110/110 seções. |
| G3 | nova inferência | confirmado c/ corr. | O gerador aleatório da urna real não é constante (274 distintos, ids de imagem uniformes). |
| G4 | nova inferência | confirmado c/ corr. | O segredo de cifragem de chaves é o mesmo nos quatro modelos dentro de uma UF; os arquivos de chave secreta são por UF (confiança média). |
| G5 | confirmação | conferido por amostragem | O limiar de correspondência da digital do mesário é pontuação ≥ 20. |
| G6 | confirmação | não reconferido | O monitor de energia se comporta como reconstruído (165/165, 181/181, limite 20). |
| G7 | nova inferência | confirmado c/ corr. | Desligamento automático por bateria em 2026 após 1.800 s (8 eventos, todos em dias de preparação). |
| G8 | confirmação | conferido por amostragem | A opção de BU cifrado estava desligada no 1º turno (129/129 BUs sem `seguranca`). |
| G9 | confirmação | não reconferido | Local constante 1 na correspondência; a separação BU/RDV de contingência é a do código. |
| G10 | nova inferência | não verificável | Previsões para o BU impresso: ASSI de 264/278 hex; 2 QR codes de certificado de 1.071 (PEM) ou 983 (DER) caracteres; um último QR de até 1.245/1.259 caracteres. |
| G11 | nova inferência | confirmado | A chave do RDV gravado depende da lista de cargos e de 28 a 30 bytes de uma tabela de hardware; 3 salts no 1º turno. |
| G12 | nova inferência | confirmado c/ corr. | A ordem do RDV não vaza nada; 3.410 votos em 108/110 seções trazem uma string digitada única. |
| G13 | confirmação | conferido por amostragem | O log publicado sempre termina quando o `log.jez` é empacotado; a assinatura e a cópia nunca aparecem nele. |
| H1 | confirmação | conferido por amostragem | O exemplo resolvido do manual de QR de 2026 (transcrição de terceiros) é reconstruído byte a byte. |
| H2 | nova inferência | confirmado c/ corr. | Assinaturas de QR de 2026: Ed521 ou ECDSA P-521 sobre o SHA-512 do HASH, certificado por urna (verificado nos dados de demonstração do TSE). |
| H3 | confirmação | conferido por amostragem | A divisão do QRCE bate com os dados de demonstração do TSE; 2 códigos para todos os 110 certificados reais. |
| H4 | nova inferência | confirmado c/ corr. | Dois QR codes de certificado são consequência do tamanho; MDUE é provavelmente o ano do modelo. |
| H5 | divergência | confirmado c/ corr. | O limite de 1.100 caracteres não é respeitado no formato 6.0 (13/110 últimos QRs reconstruídos passam dele); só é uma contradição de documento público se o manual de 2026 mantiver o limite. |
| H6 | confirmação | conferido por amostragem | O `assinatura.asn1` de 2026 do TSE recodifica 110/110 arquivos de assinatura reais. |
| H7 | confirmação | conferido por amostragem | O `bu.asn1` de 2026 do TSE é igual a `src/asn1` em 41/41 tipos. |
| H8 | divergência | confirmado c/ corr. | O `rdv.asn1` de 2026 não mudou desde 2024 e contradiz o `bu.asn1` de 2026 e o código em duas enumerações. |
| H9 | divergência | confirmado | Os pacotes de log são ZIP, não 7z como diz o documento de log de 2022. |
| H10 | divergência | confirmado c/ corr. | As linhas de log reais têm 6 campos separados por TAB com `DD/MM/YYYY HH:MM:SS`, não os 7 campos do texto de 2022. |
| H11 | divergência | confirmado | Severidade WHAT não documentada nos logs reais (só no GAP). |
| H12 | divergência | confirmado c/ corr. | Os logs do GEDAI-UE não estão no formato de log da urna; os nomes deles estão documentados no leiame de 2026. |
| H13 | divergência | confirmado | A regra de nomes dos logs de resultado de 2022 está obsoleta; o código e o README de 2026 concordam. |
| H14 | confirmação | conferido por amostragem | A ordem dos cargos no QR por `ordemImpressao`, com `IDEL` por eleição, bate com o exemplo do manual e com 102/110 BUs. |
| H15 | confirmação | conferido por amostragem | `TOTC = 2 × COMP` para Senador no exemplo do manual e em 106/106 BUs. |
| H16 | confirmação | conferido por amostragem | Os arquivos de resultado do VOTA e os nomes deles batem com o README de 2026, que omite a fase `t`. |
| H17 | questão em aberto | confirmado c/ corr. | Os certificados de demonstração são `UEAD`/`uead`, os reais de 2026 são `UEAO`/`ueao`. |
| H18 | questão em aberto | não verificável | Os rótulos impressos e o resto do manual de 2026 não puderam ser verificados. |
