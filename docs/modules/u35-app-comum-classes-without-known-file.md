# u35: classes `comum::` sem arquivo-fonte conhecido (gravadores de resultado, conversores ASN.1, contadores do cadastro de eleitores, partes de relatório)

A unidade u35 reúne **98 funções wasm** do componente `app:comum` (a biblioteca `uenux2/src/app/comum` compartilhada pelas
aplicações da urna) que as ferramentas não conseguiram associar a um arquivo original: 38 classes, a maioria conhecida apenas
por RTTI/vtables (sem registro de `std::source_location`), mais dois grupos de funções livres. Esta unidade lhes dá nome,
reconstrói o código e infere onde cada classe ficava a partir da convenção de nomes do TSE (classe `CFooBar` em
`cfoobar.h/.cpp`, junto das suas parentes mais próximas). Caminhos marcados com *(path inferred)* são esses palpites; arquivos
que SÃO atestados por srclocs (por exemplo `clocal.cpp`, `celeitores.cpp`, `crelutil.cpp`) recebem, em vez disso, um fragmento
`.u35.cpp`.

Apenas **17 das 98 funções executaram** durante os votos gravados (`analysis/runtime/*.functions.tsv`): os leitores
dos dados da eleição na inicialização (`CConversorCandidatura` 11448, `CConversorDadosCandidato` 11450,
`CConversorSecaoEleitoral` 11453, `CConversorDadoLocal` 11404, `CConversorDadoCorrespondencia` 11399/11400,
`CLocal::GetTodasSecoes` 5741, `CLocal::GetLocalID` 1933), `CCandidaturasDSSexo::operator()` (5806, a tela de
confirmação), o assinador `vota::CAssinadorVota` (1501, a cada gravação de estado), o guard de contexto da aplicação
(675), o helper de singleton (1406) e alguns helpers de string/biblioteca (1080, 1221, 1374, 1880, 2769). Tudo o que
pertence ao **encerramento** (fim da votação: BU, arquivos de resultado, cópia para a mídia de resultado) nunca executa no
simulador web, porque a página nunca chega lá.

O conteúdo da unidade, agrupado por assunto:

| assunto | classes / funções | § |
|---|---|---|
| gravadores dos arquivos de resultado do encerramento | `IGravador` (slots 2..6), `IGravadorEnvelope`, `CGravadorEnvelopeArquivo`, `CGravadorRDV`, `CGravadorLog`, `CGravadorVersoesArquivos` (destrutores) | 3 |
| cópia dos resultados para a MR | `CCopiadorMR`, `CCopiadorWSQMR` | 4 |
| assinatura dos arquivos de estado/resultado | `vota::CAssinadorVota` | 5 |
| conversores ASN.1 (23 funções, 19 classes) | `CConversorCarga`, `CConversorDadoCorrespondencia`, `CConversorCabecalhoPacote`, `CConversorDadosCandidato`, `CConversorSecaoEleitoral`, `CIndexadorFotos`, ... | 6 |
| cadastro de eleitores, local, candidatos | contadores de `CEleitores` + `Procura`, `CEleitorDetalhe::PodeVotar`, acessores de `CLocal`, `CCandidaturasDSSexo`, `CRegraIdentidadeLivre`, `CRdvPosicionadorVota`, `CServicoEstadoGeral`, `EhFaseTreinamento`, `CPath::GetPathChaves` | 7 |
| relatórios | helpers de `CRelUtil`, `CParteEleitores`, `CParteCargos`, `CParteCandidatosProporcionais`, `CGeradorRelPU`, `CGeradorRelVersaoPacoteDados`, `IEventosLog::LogaGeracaoRelatorio`, `CGeradorBUQRCodeVota::DadosEleicao` | 8 |
| infraestrutura | `CAppStateContext`, `~CApplicationContextGuard`, acessores de singleton mesclados, `CStringUtils::Trim/Split`, instâncias de biblioteca | 9 |
| as peças do BU (boletim de urna) desta unidade | passo a passo | 10 |

Glossário: *urna* urna eletrônica; *eleitor* quem vota; *mesário* membro da mesa receptora; *seção* seção eleitoral;
*seção agregada* uma seção pequena incorporada a esta (os seus eleitores votam aqui); *local de votação* o local onde ficam
as seções; *zona* zona eleitoral; *município* o município; *cargo* cargo em disputa; *consulta* pergunta de
referendo/plebiscito; *candidatura* a candidatura; *suplente/vice* companheiro de chapa; *apto* que pode votar/concorrer;
*impedimento* impedimento; *habilitação* o mesário liberar o eleitor para votar (*biométrica* pela impressão digital,
*biográfica* pela pergunta do ano de nascimento); *TTE* transferência temporária de eleitor (eleitor transferido
temporariamente para esta seção); *carga* a carga de mídia que instalou os dados da eleição nesta urna; *correspondência* o
registro que liga urna e carga; *encerramento* fim da votação; *BU* boletim de urna, o resultado por urna; *RDV* registro
digital do voto, a tabela embaralhada dos votos dados; *zerésima* o relatório zerado impresso antes da votação; *via* cópia
impressa; *MI* memória interna (a flash interna `/dsk/fi`); *MV* mídia de votação (o cartão de flash removível `/dsk/fe`);
*MR* mídia de resultado (o pendrive USB `/dsk/mr` levado ao cartório eleitoral); *SAVD* o serviço de assinatura da urna;
*fase* oficial / simulado / treinamento.

**Arquivos-fonte reconstruídos** (arquivos novos, exceto os marcados como *fragment*):

```
src/uenux2/src/app/comum/cappstate.h                                       CAppStateContext                 (path inferred)
src/uenux2/src/app/comum/gravadores/igravador.cpp                          IGravador slots 2..6             (path inferred)
src/uenux2/src/app/comum/gravadores/igravadorenvelope.{h,cpp}              IGravadorEnvelope                (path inferred)
src/uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.{h,cpp}       CGravadorEnvelopeArquivo         (path inferred)
src/uenux2/src/app/comum/gravadores/cgravadorrdv.{h,cpp}                   CGravadorRDV                     (path inferred)
src/uenux2/src/app/comum/gravadores/cgravadorlog.{h,cpp}                   CGravadorLog                     (path inferred)
src/uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.u35.cpp       fragment (file of u33)
src/uenux2/src/app/comum/gravadores/ccopiadormr.{h,cpp}                    CCopiadorMR, CCopiadorWSQMR      (path inferred)
src/uenux2/src/app/comum/gravadores/asn/cconversorcarga.{h,cpp}            CConversorCarga                  (path inferred)
src/uenux2/src/app/comum/gravadores/asn/cconversorhistoricovotoimpresso.{h,cpp}                             (path inferred)
src/uenux2/src/app/vota/comum/cassinadorvota.{h,cpp}                       vota::CAssinadorVota             (path inferred)
src/uenux2/src/app/comum/asn/cconversorhasharquivo.u35.cpp                 fragment (class of u17 fragment)
src/uenux2/src/app/comum/asn/cconversorseguranca.{h,cpp}                   CConversorSeguranca              (path inferred)
src/uenux2/src/app/comum/asn/cconversorcabecalhopacote.u35.cpp             fragment (file of u21)
src/uenux2/src/app/comum/asn/util.u35.cpp                                  fragment: Utils::ConverteDataHoraJE + 4 inlined Desconverte*
src/uenux2/src/app/comum/md/ccabecalhopacote.u35.cpp                       fragment: CCabecalhoPacote / CIDPacoteValidar / CIDEleitoral (inlined)
src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.{h,cpp}                    (path inferred)
src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.{h,cpp}             (path inferred)
src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.{h,cpp}                    (path inferred)
src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.u35.cpp      fragment (file of u21)
src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsituacoeseleicoes.{h,cpp}                    (path inferred)
src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversordetalheconsulta.{h,cpp}                      (path inferred)
src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.{h,cpp}                  (path inferred)
src/uenux2/src/app/comum/dados/asn/municipiozona/cconversorhorarioverao.u35.cpp     fragment (file of u03)
src/uenux2/src/app/comum/dados/asn/candidatura/cindexadorfotos.{h,cpp}     CIndexadorFotos                  (path inferred)
src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.cpp   DoDesconverte (header of u21)    (path inferred)
src/uenux2/src/app/comum/dados/asn/candidatura/cconversordadoscandidato.{h,cpp}                             (path inferred)
src/uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.u35.cpp        fragment (file of u21)
src/uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.u35.cpp fragment (file of u21)
src/uenux2/src/app/comum/dados/asn/cconversorpartido.u35.cpp               fragment (file of u22)
src/uenux2/src/app/comum/dados/md/candidatura/cdadoscandidato.u35.cpp      fragment: 4 inlined constructors (cdadoscandidato.cpp:35/:65)
src/uenux2/src/app/comum/dados/md/chorarioverao.u35.cpp                    fragment: inlined constructor (:29/:32)
src/uenux2/src/app/comum/dados/md/csecaoeleitoral.u35.cpp                  fragment: inlined constructor (:34/:37/:41)
src/uenux2/src/app/comum/dados/md/processoeleitoral/cdetalheconsulta.u35.cpp  fragment (:29/:32)
src/uenux2/src/app/comum/dados/md/processoeleitoral/crespostaconsulta.u35.cpp fragment (:29/:32)
src/uenux2/src/app/comum/dados/md/cregraidentidadelivre.{h,cpp}            CRegraIdentidadeLivre            (path inferred)
src/uenux2/src/app/comum/dados/crdvposicionadorvota.{h,cpp}                CRdvPosicionadorVota             (path inferred)
src/uenux2/src/app/comum/dados/clocal.u35.cpp                              fragment: GetTodasSecoes, GetLocalID, GetQtdAgregadas
src/uenux2/src/app/comum/dados/celeitores.u35.cpp                          fragment: Procura + 4 counters
src/uenux2/src/app/comum/dados/celeitordetalhe.u35.cpp                     fragment: PodeVotar
src/uenux2/src/app/comum/dados/ccandidaturas.u35.cpp                       fragment: CCandidaturasDSSexo::operator()
src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeral.cpp          CServicoEstadoGeral::GetPathArquivo (path inferred)
src/uenux2/src/app/comum/appinfo/cappinfo.u35.cpp                          fragment: EhFaseTreinamento      (file inferred)
src/uenux2/src/app/comum/cpath.u35.cpp                                     fragment: GetPathChaves
src/uenux2/src/app/comum/comparecimentomesario/cregistradormesario.u35.cpp fragment: CRegistradorMesario::Existe
src/uenux2/src/app/comum/log/ieventoslog.u35.cpp                           fragment: LogaGeracaoRelatorio
src/uenux2/src/app/comum/relatorios/crelutil.u35.cpp                       fragment: IncluiSeparador, CompletaDireita, FormataQtdAptos
src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u35.cpp               fragment: CGeradorBUQRCodeVota::DadosEleicao
src/uenux2/src/app/comum/relatorios/cpartecandidatos.u35.cpp               fragment: ~CParteCandidatosProporcionais
src/uenux2/src/app/comum/relatorios/cpartecargos.{h,cpp}                   CParteCargos                     (path inferred)
src/uenux2/src/app/comum/relatorios/cparteeleitores.{h,cpp}                CParteEleitores                  (path inferred)
src/uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp                      destructors (header shared with u37) (path inferred)
src/uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.{h,cpp}   CGeradorRelVersaoPacoteDados     (path inferred)
src/uenux2/src/api/gui/capplicationcontextstack.u35.cpp                    fragment: ~CApplicationContextGuard
src/ecourna/api/util/cstringutils.u35.cpp                                  fragment: CStringUtils::Trim(const&), Split
src/uenux2/src/app/comum/u35-foreign-fragments.cpp                         merged singleton body, error thunk, implicit dtors, library list
```

---

## 1. Onde este código fica no processo de votação

`comum` é a camada entre os arquivos de dados da eleição (ASN.1/BER, ver `docs/data-model/asn1-schemas.md`) e as
aplicações (`vota` é a aplicação de votação). As funções desta unidade tocam quatro momentos do dia da urna:

1. **Inicialização (votaInit no build web).** A urna lê os seus dados estáticos: candidatos (`-ca.dat`, decodificados com
   `CConversorCandidatura`/`CConversorDadosCandidato`), a seção (`-lo.dat`, `CConversorSecaoEleitoral`),
   cabeçalhos de pacote (`*.pid`, `CConversorCabecalhoPacote`), situações das eleições (`-ste.dat`), períodos de horário
   de verão (`-cm.dat`), fotos (`-fo.dat`, indexadas por `CIndexadorFotos` sem decodificar as imagens). Ela também lê
   e grava o seu estado persistente `eg.bin` / `vota.bin` (`CConversorDadoCorrespondencia`, `CConversorDadoLocal`,
   `CConversorLocalidadeEleitoral`, `CConversorNumViasImpressasRelatorios`) e o assina (`vota::CAssinadorVota`).
2. **Votação.** As telas de confirmação leem a candidatura atual (`CCandidaturasDSSexo`: o título do cargo concorda
   com o sexo do candidato). Cada voto dado é inserido no RDV numa posição que depende do conteúdo
   (`CRdvPosicionadorVota`), para que o RDV não revele a ordem de votação. O mesário busca eleitores
   (`CEleitores::Procura`) e cada etapa protegida empilha um "contexto da aplicação" para a tela de erro
   (`~CApplicationContextGuard`).
3. **Relatórios** impressos a partir dos menus (lista de eleitores, versões de pacotes, parâmetros da urna, zerésima, BU):
   partes de relatório e helpers de layout da §8.
4. **Encerramento.** `vota::CGravaResultado` constrói um gravador por arquivo de resultado e conduz o pipeline `IGravador`
   da §3 (gravar na área de trabalho da MI, copiar para a área de resultado da MI, assinar, copiar para a MV); depois
   `vota::CCopiaResultadoParaMR` copia tudo para a mídia de resultado com `CCopiadorMR` (§4).

---

## 2. Classes e hierarquia (RTTI)

```
comum::IResultado (u23) ─ comum::IGravador (slots 2..6 here: igravador.cpp)
   ├─ CGravadorRDV                     @1556856   rdv.dat      (dtors here; GravaResultado u18)
   ├─ CGravadorLog                     @1557196   log.jez      (dtors here; GravaResultado u12)
   ├─ CGravadorVersoesArquivos         @1557248   mr.ver       (dtors here; rest u33)
   ├─ CGravadorBU / CGravadorRCSecao / CGravadorHashes / CGravadorWSQ   (u23)
   └─ IGravadorEnvelope                @1554456   (dtor here; GravaResultado u18)
        └─ CGravadorEnvelopeArquivo    @1554512   imgbu.dat / imgze.dat (dtors here; ctor u07; LeConteudo u12)
comum::CCopiadorMR                     @1553348   ─ CCopiadorWSQMR @1553380
comum::CAssinador (u23)                @1553260   ─ vota::CAssinadorVota @1532312 (ctor here)
api::CStateContext<comum::CAppState>              ─ comum::CAppStateContext @1551312
api::IReportPart                                  ─ comum::CParteEleitores @1576596, CParteCargos @1576564,
                                                    CParteCandidatosProporcionais @1576452 (siblings: u25)
comum::CGeradorRelPU                   @1545288   (no base)
comum::CGeradorRelVersaoPacoteDados    @1576284   (no base, 6 slots)
comum::CGeradorBUQRCode (u25)                     ─ CGeradorBUQRCodeVota @1575984 (slot 3 here)
comum::md::IRegraIdentidade (u05)                 ─ CRegraTitulo, CRegraCPF (u05), CRegraIdentidadeLivre @1574652
comum::md::CRdvPosicionador                       ─ comum::CRdvPosicionadorVota @1560252
comum::IServicoEstado<CEstadoGeral, CConversorEstadoGeral>  ─ CServicoEstadoGeral @1558024
comum::asn::IConversorASN<ENTIDADE, DADO> (u21)   ─ 18 converters of §6
comum::asn::IConversorParcialASN<EntidadeFotosCandidatos, md::CIndicesFotos, CVisitanteFoto> ─ CIndexadorFotos @1562548
```

Todas as vtables de conversores têm a forma `[0] 174 (trivial dtor) [1] 144 (operator delete) [2] DoConverte [3]
DoDesconverte` (o slot padrão lança 7655/7656 "Método DoXxx() não implementado para {}", então um arquivo só de leitura
sobrescreve apenas o slot 3). O conversor parcial `CIndexadorFotos` é a exceção: a sua vtable tem 3 slots, e o slot 2
é o seu `DoDesconverte(entidade, visitante)`.

---

## 3. Gravadores de arquivos de resultado (pipeline `IGravador`)

`IGravador` (declarado por u23 em `iresultado.h`) é dono do nome de UM arquivo de resultado:
`"<fase o|s|t><pleito:05><uf><município:05><zona:04><seção:04>-<sufixo>"`, por exemplo `t02410ac0000100010001-bu.dat`.
Os cinco métodos virtuais não puros são compartilhados por todos os gravadores, exceto `CGravadorWSQ` (os mesmos slots de
tabela 2187..2191 nas vtables de CGravadorBU, CGravadorRDV, CGravadorRCSecao, CGravadorHashes, CGravadorLog,
CGravadorVersoesArquivos, IGravadorEnvelope e CGravadorEnvelopeArquivo). `CGravadorWSQ` sobrescreve os cinco. O slot 2
(func 11579, com `resultado(MI)/<nome>`) e os slots 3/6 (11577/11578 = `comum_f6041(this, 0|1)`) terminam todos no grande
corpo de empacotamento WSQ, a func 5821 (glob de `*.wsq`, `CGravadorWSQ::GetCaminhoCorretoInternal/External`,
cgravadorwsq.cpp:175/192, icompressor.cpp:61 do ecourna). Os slots 4/5 (`Grava`/`GravaMV`) e 7 são os corpos ICF vazios
218/425:

| slot | func | método (nomes de u23) | o que faz |
|---:|---:|---|---|
| 2 | 11632 | `CopiaParaResultado()` | `CopyFile(trab(MI)/nome, resultado(MI)/nome)` |
| 3 | 11630 | `CopiaParaMV()` | `CopyFile(trab(MI)/nome, trab(MV)/nome)` depois `CopyFile(resultado(MI)/nome, resultado(MV)/nome)` |
| 4 | 11634 | `Grava()` | `CFile f(trab(MI)/nome, "wb"); GravaResultado(f) /*slot 7*/; f.Close();` |
| 5 | 11633 | `GravaMV()` | o mesmo em `trab(MV)` (sem chamador em CGravaResultado) |
| 6 | 11631 | `CopiaResultadoParaMV()` | `CopyFile(trab(MV)/nome, resultado(MV)/nome)` (sem chamador em CGravaResultado; CGravadorWSQ o sobrescreve) |

`trab` = `dinamico/trab<turno>/`, `resultado` = `dinamico/res<turno>/` (`CPath::GetPathTrab/GetPathResult`, com o
turno lido de eg.bin). `CSystem::CopyFile(origem, destino, preservaAtributos=false)` (func 378).

Os gravadores concretos desta unidade só contribuem com destrutores (os seus corpos de `GravaResultado` estão em outras
unidades), que documentam os seus layouts:

| gravador | arquivo (sufixo, id SAVD) | layout (além de IResultado +0..+39) |
|---|---|---|
| `CGravadorRDV` | `-rdv.dat` (6, 37) | +40 `CDateTime` data de geração, +52 `'1'`, +56 `CDadoCorrespondencia` (96 B), +152 motivo SA opcional (flag +160), +164 `const CRdvVota*` |
| `IGravadorEnvelope` | — | +40 data de geração, +52 fase, +56 `'1'`, +60 opcional (flag +68), +72 `CEnvelopeGenerico::Tipo`, +76 `CDadoCorrespondencia` |
| `CGravadorEnvelopeArquivo` | `-imgbu.dat` (9, 43) / `-imgze.dat` (11, 44) | +172 caminho do arquivo envolvido (`trab/bu.dat`, `trab/ze.dat`) |
| `CGravadorLog` | `-log.jez` (13, 60) | +40 `dinamico/log/arquivados/`, +52 `dinamico/log/logd.dat`, +64 `trab(MI)` |
| `CGravadorVersoesArquivos` | `-mr.ver` (19, 70) | +40 string de tag, +52 `map<string,string>` módulo → versão |

Dois desses destrutores são o corpo mesclado `comum_f6043(this, offset, vtable)` (IGravadorEnvelope com offset 76,
CGravadorRDV com 56): o wasm-opt mesclou as duas funções, que diferem apenas no offset do membro
`CDadoCorrespondencia` e na constante da vtable.

---

## 4. Cópia para a mídia de resultado (`CCopiadorMR`)

`CCopiadorMR {vptr, std::string m_nome}` (16 bytes) é construído por `vota::CCopiaResultadoParaMR::CopiaResultado`
(u08) para cada um de `bu.dat, rdv.dat, jufa.dat, imgbu.dat, imgze.dat, hash.dat, log.jez, vota.vsc, mr.ver` e, numa
urna biométrica, como `CCopiadorWSQMR` para `wsqbio/wsqman/wsqmes.jez`.

* slot 2 `Copia()` (func 5874): `CopyFile(resultado(MI)/nome, /dsk/mr/nome)`. O único slot usado.
* slot 3 `CopiaDaMV()` (func 5873, nome inferido): o mesmo a partir de `resultado(MV)`. Nunca chamado pelo VOTA.

A vtable de `CCopiadorWSQMR` aponta para exatamente as mesmas funções, mas por **slots de tabela diferentes** nos slots 1..3
(2179..2181 em vez de 2176..2178); o slot 0 compartilha o slot de tabela 2175 com a base (o destrutor completo foi emitido
como alias de `~CCopiadorMR`). Um slot de tabela é criado por função no momento do link. Um destrutor de deleção por classe
é sempre emitido, então o slot 1 separado não prova nada. Já os slots 2 e 3 separados mostram que o copiador WSQ tinha
os seus próprios overrides de `Copia` / `CopiaDaMV`. Os corpos compilados deles ficaram idênticos byte a byte aos da base, e o
wasm-opt os dobrou. Neste build, o copiador WSQ se comporta exatamente como a classe base. Apenas a cópia do BU é
comparada com a sua origem depois (`CSystem::AreFilesEqual`, no chamador).

---

## 5. Assinatura (`vota::CAssinadorVota`, func 1501)

`CAssinadorVota(pacote)` = `comum::CAssinador(ESavdAplic 1 = VOTA, pacote)` com o construtor da base inlinado:

```
caminho     = CArquivosSavd::GetInst()[pacote]            // carquivossavd.cpp:49, e.g.
              //  122/123: /dsk/fi/dinamico/trab1|2/vota.vsu          (state files of the MI)
              //  158/159: /dsk/fi/dinamico/res1|2/<prefixo>-vota.vsc (result package)
m_diretorio = Diretorio(caminho) + "/"                   // func 5462: text before the last '/'
m_arquivo   = NomeArquivo(caminho)                       // func 2763: text after the last '/'
m_local     = ""                                         // filled only by CGravaResultado before AssinaArquivosResultado
```

Nas sessões gravadas, ela executou a partir de `comum_f491` (a gravação de estado, "Gravando o estado da urna"; 88 amostras
do profiler, a função mais pesada desta unidade). Outros chamadores: a sincronização do voto da urna
(`vota::impl::CSincronismoVotoEleitor` 7174, substituída pela política web no simulador) e
`CSincronizaVota::SincronizaRelatorios`. A assinatura propriamente dita
(`CAssinador::Assina`, func 1277) é pedida ao SAVD; no build web, `CWasmSavd` responde OK e os arquivos `.vsu`
contêm o literal `assinatura simulada para vota_web_wasm` (ver `docs/00-provenance.md`).

---

## 6. Conversores ASN.1

| classe | tipo ASN.1 ⇄ tipo md | func (slot) | comportamento / verificações |
|---|---|---|---|
| `CConversorHashArquivo` | `ArquivoAssinatura` ⇄ `api::hash::CHashArquivo` | 10267 (2) | `{nomeArquivo, assinatura}` a partir das duas strings (entradas de hash.dat) |
| `CConversorHistoricoVotoImpresso` | `ModuloBoletimUrna::HistoricoVotoImpresso` ⇄ `md::CHistoricoVotoImpresso` | 10276 (2) | 2 ints + `DataHoraJE`; nunca preenchido pelo VOTA (campo ausente no BU) |
| `CConversorCarga` | `ModuloTiposEcoUrna::Carga` ⇄ `md::CCarga` | 10291 (2), 10290 (3) | serial "8 hex" ⇄ OCTET STRING de 4 bytes (3605/3604), `DataHoraJE`, gerador via o conversor do ecourna |
| `CConversorSituacoesEleicoes` | `EntidadeSituacoesEleicoes` → `vector<CSituacoesEleicoes>` | 11361 (3) | mantém apenas `tipo == ativa`; eleições suspensas/canceladas/excluídas são descartadas silenciosamente |
| `CConversorDetalheConsulta` | `ModuloEleicao::DetalhePergunta` → `md::CDetalheConsulta` | 11377 (3) | texto da pergunta dividido em '\|', cada linha aparada e depois reunida; respostas: número < 100000 (8167), não vazia (8168); pergunta não vazia (8146), ≥ 1 resposta (8147) |
| `CConversorHorarioVerao` | `HorarioVerao` → `md::CHorarioVerao` | 11384 (3) | `fim > início` (8003 "Período inválido."), `diferença != 0` (8004) |
| `CConversorNumViasImpressasRelatorios` | `NumViasImpressasRelatorios` ⇄ 4 × uint8 | 11386 (2), 11387 (3) | faixa ASN.1 0..999, mas só o byte baixo é lido |
| `CConversorLocalidadeEleitoral` | `DadoSecao` ⇄ `CLocalidadeEleitoral` | 11388 (2) | município/zona/seção (11389 = u05) |
| `CConversorDadoCorrespondencia` | `DadoCorrespondencia` ⇄ `CDadoCorrespondencia` | 11399 (2), 11400 (3) | carga + secaoCarga + assinatura; **nenhuma verificação** da assinatura |
| `CConversorDadoLocal` | `DadoLocal` → `CDadoLocal` | 11404 (3) | uf, secaoCarga, tipoLocalVotacao |
| `CConversorSeguranca` | `Seguranca` ⇄ `md::CSeguranca` | 11413 (2) | `idArquivoCD` nunca é gravado (fica 0) |
| `CConversorCabecalhoPacote` | `CabecalhoPacote` → `md::CCabecalhoPacote` | 11437 (3) | inlina util.cpp:89/329/451, cideleitoral.cpp:21/29/37 (id < 100000), cidpacote.cpp:21 (fase diferente de '0'/'4'), ccabecalhopacote.cpp:33/36/39 (nome não vazio, versão com 12 caracteres, origem em 1..12, compilado como uma única comparação sem sinal `(unsigned)(origem - 13) <= 0xFFFFFFF3`); abrangência ignorada |
| `CIndexadorFotos` | `EntidadeFotosCandidatos` (parcial) → `md::CIndicesFotos` | 11441 (2) | copia a lista `{código, [início, fim)}` do visitante; mantém apenas o município da abrangência |
| `CConversorCandidatura` | `Candidatura` → `md::CCandidatura` | 11448 (3) | titular + suplentesVices (`at()` com verificação de limites); `drap` ignorado |
| `CConversorDadosCandidato` | `DadosCandidato` → `md::CDadosCandidato` | 11450 (3) | mantém código, nomeUrna, nomeFonético, sexo (util.cpp:483: 0/2/4 → 0/1/2), ordemSuplência (1..9, 7998/7999); ignora nome civil/social, nascimento, situação, cassação, partido, reeleição; "apto" = lista de onde veio |
| `CConversorSecaoEleitoral` | `SecaoEleitoral` → `md::CSecaoEleitoral` | 11453 (3) | agregadas validadas (iconversorasn.h:71), **ordenadas por número**, depois csecaoeleitoral.cpp:34/37/41: 1..9999 (8053), ≠ principal (8054), únicas (8055) |
| `CConversorIdentificacaoAgregada` | `IdentificacaoAgregada` ⇄ `CIdentificacaoAgregada` | 11456 (2) | número + tipo local |
| `CConversorPartido` | `Partido` ⇄ `md::CPartido` | 11460 (2) | nome, sigla, número |
| `CConversorCodigoCargoConsulta` | `CodigoCargoConsulta` (CHOICE) ⇄ `uebyte` | 11592 (2), 11591 (3) | ≤ 24 → `cargoConstitucional`, senão `numeroCargoConsultaLivre`; na volta: o byte de valor de qualquer uma das alternativas |

`Utils::ConverteDataHoraJE` (func 1080) produz todo `DataHoraJE` gravado pela urna:
`CDate::Format("YYYYMMDD") + "T" + CTime::Format("hhmmss")`. O eg.bin gravado no votaInit
(`analysis/runtime/memfs-after-init/dsk/fi/dinamico/eg.bin`) contém `20201231T235958`, o dataHoraCarga da fixture,
gravado por 11399.

---

## 7. Cadastro de eleitores, local, candidatos e helpers de estado

* **Contadores de `CEleitores`** (celeitores.u35.cpp). `GetQtdHabilitadosPorBiometria` (2821) e
  `GetQtdHabilitadosPorBiografia` (1935) são thunks para `QtdVotaramPorTipoHabilitacao(1|2)` (6034);
  `GetQtdCompareceramSemBiometria` (2822) é o mesmo laço compilado com a constante 0. Só são contados os eleitores com
  dados dinâmicos (o optional em `CEleitorDetalhe` +192, preenchido por `CEleitorDinamicoDAO`, cujo nome de tabela
  `eleitor_dinamico` está no binário) e com `estado == VOTOU`. Os arquivos `uenux.db` das sessões web gravadas não têm
  essa tabela (apenas `comparecimento_mesario` e `registro_justificativa`, `analysis/runtime/memfs-after-vote`).
  `GetQtdAptosSecao` (2823) conta os eleitores sem impedimento no turno atual (`PodeVotar`, 2266), separados pelo campo
  TTE (+48 do eleitor; qualquer tipo diferente de zero, inclusive a transferência por acessibilidade, conta como temporário).
* **`CEleitores::Procura`** (2264): constrói um `CEleitorIdentidade` (validado, preenchido até 11/12 dígitos), o mapeia para
  a identidade principal do eleitor (map em +112), encontra o eleitor e o torna o eleitor corrente do cadastro.
* **`CEleitorDetalhe::PodeVotar`** (2266): impedimento do turno 2 (+200) se o turno de eg.bin é `'2'`, senão o do turno 1
  (+196); apto quando 0.
* **`CLocal`** (clocal.u35.cpp): `GetTodasSecoes` (5741) = seção principal + agregadas; `GetLocalID` (1933) =
  local de votação; `GetQtdAgregadas` (2816). Cada uma chama primeiro `VerificaLido/VerificaEhSecao("<own name>")`, e foi
  assim que os nomes foram recuperados.
* **`CCandidaturasDSSexo::operator()`** (5806): sexo do titular atual, ou do suplente/vice de ordem
  `m_suplente`.
* **`CRegraIdentidadeLivre`** (11273/11274 + `return true` compartilhado): a regra para identificadores de formato livre:
  GetTipo 3, Formata = função identidade, Verifica = true (o identificador ainda precisa ser só de dígitos:
  iregraidentidade.cpp:23).
* **`CRdvPosicionadorVota::Posiciona`** (11496): `std::upper_bound` sobre os votos do cargo por (tipo, digitado);
  votos iguais são acrescentados depois dos existentes. É isso que mantém o RDV **ordenado por conteúdo** e não por
  tempo (sigilo do voto). Não é executada no build web (a sua política de sincronização não persiste votos).
* **`CServicoEstadoGeral::GetPathArquivo`** (11569): `<flash>/dinamico/eg.bin` (sem componente de turno).
* **`EhFaseTreinamento`** (1485): fase de eg.bin == `'3'`. Sempre true no simulador (todos os cenários são cargas de
  treinamento); em treinamento, os pacotes WSQ não são cifrados, a digital reconhecida não é armazenada e a trava de
  horário do fim da votação nunca é armada.
* **`CPath::GetPathChaves`** (1948): `GetPathRoot("/dsk/fi/estatico/chave/")`, o diretório de chaves (bu.pk1, wsq.pk1,
  bio.sk1, cv.ber.pri...).
* **`CRegistradorMesario::Existe`** (2221): se o cache de comparecimento de mesários existe, sem criá-lo.

---

## 8. Relatórios

* `CRelUtil::IncluiSeparador(b, alinhamento)` (1387): 38 hífens. `CRelUtil::CompletaDireita(texto, largura)` (1881):
  completa à direita com espaços. `CRelUtil::FormataQtdAptos(std::function<SQtdeAptos()>)` (1921): as três linhas do BU
  `"Eleitores aptos                   {:04}\n"`, `"          Originais da seção      {:04}\n"`,
  `"          Temporários na seção    {:04}"` (total = originais + temporários, como uint16).
* `IEventosLog::LogaGeracaoRelatorio(relatorio, termino)` (1047): registro de log
  `"Gerando relatório [<BU|ZERESIMA|...>] [INÍCIO|TÉRMINO]"` em volta de todo relatório impresso.
* `CParteEleitores::Imprime` (11206): uma subparte por eleitor do cadastro (relatório de lista de eleitores), percorrendo o
  cadastro com o cursor compartilhado do singleton. `CParteCargos` / `CParteCandidatosProporcionais`: apenas destrutores
  (os seus `Imprime` estão em u04; layouts nos headers).
* `CGeradorRelVersaoPacoteDados::MontaDados` (11222, slot 4): depois de uma linha em branco, uma linha centralizada por
  pacote de dados `"<nome><espaços><versão>"` (38 colunas), depois `"Dados:"` + os primeiros 8 caracteres Base64 do
  `hashVersoesPacotes` de eg.bin, alinhados à direita.
* `CGeradorRelPU`: `{vptr, CPaperFormBuilder}`; apenas os seus destrutores estão nesta unidade (11900/11901).
* `CGeradorBUQRCodeVota::DadosEleicao` (11241): retorna `""` (o VOTA não acrescenta dados por eleição aos QR codes do BU).

---

## 9. Infraestrutura

* `CAppStateContext(estadoInicial)` (3844): `{vptr @1551312, CAppState*}`; um por thread cooperativa.
* `~CApplicationContextGuard` (675): remove o contexto do guard da pilha global de contextos **apenas quando não há
  exceção em andamento** (`std::uncaught_exceptions() == 0`); numa exceção, o contexto fica no topo para que a
  thread de monitoramento o mostre na tela de erro fatal. Busca a partir do topo com `operator==` (5556), apaga por
  atribuição por movimento (5552).
* `GetInstCriada<T>` (1406): o corpo mesclado de nove acessores `GetInst()` (CConfiguracaoEleicao, CCargos,
  CEleitores, CTelasVota, CRdvVota, CPE, CValidadorIdentidade, CRespostas, CHV): `lock; if (!p) throw
  CPatternError(1303, "<X> - instancia nao criada"); return *p;`. Resta apenas o resíduo do unlock do mutex (não há threads).
* `CCodecWsq::GetInst` (2742, nome usado por u24 com "?"): singleton vazio criado sob demanda (corpo ICF 2900); as suas
  chamadas foram reduzidas a cópias neste build.
* `CStringUtils::Trim(const std::string&)` (1374, cópia + slot 6416) e `CStringUtils::Split(texto, sep)` (1880).
* Instâncias de biblioteca: std::sort das respostas de consulta (248, 1920, 2790), helpers de map/set (1221, 1405, 2806),
  helpers de vector (1874, 2769), cópia de `std::function` (1922), `pair<const char, CLabelParametrizado>` (1552),
  `filesystem::path::string()` (1555), `ASN1::CHOICE::operator=` (2188). Destrutores implícitos:
  `~CDadoCorrespondencia` (857), `~CCabecalhoQRCode` (2885). Thunk de erro: construtor de `CUeComumMdError` (591).

---

## 10. O BU (boletim de urna): o que esta unidade contribui

O BU em si (`EntidadeBoletimUrna`, `CGravadorBU`, `CConversorEntidadeBU`, `CGeradorBU`) está documentado em
`docs/10-boletim-de-urna.md`, `docs/bu/*.md` e nas unidades u23/u25. As peças que ficam nesta unidade são:

1. **Contadores do BU** (`DetalhamentoComparecimento` do arquivo do BU, campos de QR `APTO/APTS/APTT` e
   `HBSB/HBBM/HBBG`, linhas impressas "Eleitores aptos / Originais da seção / Temporários na seção" e
   "Habilitação sem biometria / biométrica / biográfica"):
   * aptos do cabeçalho do QR e do BU impresso = `CEleitores::GetQtdAptosSecao()` (2823): eleitores com
     impedimento 0 no turno atual, separados em originais / TTE; impressos por `CRelUtil::FormataQtdAptos` (1921) como
     `{:04}`. (Os `qtdEleitoresAptos/AptosSecao/AptosTTE` por eleição de `ResultadoVotacaoPorEleicao` no arquivo do BU
     vêm do map por abrangência `CEleitores::GetQtdAptos()`, func 2824, unidade u04, que aplica a mesma
     regra `PodeVotar`.)
   * `qtdEleitoresCompareceramSemBiometria` = 2822, `qtdEleitoresHabilitadosPorBiometria` = 2821,
     `qtdEleitoresHabilitadosPorBiografia` = 1935: eleitores com `estado VOTOU` e o `tipoHabilitacao` dado
     (0/1/2) de `eleitor_dinamico`.
   * `LOCA` / identificação: `CLocal::GetLocalID` (1933); `AGRE`: `CLocal::GetQtdAgregadas`/`GetTodasSecoes`.
2. **Carga / correspondência no bloco "urna" de todo arquivo de resultado**: `CConversorCarga` grava
   `Carga {numeroInternoUrna INTEGER, numeroSerieFC OCTET STRING(4) from 8 hex digits, identificadorGeradorMidia
   {nome, serialCertificadoTPM, serialInstalacao}, dataHoraCarga "YYYYMMDDThhmmss", codigoCarga (24 digits)}`.
   Os mesmos dados são mantidos em eg.bin como `DadoCorrespondencia` (+ `secaoCarga`, `assinatura`) por
   `CConversorDadoCorrespondencia`; no simulador, a fixture tem `numeroInternoUrna 87654321`, `codigoCarga
   "123456789012345678901234"`, `dataHoraCarga 20201231T235958`, gerador `nome_maquina` e uma
   `assinatura` de teste de 139 bytes, que é copiada mas nunca verificada.
3. **`historicoVotoImpresso`** (histórico do módulo de voto impresso): o conversor 10276 existe, mas o VOTA nunca o preenche.
4. **QR codes do BU**: `CGeradorBUQRCodeVota::DadosEleicao` retorna `""`; o cabeçalho `CCabecalhoQRCode` (33 strings) é
   destruído por 2885.
5. **Log**: `"Gerando relatório [BU] [INÍCIO]"` / `"[TÉRMINO]"` (1047) em volta da impressão do BU (`vota::CGeraBU`).
6. **Vias impressas**: a imagem do BU impresso `trab/bu.dat` vira `<prefixo>-imgbu.dat` via
   `CGravadorEnvelopeArquivo` (EntidadeEnvelopeGenerico, tipo "BU impresso"); a imagem da zerésima `trab/ze.dat` vira
   `-imgze.dat`.
7. **Gravação, cópia, assinatura** (a ordem de `vota::CGravaResultado`, u07):
   1. `Grava()` de cada gravador: `trab(MI)/<prefixo>-bu.dat`, `-rdv.dat`, `-jufa.dat`, `-imgbu.dat`, `-imgze.dat`,
      `-hash.dat`, `-mr.ver`, `-log.jez` (arquivo aberto com `"wb"`, o slot 7 codifica, depois é fechado).
      `CGravadorWSQ::Grava()` é um no-op.
   2. `CopiaParaResultado()`: cada arquivo é copiado para `resultado(MI)` = `dinamico/res<turno>/`. Para os gravadores
      `wsq*.jez`, este é o override func 11579, que roda o corpo de empacotamento WSQ (func 5821) para
      `resultado(MI)/<nome>`. Nada foi gravado pelo `Grava()` deles.
   3. `CAssinador::AssinaArquivosResultado` com o objeto `CAssinadorVota(158|159)`: o SAVD assina a lista
      de arquivos em `res<turno>/<prefixo>-vota.vsc` (simulado no build web).
   4. Se a MV está presente, `CopiaParaMV()`: cópias de trab e de resultado em `/dsk/fe`, depois o `.vsc` é copiado.
   5. `vota::CCopiaResultadoParaMR`: `CCopiadorMR::Copia()` para cada arquivo → `/dsk/mr/<prefixo>-<sufixo>`; apenas a
      cópia do BU é comparada com a origem (`AreFilesEqual`, "Nao copiou" em caso de divergência, código 9370).
8. **RDV** (a fonte da verdade do BU para as contagens): cada voto é inserido em `CRdvPosicionadorVota::Posiciona`
   (ordenado por tipo de voto e dígitos), então `-rdv.dat` lista os votos em ordem de conteúdo.

---

## 11. Particularidades do build web

* Nenhum código de encerramento (§3, §4, §10.7) executa: a página nunca encerra a eleição.
* `CAssinadorVota` executa, mas o SAVD é `CWasmSavd` (sempre OK); os arquivos `.vsu` são texto falso.
* `EhFaseTreinamento()` é sempre true (cenários de treinamento), então todo ramo "só na fase oficial" é código morto
  no simulador.
* O posicionador do RDV não é exercitado: a política web `CSincronismoVotoEleitorWeb` não armazena votos.
* `CCodecWsq` e os caminhos de impressão digital são stubs.

## 12. Observações notáveis de wasm / Emscripten

* **Overrides dobrados detectáveis pelos slots de tabela**: os slots 1..3 de `CCopiadorWSQMR` apontam para as funções da
  base por slots de tabela distintos (§4), então eram funções distintas no momento do link.
* **Corpos de destrutor mesclados**: `~CGravadorRDV` e `~IGravadorEnvelope` são thunks para `comum_f6043(this,
  offset, vtable)` (o merge-similar-functions transformou o offset do membro 56/76 e a vtable em parâmetros);
  `~CCopiadorMR` (shared_f1722/1723), `~CParteEleitores` (api_f6026) e `~CGeradorRelPU` (shared_f6055 / unknown_f6054)
  compartilham corpos com classes não relacionadas (RHVoice, api::CFormPart). Alguns destrutores de deleção chamam o
  completo (`CGravadorLog` 11583 → 5823), outros repetem o código dele inline (`CGravadorVersoesArquivos` 11580,
  `CGravadorRDV` 11585).
* **Construtores inlinados nos conversores**: as classes md `CDadosCandidato`, `CHorarioVerao`, `CSecaoEleitoral`,
  `CDetalheConsulta`, `CRespostaConsulta`, `CCabecalhoPacote`, `CIDPacote`, `CIDEleitoral` não têm código fora de linha:
  os seus registros de srcloc (arquivo:linha de cada throw) só aparecem dentro das funções DoDesconverte desta unidade. Os
  fragmentos em `dados/md/**.u35.cpp` e `md/ccabecalhopacote.u35.cpp` os escrevem por extenso.
* **Nomes de slots de vtable**: todo slot de conversor é resolvido pelo layout de `IConversorASN`; os nomes `vfN` das
  ferramentas são substituídos por `DoConverte` (slot 2) / `DoDesconverte` (slot 3).
* **Nenhum artefato do descompilador em 11569 / 11274** (correção): `decompiled/app-comum/_by_index/11500.dcmp` e
  `11000.dcmp` mostram cada chamada uma vez, como o texto wasm. Um cartão `q.py f` lista as funções chamadas na sua linha
  `calls` e depois imprime o corpo, então o nome de uma função chamada aparece duas vezes no cartão; isso não é código
  duplicado.
* **Exceções do Emscripten**: `~CApplicationContextGuard` chama `env.__cxa_uncaught_exceptions` (lado JS), e
  `Split`/`CompletaDireita` chamam via trampolins `invoke_*` apenas para alcançar os seus landing pads de limpeza (a
  string em `CompletaDireita`; a string temporária e o resultado `std::vector<std::string>`, func 2255, em `Split`).
* As funções atribuídas a `app:comum` incluem código do ecourna (`CStringUtils::Trim/Split`), da api
  (`~CApplicationContextGuard`), do vota (`CAssinadorVota`) e de biblioteca: o classificador seguiu os chamadores.

---

## 13. Código estranho / riscos (detalhes na lista "suspicious" do StructuredOutput)

* `CGeradorRelVersaoPacoteDados::MontaDados` (11222): `std::string(38 - (nome + versão), ' ')` lança
  `std::length_error` para um nome de pacote com mais de 26 caracteres (o mais longo nos cenários tem 25).
* `CConversorNumViasImpressasRelatorios::DoDesconverte` (11387): contadores 0..999 truncados para 8 bits.
* `CConversorSituacoesEleicoes` (11361): eleições não ativas desaparecem silenciosamente.
* `CConversorDadoCorrespondencia` (11400): a `assinatura` da carga é transportada, mas nunca verificada aqui.
* `CRegraIdentidadeLivre` + a busca de regras da func 566: uma regra de identificador livre colocada antes da regra de
  título/CPF desativa os dígitos verificadores destas. A busca para na primeira regra cujo `GetTipo()` é 3 ou o tipo
  pedido. As regras são criadas na ordem da lista de tipos de identificador da configuração (laço na func 7787: 1 →
  CRegraTitulo, 2 → CRegraCPF, 3 → CRegraIdentidadeLivre, qualquer outro valor lança "Tipo de identificador principal
  inválido {}"). Portanto, a ordem depende dos dados de configuração. Nada no código impede isso.
* `CParteEleitores::Imprime` (11206) move o cursor compartilhado do cadastro para o fim.
* `~CApplicationContextGuard` (675): um contexto cujo escopo foi deixado por uma exceção que depois é capturada nunca é
  removido (contexto obsoleto na pilha de erros).
* `CConversorCabecalhoPacote` (11437): a validação do cabeçalho NÃO é fraca (correção). `origem` deve estar em 1..12
  (uma comparação sem sinal, `i32.le_u`) e `DesconverteIdSistema` só pode retornar 1..12 de qualquer forma.
  `DesconverteFase` (2843, util.cpp:116) rejeita tudo fora de 1..3 antes que `CIDPacoteValidar` repita um teste de
  sentinela ('0'/'4'). Apenas a abrangência opcional é ignorada, porque `md::CCabecalhoPacote` não tem membro para ela.

## 14. Questões em aberto

* Nomes reais dos slots 5/6 de `IGravador` e do slot 3 de `CCopiadorMR` (sem chamador no VOTA); se `CopiaDaMV` é usada
  por outra aplicação (SA / recuperação).
* `md::CIndicesFotos` mantém apenas um int da abrangência (+16 = município): é de fato o município ou um
  "código de abrangência"?
* A classe dona de `Split` (`CStringUtils` do ecourna vs `api::CStringUtils` do uenux2 vs `util` do comum), e
  `CompletaDireita` vs `PadRight` (1881).
* `CCodecWsq` (2742): nome real da classe e arquivo.
* Layout do optional em +60 de `IGravadorEnvelope` / +152 de `CGravadorRDV` (tipo de valor de 8 bytes?).

---

## 15. Tabela de mapeamento completa (todas as 98 funções)

"executou" = observada executando nos votos gravados. "arquivo src" = onde a reconstrução está (ou por que não há nenhuma).

| func | tamanho | executou | símbolo reconstruído | arquivo src | arquivo original | conf. |
|---:|---:|:-:|---|---|---|---|
| 248 | 320 |  | `std::iter_swap<md::CRespostaConsulta*> (std::sort helper)` | helper de biblioteca/inlinado: std::sort das respostas de consulta (elementos de 28 bytes) | biblioteca | alta |
| 591 | 18 |  | `comum::md::CUeComumMdError::CUeComumMdError (merged ctor thunk)` | src/uenux2/src/app/comum/u35-foreign-fragments.cpp (comentário) | uenux2/src/app/comum/md/cabrangencia.h | alta |
| 675 | 211 | ✓ | `api::CApplicationContextGuard::~CApplicationContextGuard` | src/uenux2/src/api/gui/capplicationcontextstack.u35.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp | alta |
| 857 | 38 |  | `comum::md::estadoaplicacao::CDadoCorrespondencia::~CDadoCorrespondencia` | src/uenux2/src/app/comum/u35-foreign-fragments.cpp (comentário) | uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocorrespondencia.cpp | alta |
| 948 | 15 |  | `comum::CLogComum::GetInst` | src/uenux2/src/app/comum/log/clogcomum.cpp (u24) | uenux2/src/app/comum/log/clogcomum.cpp | alta |
| 1047 | 635 |  | `comum::IEventosLog::LogaGeracaoRelatorio` | src/uenux2/src/app/comum/log/ieventoslog.u35.cpp | uenux2/src/app/comum/log/ieventoslog.cpp | média |
| 1080 | 609 | ✓ | `comum::asn::Utils::ConverteDataHoraJE` | src/uenux2/src/app/comum/asn/util.u35.cpp | uenux2/src/app/comum/asn/util.cpp | média |
| 1221 | 268 | ✓ | `std::map<std::string,std::string>::__emplace_hint_unique` | helper de biblioteca/inlinado: insert com hint em map<string,string> | biblioteca | alta |
| 1374 | 12 | ✓ | `ecourna::api::util::CStringUtils::Trim(const std::string&)` | src/ecourna/api/util/cstringutils.u35.cpp | ecourna-lib/ecourna/api/util/cstringutils.cpp | média |
| 1387 | 169 |  | `comum::CRelUtil::IncluiSeparador` | src/uenux2/src/app/comum/relatorios/crelutil.u35.cpp | uenux2/src/app/comum/relatorios/crelutil.cpp | média |
| 1391 | 84 |  | `comum::CJustificador::GetInst` | src/uenux2/src/app/comum/justificativa/cjustificador.cpp (u24) | uenux2/src/app/comum/justificativa/cjustificador.cpp | alta |
| 1405 | 32 |  | `std::__tree<map<md::ETipoAbrangencia,SQtdeAptos>>::destroy` | helper de biblioteca/inlinado: destroy recursivo da árvore (valores triviais) | biblioteca | alta |
| 1406 | 72 | ✓ | `comum::(anonymous)::GetInstCriada<T> (merged singleton GetInst body)` | src/uenux2/src/app/comum/u35-foreign-fragments.cpp | (mesclado pelo wasm-opt; uma cópia por GetInst) | média |
| 1485 | 14 |  | `comum::EhFaseTreinamento` | src/uenux2/src/app/comum/appinfo/cappinfo.u35.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | média |
| 1501 | 318 | ✓ | `vota::CAssinadorVota::CAssinadorVota` | src/uenux2/src/app/vota/comum/cassinadorvota.cpp | uenux2/src/app/vota/comum/cassinadorvota.cpp | alta |
| 1552 | 254 |  | `std::pair<const char, ecourna::app::dados::CLabelParametrizado>::pair` | helper de biblioteca/inlinado: construtor de pair (char + {int, 4 strings}) | biblioteca | alta |
| 1555 | 49 |  | `std::filesystem::path::string` | helper de biblioteca/inlinado: retorna uma cópia da string nativa | biblioteca | alta |
| 1874 | 82 |  | `std::vector<char>::vector(const vector&)` | helper de biblioteca/inlinado: construtor de cópia de vector<char> | biblioteca | alta |
| 1880 | 1037 | ✓ | `ecourna::api::util::CStringUtils::Split` | src/ecourna/api/util/cstringutils.u35.cpp | ecourna-lib/ecourna/api/util/cstringutils.cpp | média |
| 1881 | 171 |  | `comum::CRelUtil::CompletaDireita` | src/uenux2/src/app/comum/relatorios/crelutil.u35.cpp | uenux2/src/app/comum/relatorios/crelutil.cpp | baixa |
| 1920 | 300 |  | `std::__sort4<..., md::CRespostaConsulta*>` | helper de biblioteca/inlinado: std::sort | biblioteca | alta |
| 1921 | 1346 |  | `comum::CRelUtil::FormataQtdAptos` | src/uenux2/src/app/comum/relatorios/crelutil.u35.cpp | uenux2/src/app/comum/relatorios/crelutil.cpp | média |
| 1922 | 83 |  | `std::function<comum::SQtdeAptos()>::function(const function&)` | helper de biblioteca/inlinado: construtor de cópia de std::function | biblioteca | alta |
| 1933 | 95 | ✓ | `comum::CLocal::GetLocalID` | src/uenux2/src/app/comum/dados/clocal.u35.cpp | uenux2/src/app/comum/dados/clocal.cpp | alta |
| 1935 | 9 |  | `comum::CEleitores::GetQtdHabilitadosPorBiografia` | src/uenux2/src/app/comum/dados/celeitores.u35.cpp | uenux2/src/app/comum/dados/celeitores.cpp | média |
| 1948 | 15 |  | `comum::CPath::GetPathChaves` | src/uenux2/src/app/comum/cpath.u35.cpp | uenux2/src/app/comum/cpath.cpp | média |
| 2188 | 76 |  | `ASN1::CHOICE::operator=(const CHOICE&)` | helper de biblioteca/inlinado: atribuição por cópia de CHOICE | biblioteca (runtime ASN.1 da III) | média |
| 2221 | 23 |  | `comum::CRegistradorMesario::Existe` | src/uenux2/src/app/comum/comparecimentomesario/cregistradormesario.u35.cpp | uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp | média |
| 2264 | 203 |  | `comum::CEleitores::Procura` | src/uenux2/src/app/comum/dados/celeitores.u35.cpp | uenux2/src/app/comum/dados/celeitores.cpp | média |
| 2266 | 28 |  | `comum::CEleitorDetalhe::PodeVotar` | src/uenux2/src/app/comum/dados/celeitordetalhe.u35.cpp | uenux2/src/app/comum/dados/celeitordetalhe.cpp | média |
| 2729 | 22 |  | `comum::CPedeTituloMesario::GetInst` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp (u22) | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | alta |
| 2742 | 15 |  | `comum::CCodecWsq::GetInst` | src/uenux2/src/app/comum/u35-foreign-fragments.cpp | uenux2/src/app/comum/reconhecimentobiometrico/ccodecwsq.cpp (?) | baixa |
| 2769 | 45 | ✓ | `std::vector<char>::__vallocate` | helper de biblioteca/inlinado | biblioteca | alta |
| 2790 | 1123 |  | `std::__insertion_sort_incomplete<..., md::CRespostaConsulta*>` | helper de biblioteca/inlinado: std::sort | biblioteca | alta |
| 2806 | 57 |  | `std::__tree<std::string>::destroy` | helper de biblioteca/inlinado: destroy de nó de set<string> | biblioteca | alta |
| 2816 | 158 |  | `comum::CLocal::GetQtdAgregadas` | src/uenux2/src/app/comum/dados/clocal.u35.cpp | uenux2/src/app/comum/dados/clocal.cpp | alta |
| 2821 | 9 |  | `comum::CEleitores::GetQtdHabilitadosPorBiometria` | src/uenux2/src/app/comum/dados/celeitores.u35.cpp | uenux2/src/app/comum/dados/celeitores.cpp | média |
| 2822 | 132 |  | `comum::CEleitores::GetQtdCompareceramSemBiometria` | src/uenux2/src/app/comum/dados/celeitores.u35.cpp | uenux2/src/app/comum/dados/celeitores.cpp | média |
| 2823 | 138 |  | `comum::CEleitores::GetQtdAptosSecao` | src/uenux2/src/app/comum/dados/celeitores.u35.cpp | uenux2/src/app/comum/dados/celeitores.cpp | média |
| 2885 | 895 |  | `comum::CCabecalhoQRCode::~CCabecalhoQRCode` | src/uenux2/src/app/comum/u35-foreign-fragments.cpp (comentário) | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp | alta |
| 3844 | 21 |  | `comum::CAppStateContext::CAppStateContext` | src/uenux2/src/app/comum/cappstate.h | uenux2/src/app/comum/cappstate.h (inferido) | alta |
| 3867 | 403 |  | `comum::CParteCandidatosProporcionais::~CParteCandidatosProporcionais` | src/uenux2/src/app/comum/relatorios/cpartecandidatos.u35.cpp | uenux2/src/app/comum/relatorios/cpartecandidatos.cpp | alta |
| 5741 | 1295 | ✓ | `comum::CLocal::GetTodasSecoes` | src/uenux2/src/app/comum/dados/clocal.u35.cpp | uenux2/src/app/comum/dados/clocal.cpp | alta |
| 5806 | 154 | ✓ | `comum::CCandidaturasDSSexo::operator()` | src/uenux2/src/app/comum/dados/ccandidaturas.u35.cpp | uenux2/src/app/comum/dados/ccandidaturas.cpp | alta |
| 5823 | 124 |  | `comum::CGravadorLog::~CGravadorLog` | src/uenux2/src/app/comum/gravadores/cgravadorlog.cpp | uenux2/src/app/comum/gravadores/cgravadorlog.cpp (inferido) | alta |
| 5872 | 12 |  | `comum::CCopiadorMR::~CCopiadorMR (deleting)` | src/uenux2/src/app/comum/gravadores/ccopiadormr.cpp | uenux2/src/app/comum/gravadores/ccopiadormr.cpp (inferido) | alta |
| 5873 | 606 |  | `comum::CCopiadorMR::CopiaDaMV` | src/uenux2/src/app/comum/gravadores/ccopiadormr.cpp | uenux2/src/app/comum/gravadores/ccopiadormr.cpp (inferido) | baixa |
| 5874 | 606 |  | `comum::CCopiadorMR::Copia` | src/uenux2/src/app/comum/gravadores/ccopiadormr.cpp | uenux2/src/app/comum/gravadores/ccopiadormr.cpp (inferido) | média |
| 10267 | 265 |  | `comum::asn::CConversorHashArquivo::DoConverte` | src/uenux2/src/app/comum/asn/cconversorhasharquivo.u35.cpp | uenux2/src/app/comum/asn/cconversorhasharquivo.cpp (inferido) | alta |
| 10276 | 241 |  | `comum::asn::CConversorHistoricoVotoImpresso::DoConverte` | src/uenux2/src/app/comum/gravadores/asn/cconversorhistoricovotoimpresso.cpp | uenux2/src/app/comum/gravadores/asn/cconversorhistoricovotoimpresso.cpp (inferido) | alta |
| 10290 | 321 |  | `comum::asn::CConversorCarga::DoDesconverte` | src/uenux2/src/app/comum/gravadores/asn/cconversorcarga.cpp | uenux2/src/app/comum/gravadores/asn/cconversorcarga.cpp (inferido) | alta |
| 10291 | 477 |  | `comum::asn::CConversorCarga::DoConverte` | src/uenux2/src/app/comum/gravadores/asn/cconversorcarga.cpp | uenux2/src/app/comum/gravadores/asn/cconversorcarga.cpp (inferido) | alta |
| 11204 | 12 |  | `comum::CParteEleitores::~CParteEleitores (deleting)` | src/uenux2/src/app/comum/relatorios/cparteeleitores.cpp | uenux2/src/app/comum/relatorios/cparteeleitores.cpp (inferido) | alta |
| 11206 | 75 |  | `comum::CParteEleitores::Imprime` | src/uenux2/src/app/comum/relatorios/cparteeleitores.cpp | uenux2/src/app/comum/relatorios/cparteeleitores.cpp (inferido) | alta |
| 11207 | 119 |  | `comum::CParteCargos::~CParteCargos (deleting)` | src/uenux2/src/app/comum/relatorios/cpartecargos.cpp | uenux2/src/app/comum/relatorios/cpartecargos.cpp (inferido) | alta |
| 11208 | 116 |  | `comum::CParteCargos::~CParteCargos` | src/uenux2/src/app/comum/relatorios/cpartecargos.cpp | uenux2/src/app/comum/relatorios/cpartecargos.cpp (inferido) | alta |
| 11210 | 10 |  | `comum::CParteCandidatosProporcionais::~CParteCandidatosProporcionais (deleting)` | src/uenux2/src/app/comum/relatorios/cpartecandidatos.u35.cpp | uenux2/src/app/comum/relatorios/cpartecandidatos.cpp | alta |
| 11222 | 1130 |  | `comum::CGeradorRelVersaoPacoteDados::MontaDados` | src/uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp | uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (inferido) | baixa |
| 11241 | 16 |  | `comum::CGeradorBUQRCodeVota::DadosEleicao` | src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u35.cpp | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp | média |
| 11273 | 4 |  | `comum::md::CRegraIdentidadeLivre::GetTipo` | src/uenux2/src/app/comum/dados/md/cregraidentidadelivre.cpp | uenux2/src/app/comum/dados/md/cregraidentidadelivre.cpp (inferido) | alta |
| 11274 | 49 |  | `comum::md::CRegraIdentidadeLivre::Formata` | src/uenux2/src/app/comum/dados/md/cregraidentidadelivre.cpp | uenux2/src/app/comum/dados/md/cregraidentidadelivre.cpp (inferido) | média |
| 11361 | 173 |  | `comum::asn::CConversorSituacoesEleicoes::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsituacoeseleicoes.cpp | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsituacoeseleicoes.cpp (inferido) | alta |
| 11377 | 2542 |  | `comum::asn::CConversorDetalheConsulta::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversordetalheconsulta.cpp | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversordetalheconsulta.cpp (inferido) | alta |
| 11384 | 218 |  | `comum::asn::CConversorHorarioVerao::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/municipiozona/cconversorhorarioverao.u35.cpp | uenux2/src/app/comum/dados/asn/municipiozona/cconversorhorarioverao.cpp | alta |
| 11386 | 86 |  | `comum::asn::CConversorNumViasImpressasRelatorios::DoConverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.cpp (inferido) | alta |
| 11387 | 44 |  | `comum::asn::CConversorNumViasImpressasRelatorios::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.cpp (inferido) | alta |
| 11388 | 143 |  | `comum::asn::CConversorLocalidadeEleitoral::DoConverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.cpp (inferido) | alta |
| 11399 | 594 | ✓ | `comum::asn::CConversorDadoCorrespondencia::DoConverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.cpp (inferido) | alta |
| 11400 | 1107 | ✓ | `comum::asn::CConversorDadoCorrespondencia::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.cpp (inferido) | alta |
| 11404 | 274 | ✓ | `comum::asn::CConversorDadoLocal::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.u35.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.cpp | alta |
| 11413 | 167 |  | `comum::asn::CConversorSeguranca::DoConverte` | src/uenux2/src/app/comum/asn/cconversorseguranca.cpp | uenux2/src/app/comum/asn/cconversorseguranca.cpp (inferido) | alta |
| 11437 | 2893 |  | `comum::asn::CConversorCabecalhoPacote::DoDesconverte` | src/uenux2/src/app/comum/asn/cconversorcabecalhopacote.u35.cpp | uenux2/src/app/comum/asn/cconversorcabecalhopacote.cpp | alta |
| 11441 | 922 |  | `comum::asn::CIndexadorFotos::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/candidatura/cindexadorfotos.cpp | uenux2/src/app/comum/dados/asn/candidatura/cindexadorfotos.cpp (inferido) | alta |
| 11448 | 1570 | ✓ | `comum::asn::CConversorCandidatura::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.cpp | uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.cpp | alta |
| 11450 | 3280 | ✓ | `comum::asn::CConversorDadosCandidato::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/candidatura/cconversordadoscandidato.cpp | uenux2/src/app/comum/dados/asn/candidatura/cconversordadoscandidato.cpp (inferido) | alta |
| 11453 | 1694 | ✓ | `comum::asn::CConversorSecaoEleitoral::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.u35.cpp | uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.cpp | alta |
| 11456 | 116 |  | `comum::asn::CConversorIdentificacaoAgregada::DoConverte` | src/uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.u35.cpp | uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.cpp | alta |
| 11460 | 290 |  | `comum::asn::CConversorPartido::DoConverte` | src/uenux2/src/app/comum/dados/asn/cconversorpartido.u35.cpp | uenux2/src/app/comum/dados/asn/cconversorpartido.cpp | alta |
| 11496 | 257 |  | `comum::CRdvPosicionadorVota::Posiciona` | src/uenux2/src/app/comum/dados/crdvposicionadorvota.cpp | uenux2/src/app/comum/dados/crdvposicionadorvota.cpp (inferido) | média |
| 11569 | 176 |  | `comum::CServicoEstadoGeral::GetPathArquivo` | src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeral.cpp | uenux2/src/app/comum/appinfo/servicos/cservicoestadogeral.cpp (inferido) | alta |
| 11580 | 90 |  | `comum::CGravadorVersoesArquivos::~CGravadorVersoesArquivos (deleting)` | src/uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.u35.cpp | uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.cpp | alta |
| 11581 | 87 |  | `comum::CGravadorVersoesArquivos::~CGravadorVersoesArquivos` | src/uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.u35.cpp | uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.cpp | alta |
| 11583 | 10 |  | `comum::CGravadorLog::~CGravadorLog (deleting)` | src/uenux2/src/app/comum/gravadores/cgravadorlog.cpp | uenux2/src/app/comum/gravadores/cgravadorlog.cpp (inferido) | alta |
| 11585 | 61 |  | `comum::CGravadorRDV::~CGravadorRDV (deleting)` | src/uenux2/src/app/comum/gravadores/cgravadorrdv.cpp | uenux2/src/app/comum/gravadores/cgravadorrdv.cpp (inferido) | alta |
| 11586 | 14 |  | `comum::CGravadorRDV::~CGravadorRDV` | src/uenux2/src/app/comum/gravadores/cgravadorrdv.cpp | uenux2/src/app/comum/gravadores/cgravadorrdv.cpp (inferido) | alta |
| 11591 | 10 |  | `comum::asn::CConversorCodigoCargoConsulta::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.cpp | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.cpp (inferido) | alta |
| 11592 | 84 |  | `comum::asn::CConversorCodigoCargoConsulta::DoConverte` | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.cpp | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.cpp (inferido) | alta |
| 11621 | 100 |  | `comum::CGravadorEnvelopeArquivo::~CGravadorEnvelopeArquivo (deleting)` | src/uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.cpp | uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.cpp (inferido) | alta |
| 11622 | 97 |  | `comum::CGravadorEnvelopeArquivo::~CGravadorEnvelopeArquivo` | src/uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.cpp | uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.cpp (inferido) | alta |
| 11625 | 15 |  | `comum::IGravadorEnvelope::~IGravadorEnvelope` | src/uenux2/src/app/comum/gravadores/igravadorenvelope.cpp | uenux2/src/app/comum/gravadores/igravadorenvelope.cpp (inferido) | alta |
| 11630 | 1207 |  | `comum::IGravador::CopiaParaMV` | src/uenux2/src/app/comum/gravadores/igravador.cpp | uenux2/src/app/comum/gravadores/igravador.cpp (inferido) | média |
| 11631 | 608 |  | `comum::IGravador::CopiaResultadoParaMV` | src/uenux2/src/app/comum/gravadores/igravador.cpp | uenux2/src/app/comum/gravadores/igravador.cpp (inferido) | média |
| 11632 | 608 |  | `comum::IGravador::CopiaParaResultado` | src/uenux2/src/app/comum/gravadores/igravador.cpp | uenux2/src/app/comum/gravadores/igravador.cpp (inferido) | média |
| 11633 | 438 |  | `comum::IGravador::GravaMV` | src/uenux2/src/app/comum/gravadores/igravador.cpp | uenux2/src/app/comum/gravadores/igravador.cpp (inferido) | média |
| 11634 | 438 |  | `comum::IGravador::Grava` | src/uenux2/src/app/comum/gravadores/igravador.cpp | uenux2/src/app/comum/gravadores/igravador.cpp (inferido) | média |
| 11638 | 12 |  | `comum::CCopiadorMR::~CCopiadorMR` | src/uenux2/src/app/comum/gravadores/ccopiadormr.cpp | uenux2/src/app/comum/gravadores/ccopiadormr.cpp (inferido) | alta |
| 11900 | 12 |  | `comum::CGeradorRelPU::~CGeradorRelPU (deleting)` | src/uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp | uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp (inferido) | alta |
| 11901 | 12 |  | `comum::CGeradorRelPU::~CGeradorRelPU` | src/uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp | uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp (inferido) | alta |
