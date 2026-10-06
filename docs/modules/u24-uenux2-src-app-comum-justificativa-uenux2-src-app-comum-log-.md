# u24: `comum/justificativa`, `comum/log`, `comum/md`, `comum/nomearquivo`, `comum/reconhecimentobiometrico`

A unidade u24 cobre 39 funções wasm de cinco diretórios pequenos de `uenux2/src/app/comum/` ("comum" é o
código compartilhado pelas aplicações da urna). São helpers não relacionados entre si, usados em torno do voto:

| diretório | o que faz no processo de votação |
|---|---|
| `justificativa/` | **justificativa eleitoral**: um eleitor que está fora do seu domicílio eleitoral vai a qualquer urna e *justifica* sua ausência (título + ano de nascimento). Mantém a lista em memória (`CJustificador`) e no SQLite (`registro_justificativa`). |
| `log/` | textos fixos do **log de eventos** da urna (`logd.dat`): geração de resultados no *encerramento*, sessões de assinatura, eventos de bateria/alimentação, nomes dos relatórios impressos. |
| `md/` | "modelo de dados": pequenas classes de valor validadas usadas nos arquivos de resultado: `CAbrangencia` (abrangência de um dado), `CCabecalhoEntidade` (cabeçalho de toda entidade), `CIdentificacaoUrna` / `CIdentificacaoSecao` (quem produziu um resultado), `CSeguranca` (bloco de cifração de um envelope). |
| `nomearquivo/` | nomes dos **arquivos de dados** da eleição que a urna carrega (`t02411ac00001-ca.dat` ...). |
| `reconhecimentobiometrico/` | armazenamento de **imagens de impressões digitais** (WSQ): cifra com a chave pública do TSE, embrulha em um envelope, grava com um nome aleatório nas duas flashes. |

Seis das 39 funções executaram nas sessões gravadas (todas no `votaInit`): 348, 1161, 1705, 3744, 3773, 5887.
Todo o resto pertence ao terminal do mesário (justificativa, biometria) ou ao fim do dia
(arquivos de resultado), que o simulador público nunca aciona.

Arquivos-fonte reconstruídos (todos sob `src/uenux2/src/app/comum/`):

```
justificativa/cjustificador.{h,cpp}           justificativa/dao/cjustificadordao.{h,cpp}
log/ieventoslog.{h,cpp}                       log/clogcomum.{h,cpp}
md/cabrangencia.{h,cpp}  md/ccabecalhoentidade.{h,cpp}  md/cidentificacaourna.{h,cpp}
md/cidentificacaosecao.{h,cpp}  md/cseguranca.{h,cpp}
nomearquivo/cnomearquivo.{h,cpp}              (plus cnomearquivo.u02.cpp from unit u02)
reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.{h,cpp}
u24-foreign-fragments.cpp                     (348 CArquivosResultado::GetInst, 11412 CConversorSeguranca,
                                               10888 ~CFormPart, 11205 ~CParteEleitores)
```

## 1. Glossário

| termo | significado |
|---|---|
| justificativa (eleitoral) | declaração de ausência feita por um eleitor fora do seu domicílio; registrada no lugar de um voto |
| título / número de inscrição eleitoral | o número de inscrição do eleitor, com 12 dígitos (`CNumeroInscricaoEleitoral`) |
| BUJ | *Boletim de Justificativa*, a lista impressa das justificativas (relatório `ERelatoriosUE::BUJ`) |
| abrangência | escopo geográfico: municipal (UF + município), estadual (UF), federal (país) |
| pleito / eleição | o dia da eleição (todas as votações) / uma eleição desse dia (por exemplo, a municipal) |
| fase | `'1'` oficial (`o`), `'2'` simulado (`s`), `'3'` treinamento (`t`); todo cenário do simulador é `t` |
| MI / FI, ME / FE, MV | mídia/flash interna (`/dsk/fi/`), mídia/flash externa (`/dsk/fe/`, o cartão de votação removível, "MV" = memória de votação) |
| MSD / MSE | o módulo de segurança da urna (HSM), MSD nos modelos mais antigos, MSE na UE2020+ (`IUrna::GetModelo() >= 2020`) |
| WSQ | formato de compressão de imagens de impressões digitais do FBI; a urna armazena as digitais capturadas como `.wsq` |
| CEPESC | o esquema de cifração do TSE/CEPESC usado nos envelopes de resultado (`ecourna::api::cepesc`) |
| encerramento | fechamento da seção: os arquivos de resultado (BU, RDV, ...) são gerados, assinados e copiados |

## 2. Classes e hierarquia (a partir do RTTI)

```
comum::IEventosLog                     (class, vtable @1552860 {174 D1, 144 D0}, 8 bytes: +4 ELogAplicativos)
  └─ vota::CLogVota                    (si, vtable @1532792; the only implementation, pushed in CPolySingletonList)
comum::CLogComum                       (no RTTI, empty class, lazy singleton = wasm 948)

comum::servico::CJustificadorServico   (class, vtable @1560936 {3740 D1, 11464 D0}, 12 bytes)
comum::CJustificador                   (no RTTI, 40 bytes) : api::CDataMap<CNumeroInscricaoEleitoral,
                                                             CJustificadorDetalhe>  + member CJustificadorServico
comum::CJustificadorDadoTitulo         (no RTTI; static Text())
api::persistencia::IDAO
  └─ IUenuxGenericDAO<ecourna::app::dados::CIdentificacaoJustificativa, std::string>
       └─ comum::dao::IJustificadorDAO
            └─ comum::dao::CJustificadorDAO   (vtable @1560768, 12 bytes: +4 shared_ptr<ISqlConnection>)

comum::md::CAbrangencia (20 B), CCabecalhoEntidade (20 B), CSeguranca (16 B)        (no RTTI)
comum::md::CIdentificacaoUrna (12 B) ─┬─ CIdentificacaoSecao (20 B, this unit)       (no RTTI)
                                      └─ CIdentificacaoUrnaContingencia (other unit, ctor 5888)
comum::CNomeArquivo                    (no RTTI; static functions + anonymous-namespace helpers)
comum::CControlaArmazenamentoDeImagens (no RTTI; static functions only)
```

### Layout de `CJustificador` (a func 3742 o constrói, a 2810 o destrói)

| offset | membro |
|---|---|
| +0 | `std::map<CNumeroInscricaoEleitoral, CJustificadorDetalhe>` (contêiner do CDataMap; nó: chave em +16, 24 bytes; ano em +40) |
| +12 | cursor `m_atual` (== end() quando não posicionado) |
| +16 | `std::string m_nome = "CJustificador"` (nome do CDataMap usado nas suas mensagens de erro) |
| +28 | `servico::CJustificadorServico m_servico` { vptr, `shared_ptr<IJustificadorDAO>` +32/+36 } |

`CJustificador::GetInst()` (func 1391, outra unidade) o cria de forma lazy no `unique_ptr` @1839012; o
construtor (3742) obtém o DAO de `api::persistencia::CDAORepositorio::Entregar<IJustificadorDAO>()`,
que clona o protótipo `CJustificadorDAO` registrado na inicialização por
`CInformacaoEleitor::InicializarPersistencia` (inlinada na 7787).

## 3. Justificativa: fluxo de controle e dados

1. **Inicialização** (7787): `CJustificadorDAO(<trab MI>/uenux.db)` executa
   `CREATE TABLE IF NOT EXISTS registro_justificativa ( \nnumero_titulo     BIGINT PRIMARY KEY UNIQUE NOT NULL, \nano_nascimento    SMALLINT \n)`
   e é registrado em `CDAORepositorio` sob `"N5comum3dao16IJustificadorDAOE"`. Depois,
   `comum::CEleitores::CompleteLoad` (6734, celeitores.cpp:205..238, chamada pela 7787 `CarregarDadosEstaticos` e pela
   11865 `CGeraDadosDinamicos::StartState`) limpa o `CJustificador` (destruição da árvore 2809, cursor reposto em `end()`) e
   o preenche de novo a partir de `RecuperarTodos()` (slot 8 do DAO, alcançado por meio do helper do serviço `comum_f5723`), com um
   `CDataMap::Add` (5725) por linha.
2. **O mesário registra uma justificativa** (`vota::CPedeAnoNascimento`, 10590, unidade u17), que inlina
   `CJustificador::Justifica(titulo, ano)` (cjustificador.cpp:55): se o título já está no map, o cursor
   é movido para ele e `8800 "Já havia justificativa para o título"` é lançado; senão, `Add({titulo, {ano}})`.
3. **Gravação** (inlinada na 10590; cjustificador.cpp:83 `SaveCurrentInternal`): lança `8801 "Não há dados a serem
   salvos na MI"` quando o map está vazio; caso contrário, constrói `CIdentificacaoJustificativa(CRegistroIdentificacaoEleitor(
   make_shared<CNumeroInscricaoEleitoral>(titulo)), ano)` e chama `dao.Recuperar(numero)`; somente se não existir linha,
   `dao.Inserir(...)`. O código ao redor (na mesma função 10590) protege com *"Gravando a justificativa na MI"*,
   salva `vota.bin` (SalvaVotaInterno), assina `uenux.db` (arquivo SAVD 122/123), sincroniza e depois *"Gravando a
   justificativa na MV"*: SalvaVotaExterno e cópia do banco de dados para a flash externa.
4. **Relatórios**: `CJustificadorDadoTitulo::Text()` (11475, slot 2949) retorna o título sob o cursor
   (`8802 "Justificativa inválida"` no fim), usado enquanto as telas do BUJ / de justificativa percorrem o map com
   `CDataMap::Next()` (unidade u08/u09). O slot 2948 (11474, unidade u36) imprime a contagem.

SQL de `CJustificadorDAO` (texto exato, incluindo o espaço inicial e `" \n"`):

| slot | método | SQL / comportamento |
|---|---|---|
| 2 | `Clonar` 11468 | `new CJustificadorDAO(*this)` (compartilha a conexão) |
| 3 | `Inserir` 11473 | ` INSERT INTO registro_justificativa (numero_titulo, ano_nascimento) VALUES (?, ?) \n` com `SetInt64(1, ToQWord(título))`, `SetInt(2, ano)` |
| 4 | `Excluir` 11467 (u20) | padrão: lança 6804 |
| 5 | `ExcluirID` 11472 | lança 8804 "Nao é esperado que o CJustificadorDAO exclua registros do banco" (:56) |
| 6 | `Atualizar` 11471 | lança 8805 "Nao é esperado que o CJustificadorDAO atualize registros do banco" (:63) |
| 7 | `Recuperar` 11470 | ` SELECT numero_titulo, ano_nascimento FROM registro_justificativa WHERE numero_titulo = ? \n` → `shared_ptr` ou nulo |
| 8 | `RecuperarTodos` 11469 | ` SELECT numero_titulo, ano_nascimento FROM registro_justificativa \n` → `vector<CIdentificacaoJustificativa>` |

As linhas são lidas como `std::format("{}", GetInt64(0))` (o preenchimento até 12 dígitos é restaurado por `CNumeroInscricaoEleitoral`)
e `GetInt(1)` truncado para 16 bits e com a faixa 0..9999 verificada por `CBaseType<0,9999>` (um ano NULL é lido como 0).
A tabela **não contém nenhum voto**: apenas quem justificou.

## 4. Log (`IEventosLog`, `CLogComum`)

Todos os registros vão para `api::CLoga::loga(ELogAplicativos, ESeveridade, text)` por meio de
`api::CPolySingletonList::instance<IEventosLog>()` (= `vota::CLogVota`, id de aplicação 1). O `srcloc` de todo
método de `CLogComum` é a linha onde ele obtém essa instância.

| método | linha | wasm | texto |
|---|---|---|---|
| `LogaIniciaSessaoMSD` / `FinalizaSessaoMSD` | 44 / 50 | inlinado na 12098 | "Inicia uma sessão no MSD" / "Finaliza a sessão no MSD" |
| `LogaIniciaSessaoMSE` / `FinalizaSessaoMSE` | 56 / 62 | inlinado na 12098 | "Inicia uma sessão no MSE" / "Finaliza a sessão no MSE" |
| `LogaCopiandoArqResParaFI(ext)` | 76 | 5878 → 6045 | "Copiando arquivo de resultado para MI: [{}]" |
| `LogaResultadoCopiadoResFE(ext)` | 84 | 3828 → 6045 | "Copiando arquivo de resultado para ME: [{}]" |
| `LogaGerandoResultados(ext, status)` | 92 | 5876 | "Gerando arquivo de resultado [{}] + [{}]" (nome, "Início"/"Término") |
| `LogaNivelBateria(fonte, status)` | 106 | 5875 | "Carga da [{}]: [{}]" (severidade 2 se CRÍTICA, senão 1) |
| `LogaSuspensoMonitoramenteRepeticao(n)` | 136 | inlinado na 10226 | "Suspenso monitoramento de alimentação devido repetição de eventos. [ {} ] eventos" |
| `LogaTipoBateria` | 144/145/158 | inlinado na 10226 | "Urna operando na rede elétrica / bateria interna / bateria externa"; o código 3 lança 8850 "Tipo de alimentação não implementado" |
| `LogaInicio/TerminoProcedimentoAssinatura` | 166 / 172 | inlinado na 12098 | "Início/Término do procedimento de assinatura dos arquivos de resultados" |
| `LogaPreparandoAssinaturaArquivosResultado` | 178 | inlinado na 12098 | "Preparando para assinatura dos arquivos de resultados" |
| `LogaScoreReconhecimentoMesario(score)` | 214 | inlinado na 5372 | "Batimento de digitais retornou o score {}" |

`ext` é transformado no sufixo do arquivo por `CArquivosResultado::GetInst()[ext]` (func 348 + 347): `bu.dat`, `rdv.dat`,
`jufa.dat`, `imgbu.dat`, `hash.dat`, `log.jez`, `wsqbio.jez`, ... (ver unidade u21).

Conversores de `IEventosLog` (os textos estão em Latin-1 no binário; valores fora da faixa lançam `CUeComumLogError`
"Valor inválido"):

* `ConverteERelatoriosUE` (5879, :426, 8852): 0 ZERÉSIMA (Latin-1 `5A 45 52 C9 53 49 4D 41`, com o acento), 1 ZERÉSIMA DE APURAÇÃO, 2 ZERÉSIMA DE SEÇÃO, 3 BU, 4 BUJ,
  5 RDV, 6 AUTOTESTE, 7 COMPROVANTE DE CARGA, 8 HASHES ARQUIVOS, 9 BIM, 10 ARQUIVOS DE ELEITORES E CANDIDATOS,
  11 RESUMO DA ZERÉSIMA, 12 ELEITORES HABILITADOS BIOGRAFICAMENTE. Usado por `CLogVota::LogaImpressaoRelatorio`:
  "Imprimindo relatório [BU] via nº [n]".
* `ConverteFonteAlim` (:440, 8853): 0 ALIMENTAÇÃO AC, 1 ALIMENTAÇÃO BATERIA INTERNA, 2 ALIMENTAÇÃO BATERIA EXTERNA.
* `ConverteStatusBateria` (:456, 8854): 0 PLENA, 1 PARCIAL, 2 CRÍTICA, 3 AUSENTE.

## 5. Classes de valor de `md` (regras de validação)

Erros: `CBaseError<EUeComumMdError, {8900, 8950}>` (thunk `comum_f591`). Os membros são armazenados primeiro e depois verificados.

| classe (wasm) | campos | verificações, em ordem (linha, código, mensagem) |
|---|---|---|
| `CAbrangencia(tipo, uf, município)` 3739 | +0 tipo, +4 uf, +16 município | :31 8900 tamanho da uf diferente de 0/2 "UF inválida [uf]"; :34 8901 uf vazia e não federal; :39 8902 municipal com município 0; :42 8903 município ≠ 0 e não municipal; :47 8904 federal com UF; :51 8905 tipo ≥ 3 "Tipo de abrangência inválido: {}" |
| `CCabecalhoEntidade(data, id, tipo)` 1945 | +0 CDateTime, +12 id, +16 tipo (0 processo, 1 pleito, 2 eleição) | :24 8906 id ≥ 100000 "ID inválido."; :27 8907 tipo ≥ 3 "Tipo de ID inválido." |
| `CIdentificacaoUrna(tipo, município, zona)` 5886 | +0 tipo, +4 município, +8 zona (u16) | :25 8918 tipo fora de '1'..'4' "Tipo inválido."; :28 8919 município ≥ 100000; :31 8920 zona ≥ 10000 |
| `CIdentificacaoSecao(tipo, mun, zona, local, seção)` 5887 | base + +12 local, +16 seção (u16) | (:33 tipo ≠ '1': eliminada por dobramento de constantes, ver §10); :38 8916 local ≥ 10000 "Local inválido."; :41 8917 seção 0 ou ≥ 10000 "Seção inválida." |
| `CSeguranca(tipoArquivo, idCripto, chave)` 3823 | +0 u8, +1 u8, +4 vector | :24 8925 tipo ≥ 3; :27 8926 id fora de 1..3; :30 8927 "Chave vazia." |

`CSeguranca` não tem `idArquivoCD`: `CConversorSeguranca` (11412 leitura / 11413 gravação) o descarta e grava 0.

## 6. Nomes de arquivo (`CNomeArquivo`)

`<fase><id:05><uf>[<município:05>[<zona:04><seção:04>]]-<sufixo>.<extensão>`, com os helpers
`FormataFase` ('1'/'2'/'3' → o/s/t, senão 8950 "Fase inválida: {}", em que o enum passa pelo formatador de enums compartilhado do TSE, func 536, e imprime seu inteiro, por exemplo `52`), `FormataNumero(n, casas)` (`std::format("{:0{}}")`; 8951 se for mais longo),
`FormataUF` (2 letras, senão 8952, convertidas para minúsculas por `CStringUtils::ToLower`).

* `MontaNome(fase, id, uf, sufixo, ext)` (1705, nome inferido): nível de pleito/UF. Na inicialização, a 7787 constrói
  `t02400ac-pu.dat`, `t00000ac-pu.dat` e `t00000br-pu.dat` (partidos do pleito, da UF, nacional).
* `MontaNomesEleicao(id, pleito, município, idEleição, sufixo, ext)` (3744, nome inferido): copia a eleição
  (`CPleito::GetEleicao`, cpleito.cpp:139, lança 8162 "Eleição não encontrada: {}") e, se ela tiver pelo menos um cargo eletivo (um cargo
  com `DetalheCargo`), aplica `ajustaAbrangenciaUFMunicipio` (:208: municipal mantém UF+município, estadual define
  município 0, federal define município 0 e UF "BR", senão 8953) e retorna `{abrangência → name}`:
  `t02411ac00001-ca.dat` (municipal), `t02512ac00000-ca.dat` (estadual), `t02511br00000-ca.dat` (federal).
  Os sufixos por eleição vistos nos cenários são `ca`, `co`, `fe`, `fo`, `le`, `pa`, `pi`, `rdj`.
* `MontaNome` com zona/seção (3772) e `NomesPorAbrangencia` (2811) estão em `cnomearquivo.u02.cpp`.

## 7. Armazenamento de imagens de impressões digitais (`CControlaArmazenamentoDeImagens::Armazena`, 2725)

Chamada com (wsq, diretório interno, diretório externo, prefixo) por `CMostraEleitorVotando::SalvaHabilitacaoEleitor`
(prefixo `""`, diretórios `<trab>/wsq/habilitado|nao-habilitado|operador/`) e por `CPedeDigitalMesario::GetControlador`
(prefixo `"me"`). Etapas:

1. `statfs("/dsk/fe/dinamico/")`: se tiver sucesso e `f_bavail * f_bsize < 5 MiB` → retorna 999999 (não armazenada).
2. `GerarCaminhosUnicos` (:73): repete `id = (uint32)IRng::Gera() % 999999`, `nome = "{prefix}{id:06}.wsq"`,
   até que `<internal dir>/nome` não exista (o caminho externo não é testado).
3. `imagem = <WSQ codec singleton 2742>(wsq)` (reduzido a uma cópia aqui); vazia → retorna 999999.
4. `CifrarWsq` (:149/:152): tabela de 1024 bytes do slot 4 de `IUrna`, 32 bytes aleatórios do slot 4 de `IRng`,
   `LeChavePublica()`, depois `CPlainText(0, 1, zona, seção, tabela, aleatório, chave, imagem)` →
   `CCepescCipher::Cifra` → `CCipheredOut {chave, conteúdo}`.
   * `LeChavePublica` (:117..:135): `/dsk/fi/estatico/chave/<name>` precisa existir (senão 9000 "O arquivo … não
     existe"). O nome **não pode ser recuperado deste binário**: ele é formatado a partir de uma `string_view`
     constante `{data = 117, size = 7}` (`i64.const 30065124469`, tipos de argumento de formatação 428 = diretório `const char*` +
     nome `string_view`). O endereço 117 está abaixo do primeiro segmento de dados (1024) e contém 7 bytes zero em tempo de execução
     (lidos depois do `votaInit`), então o build web procuraria `/dsk/fi/estatico/chave/` + sete NULs. `wsq.pk1`
     (7 caracteres, o nome que `CGravadorWSQ::ValidaTipoBiometria` 3796 usa) é uma inferência. BER `EntidadeChave`; seu
     campo `chave` é decifrado com `CSymmetricCipherFactory::Cria(secret of IKernelHSM slot 3)`; resultado vazio →
     9001 "O arquivo … está vazio". O flag `cifrado` não é consultado.
5. Abre `<internal>` com `"w+b"`, `FM_NOATIME`; grava `EntidadeEnvelopeGenerico` { cabecalho = (data do pleito às
   **00:00:00**, id do pleito, tipo pleito), fase, município, zona, local (opcional), seção,
   `tipoEnvelope` = C++ 3 → ASN.1 5 `envelopeImagemBiometria`, `seguranca` = `CSeguranca(0, 1, out.chave)`,
   `conteudo` = `out.conteudo`, tipo de urna '1' } via `CFileASN::CodeObjectFunction`.
6. `utimes(internal, {0,0})`: os tempos de acesso/modificação são zerados para 1970.
7. `CSystem::CopyFile(internal, external, preservaAtributos = true)` (os tempos zerados também são copiados); retorna o id.

O nome aleatório, a data de meia-noite no cabeçalho (etapa 5) e os tempos de arquivo zerados (etapa 6) mantêm o horário da captura
fora do envelope e fora dos metadados do arquivo, de modo que as imagens não podem ser ordenadas, a partir desses campos, na ordem em que os eleitores
foram habilitados (a ordem no diretório / números de inode estão fora deste código).

## 8. Boletim de Urna (BU): onde esta unidade está envolvida

Esta unidade não gera o BU. Suas peças usadas pelo código do BU (unidades u09/u23):

* **`CCabecalhoEntidade`** (1945) é o cabeçalho de `EntidadeBoletimUrna` e dos envelopes em torno de `bu.dat`,
  `rdv.dat`, `imgbu.dat`, `hash.dat`: construído por `CGravadorBU`/`CGravadorRDV`/`CGravadorHashes`/`IGravadorEnvelope`
  com `(dhGeracao, CConfiguracaoEleicao pleito id, ETipoCabecalho::Pleito = 1)`; validado com id < 100000, tipo < 3.
  ASN.1: `CabecalhoEntidade { dataGeracao DataHoraJE, idEleitoral CHOICE { [1] idProcessoEleitoral,
  [2] idPleito, [3] idEleicao } }`.
* **`CSeguranca`** (3823) é o `seguranca` do envelope cifrado do BU quando o parâmetro "criptografar BU" está
  ativado: `CSeguranca(idTipoArquivo 0, idCriptografia 1, idArquivoChave = CEPESC out.chave)`; o fluxo do BU lê
  `bu.pk1` com o mesmo código de `LeChavePublica` acima (cgravadorbu.cpp:542..560).
* **`CIdentificacaoSecao`** (5887) identifica a seção em `CGravaResultado` (12098) e nos dados da seção.
* **Logs do encerramento** (`CGravacaoResultados::Executa`): por arquivo de resultado, "Gerando arquivo de resultado
  [bu.dat] + [Início]" … "[Término]", "Copiando arquivo de resultado para MI: [bu.dat]"; depois "Preparando para
  assinatura…", "Início do procedimento…", "Inicia uma sessão no MSE|MSD", "Finaliza a sessão…", "Término do
  procedimento…"; e "Copiando arquivo de resultado para ME: […]" para as cópias no cartão de votação.
  A impressão das vias do BU é registrada no log como "Imprimindo relatório [BU] via nº [n]" usando `ConverteERelatoriosUE`.
* As justificativas (`registro_justificativa`) se tornam o arquivo de resultado `jufa.dat` e o BUJ impresso; elas são
  armazenadas separadamente dos votos.

## 9. Particularidades do build web

* O simulador aciona apenas o terminal do eleitor: justificativas, armazenamento biométrico, logs do encerramento e
  logs de bateria nunca executam. `registro_justificativa` existe (vazia) no `uenux.db` do MEMFS.
* `CControlaArmazenamentoDeImagens` não consegue armazenar nada: o pipeline de WSQ foi removido da compilação (imagens vazias,
  u10), nenhum `/dsk/fi/estatico/chave/` é distribuído e nenhum `api::IKernelHSM` é registrado; `CCepescCipher` é uma
  cifra identidade (u01). `IRng` é `CPrng(seed 0)`, ressemeado a cada chamada, então o nome "aleatório" é constante.
* O `statfs` do MEMFS do Emscripten retorna valores fixos (sempre muito acima de 5 MiB).
* `IPower` é `api::teste::CPowerMock`; `IEventosLog` é `vota::CLogVota`, que grava `dsk/fi/dinamico/log/logd.dat`.

## 10. Observações sobre Wasm / Emscripten

* **merge-similar-functions**: 6045 é o corpo de dois métodos de `CLogComum`, com a string de formatação (início, fim)
  e o srcloc como parâmetros; 2899 é o corpo do destrutor de quatro classes `{vptr, shared_ptr}` não relacionadas, com a
  vtable como parâmetro; 2900 atende quatro singletons vazios (348, 948, 2742, 3603); os thunks de construtor de erro 2260,
  2827, 2861 e 5369 chamam todos a 710 do ecourna com uma vtable.
* **Eliminação de argumentos mortos**: os métodos de `CLogComum` e `IEventosLog::Convert*` são não estáticos no código-fonte
  (não há "static" nos pretty names dos srclocs), mas não recebem `this` em wasm.
* **Propagação de constantes**: `CIdentificacaoSecao` perdeu seu parâmetro `EUrnaTipo` (todos os chamadores passam '1'); a
  verificação da linha 33 desapareceu, deixando sem referência a string "Tipo inválido para urna de seção." (@361994) e um
  código de erro 8915 não usado.
* **Nomes errados por inlining**: a 2725 carrega seis srclocs de três helpers inlinados (:73 `GerarCaminhosUnicos`,
  :117/:127/:135 `LeChavePublica`, :149/:152 `CifrarWsq`), daí o nome `LeChavePublica` dado pelas ferramentas;
  a 5875 (na verdade `CLogComum::LogaNivelBateria`) foi nomeada segundo a sua `ConverteFonteAlim` inlinada; a 3744, segundo a sua
  `ajustaAbrangenciaUFMunicipio` inlinada.
* `std::format` empacota os tipos de argumento com 5 bits cada (por exemplo, 205 = string_view, unsigned).
* Os locks `std::mutex` dos singletons lazy são no-ops (sem pthreads); só resta `mutex::unlock` (150).

## 11. Códigos de erro desta unidade

| família (limites) | código | onde | mensagem |
|---|---|---|---|
| Justificativa {8800,8850} | 8800 / 8801 / 8802 / 8804 / 8805 | cjustificador :55 / :83 / :104, dao :56 / :63 | ver §3 |
| Log {8850,8900} | 8850 / 8852 / 8853 / 8854 (8851 não usado: os dois `i32.const 8851` do módulo são um slot de tabela) | clogcomum :158, ieventoslog :426 / :440 / :456 | "Tipo de alimentação não implementado" / "Valor inválido" |
| Md {8900,8950} | 8900–8907, 8916–8920, 8925–8927 | §5 | §5 |
| NomeArquivo {8950,9000} | 8950 / 8951 / 8952 / 8953 | cnomearquivo :38 / :48 / :73 / :208 | Fase / Número / UF / Abrangência inválida |
| ReconhecimentoBiometrico {9000,9050} | 9000 / 9001 | ccontrolaarmazenamentodeimagens :117 / :135 | "O arquivo " + caminho + " não existe" / " está vazio" (concatenação, não `std::format`) |

## 12. Código estranho ou arriscado (detalhes na lista "suspicious" retornada)

1. `GerarCaminhosUnicos` faz um laço `while (file exists)` sem limite; com o `CPrng(0)` determinístico do build, o
   mesmo id volta para sempre assim que existir uma imagem → travamento (latente: inalcançável no build web).
2. `LeChavePublica` ignora `EntidadeChave.cifrado` e libera o segredo do HSM (o `vector` retornado e sua
   cópia `std::string` passada para `Cria`) sem apagá-lo.
3. `SaveCurrentInternal` (inlinada na 10590, não na 2810) testa `empty()`, mas desreferencia o cursor (seguro no
   único caminho de chamada: `CDataMap::Add` 5725 posiciona o cursor no nó inserido logo antes).
4. `LogaTipoBateria` (inlinada em `CThreadMonitor::Run` 10226, não na 5875) lança (8850) para o código de fonte de alimentação 3
   em vez de registrar no log. O throw é um `__cxa_throw` direto na 10226 (não há try envolvente ali) e o trampolim
   da thread 10255 chama `Run` sem `invoke`, então nada no monitor o captura. No build web, a
   thread de monitoramento nunca é iniciada (`CThread::Start` só é alcançada por meio de `CExecucaoVota`, ver u07), e a própria 5875
   não consegue lançar (sua fonte é a constante 1 ou 2 e seu status é um campo de 2 bits).
5. `MontaNomesEleicao` copia a eleição inteira (todos os cargos) para ler dois campos (apenas desempenho).
6. O nome do arquivo de chave de `LeChavePublica` na 2725 é uma `string_view {117, 7}` apontando para abaixo dos segmentos de dados
   (7 bytes zero em tempo de execução, ver §7 etapa 4); os outros leitores de chave (`CGravadorBU` `bu.pk1`, `CGravadorRCSecao`
   `jufa.pk1`, `CConversorBiometriaEleitorCifrada` `bio.sk1`) anexam um literal de string real. Sem explicação; inofensivo
   no build web porque a função nunca chega tão longe.

## 13. Tabela de mapeamento completa (39 funções)

Os caminhos da última coluna são relativos a `src/uenux2/src/app/comum/`; `run` = observada em execução nos votos gravados.

| wasm | tamanho | run | símbolo reconstruído | arquivo-fonte |
|---|---|---|---|---|
| 348 | 15 | * | `comum::CArquivosResultado::GetInst()` | u24-foreign-fragments.cpp (orig. carquivosresultado.cpp) |
| 1161 | 893 | * | `comum::(anon)::FormataNumero(uedword, uedword)` (:48) | nomearquivo/cnomearquivo.cpp |
| 1705 | 787 | * | `comum::CNomeArquivo::MontaNome(EUrnaFase, uedword, uf, sufixo, ext)` (nome inferido) | nomearquivo/cnomearquivo.cpp |
| 1945 | 157 | | `comum::md::CCabecalhoEntidade::CCabecalhoEntidade` (:24/:27) | md/ccabecalhoentidade.cpp |
| 2260 | 18 | | thunk de ctor de `CUeComumJustificativaError` (CBaseError<EUeComumJustificativaError,{8800,8850}>) | justificativa/cjustificador.cpp (comentário) |
| 2725 | 5103 | | `comum::CControlaArmazenamentoDeImagens::Armazena` (+ GerarCaminhosUnicos :73, LeChavePublica :117–135, CifrarWsq :149/152 inlinadas) | reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp |
| 2810 | 46 | | `comum::CJustificador::~CJustificador()` | justificativa/cjustificador.cpp |
| 2827 | 18 | | thunk de ctor de `CUeComumNomeArquivoError` ({8950,9000}) | nomearquivo/cnomearquivo.cpp (comentário) |
| 2828 | 530 | | `comum::(anon)::FormataFase(EUrnaFase)` (:38) | nomearquivo/cnomearquivo.cpp |
| 2861 | 18 | | thunk de ctor de `CBaseError<comum::EUeComumLogError,{8850,8900}>` | log/ieventoslog.cpp (comentário) |
| 2899 | 63 | | corpo de destrutor ICF `{vptr, shared_ptr}` (~CJustificadorServico / ~CComparecimentoMesarioServico / ~CFormPart / ~CParteEleitores) | justificativa/cjustificador.cpp (comentário) |
| 3739 | 865 | | `comum::md::CAbrangencia::CAbrangencia` (:31–:51) | md/cabrangencia.cpp |
| 3740 | 12 | | `comum::servico::CJustificadorServico::~CJustificadorServico()` (D1, slot 0) | justificativa/cjustificador.cpp |
| 3744 | 2521 | * | `comum::CNomeArquivo::MontaNomesEleicao` (nome inferido) + `ajustaAbrangenciaUFMunicipio` inlinada (:208) | nomearquivo/cnomearquivo.cpp |
| 3773 | 540 | * | `comum::(anon)::FormataUF(const std::string&)` (:73) | nomearquivo/cnomearquivo.cpp |
| 3823 | 305 | | `comum::md::CSeguranca::CSeguranca` (:24/:27/:30) | md/cseguranca.cpp |
| 3828 | 20 | | `comum::CLogComum::LogaResultadoCopiadoResFE` (:84) | log/clogcomum.cpp |
| 5369 | 18 | | thunk de ctor de `CUeComumReconhecimentoBiometricoError` ({9000,9050}) | reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp (comentário) |
| 5831 | 259 | | biblioteca: `std::__uninitialized_allocator_relocate` para `vector<CIdentificacaoJustificativa>` | justificativa/dao/cjustificadordao.cpp (comentário) |
| 5875 | 1137 | | `comum::CLogComum::LogaNivelBateria(uebyte, uebyte)` (:106; ConverteFonteAlim :440 e ConverteStatusBateria :456 inlinadas) | log/clogcomum.cpp (+ ieventoslog.cpp) |
| 5876 | 642 | | `comum::CLogComum::LogaGerandoResultados` (:92) | log/clogcomum.cpp |
| 5878 | 20 | | `comum::CLogComum::LogaCopiandoArqResParaFI` (:76) | log/clogcomum.cpp |
| 5879 | 885 | | `comum::IEventosLog::ConverteERelatoriosUE` (:426) | log/ieventoslog.cpp |
| 5886 | 199 | | `comum::md::CIdentificacaoUrna::CIdentificacaoUrna` (:25/:28/:31) | md/cidentificacaourna.cpp |
| 5887 | 160 | * | `comum::md::CIdentificacaoSecao::CIdentificacaoSecao` (:38/:41) | md/cidentificacaosecao.cpp |
| 6045 | 508 | | corpo mesclado de `LogaCopiandoArqResParaFI` / `LogaResultadoCopiadoResFE` | log/clogcomum.cpp (comentário) |
| 6236 | 132 | | biblioteca: `utimes()` da musl (→ `__futimesat` → `utimensat`) | reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp (comentário) |
| 10888 | 12 | | `api::CFormPart::~CFormPart()` (thunk → 2899) | u24-foreign-fragments.cpp (orig. api/gui/cformpart.cpp, caminho inferido) |
| 11205 | 12 | | `comum::CParteEleitores::~CParteEleitores()` (thunk → 2899) | u24-foreign-fragments.cpp (orig. comum/relatorios/cparteeleitores.cpp, caminho inferido) |
| 11412 | 203 | | `comum::asn::CConversorSeguranca::DoDesconverte` (slot 3) | u24-foreign-fragments.cpp (orig. comum/asn/cconversorseguranca.cpp, caminho inferido) |
| 11464 | 13 | | `comum::servico::CJustificadorServico::~CJustificadorServico()` (D0, slot 1) | justificativa/cjustificador.cpp (comentário) |
| 11468 | 12 | | `comum::dao::CJustificadorDAO::Clonar` (slot 2) | justificativa/dao/cjustificadordao.cpp |
| 11469 | 1662 | | `comum::dao::CJustificadorDAO::RecuperarTodos` (slot 8) | justificativa/dao/cjustificadordao.cpp |
| 11470 | 1399 | | `comum::dao::CJustificadorDAO::Recuperar` (slot 7) | justificativa/dao/cjustificadordao.cpp |
| 11471 | 51 | | `comum::dao::CJustificadorDAO::Atualizar` (slot 6, :63) | justificativa/dao/cjustificadordao.cpp |
| 11472 | 51 | | `comum::dao::CJustificadorDAO::ExcluirID` (slot 5, :56) | justificativa/dao/cjustificadordao.cpp |
| 11473 | 412 | | `comum::dao::CJustificadorDAO::Inserir` (slot 3) | justificativa/dao/cjustificadordao.cpp |
| 11475 | 164 | | `comum::CJustificadorDadoTitulo::Text()` (:104) | justificativa/cjustificador.cpp |
| 11477 | 37 | | destrutor atexit de `CJustificador::s_pInstancia` (`~unique_ptr<CJustificador>`) | justificativa/cjustificador.cpp (comentário) |

Funções relacionadas fora da unidade, identificadas durante a leitura: 948 = `CLogComum::GetInst`, 1391 =
`CJustificador::GetInst`, 3742 = `CJustificador::CJustificador` (a u20 concorda), 5888 = ctor de
`CIdentificacaoUrnaContingencia`, 5168 = ctor delegante de `CPlainText` (0, 1, …), 2742 = singleton vazio do codec WSQ, 11640 / 11660 = resets
atexit dos singletons `CLogComum` / `CArquivosResultado`.

## 14. Questões em aberto

* Os nomes exatos de 1705 / 3744 (sobrecarga de `MontaNome`, `MontaNomesEleicao`) e da struct do primeiro parâmetro da 3744
  (fase +0, uf +8; a u02 chama a mesma forma de `SIdentificacaoCarga`); por que a 3744 retorna um map com no máximo uma entrada.
* O método do singleton do codec WSQ (2742) chamado em `Armazena` e se ele é de fato uma identidade no
  build da urna.
* O nome da função que envolve `SaveCurrentInternal` na 10590 (as proteções da MI/MV) e se ela pertence a
  `CJustificador` ou a `CPedeAnoNascimento`.
* O código de erro 8803 (não usado) e o código da verificação da linha 33 de `CIdentificacaoSecao`, eliminada por dobramento de constantes (8915 presumido).
* Por que o nome do arquivo de chave na 2725 é uma `string_view` no endereço 117 (abaixo de `GLOBAL_BASE` 1024) em vez de um
  literal em `.rodata`, e se ele é de fato `wsq.pk1`.
