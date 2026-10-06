# u22 — registro de mesários, acessores do modelo da eleição, gravação da cédula no RDV, caminhos de armazenamento e verificação de assinaturas (`uenux2/src/app/comum`: `comparecimentomesario/`, `dados/md/…`, `cpath`, `cpacotearquivos`, `cinfomtlcd`)

A unidade u22 tem **107 funções wasm**; **16 delas rodaram** durante os votos web registrados. O builder as agrupou
por diretório de origem, então a unidade é uma coleção de cinco partes independentes de `uenux2/src/app/comum`,
mais algumas funções de outros diretórios que foram atribuídas aqui:

| parte | arquivos originais | funções | rodou no simulador? |
|---|---|---|---|
| **A. Registro de mesários** (*comparecimento de mesários*) | `comparecimentomesario/` (17 arquivos: controlador, DAO, 15 arquivos de estado) | 52 | não (0/52) — a thread do operador nunca roda no build web |
| **B. Gravação da cédula de um eleitor no RDV em memória** | `dados/md/rdv/cvoto.cpp`, `cvotoscargos.cpp`, `cvotoseleicoesvota.cpp` (+ func 4454 de `vota/eleitor/celeitorvotando.cpp`) | 9 | **sim** (4/9, em todo voto) |
| **C. Acessores e validação do modelo da eleição** (cargo, pleito, eleição, processo; dois conversores ASN.1; a fonte de dados do nome do cargo) | `dados/md/processoeleitoral/*.cpp`, `dados/asn/…`, `vota/eleitor/comum/ctelasvota.cpp` | 20 | em parte (5/20: nomes, "1ª vaga", carga do pleito) |
| **D. Caminhos de armazenamento, a tabela de arquivos do SAVD e a verificação de assinaturas na inicialização** | `cpath.cpp`, `carquivossavd.cpp` (construtor), `cpacotearquivos.cpp` | 14 | **sim** (5/14, durante `votaInit`) |
| **E. Ícone de bateria / imagens no LCD do terminal do mesário** | `cinfomtlcd.cpp` (+ código de template de `BatteryIconDataSource`) | 9 | `Update` (1/9) |
| helpers genéricos de biblioteca emitidos aqui | `std::to_string(int)`, `operator+(const char*, string&&)`, o thunk de fábrica de `CDadosError` | 3 | `to_string` (1/3) |

Arquivos-fonte reconstruídos (todos em `src/uenux2/src/app/`):
`comum/cinfomtlcd.{h,cpp}`, `comum/cpath.{h,cpp}`, `comum/cpacotearquivos.{h,cpp}`,
`comum/carquivossavd.u22.cpp` (fragmento: o construtor de 77 KB), `comum/comparecimentomesario/`
`icontroladorregistramesarios.h` *(caminho inferido)*, `ccontroladorreconhecimetomesario.{h,cpp}`,
`md/ccomparecimentomesario.h` *(caminho inferido)*, `dao/ccomparecimentomesariodao.{h,cpp}`,
`estados/estadosregistromesarios.h` *(um header para todas as classes de estado, inferido)* e um `.cpp` por arquivo de estado,
`comum/dados/md/processoeleitoral/{ccargo.h,ccargo.cpp,cdetalhecandidato.cpp,celeicaope.cpp,cpleito.cpp,cprocessoeleitoral.cpp}`,
`comum/dados/md/rdv/{cvoto.cpp,cvotoscargos.h,cvotoscargos.cpp,cvotoseleicoesvota.cpp,cvotos.u22.cpp}`,
`comum/dados/asn/processoeleitoral/cconversornomescargo.cpp` *(caminho inferido)*,
`comum/dados/asn/cconversorpartido.cpp` *(caminho inferido)*, e dois fragmentos para outros donos:
`vota/eleitor/celeitorvotando.u22.cpp` (func 4454 `GravaVotos`) e `vota/eleitor/comum/ctelasvota.u22.cpp`
(func 12587). O mapa completo de funções está no §13.

---

## 1. Glossário

| termo | significado |
|---|---|
| mesário | membro da mesa receptora da seção (normalmente 4: presidente, 1º/2º mesário, secretário) |
| comparecimento de mesários / registro de mesários | registro de quais mesários estiveram presentes; armazenado por (título, período) na tabela SQLite `comparecimento_mesario` de `uenux.db` e impresso no BIM |
| período presente | 1 = abertura (registrado antes ou durante a votação), 2 = encerramento (após a votação) |
| MT (micro-terminal, *terminal do mesário*) | o teclado do operador com um LCD de texto 4×40, um pequeno LCD gráfico, LED, buzzer e sensor de impressão digital |
| título (de eleitor) | número de 12 dígitos do título de eleitor; o mesário se identifica com ele |
| digital / batimento | impressão digital / comparação de impressões digitais ("batimento de digitais") |
| WSQ | formato de imagem de impressão digital; as digitais capturadas são armazenadas cifradas como `me<nnnnnn>.wsq` |
| cédula | os pares (cargo, voto) que um eleitor confirmou para uma eleição |
| RDV (*registro digital do voto*) | a lista de todos os votos por eleição e cargo, sem vínculo com nenhum eleitor, mantida ordenada |
| comparecimento | número de cédulas depositadas |
| cargo / consulta | cargo em disputa / pergunta de referendo (modelada como um cargo) |
| escolha / vaga | vaga de um cargo com várias vagas (Senador com 2 vagas: "1ª vaga", "2ª vaga") |
| suplente / suplência | quem compõe a chapa com o titular (vice, 1º/2º suplente de senador) |
| pleito | turno (1º / 2º turno) do processo eleitoral |
| SAVD | o serviço de assinatura/verificação de arquivos da urna (`comum::IInterfaceSavd`); `.vsu` = pacote de assinatura na urna, `.vsc` = na mídia de resultado |
| MI / MV | memória interna (`/dsk/fi`, flash interna) / memória de votação (`/dsk/fe`, cartão flash removível) |
| MR | memória de resultado (`/dsk/mr`, o pendrive de resultados) |
| SA | *sistema de apuração*, a aplicação de apuração usada quando uma urna falha (tem a sua própria árvore `dinamico/sa/<seção>/`) |
| BIM | *boletim de identificação de mesários*, impresso no fim do dia com os nomes dos mesários |

---

## 2. Classes e hierarquia (RTTI)

```
api::CState
└─ comum::CAppState                                        (+4 m_proximoEstado, +8..+10 msg/key/tick flags)
   ├─ comum::CRegistrarMesarios            vt @1593592     "Registrar mesário?"              28 B, flags 2
   ├─ comum::CPedeTituloMesario            vt @1593728     dispatcher by period              12 B, flags 0
   ├─ comum::IPedeTituloMesario            vt @1595584     "Informe o título do mesário:"    28 B, flags 2 (slot 9 pure)
   │    ├─ CPedeTituloMesarioInicial       vt @1593960     slot 9 -> 1 (abertura)
   │    ├─ CPedeTituloMesarioVotacao       vt @1594264     slot 9 -> 1 (abertura)
   │    └─ CPedeTituloMesarioFinal         vt @1594112     slot 9 -> 2 (encerramento)
   ├─ comum::CTituloMesarioVazio           vt @1594568     "Título inválido: número vazio"
   ├─ comum::CTituloMesarioInvalido        vt @1594656     "Título inválido: <título>"
   ├─ comum::CTituloMesarioJaRegistrado    vt @1594760     "Título: <título> / Já registrado"
   ├─ comum::CPedeDigitalMesario           vt @1595336     fingerprint capture               32 B, flags 6
   ├─ comum::CDigitalMesarioNaoReconhecida vt @1595248     time-out screen                   20 B, flags 2
   ├─ comum::CGestorDadoMesario            vt @1595176     dispatcher by period              12 B, flags 0
   ├─ comum::IGestorDadoMesario            ti @1594976     saves the row (slot 2), slot 9 = period
   │    ├─ CGestorDadoMesarioInicial       vt @1594996     slot 9 -> 1
   │    ├─ CGestorDadoMesarioVotacao       vt @1595056     slot 9 -> 1
   │    └─ CGestorDadoMesarioFinal         vt @1595116     slot 9 -> 2
   ├─ comum::CMesarioRegistrado            vt @1594880     "Mesário registrado com sucesso."
   ├─ comum::CLimiteMesariosRegistradosAtingido vt @1594416 "Limite de mesários registrados atingido"
   ├─ comum::CConfirmaFimRegistroMesarios  vt @1593800     "Finalizar registro de mesários?"
   └─ comum::CEncerraRegistroMesarios      vt @1593888     hands control back to VOTA        28 B, flags 0

comum::IControladorRegistraMesarios  (ti @1534552)  ── vota::CControladorRegistraMesariosVota (vt @1586608, 37 slots)
comum::CControladorReconhecimentoMesario      (no RTTI, singleton @1909960, 24 B)
comum::CRegistradorMesario                     (no RTTI, singleton @1909932, 52 B: CDataMap<PK,row> + CComparecimentoMesarioServico)
comum::dao::CComparecimentoMesarioDAO : IComparecimentoMesarioDAO : IUenuxGenericDAO<CComparecimentoMesario, CComparecimentoMesarioPK> : IDAO
comum::md::CComparecimentoMesario (52 B), CComparecimentoMesarioPK (20 B)          (no RTTI)

comum::CInfoMTLCD : api::IObserver<api::SharedIImage>            vt @1551828 (singleton @1838572, 64 B)
api::BatteryIconDataSource<Vertical|Horizontal> : api::IObservable<SharedIImage>   vt @1551916 / @1581580

comum::asn::CConversorNomesCargo : IConversorASN<ModuloEleicao::NomesCargo, md::CNomesCargo>   vt @1570172
comum::asn::CConversorPartido    : IConversorASN<ModuloPartidos::Partido, md::CPartido>         vt @1561268
```

As classes não polimórficas desta unidade (`CPath`, `CArquivosSavd`, `CPacoteArquivos`, `md::CCargo`,
`md::CPleito`, `md::CEleicaoPE`, `md::CDetalheCandidato`, `md::CProcessoEleitoral`, `md::CVoto`,
`md::CVotos`, `md::CVotosCargos`, `md::CVotosEleicoesVota`) não têm RTTI; seus nomes vêm de assinaturas de srcloc,
e seus layouts, dos acessos a campos (documentados nos headers).

---

## 3. Parte A — registro de mesários (`comparecimentomesario/`)

### 3.1 Para que serve

Antes de a votação começar, durante a votação e depois que ela termina, os mesários da seção se identificam
no **MT** (terminal do operador) com o seu título; se a eleição usa biometria, a impressão digital deles também
é capturada. Cada registro vira uma linha da tabela SQLite `comparecimento_mesario` (em
`dinamico/trab<turno>/uenux.db`, assinada como `uenux.vsu` e copiada para a MV). No fim do dia, o VOTA
imprime o **BIM** ("comparecimento de mesários na abertura / no fechamento", u08/u09) a partir dessas linhas, usando
`CImprimirIdentificacaoMesariosFinal::GetNomeMesario` (func 5385) para imprimir cada nome.

Os estados de comum são independentes do host: tudo o que depende do VOTA passa por
`comum::IControladorRegistraMesarios` (reconstruído com todos os 37 slots em
`icontroladorregistramesarios.h`; os nomes dos slots são inferidos de `vota::CControladorRegistraMesariosVota`,
funcs 10766–10800). Os mais importantes:

| slot | implementação no VOTA | significado |
|---|---|---|
| 5 `AtualizaEstadoRegistro` | `estadoVota` '6'→'7', '9'→':' (e '8'→'7' quando ninguém votou e nenhum mesário está registrado, fora do treinamento) | entra na fase de registro |
| 6 `GetPeriodoRegistro` | tabela @534720 `{1,2,0,3}` indexada por `estadoVota - '7'` | 1 INICIAL ('7' REGISTROMESARIOINICIAL), 2 VOTACAO ('8' VOTAR), 3 FINAL (':' REGISTROMESARIOFINAL), 0 caso contrário |
| 7 `LimiteMesariosAtingido` | mais de 5 linhas para o período (abertura para INICIAL/VOTACAO, encerramento para FINAL) | máximo de **6** mesários por período |
| 9/11/12 | `CAguardaInicio` / `CRegistroMesarioEncerrado` / `CFinalizaOperador` | para onde a thread do operador vai depois |
| 10/15 | envia a mensagem 11 / a mensagem 7 à thread do eleitor | 7 = início da cadeia de geração do BU (`CAguardaMensagem::vf6(7)` → `CGeraBU`, u09) |
| 13/14 | busca em `CEleitores` do título digitado (tipo 1) | o mesário é eleitor desta seção? |
| 16 | `vota_f4662` = `CSincronizaVota::SincronizaBancoDados` | assina `uenux.db` e o copia para a MV |
| 17/18 | `CThreadOperador` +96 | o título digitado |
| 19–36 | entradas de `CLogVota` | "Mesário {} registrado", "Operador confirmou o registro de mesários", … (textos exatos no header) |

### 3.2 Máquina de estados (thread do operador)

```
VOTA (CAguardaInicio / CEscolheOpcao "3-Registrar mesários" / CFimAquisicaoVotos)
  └─> CRegistrarMesarios  StartState: log "indagado"; AtualizaEstadoRegistro; limit? -> CLimite…
        MT: "Registrar mesário?" / "CORRIGE: Cancelar  CONFIRMA: Prosseguir"      (buzzer 51,10)
        CONFIRMA (log "confirmou") -> CPedeTituloMesario        CORRIGE (log "cancelou") -> CConfirmaFimRegistroMesarios
CPedeTituloMesario  (dispatcher, by GetPeriodoRegistro)
        1 -> CPedeTituloMesarioInicial   2 -> …Votacao   3 -> …Final   0 -> stays (nothing drawn)
IPedeTituloMesario  MT: "Informe o título do mesário: " [clock] / [12-digit input] /
                         "Quantidade de mesários registrados:" [nn] / "CORRIGE: Encerrar  CONFIRMA: Prosseguir"
        (the constructor logs "Registrando mesários antes|durante|após a votação" - once per process)
        CORRIGE with empty input -> CConfirmaFimRegistroMesarios   (with digits: the field erases them)
        CONFIRMA: empty -> CTituloMesarioVazio
                  título = format("{:0>{}}", digits, 12); SetTituloMesario(título)
                  !CValidadorIdentidade::Valida(TITULO, título) || título == "000000000000" -> CTituloMesarioInvalido
                  CRegistradorMesario already has (título, período) -> CTituloMesarioJaRegistrado
                  urna abroad (UF "ZZ") or biometrics disabled (cfg +665) -> CGestorDadoMesario
                  else -> CPedeDigitalMesario
CTituloMesarioVazio / Invalido / JaRegistrado   CORRIGE -> CPedeTituloMesario   (log "título inválido" / "já registrado")
CPedeDigitalMesario (15 s time-out tick, 50 ms read tick)
        MT: "Solicite que o(a) mesário(a)" / <name> / "posicione POLEGAR ou INDICADOR no sensor" / "CORRIGE: Cancelar"
        CORRIGE -> CPedeTituloMesario;   15 s -> CDigitalMesarioNaoReconhecida ("Mesário(a) não reconhecido(a)." CONFIRMA -> CPedeTituloMesario)
        50 ms: frames (3, entropy > 4 bits; first frame on models 2009/2010/2020/2022), finger present ->
               "Por favor, aguarde."; LED 1; [emscripten_sleep(1000)]; invert/flip image; WSQ-encode it
               (the shared image used below is the ENCODER OUTPUT, empty in this build: stub); then
               mesário is a voter of the section:
                   with fingerprints in the roll: estado = 2; compare with fingers 1,6,2,7 (threshold 20);
                        match -> log "Realizada a conferência…", dedo = finger;  no match -> log scores, store image
                   without fingerprints:        estado = 3; store image
                   ("store image" = SalvaDigitalMesario, func 5382: always sets dedo = 0; sets id_arquivo only
                    when the image was written)
                   log "Mesário {} é eleitor da seção"
               not a voter of the section:       estado = 3; store image; log "… não é eleitor da seção"
               -> CGestorDadoMesario
CGestorDadoMesario (dispatcher by period) -> CGestorDadoMesario{Inicial,Votacao,Final}
IGestorDadoMesario::StartState: row = CComparecimentoMesario(título, período (1|2), pertenceSeção, estado, dedo,
        now, [idArquivo]); CRegistradorMesario.Update(row); SaveCurrent (DAO Inserir); SincronizaBancoDados
        -> CMesarioRegistrado
CMesarioRegistrado  log "Mesário {} registrado"; limit reached? -> CLimite…; else log "indagado se continua"
        MT: "Mesário registrado com sucesso." / "Continuar registrando mesários?" / "CORRIGE: Encerrar  CONFIRMA: Prosseguir"
        CONFIRMA -> CPedeTituloMesario        CORRIGE -> CConfirmaFimRegistroMesarios
CLimiteMesariosRegistradosAtingido  MT: "Limite de mesários registrados atingido" / "Quantidade máxima permitida: 6" /
        "CONFIRMA: Finalizar";  log "Limite … Limite máximo: 6";  CONFIRMA -> CEncerraRegistroMesarios
CConfirmaFimRegistroMesarios  MT: "Finalizar registro de mesários?" / "CORRIGE: Voltar  CONFIRMA: Finalizar"
        CORRIGE -> (log "confirmou") CPedeTituloMesario        CONFIRMA -> CEncerraRegistroMesarios
CEncerraRegistroMesarios (by period)  0/1: -> VOTA slot 9 (CAguardaInicio), log "encerrou", message 11 to the voter
                                      2:   -> slot 11 (CRegistroMesarioEncerrado), log
                                      3:   -> slot 12 (CFinalizaOperador), log, message 7 (BU generation starts)
```

Enquanto isso, todo estado mostra o mesmo formulário na **tela do eleitor** (func 1255): cabeçalho de status 7,
"REGISTRO DE MESÁRIOS" em (320,150) e "Siga as instruções no terminal do mesário." em (320,250).

### 3.3 Dados gravados

`CComparecimentoMesario` (52 bytes) ↔ linha de `comparecimento_mesario` (DDL executada na inicialização, u02 §3.3):

| campo (offset) | coluna | valores |
|---|---|---|
| `CEleitorIdentidade` (+0: string, +12: tipo) | `numero_titulo` BIGINT (`CStringUtils::ToQWord`), `tipo_identificador` | tipo sempre 1 (título) aqui |
| período (+16) | `periodo_presente` | 1 abertura (Inicial e Votação!), 2 encerramento |
| pertence à seção (+20) | `pertence_secao` | slot 13 do IControlador |
| estado (+24) | `estado_biometria` | 1..3 (`ConverteEstadoBiometria` rejeita os demais: 7755) — 1 sem captura, 2 as digitais do cadastro foram comparadas, 3 captura armazenada |
| dedo (+28) | `dedo_habilitacao` | 0..10 (`ConverteDedoHabilitacao`, 7756) — dedo que coincidiu, 0 caso contrário |
| `api::CDateTime` (+32) | `data_hora_registro` | µs desde a época juliana: `timegm(...) * 1e6 + 210866803200000000` |
| `optional<uint32>` (+44, flag +48) | `id_arquivo` (NULL quando ausente) | id do `me<id>.wsq` armazenado |

DML: `INSERT INTO comparecimento_mesario (...) VALUES (?, ?, ?, ?, ?, ?, ?, ?)` (Inserir, 10403);
`SELECT … WHERE numero_titulo = ? and periodo_presente = ?` (Recuperar, 10402);
`SELECT … ORDER BY pertence_secao ASC, numero_titulo ASC, periodo_presente ASC, dedo_habilitacao ASC`
(RecuperarTodos, 10401). Imagens capturadas: WSQ cifrado em `<trab>/wsq/operador/` (nas duas flashes, via
`CControlaArmazenamentoDeImagens`, u24) e `me{:06}.wsq` em `<trab MV>/wsq/registrado/` (func 5371,
exige ≥ 5 MiB livres).

### 3.4 Build web

Nenhuma destas 52 funções rodou: o simulador nunca roda a thread do operador (u06/u10), nenhum
`IControladorRegistraMesarios` é registrado no modo web (`CAjusteInicial`, que empilha o controlador do
VOTA, é pulado), e `IFingerScanner` / `IFingerMatcher` / `IFingerDetection` não têm implementação.
A tabela `comparecimento_mesario` é criada (ela existe, vazia, nos snapshots do MEMFS), mas nunca é gravada.
O extrator de templates e o codificador WSQ são stubs vazios neste build (u10 §6.4), então até
`CControladorReconhecimentoMesario::ComparaDigitais` (func 5372) compara dois templates vazios; seus dois
parâmetros vector foram removidos pela eliminação de argumentos mortos do LTO (a função wasm só recebe `this`).

---

## 4. Parte B — gravando uma cédula no RDV (`dados/md/rdv/`, func 4454)

`vota::CEleitorVotando::GravaVotos` (func 4454, chamada de "CVotosCargos::ConfereCedula" pelas ferramentas) roda quando
o eleitor confirma o último cargo (a partir de `NeedChangeState`) ou é suspenso. Ela foi **observada nos dois votos
registrados**. Passo a passo:

1. `CThreadEleitor::StopTick(m_tickEleitorDemorando)`.
2. **Separação por eleição.** Para cada `(cargo, CVoto)` em `g_votosEleitor` (@1833300), encontra a eleição do cargo
   na lista (eleição, cargo) de `CConfiguracaoEleicao` (func 3774); cargo desconhecido → eleição 0. Monta
   `std::map<TEleicaoID, CCedula>` (uma eleição municipal gera uma cédula; uma eleição geral, duas).
3. Para cada cédula: `CRdvVota::GetInst().RecebeCedula(eleição, cédula)` →
   `CVotosEleicoesVota::RecebeCedula` (:215): busca a eleição (func 3708, erro 4735 "Eleicao (id) nao
   encontrada") e depois
   * **`CVotosCargos::ConfereCedula`** (:248–:295):
     - a cédula tem exatamente Σ qtdEscolhas votos (func 5642) — 4672 "Tamanho da cedula (n) difere do esperado (m)";
     - todo cargo da cédula existe — 4673; cada cargo tem exatamente `qtdEscolhas` votos — 4674;
     - um voto NOMINAL tem exatamente `numeroDigitos` dígitos — 4675;
     - um voto NULO_POR_REPETICAO só é permitido em um cargo com várias vagas (4676), e a cédula deve conter o
       voto nominal que ele repete (mesmo cargo, tipo 2, mesmos dígitos) — 4677;
   * **`CVotosCargos::InsereCedula`** (:199): copia o mapa inteiro de cargos (um cargo ausente da cópia lança 4669
     "Cargo nao encontrado n", :199), insere cada voto no índice retornado pelo
     posicionador (`CRdvPosicionadorVota`, func 11496 — busca binária por (tipo, digitado), de modo que o RDV permanece
     **ordenado** e a ordem de votação se perde), `CVotos::Insere` (cvotos.cpp:40, 4662 se o índice estiver fora do
     intervalo), e então troca os mapas (tudo ou nada por eleição).
4. `CEstadoGeralVota::MarcaUltimoVoto` (cestadogeralvota.cpp:131/136): exige que a aquisição tenha começado (+32, senão
   8086) e não tenha terminado (+48, senão 8087); armazena `CDateTime::Now()` como hora do último voto (+56) e incrementa
   o contador de eleitores (+52, uint16).
5. `m_proximoEstado = CSincronismoEleitor` (persiste `rdv.dat`/`vota.bin` na urna; um no-op no build web,
   então a cédula fica apenas na memória), finaliza e descarta o subestado do cargo.

`md::CVoto` (func 2256, :80) impõe quais tipos de voto carregam dígitos — obrigatórios para legenda (1), nominal (2)
e nulo por repetição (7); proibidos para branco (3), branco/nulo após suspensão (5, 6) e os dois
nulos de cargo sem candidato (8, 9); livres para nulo (4); qualquer outro valor dá "CVoto - tipo de voto invalido n" (4661).
`CVotosCargos(TMapa)` (func 5643) verifica, na carga, que todo cargo de uma eleição contém o mesmo número de
cédulas (4666 mapa vazio, 4667 não é múltiplo de qtdEscolhas, 4668 diferente do primeiro cargo), e
`CVotosEleicoesVota(TMapa)` (func 5640, com `MontaMapa` inlinado), que nenhum cargo pertence a duas eleições (4724)
e que o mapa não está vazio (4723). `Comparecimento(eleição)` (func 5639, :118) = votos do primeiro cargo /
sua qtdEscolhas.

---

## 5. Parte C — modelo da eleição (`dados/md/processoeleitoral/`)

| função | comportamento | erros (EUeComumDadosError) |
|---|---|---|
| `CCargo::Valida` (5657) | abrangência < 3; 1 ≤ dígitos ≤ 5; escolhas ≥ 1; 1 ≤ página de impressão ≤ 5; consulta exige `DetalheConsulta`, majoritário/proporcional exigem `DetalheCandidato`, outros tipos são inválidos | 8138..8144 (:212..:240) |
| `CCargo::GetDetalheCandidato` (1388) / `GetDetalheConsulta` (1923) | acessores de optional | 8134 "Não é cargo de candidato." / 8135 "Não é cargo de consulta." |
| `GetNome` (1547), `GetNomeMasculino` (3716), `GetNomeAbreviado` (2797), `GetNome(sexo)` (2796), `GetNomeSuplente(n, sexo)` (3715) | formas do nome em `NomesCargo` (neutro/masculino/feminino/abreviado), ou o nome da consulta | via 8135 / 8145 |
| `CCargo::GetOrdinalEscolha` (2258) | "" quando há 1 vaga, senão `"{}ª vaga"` (`ª` em Latin-1) | 8137 "Cargo {} com escolha invalida {}" |
| `CDetalheCandidato::GetSuplente(n)` (1545) | base 1 | 8145 "Suplente inexistente" |
| `CEleicaoPE::ValidaOrdem` (3714) / `GetCargos` (2257) | valores de ordem únicos / lista de cargos não vazia | 8153 / 8154 |
| `CPleito::CPleito` (5652) | ids de eleição únicos; #versões ≥ #eleições; #situações == #eleições (apenas contagens) | 8159 / 8160 / 8161 |
| `CPleito::GetEleicao` (5651) / `GetSituacoesEleicoes` (5648) | busca linear | 8162 / 8164 |
| `CProcessoEleitoral::GetPleito2` (1267) | 2º turno opcional | 8165 "Não há pleito 2." |
| `CConversorNomesCargo::DoDesconverte` (11369) | os quatro nomes não podem ser vazios | 8155..8158 |
| `CConversorPartido::DoDesconverte` (11459) | número < 100, sigla < 25 caracteres, nome < 56 caracteres (vazio permitido) | 8050..8052 |
| `CDataText<DS_NomeCargoNeutroComEscolha>` (12587) | linha superior de toda tela de votação: `GetNome()` + `" - " + GetOrdinalEscolha(g_numeroEscolha)` para cargos com várias vagas | — |

---

## 6. Parte D — caminhos, a tabela de arquivos do SAVD, a verificação de assinaturas na inicialização

### 6.1 `CPath`

`ms_raizesFlash = {"/dsk/fi/", "/dsk/fe/"}` (@1838576) e `ms_raiz = "/"` (@1838600) são definidos por
`__wasm_call_ctors`. `GetPathTrab(origem, turno)` = `<flash>dinamico/trab<1|2>/` (func 358, 7227 "Turno
invalido: {}"), `GetPathResult` = `<flash>dinamico/res<1|2>/` (func 1399, 7226), `GetPathRootSemSA(origem)` =
`ms_raiz / "/dsk/fi/".relative_path()` (func 634, 7224 "Mídia inválida: {}"). Os outros membros
(`GetPathDinamico` 1082, `GetPathEstatico` 762, `GetPathLog` 5899, `GetPathMR` 949, `GetPathRoot` 6048) estão
listados em outras unidades, mas são reconstruídos no mesmo `cpath.cpp`.

### 6.2 `CArquivosSavd` (func 1164)

`CArquivosSavd::GetInst()` constrói, uma única vez, três `std::map`s (o construtor está totalmente inlinado: 77,002 bytes).
Seu conteúdo foi **extraído do simulador em execução** (um hook adicionado a uma cópia de `tools/run/headless.mjs`
percorre as três árvores rubro-negras do objeto @1838544 após `votaInit`) e confere com o código:

* `m_pacotes` (84 entradas, ids 120–203, lidas por `operator[](ESavdPacote)` func 275): pacotes de assinatura, p. ex.
  120/121 `…/dinamico/eg.vsu` (fi/fe), 122–125 `trab{1,2}/vota.vsu` (fi t1, fi t2, fe t1, fe t2), 126–129 `bu.vsu`,
  130–133 `buj.vsu`, 134–137 `rdv.vsu`, 138–141 `ze.vsu`, 142–145 `rze.vsu`, 146–149 `gap.vsu`, 150/151 `sa.vsu`
  (apenas fi), 152–155 `red.vsu`, 156 `/dsk/fe/dinamico/trab/tabcorr.vsc`, 157 `/dsk/fe/estatico/{:05}{:04}{:04}-lo.vsc`,
  158–161 `…/dinamico/res{1,2}/{:c}{:05}{}{:05}{:04}{:04}-vota.vsc` (o pacote que assina os arquivos de resultado),
  162–165 `{:c}{:05}{}{:05}{:04}{:04}-sa.vsc` (**sem diretório**), 166–169 `…-red.vsc`, 170–189 pacotes do SA em
  `dinamico/sa/{}/trab{1,2}/`, 190 `/dsk/mr/sieco-dados/`, 191–194 `bim.vsu`, 195–198 `behb.vsu`, 199–202
  `uenux.vsu`, 203 `/dsk/fe/estatico/dadoscarga.vsu`.
* `m_arquivos` (84 entradas, ids 25–115, lidas por `operator[](ESavdArquivoUE)` func 680): os arquivos assinados, como nomes
  ou padrões de `std::format` — 25/26 `eg.bin`, 27–30 `gap.bin`, 31/32 `vota.bin`, 33 `sa.bin`, 34 `tabcorr.dat`,
  35 `…-bu.dat`, 36 `…-busa.dat`, 37 `…-rdv.dat`, 39/41/42 `…-jufa.dat`, 43 `…-imgbu.dat`, 44 `…-imgze.dat`,
  45 `…-imgbusa.dat`, 46/47 `…-hash.dat`, 48–59 `…-wsq{bio,man,mes}.jez`, 60 `…-log.jez`, 61 `…-logsa.jez`,
  63–82 os mesmos nomes de resultado para outras aplicações, 83/98 `rdv.dat`, 84/85 `buj.dat`, 86/87/100 `bu.dat`,
  88/99 `ze.dat`, 89 `rze.dat`, 90–97 `bur/bujr/bimr/behbr.dat`, 101 `saraiz.bin`, 102 `sasecao.bin`, 103 `busa.dig`,
  104/105 as expressões regulares `uesieco-…-cert.dat` / `-aut.dat`, 106/107 `bim.dat`, 108/109 `behb.dat`,
  110/111 `uenux.db`, 112 `{:05}{:04}{:04}-lo.dat`, 113 `dadoscarga.dat`, 115 `red.bin`.
  (`…` = `{:c}{:05}{}{:05}{:04}{:04}` = fase, pleito, UF, município, zona, seção.)
* `m_aplicacoes` (20 entradas, ids 1–22): `vota.of`, `vota.si`, `vota.te`, `vota.tm`, `sa.of`, `sa.si`, `sa.tr`,
  `gap`, `red`, `vpp`, `adh`, `atue`, `ste`, `turno2.jez`, `savd`, `partido`, `partido.app`, `partido.pub`,
  `partido.id`, `infomidia.pwd`. **Nenhuma função deste build lê este mapa.**

A tabela completa e ordenada está em `src/uenux2/src/app/comum/carquivossavd.u22.cpp`.

### 6.3 Verificação de assinaturas na inicialização (func 4625)

`CPacoteArquivos::ValidaAssinaturasArquivosTrabalho(dir)` *(nome inferido)* é chamada duas vezes pela rotina de
inicialização do eleitor (func 7787) com `GetPathTrab(INTERNA)` e `GetPathTrab(EXTERNA)`. Para cada par
`{bim.dat:bim.vsu, bu.dat:bu.vsu, buj.dat:buj.vsu, rdv.dat:rdv.vsu, rze.dat:rze.vsu, vota.bin:vota.vsu, ze.dat:ze.vsu}`
cujo arquivo de dados existe (tipo de `fs::status` nem none nem not_found), ela monta
`CPacoteArquivos(dir/<name>.vsu, "<data file>", ESavdChaveValidar::UE ('5'), aplicação 1)`
(construtor: `ValidarChaveEAplicacaoValida` — chave '1'..';' (7219), aplicação 1..9 (7220); tipo
0x1001 "arquivos listados") e chama `Validar(CInterfaceMensagemVazia)`: para cada arquivo listado,
`IInterfaceSavd::ValidaAssinaturaArquivo(aplicação, chave, pacote, arquivo)` (func 5892); em caso de falha,
`TrataErroPacoteArquivo` (7223, ou `TrataErroPacote` 7222 para os erros 1–3 do SAVD). Nomes das chaves
(`RetornaNomeChave`, func 5900): '1' TSE, '2' SECAD, '3' SECINP, '4' SEVIN, '5' UE, '6' SCUE, '7' CLOGI,
'8' CLOGI_CERT, '9' CLOGI_UPDATE, ':' PU, ';' SEINT (7221 caso contrário).

No simulador isto rodou (observado) e sempre tem sucesso: o SAVD é `(anonymous)::CWasmSavd`, cujo
`RecebeMensagem` (func 10949) ignora a requisição e retorna uma resposta fixa de 12 bytes, e os arquivos `.vsu` contêm
"assinatura simulada para vota_web_wasm".

---

## 7. Parte E — `CInfoMTLCD`

Um singleton de 64 bytes (@1838572) que observa `BatteryIconDataSource<Vertical>` (timer de 1 s). `Update` (11653,
observado) centraliza o ícone de bateria atual no LCD gráfico do MT (`IScreenMT` slot 4 com flag = false; o
tamanho do ícone vem do renderizador da tela do eleitor, `IScreen` slot 34 → slot 13). `MostrarImagemNoLCD` (3836)
desenha uma imagem arbitrária (a foto do eleitor, `CBiometriaEleitor::GetFoto`); seu parâmetro `bool` foi
propagado como constante (os chamadores passam `true`). A flag também controla o restante da função: com `true`, ela primeiro
desanexa o observador de bateria e retorna se a imagem estiver vazia; a cópia inlinada em `Update` (flag `false`)
contém apenas a chamada ao slot 4 de `IScreenMT` (caso contrário, todo tick de bateria desanexaria o observador). O destrutor (5902) se desanexa;
as funcs 3837/5510/6049/10990/11652 são as duas instanciações do destrutor de `BatteryIconDataSource` (um corpo mesclado,
6049, que recebe a vtable como parâmetro).

---

## 8. BOLETIM DE URNA: o que esta unidade contribui

Esta unidade não gera, não assina e não imprime o BU (isso é u08/u09: `CGeraBU`, `CGravaResultado`,
`CImprimindoBU`). Ela fornece cinco das entradas do BU:

1. **Os votos contados pelo BU.** Toda cédula confirmada entra no RDV em memória por meio de `GravaVotos` (§4):
   uma `CCedula` por eleição, validada (`ConfereCedula`: tamanho = Σ qtdEscolhas, um voto por vaga, votos nominais
   com a quantidade de dígitos do cargo, nulos por candidato repetido apenas em cargos com várias vagas e apenas ao lado do voto
   nominal que eles repetem) e inserida ordenada por (tipo, digitado). Os totais por cargo do BU (`CVotosEleicoesVota`
   `Nominais/Legendas/Nulos/Brancos/Candidato/Partido`, u05) e o **comparecimento** da seção
   (`Comparecimento(eleição)` = cédulas da eleição, func 5639) são calculados a partir dessa estrutura. O
   contador do estado geral incrementado por `MarcaUltimoVoto` (+52) e a hora do último voto (+56) vêm da
   mesma função.
2. **Os nomes de arquivo e os pacotes de assinatura** dos arquivos de resultado, de `CArquivosSavd` (§6.2). Para o BU:
   `ESavdArquivoUE` 35 = `"{:c}{:05}{}{:05}{:04}{:04}-bu.dat"` (formatado com o caractere da fase, pleito, UF, município
   (5), zona (4), seção (4), p. ex. `t02410ac0000100010001-bu.dat`), 43 `…-imgbu.dat` (envelope da imagem do BU
   impresso), 84/85 `buj.dat`, 86/87/100 `bu.dat` (as cópias de trabalho), 90/94 `bur.dat`; os pacotes 126–129
   `dinamico/trab{1,2}/bu.vsu` (MI/MV) assinam a cópia de trabalho, e 158/159 (MI) e 160/161 (MV)
   `dinamico/res{1,2}/<prefix>-vota.vsc` é o pacote que `CGravaResultado` passa a `CAssinadorVota` (158 no
   1º turno, 159 no 2º; u09 §5.2) para assinar todos os arquivos de resultado.
3. **Para onde vão:** `CPath::GetPathResult(flash, turno)` = `/dsk/fi|fe/dinamico/res1|2/` (func 1399) e
   `GetPathTrab` = `/dsk/fi|fe/dinamico/trab1|2/` (func 358).
4. **Integridade no reinício:** antes de qualquer outra coisa, `votaInit`/a inicialização verifica `bu.dat` (e `vota.bin`,
   `rdv.dat`, `ze.dat`, `rze.dat`, `buj.dat`, `bim.dat`) das duas áreas de trabalho contra seus pacotes `.vsu` com a
   chave UE da urna (§6.3); uma falha aborta a inicialização com CUeComumError 7222/7223. No simulador, o SAVD é
   mockado e a verificação não pode falhar.
5. **O fluxo de encerramento e o BIM.** Quando o registro final de mesários termina
   (`CEncerraRegistroMesarios`, período FINAL), a thread do operador envia a **mensagem 7** à thread do eleitor
   (slot 15 do IControlador), o que inicia a cadeia do BU (`CAguardaMensagem::vf6(7)` → `CGeraBU` → `CGeraRelatorios`
   → `CImprimindoBU` → `CGravaResultado`, u09). `CGeraRelatorios` imprime o BIM com os mesários registrados em
   `comparecimento_mesario`, cada nome obtido por `CImprimirIdentificacaoMesariosFinal::GetNomeMesario(título)`
   (func 5385: busca no cadastro pelo título, UEASSERT 3409 se ausente, nome social se presente, senão nome, cortado em 40
   caracteres). Os nomes de cargo impressos no BU vêm de `CCargo::GetNome*` (§5).

---

## 9. Particularidades do build web

* A parte A é código morto no simulador (a thread do operador não roda, não há `IControladorRegistraMesarios`, não há hardware
  de digitais). A extração de templates e a codificação WSQ são stubs vazios, então a comparação biométrica compara dados vazios.
* `GravaVotos` roda para todo eleitor; a cédula vai para o RDV em memória, mas `CSincronismoVotoEleitorWeb`
  (u07) nunca grava `rdv.dat`, `vota.bin` nem suas assinaturas.
* `CArquivosSavd` é construído durante `votaInit` (observado) e usado pelo código de persistência da inicialização; o SAVD
  por trás de `CPacoteArquivos` é `CWasmSavd` (respostas prontas), e todo `.vsu` é o texto "assinatura simulada
  para vota_web_wasm".
* `CInfoMTLCD::Update` roda no timer de bateria (observado) e calcula a posição centralizada, mas não desenha
  nada: o slot 4 de `simulador::CWasmScreenMT` (a chamada de imagem) é um no-op do ICF (func 1870). De qualquer forma, `IPower` nunca
  informa uma mudança, então o ícone seria sempre o mesmo.
* Nada nesta unidade chama um import `js_*`, lê parâmetros de URL ou acessa a rede.

## 10. Observações sobre wasm / Emscripten

* **Nomes pelo primeiro srcloc.** As ferramentas nomeiam uma função a partir do primeiro `std::source_location` encontrado nela, então
  muitas entradas foram nomeadas errado: 4454 ("CVotosCargos::ConfereCedula" → `CEleitorVotando::GravaVotos`), 4625
  ("ValidarChaveEAplicacaoValida" → a verificação de diretório que a inlina), 10313 ("(anonymous)::GetControlador" →
  `IPedeTituloMesario::ProcessInput`), 10337 (→ `IGestorDadoMesario::StartState`), 5382, 5386, 1547/2796/2797/3715/3716
  (todas "GetDetalheConsulta/Candidato" → os getters de nome que inlinam esses acessores), 5640 ("MontaMapa" → o
  construtor de `CVotosEleicoesVota`). Overrides são retornados para todas elas.
* **Construtores inteiros inlinados em `GetInst` lazy.** A maioria dos construtores de estado só existe dentro do acessor
  que cria o singleton, e vários acessores são, eles próprios, inlinados no estado que transita para
  eles (10313 contém quatro construtores; 10388, três). Restam apenas os stubs de `mutex_unlock` dos `lock_guard`s
  (build de thread única).
* **Corpos mesclados pelo wasm-opt com a constante variável como parâmetro:** 6011 (`ProcessInput` de
  CRegistrarMesarios / CMesarioRegistrado, srclocs como parâmetros), 2895 (`ProcessInput` que volta para
  CPedeTituloMesario em uma tecla dada: 5 ou 9), 6010, 6008, 6009, 6012, 6029 (optional-ou-throw com offset,
  mensagem e código), 6049 (destrutor com a vtable), 3894 (`Clone` do DAO com a vtable), 170/3611 (fábricas
  de erro).
* **Eliminação de argumentos mortos / propagação de constantes:** `ComparaDigitais` perdeu os dois parâmetros vector, e
  `MostrarImagemNoLCD`, o seu `bool` (sempre `true` a partir dos chamadores fora de linha; a cópia inlinada em `Update` passa
  `false`, o que também remove o `Detach` do observador e o teste de imagem vazia que a flag protege).
* **`__datasizeof` da libc++ 21:** copiar `std::vector<CSituacoesEleicoes>` (elementos de 8 bytes com 7 bytes de dados)
  faz memmove de `n*8 - 1` bytes (func 5652).
* O construtor de `CArquivosSavd` é a segunda maior função de aplicação do módulo (77 KB, 35,493
  instruções), embora apenas preencha três mapas: cada uma das ~190 inserções expande uma concatenação de
  `filesystem::path`, um `find`/insert de map e três destrutores de string.
* As strings estão em **Latin-1** no segmento de dados ("{}ª vaga" tem 8 bytes, "REGISTRO DE MESÁRIOS", 20).

## 11. Código suspeito / notável

1. **`emscripten_sleep(1000)` em `CPedeDigitalMesario::ProcessTick` (func 10316).** Depois que um dedo é detectado,
   o código dorme 1 s quando o byte @1584624 é 1 (ele é, e nada o grava). O build não tem Asyncify, então
   o glue aborta. Inalcançável no simulador público (a thread do operador não roda; as interfaces de digitais
   lançariam exceção antes). Mesmo padrão de `ObtemEstadoPosReconhecimentoBiometrico` da u10.
2. **`id_arquivo` não é reiniciado entre mesários (funcs 10316/5382/5371/10337).** `CControladorReconhecimentoMesario`
   (@1909960) é um singleton de todo o processo. No caminho biométrico, `m_estado` (2 ou 3) e `m_dedo` (o dedo
   que coincidiu, ou 0: `SalvaDigitalMesario` sempre o limpa, mesmo quando o armazenamento falha) são regravados para todo mesário,
   mas `m_idArquivo` só é *definido* (func 5371, quando uma imagem é gravada) e nenhum código deste build o limpa.
   Assim, um mesário reconhecido pela digital (sem imagem armazenada), ou um cuja imagem não pôde ser armazenada
   (`Armazena` → 999999), depois de um mesário cuja imagem foi armazenada, é salvo com o
   `id_arquivo` do mesário anterior. (Qualidade de dados na urna real, se o mesmo código for embarcado; nenhum efeito no simulador.)
3. **A gravação da cédula não é atômica entre eleições (func 4454).** `InsereCedula` troca o mapa por uma cópia, então uma
   eleição é tudo ou nada, mas em uma eleição geral a cédula federal é inserida antes de a estadual ser
   conferida; se o segundo `RecebeCedula` lançar exceção, a primeira fica no RDV e `MarcaUltimoVoto` é pulado. Isso
   requer uma cédula inconsistente (um erro de programação), e o contexto do eleitor então informa "O voto do eleitor NÃO
   foi registrado".
4. **Estados despachantes podem congelar o MT.** `CPedeTituloMesario::StartState` (10388) e
   `CGestorDadoMesario::StartState` (10327) deixam `m_proximoEstado = this` quando `GetPeriodoRegistro()` retorna 0
   (seu `br_table` só cobre 1..3); esses estados têm flags 0 (sem teclas, ticks ou mensagens) e não desenham nada,
   então a thread do operador esperaria para sempre. `CEncerraRegistroMesarios::StartState` (10380) *não* é afetado por 0
   (ele trata 0 como 1 e volta ao slot 9 do VOTA); só ficaria parado com um valor > 3, que a tabela do VOTA
   não pode retornar. O VOTA só entra no fluxo nos estados '6'..'9', que `AtualizaEstadoRegistro` mapeia para os períodos
   1..3, então isso não é alcançável na prática.
5. **Peculiaridades da tabela do SAVD (func 1164).** Os pacotes 162–165 (`…-sa.vsc`) não têm diretório, enquanto todos os irmãos
   têm, e 172/173 e 178/179 duplicam 170/171 e 176/177 (flash interna duas vezes, onde o padrão sugere que
   um deles deveria ser a flash externa). Apenas a aplicação SA os usaria.
6. **Segurança simulada no build web.** A verificação de assinaturas dos arquivos de trabalho na inicialização (func 4625,
   observada) não pode falhar: `CWasmSavd::RecebeMensagem` retorna uma resposta fixa e os arquivos `.vsu` são texto
   de placeholder. A comparação de digitais compara templates vazios (extrator stub). Nada aqui deve ser lido como
   evidência sobre as verificações da urna real.
7. **Pequenas lacunas de validação.** `CConversorPartido` aceita sigla/nome vazios (apenas os limites superiores são verificados);
   `CPleito` compara apenas as *contagens* de versões de pacote e de situações com o número de eleições (uma entrada
   para um id de eleição errado não é detectada); `CPath::GetPathTrab/GetPathResult` indexam o array de raízes de 2 elementos com
   um `EFlashOrigem` não verificado (apenas constantes são passadas).
8. **Efeito colateral no log de auditoria em construtores.** "Registrando mesários antes/durante/após a votação" é registrado no log pelos
   construtores dos três singletons `CPedeTituloMesario*` (10388), ou seja, apenas na primeira vez que cada tipo de
   registro acontece em uma execução do processo.
9. **Trabalho desperdiçado.** `CVotosCargos::QtdVotosPorCedula` (5642) percorre o mapa de cargos por valor, então cada chamada
   copia (e libera) o vetor completo de votos de cada cargo, ou seja, todos os votos registrados até então na seção. Ela é
   chamada uma vez por cédula no caminho normal (duas quando 4672 é lançado), então o custo de gravar um voto
   cresce linearmente com o número de eleitores (quadrático ao longo do dia; desprezível com algumas centenas de eleitores, mas
   inútil); `SalvaDigitalMesario` (5382) monta um nome descritivo
   `<título>_Operador` e nunca o usa (as imagens recebem nomes aleatórios `me<nnnnnn>`, como na u10 §6.3).

## 12. Questões em aberto

* Nomes reais dos slots de `IControladorRegistraMesarios` (sem RTTI/srcloc; nomes inferidos da implementação do VOTA)
  e do slot 8 (sempre `true`, nunca chamado pelo código de comum).
* Nome real da func 4625 (o laço externo sobre os sete arquivos de trabalho): não está no binário; um membro estático de
  `CPacoteArquivos` ou uma função livre do arquivo de inicialização são ambos possíveis.
* O significado de `m_aplicacoes` (map +24 de `CArquivosSavd`, ids 1–22): não existe acessor neste build.
* A semântica exata de `EstadoReconhecimentoBiometrico` 1/2/3 versus `EstadoColetaDigital` do ASN.1 (1..4) — a
  conversão para o arquivo de resultado ASN.1 (`CConversorComparecimentoMesario`, ecourna 9103/9108) pertence à u14.
* Por que o construtor de `CLimiteMesariosRegistradosAtingido` busca o controlador (:53), mas imprime uma constante 6
  (getter desvirtualizado, ou uma chamada remanescente).

## 13. Tabela de mapeamento completa (107 funções)

"✓" = observada em execução nos votos registrados. "nome nas ferramentas" é o nome em `analysis/functions.tsv`.

| idx | tamanho | executou | nome nas ferramentas | símbolo reconstruído | reconstrução (src/…) | arquivo original | conf. |
|---|---|---|---|---|---|---|---|
| 170 | 21 |  | `comum_f170` | `comum::CDadosError factory thunk (CBaseError<EUeComumDadosError,{7800,8600}> ctor via func 2294)` | helper de biblioteca/inlinado (thunk mesclado de construtor de erro, vtable @1528096 passada para 2294) | — | alta |
| 296 | 123 | ✓ | `ecourna_f296` | `std::to_string(int)` | helper de biblioteca (libc++ std::to_string(int): __to_chars_itoa + string(first,last)) | — | alta |
| 358 | 901 | ✓ | `comum::CPath::GetPathTrab` | `comum::CPath::GetPathTrab` | src/uenux2/src/app/comum/cpath.cpp | uenux2/src/app/comum/cpath.cpp | alta |
| 567 | 50 |  | `ecourna_f567` | `std::operator+(const char*, std::string&&)` | helper de biblioteca (libc++ operator+(const char*, string&&) = rhs.insert(0, lhs)) | — | alta |
| 634 | 1142 | ✓ | `comum::CPath::GetPathRootSemSA` | `comum::CPath::GetPathRootSemSA` | src/uenux2/src/app/comum/cpath.cpp | uenux2/src/app/comum/cpath.cpp | alta |
| 941 | 385 |  | `vota_f941` | `api::CFormBuilderMT::Add<api::CBuzzFieldMT>` | helper de biblioteca/inlinado (instanciação de template do builder de formulários do MT: push_back(shared_ptr<IFormField>(new CBuzzFieldMT(a,b))) | uenux2/src/api/gui/cformbuildermt.h | média |
| 1164 | 77002 | ✓ | `comum_f1164` | `comum::CArquivosSavd::GetInst` | src/uenux2/src/app/comum/carquivossavd.u22.cpp | uenux2/src/app/comum/carquivossavd.cpp | alta |
| 1255 | 612 |  | `comum_f1255` | `comum::CriaFormEleitorRegistroMesarios` | src/uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | média |
| 1267 | 75 |  | `comum::md::CProcessoEleitoral::GetPleito2` | `comum::md::CProcessoEleitoral::GetPleito2` | src/uenux2/src/app/comum/dados/md/processoeleitoral/cprocessoeleitoral.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/cprocessoeleitoral.cpp | alta |
| 1348 | 249 |  | `comum_f1348` | `std::pair<std::string,std::string>::pair(const char*, const char*)` | helper de biblioteca (pair de dois ctors string(const char*)), emitido com cpacotearquivos.cpp | — | alta |
| 1378 | 116 |  | `comum::IPedeTituloMesario::vf0` | `comum::IPedeTituloMesario::~IPedeTituloMesario` | src/uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | alta |
| 1388 | 73 |  | `comum::md::CCargo::GetDetalheCandidato@1388` | `comum::md::CCargo::GetDetalheCandidato` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | alta |
| 1399 | 745 |  | `comum::CPath::GetPathResult` | `comum::CPath::GetPathResult` | src/uenux2/src/app/comum/cpath.cpp | uenux2/src/app/comum/cpath.cpp | alta |
| 1545 | 99 |  | `comum::md::CDetalheCandidato::GetSuplente` | `comum::md::CDetalheCandidato::GetSuplente` | src/uenux2/src/app/comum/dados/md/processoeleitoral/cdetalhecandidato.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalhecandidato.cpp | alta |
| 1547 | 184 | ✓ | `comum::md::CCargo::GetDetalheConsulta@1547` | `comum::md::CCargo::GetNome` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | média |
| 1651 | 160 | ✓ | `comum_f1651` | `std::filesystem::path::__relative_path` | helper de biblioteca (libc++ path::__relative_path: PathParser + ConsumeRootDir -> string_view) | — | alta |
| 1923 | 22 |  | `comum::md::CCargo::GetDetalheConsulta@1923` | `comum::md::CCargo::GetDetalheConsulta` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | alta |
| 2256 | 1398 | ✓ | `comum::md::CVoto::CVoto` | `comum::md::CVoto::CVoto` | src/uenux2/src/app/comum/dados/md/rdv/cvoto.cpp | uenux2/src/app/comum/dados/md/rdv/cvoto.cpp | alta |
| 2257 | 78 | ✓ | `comum::md::CEleicaoPE::GetCargos` | `comum::md::CEleicaoPE::GetCargos` | src/uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp | alta |
| 2258 | 906 | ✓ | `comum::md::CCargo::GetOrdinalEscolha` | `comum::md::CCargo::GetOrdinalEscolha` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | alta |
| 2730 | 68 |  | `comum_f2730` | `comum::md::CComparecimentoMesario::CComparecimentoMesario (with idArquivo)` | src/uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.h | uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.cpp | média |
| 2731 | 68 |  | `comum_f2731` | `comum::md::CComparecimentoMesario::CComparecimentoMesario (idArquivo NULL)` | src/uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.h | uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.cpp | média |
| 2738 | 76 |  | `comum_f2738` | `comum::md::CComparecimentoMesarioPK::CComparecimentoMesarioPK` | src/uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.h | uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.cpp | média |
| 2796 | 221 |  | `comum::md::CCargo::GetDetalheConsulta@2796` | `comum::md::CCargo::GetNome(CSexo::ESexo)` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | média |
| 2797 | 184 |  | `comum::md::CCargo::GetDetalheConsulta@2797` | `comum::md::CCargo::GetNomeAbreviado` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | média |
| 3608 | 61 |  | `comum_f3608` | `comum::IPedeTituloMesario::IPedeTituloMesario` | src/uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | alta |
| 3611 | 18 |  | `comum_f3611` | `comum::CComparecimentoMesarioError factory thunk (CBaseError<EUeComumComparecimentoMesarioError,{7750,7800}>)` | helper de biblioteca/inlinado (thunk mesclado de construtor de erro: ecourna_f710 + vtable @1593524) | — | alta |
| 3708 | 173 | ✓ | `comum_f3708` | `comum::md::(anonymous namespace)::BuscaEleicao` | src/uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | média |
| 3709 | 32 |  | `comum_f3709` | `std::__tree<std::__value_type<TCargoID,int>>::destroy` | helper de biblioteca (destrutor de nó de std::map do contador de votos de ConfereCedula) | — | alta |
| 3714 | 680 |  | `comum::md::CEleicaoPE::ValidaOrdem` | `comum::md::CEleicaoPE::ValidaOrdem` | src/uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp | alta |
| 3715 | 574 |  | `comum::md::CCargo::GetDetalheCandidato@3715` | `comum::md::CCargo::GetNomeSuplente` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | média |
| 3716 | 184 |  | `comum::md::CCargo::GetDetalheConsulta@3716` | `comum::md::CCargo::GetNomeMasculino` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | média |
| 3750 | 135 |  | `comum_f3750` | `std::__tree<std::__value_type<TEleicaoID,CCedula>>::destroy` | helper de biblioteca (destrutor de nó de std::map<TEleicaoID, CCedula> usado por GravaVotos) | — | alta |
| 3836 | 292 |  | `comum::CInfoMTLCD::MostrarImagemNoLCD` | `comum::CInfoMTLCD::MostrarImagemNoLCD` | src/uenux2/src/app/comum/cinfomtlcd.cpp | uenux2/src/app/comum/cinfomtlcd.cpp | alta |
| 3837 | 12 |  | `api::BatteryIconDataSource<(api::CPowerInformation::IconOrientation)1>::vf0` | `api::BatteryIconDataSource<Vertical>::~BatteryIconDataSource` | helper de biblioteca/inlinado (dtor de template, ver nota em src/uenux2/src/app/comum/cinfomtlcd.cpp) | uenux2/src/api/gui/cpowerinformation.cpp (template definido ali, srcloc :119/:131) | alta |
| 3839 | 57 |  | `comum_f3839` | `std::__tree<ESavdPacote,string>::destroy (CArquivosSavd::m_pacotes)` | helper de biblioteca (destrutor de nó de map, emitido com carquivossavd.cpp) | — | alta |
| 3840 | 57 |  | `comum_f3840` | `std::__tree<ESavdArquivoUE,string>::destroy (CArquivosSavd::m_arquivos)` | helper de biblioteca (destrutor de nó de map, emitido com carquivossavd.cpp) | — | alta |
| 3841 | 57 |  | `comum_f3841` | `std::__tree<ESavdAplicacao,string>::destroy (CArquivosSavd::m_aplicacoes)` | helper de biblioteca (destrutor de nó de map, emitido com carquivossavd.cpp) | — | alta |
| 3881 | 32 |  | `comum_f3881` | `std::unique_ptr<comum::IPedeTituloMesario>::reset` | helper de biblioteca (reset de unique_ptr com o destrutor desvirtualizado 1378; usado pelos stubs de atexit 10366/10372/10379) | — | alta |
| 4454 | 5617 | ✓ | `comum::md::CVotosCargos::ConfereCedula` | `vota::CEleitorVotando::GravaVotos` | src/uenux2/src/app/vota/eleitor/celeitorvotando.u22.cpp | uenux2/src/app/vota/eleitor/celeitorvotando.cpp | média |
| 4625 | 3584 | ✓ | `comum::CPacoteArquivos::ValidarChaveEAplicacaoValida` | `comum::CPacoteArquivos::ValidaAssinaturasArquivosTrabalho` | src/uenux2/src/app/comum/cpacotearquivos.cpp | uenux2/src/app/comum/cpacotearquivos.cpp | média |
| 5372 | 720 |  | `comum::CControladorReconhecimentoMesario::ComparaDigitais` | `comum::CControladorReconhecimentoMesario::ComparaDigitais` | src/uenux2/src/app/comum/comparecimentomesario/ccontroladorreconhecimetomesario.cpp | uenux2/src/app/comum/comparecimentomesario/ccontroladorreconhecimetomesario.cpp | alta |
| 5382 | 994 |  | `comum::CPedeDigitalMesario::GetControlador` | `comum::CPedeDigitalMesario::SalvaDigitalMesario` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | baixa |
| 5385 | 371 |  | `comum::CImprimirIdentificacaoMesariosFinal::GetNomeMesario` | `comum::CImprimirIdentificacaoMesariosFinal::GetNomeMesario` | src/uenux2/src/app/comum/comparecimentomesario/estados/cimprimiridentificacaomesariosfinal.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cimprimiridentificacaomesariosfinal.cpp | alta |
| 5386 | 1465 |  | `comum::CLimiteMesariosRegistradosAtingido::GetControlador@5386` | `comum::CLimiteMesariosRegistradosAtingido::GetInst` | src/uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | alta |
| 5389 | 983 |  | `comum_f5389` | `comum::CConfirmaFimRegistroMesarios::GetInst` | src/uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | alta |
| 5390 | 472 |  | `comum::dao::CComparecimentoMesarioDAO::ConverteDedoHabilitacao` | `comum::dao::CComparecimentoMesarioDAO::ConverteDedoHabilitacao` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | alta |
| 5391 | 475 |  | `comum::dao::CComparecimentoMesarioDAO::ConverteEstadoBiometria` | `comum::dao::CComparecimentoMesarioDAO::ConverteEstadoBiometria` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | alta |
| 5510 | 12 |  | `api::BatteryIconDataSource<(api::CPowerInformation::IconOrientation)0>::vf0` | `api::BatteryIconDataSource<Horizontal>::~BatteryIconDataSource` | helper de biblioteca/inlinado (dtor de template, ver nota em src/uenux2/src/app/comum/cinfomtlcd.cpp) | uenux2/src/api/gui/cpowerinformation.cpp (template definido ali, srcloc :119/:131) | alta |
| 5639 | 45 |  | `comum::md::CVotosEleicoesVota::Comparecimento` | `comum::md::CVotosEleicoesVota::Comparecimento` | src/uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | alta |
| 5640 | 1152 | ✓ | `comum::md::(anonymous namespace)::MontaMapa` | `comum::md::CVotosEleicoesVota::CVotosEleicoesVota` | src/uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | média |
| 5642 | 291 |  | `comum_f5642` | `comum::md::CVotosCargos::QtdVotosPorCedula` | src/uenux2/src/app/comum/dados/md/rdv/cvotoscargos.cpp | uenux2/src/app/comum/dados/md/rdv/cvotoscargos.cpp | baixa |
| 5643 | 665 |  | `comum::md::CVotosCargos::CVotosCargos` | `comum::md::CVotosCargos::CVotosCargos` | src/uenux2/src/app/comum/dados/md/rdv/cvotoscargos.cpp | uenux2/src/app/comum/dados/md/rdv/cvotoscargos.cpp | alta |
| 5648 | 517 |  | `comum::md::CPleito::GetSituacoesEleicoes` | `comum::md::CPleito::GetSituacoesEleicoes` | src/uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | alta |
| 5651 | 517 |  | `comum::md::CPleito::GetEleicao` | `comum::md::CPleito::GetEleicao` | src/uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | alta |
| 5652 | 844 | ✓ | `comum::md::CPleito::CPleito` | `comum::md::CPleito::CPleito` | src/uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | alta |
| 5657 | 403 |  | `comum::md::CCargo::Valida` | `comum::md::CCargo::Valida` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | alta |
| 5900 | 948 |  | `comum::CPacoteArquivos::RetornaNomeChave` | `comum::CPacoteArquivos::RetornaNomeChave` | src/uenux2/src/app/comum/cpacotearquivos.cpp | uenux2/src/app/comum/cpacotearquivos.cpp | alta |
| 5901 | 594 |  | `comum::CPacoteArquivos::TrataErroPacote` | `comum::CPacoteArquivos::TrataErroPacote` | src/uenux2/src/app/comum/cpacotearquivos.cpp | uenux2/src/app/comum/cpacotearquivos.cpp | alta |
| 5902 | 188 |  | `comum::CInfoMTLCD::vf0` | `comum::CInfoMTLCD::~CInfoMTLCD` | src/uenux2/src/app/comum/cinfomtlcd.cpp | uenux2/src/app/comum/cinfomtlcd.cpp | alta |
| 5907 | 38 |  | `comum_f5907` | `comum::CArquivosSavd::~CArquivosSavd` | src/uenux2/src/app/comum/carquivossavd.u22.cpp | uenux2/src/app/comum/carquivossavd.cpp | alta |
| 6029 | 68 |  | `comum_f6029` | `merged body: optional<T> engaged check or throw CDadosError (CCargo::GetDetalheConsulta / CLocal::GetContingencia)` | helper de biblioteca/inlinado (corpo do merge-similar-functions do wasm-opt: (this, offset, srcloc, msg, code)) | — | alta |
| 6049 | 159 |  | `api_f6049` | `api::BatteryIconDataSource<O>::~BatteryIconDataSource (shared body)` | helper de biblioteca/inlinado (corpo de destrutor mesclado das duas instanciações de BatteryIconDataSource, vtable como parâmetro) | uenux2/src/api/gui/cpowerinformation.cpp (template definido ali, srcloc :119/:131) | alta |
| 10313 | 6002 |  | `comum::(anonymous namespace)::GetControlador` | `comum::IPedeTituloMesario::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | alta |
| 10314 | 47 |  | `comum::IPedeTituloMesario::vf2` | `comum::IPedeTituloMesario::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | alta |
| 10315 | 17 |  | `comum::CPedeDigitalMesario::FinishState` | `comum::CPedeDigitalMesario::FinishState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | alta |
| 10316 | 3808 |  | `comum::CPedeDigitalMesario::ProcessTick` | `comum::CPedeDigitalMesario::ProcessTick` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | alta |
| 10317 | 130 |  | `comum::CPedeDigitalMesario::vf7` | `comum::CPedeDigitalMesario::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | alta |
| 10318 | 257 |  | `comum::CPedeDigitalMesario::StartState` | `comum::CPedeDigitalMesario::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | alta |
| 10319 | 14 |  | `comum::(anonymous namespace)::CNomeMesariosUrnaDS::Text@10319` | `comum::(anonymous namespace)::CNomeMesariosUrnaDS::Text` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | alta |
| 10322 | 14 |  | `comum::CDigitalMesarioNaoReconhecida::vf7` | `comum::CDigitalMesarioNaoReconhecida::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/cdigitalmesarionaoreconhecida.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cdigitalmesarionaoreconhecida.cpp | alta |
| 10323 | 14 |  | `comum::(anonymous namespace)::CNomeMesariosUrnaDS::Text@10323` | `comum::(anonymous namespace)::CNomeMesariosUrnaDS::Text` | src/uenux2/src/app/comum/comparecimentomesario/estados/cdigitalmesarionaoreconhecida.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cdigitalmesarionaoreconhecida.cpp | alta |
| 10327 | 403 |  | `comum::CGestorDadoMesario::StartState` | `comum::CGestorDadoMesario::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp | alta |
| 10337 | 1733 |  | `comum::IGestorDadoMesario::GetControlador` | `comum::IGestorDadoMesario::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/igestordadomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/igestordadomesario.cpp | alta |
| 10338 | 17 |  | `comum::CMesarioRegistrado::GetControlador@10338` | `comum::CMesarioRegistrado::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp | alta |
| 10339 | 254 |  | `comum::CMesarioRegistrado::GetControlador@10339` | `comum::CMesarioRegistrado::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp | alta |
| 10342 | 14 |  | `comum::CTituloMesarioJaRegistrado::vf7` | `comum::CTituloMesarioJaRegistrado::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | alta |
| 10343 | 149 |  | `comum::CTituloMesarioJaRegistrado::StartState` | `comum::CTituloMesarioJaRegistrado::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | alta |
| 10344 | 14 |  | `comum::(anonymous namespace)::CTituloMesarioJaRegistradoDS::Text` | `comum::(anonymous namespace)::CTituloMesarioJaRegistradoDS::Text` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | alta |
| 10347 | 14 |  | `comum::CTituloMesarioInvalido::vf7` | `comum::CTituloMesarioInvalido::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | alta |
| 10348 | 12 |  | `comum::CTituloMesarioInvalido::GetControlador` | `comum::CTituloMesarioInvalido::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | alta |
| 10349 | 14 |  | `comum::(anonymous namespace)::CTituloMesarioInvalidoDS::Text` | `comum::(anonymous namespace)::CTituloMesarioInvalidoDS::Text` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | alta |
| 10352 | 14 |  | `comum::CTituloMesarioVazio::vf7` | `comum::CTituloMesarioVazio::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | alta |
| 10353 | 12 |  | `comum::CTituloMesarioVazio::GetControlador` | `comum::CTituloMesarioVazio::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | alta |
| 10357 | 130 |  | `comum::CLimiteMesariosRegistradosAtingido::vf7` | `comum::CLimiteMesariosRegistradosAtingido::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | alta |
| 10358 | 105 |  | `comum::CLimiteMesariosRegistradosAtingido::GetControlador@10358` | `comum::CLimiteMesariosRegistradosAtingido::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | alta |
| 10366 | 12 |  | `comum_f10366` | `atexit: CPedeTituloMesarioVotacao::GetInst()::s_inst.reset()` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | alta |
| 10372 | 12 |  | `comum_f10372` | `atexit: CPedeTituloMesarioFinal::GetInst()::s_inst.reset()` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | alta |
| 10379 | 12 |  | `comum_f10379` | `atexit: CPedeTituloMesarioInicial::GetInst()::s_inst.reset()` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | alta |
| 10380 | 445 |  | `comum::CEncerraRegistroMesarios::GetControlador` | `comum::CEncerraRegistroMesarios::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cencerraregistromesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cencerraregistromesarios.cpp | alta |
| 10383 | 194 |  | `comum::CConfirmaFimRegistroMesarios::GetControlador@10383` | `comum::CConfirmaFimRegistroMesarios::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | alta |
| 10385 | 106 |  | `comum::CConfirmaFimRegistroMesarios::GetControlador@10385` | `comum::CConfirmaFimRegistroMesarios::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | alta |
| 10388 | 4789 |  | `comum::CPedeTituloMesario::StartState` | `comum::CPedeTituloMesario::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | alta |
| 10391 | 17 |  | `comum::CRegistrarMesarios::GetControlador@10391` | `comum::CRegistrarMesarios::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | alta |
| 10392 | 209 |  | `comum::CRegistrarMesarios::GetControlador@10392` | `comum::CRegistrarMesarios::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | alta |
| 10399 | 12 |  | `comum::dao::CComparecimentoMesarioDAO::vf2` | `comum::dao::CComparecimentoMesarioDAO::Clone` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | média |
| 10401 | 3016 |  | `comum::dao::CComparecimentoMesarioDAO::vf8` | `comum::dao::CComparecimentoMesarioDAO::RecuperarTodos` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | média |
| 10402 | 2308 |  | `comum::dao::CComparecimentoMesarioDAO::vf7` | `comum::dao::CComparecimentoMesarioDAO::Recuperar` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | média |
| 10403 | 558 |  | `comum::dao::CComparecimentoMesarioDAO::vf3` | `comum::dao::CComparecimentoMesarioDAO::Inserir` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | média |
| 10990 | 10 |  | `api::BatteryIconDataSource<(api::CPowerInformation::IconOrientation)0>::vf1` | `api::BatteryIconDataSource<Horizontal>::~BatteryIconDataSource (deleting)` | helper de biblioteca/inlinado (destrutor de deleção: 5510 + free) | uenux2/src/api/gui/cpowerinformation.cpp (template definido ali, srcloc :119/:131) | alta |
| 11369 | 871 |  | `comum::asn::CConversorNomesCargo::vf3` | `comum::asn::CConversorNomesCargo::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversornomescargo.cpp | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversornomescargo.cpp | alta |
| 11459 | 535 |  | `comum::asn::CConversorPartido::vf3` | `comum::asn::CConversorPartido::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/cconversorpartido.cpp | uenux2/src/app/comum/dados/asn/cconversorpartido.cpp | alta |
| 11652 | 10 |  | `api::BatteryIconDataSource<(api::CPowerInformation::IconOrientation)1>::vf1` | `api::BatteryIconDataSource<Vertical>::~BatteryIconDataSource (deleting)` | helper de biblioteca/inlinado (destrutor de deleção: 3837 + free) | uenux2/src/api/gui/cpowerinformation.cpp (template definido ali, srcloc :119/:131) | alta |
| 11653 | 257 | ✓ | `comum::CInfoMTLCD::Update` | `comum::CInfoMTLCD::Update` | src/uenux2/src/app/comum/cinfomtlcd.cpp | uenux2/src/app/comum/cinfomtlcd.cpp | alta |
| 11654 | 13 |  | `comum::CInfoMTLCD::vf1` | `comum::CInfoMTLCD::~CInfoMTLCD (deleting)` | src/uenux2/src/app/comum/cinfomtlcd.cpp | uenux2/src/app/comum/cinfomtlcd.cpp | alta |
| 11658 | 37 |  | `comum_f11658` | `atexit: CArquivosSavd::GetInst()::s_instancia.reset()` | src/uenux2/src/app/comum/carquivossavd.u22.cpp | uenux2/src/app/comum/carquivossavd.cpp | alta |
| 12587 | 467 | ✓ | `api::CDataText<vota::(anonymous namespace)::DS_NomeCargoNeutroComEscolha>::vf2` | `api::CDataText<vota::(anonymous namespace)::DS_NomeCargoNeutroComEscolha>::GetText` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u22.cpp | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | média |
