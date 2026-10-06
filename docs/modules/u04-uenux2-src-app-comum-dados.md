# u04: `uenux2/src/app/comum/dados`: cargos, configuração da eleição, eleitores, local, horário de verão, processo eleitoral

A unidade u04 contém 83 funções wasm. O montador de unidades as atribuiu a oito arquivos originais do diretório
`uenux2/src/app/comum/dados/` (`comum` = código compartilhado pelos aplicativos da urna, `dados` = dados):

| arquivo | classe | o que é no processo de votação |
|---|---|---|
| `ccargos.cpp` | `comum::CCargos` | cursor sobre os **cargos** (Presidente, Governador, Prefeito, Vereador… e perguntas de referendo, as *consultas*) de cada **eleição** do dia. As telas do eleitor o percorrem, assim como o BU, o RDV e os QR codes do BU. |
| `cconfiguracaoeleicao.cpp` | `comum::CConfiguracaoEleicao` | configuração das eleições nesta urna: *processo eleitoral*, *pleito* (as eleições realizadas no mesmo dia), cargos, versões dos pacotes de dados, identificação da seção. |
| `celeitordetalhe.cpp` | `comum::CEleitorDetalhe` | tudo o que se sabe sobre **um eleitor**: dados estáticos do cadastro, dados dinâmicos de comparecimento, impedimentos. |
| `celeitores.cpp` | `comum::CEleitores` (+ fontes de texto `CEleitorDado*`) | a lista de eleitores da seção (*cadastro de eleitores da seção*). Cria e carrega a tabela SQLite "dinâmica" de eleitores, conta os eleitores que podem votar (*aptos*) e registra como um eleitor foi *habilitado*. |
| `chv.cpp` | `comum::CHV` | *horário de verão* do município. |
| `cintegridadereferencial.cpp` | `comum::CIntegridadeReferencial` | transforma uma verificação de integridade referencial que falhou em uma exceção. |
| `clocal.cpp` | `comum::CLocal` | onde a urna está: uma seção eleitoral ou uma *urna de contingência*. |
| `cpe.cpp` | `comum::CPE` | o *processo eleitoral* lido de `<fase><pe>-cp.dat`, usado pelos cabeçalhos de relatórios. |

Cerca de um terço das funções (29 de 83) vem de **outros** arquivos originais. Elas acabaram na u04 porque
inlinam ou chamam essas classes. A mais importante é a **func 5604, o gerador de QR codes do BU** (22.8 KB,
`comum::CGeradorBUQRCode`). O banco de dados a nomeou `CCargos::GetCurrentEleicaoVersaoPacote` porque ela inlina
esse método de 6 linhas. Veja a §6. Todas elas estão nomeadas na tabela de mapeamento (§10). Quando o arquivo real pertence a
outra unidade, a reconstrução fica em um arquivo de fragmento.

Arquivos-fonte reconstruídos:

* `src/uenux2/src/app/comum/dados/{ccargos,cconfiguracaoeleicao,celeitordetalhe,celeitores,chv,cintegridadereferencial,clocal,cpe}.{h,cpp}`
* `src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp`: QR codes do BU (funcs 5603–5608, 5616, 5617, 6028)
* `src/uenux2/src/app/comum/dados/u04-foreign-fragments.cpp`: as demais funções de outros arquivos, agrupadas por arquivo original

Dez funções foram executadas durante os votos gravados (`analysis/runtime`): 187, 273, 332, 861, 1938, 5764, 11724,
12742, 12811 e 13550. Todas elas desenham as telas de votação (cargo corrente, número do candidato, barra de progresso,
próximo estado depois de digitar um número) ou executam `CEleitores::StaticLoad` na inicialização (5764).

---

## 1. Classes e relações

Nenhuma das oito classes da unidade tem RTTI, exceto `CEleitores`. São singletons simples: um ponteiro estático
mais um mutex, `GetInst()` e `CreateInst()`.

```
comum::CEleitores  [vmi, typeinfo @1559444, vtable @1559120: exactly ONE slot]
 ├─ api::CDataMap<md::CEleitorIdentidade, comum::CEleitorDetalhe>  (non-polymorphic, base #0) -> at +4
 │     std::map (+4 begin, +8 root, +12 size), current iterator (+16), record name "CEleitores" (+20)
 └─ comum::IDataSourceAptos            (polymorphic, base #1, 1 pure virtual, no virtual dtor) -> vptr at +0
       slot 0: std::map<md::ETipoAbrangencia, SQtdeAptos> GetQtdAptos() const   = wasm 2824
   (the vmi base list gives the declaration order, CDataMap first; the only dynamic base becomes the
    primary base at offset 0, so the layout order is the reverse of the declaration order)

std::function lambdas with RTTI (the loaders, not in u04):
   CEleitores::GetEleitoresEstaticos(const std::string&)::$_0            -> vector<md::CEleitor>
   CEleitores::GetEleitoresImpedidos(const std::vector<std::string>&)::$_0 -> vector<md::CImpedido>

non-polymorphic singletons (object size, address of the static pointer):
   CCargos 28 B @1838720   CConfiguracaoEleicao @1838752   CEleitores @1838792
   CHV 28 B @1838872       CPE 204 B @1839112              CLocal 4 B @1838900 (lazy, comum_f401)
error types: CBaseError<comum::EUeComumDadosError>  typeinfo @1528076, vtable @1528096 (thunk comum_f170)
             CBaseError<ecourna::api::pattern::EPatternErr> (singletons: 1303 "… - instancia nao criada",
                                                             1304 "… - instancia ja criada")
```

Layouts de objetos recuperados a partir dos acessos da u04 (detalhes nos headers):

| objeto | layout |
|---|---|
| `CCargos` | `+0 size_t m_indice`, `+4 vector<{TEleicaoID eleicao; TCargoID cargo}> m_todos`, `+16 vector<…> m_cargos` (filtrado para o eleitor corrente) |
| `CConfiguracaoEleicao` | `+0 id processo` (QR `PROC`), `+20 origem` (2 = comunitária), `+28 md::CPleito` (`+28 id`, `+44 data`, `+52 vector<CEleicaoPE>` de 52 bytes, `+64 map<TEleicaoID,string>` versões dos pacotes), `+140 uint8` limite de verificações de dado do eleitor, `+620…` identificação da seção usada nos nomes de arquivos (fase, processo, UF, município, zona) |
| `CEleitorDetalhe` (204 B) | `+0 md::CEleitorDecorator` (108 B), `+108 optional<md::CEleitorDinamico>` (84 B, flag `+192`), `+196 impedimentoP1`, `+200 impedimentoP2` |
| `md::CEleitorDecorator` | `+0 sequencial`, `+4 uint16 seção`, `+8 vector<CEleitorIdentidade>`, `+24 nome`, `+36 nomeSocial`, `+48 tipo de transferência temporária` (0 = nenhuma), `+52 UF` e `+64 município` do domicílio, `+80 optional<{bytes, offset, tamanho}>` localização da biometria cifrada (flag `+100`), `+104 tipo do identificador principal` |
| `md::CEleitorDinamico` (84 B, uma linha de `eleitor_dinamico`) | `+0 título`, `+16 identidade usada na habilitação`, `+32 estado de comparecimento`, `+36 tipo de habilitação`, `+40 dedo`, `+44 score`, `+46 tentativas`, `+48 tipo de ativação do áudio` (padrão 2), `+52 erro ao decifrar biometria`, `+56 optional título do mesário`, `+76/+80 apresentação da foto` |
| `CEleitores` | `+32 UF` e `+44 município` da urna, `+48 bool` urna biométrica (StaticLoad lança 7840 para um registro do cadastro com biometria quando ele é false), `+52 set<ETipoAbrangencia>` dos cargos votados aqui, `+64`/`+76 vector<string>` nomes dos arquivos de cadastro lidos por `GetEleitoresEstaticos`, `+88 vector<string>` arquivos de impedimentos, `+100 tipo do identificador principal`, `+104` `qtd votaram` de 32 bits (stores i32), `+108 bool dinâmicos carregados`, `+112 map<identidade, identidade principal>` |
| `CLocal` | `+0 unique_ptr<md::CLocal>`. `md::CLocal`: UF `+16`, `optional<CSecaoEleitoral>` em `+84` (zona `+96`, seção `+104`, flag `+120`), `optional<CIdentificacaoUrnaContingencia>` em `+124` (flag `+136`) |
| `CPE` | `+0 md::CProcessoEleitoral` (176 B: `pleito1 +36`, `optional pleito2 +96`), `+176 turno`, `+180 {fase, processo, string, const CPleito*}` |

Enumerações cujos números vêm do código. Os arquivos ASN.1 usam números a partir de 1; os enums `md` em memória
começam em 0:

* `md::ETipoAbrangencia`: `0 MUNICIPAL`, `1 ESTADUAL`, `2 FEDERAL`. Isso corresponde ao `TipoAbrangencia` do ASN.1; `DesconverteAbrangencia` (5827) é a identidade.
* `md::ETipoCargo` (`CCargo +4`): `0` majoritário, `1` proporcional, `2` consulta (`CCargo::Valida`). O campo `TIPO:` do QR do BU imprime esse valor.
* `md::EEstadoComparecimento` (`CEleitorDinamico +32`): `0` faltou, `1` sem cargo para votar, `2` habilitado mas não votou, `3` votou. Os nomes são inferidos; `VOTOU == 3` é certo, pois é testado em cinco lugares.
* `md::ETipoHabilitacao` (`+36`): `0` sem biometria, `1` biometria reconhecida, `2` habilitado com o código do mesário.

## 2. Cargos: `CCargos` e `CConfiguracaoEleicao`

`CCargos::CreateInst` (inlinada na função de inicialização 7787) monta `m_todos` e `m_cargos`: uma entrada para
cada cargo de cada eleição do pleito (`comum_f3774`). Ela ordena `m_cargos` na **ordem de aquisição**
(a ordem em que o eleitor é consultado): `ordemAquisicao` da eleição, depois `CCargo::ordemAquisicao` (+15), com
o comparador 11559.

* Votação (`vota::CEleitorVotando::IniciaCiclo`, u06): `FiltraPorAbrangencia(voter's abrangência)` (§3.2), depois
  `First()` (2269). Em seguida as telas chamam `GetCurrent` (332), `GetCurrentCargoID` (1938), … e `Next`
  (1708) após cada cargo confirmado.
* Resultados (BU, RDV, QR code): `CGeraBU` (12110) e `CGravaResultado` (12098) primeiro restauram a lista completa
  (comum_f3784: `m_cargos = m_todos`, desfazendo o filtro do último eleitor), depois `OrdenaPorOrdemImpressao`
  (comum_f3782, comparador 11558, `CCargo +16`), depois `First/IsEnd/Next`.

Todo acessor verifica o índice e lança `EUeComumDadosError` 7816–7820 `"Cargo não posicionado."`. `Next`
lança 7815 `"Operação inválida."` se for chamado no fim.

`CConfiguracaoEleicao::GetCargo(id)` (861) busca linearmente nos cargos de cada eleição do pleito (140 B
cada) e lança 7827 `"Cargo não encontrado {}"`. `GetEleicao(id)` (inlinada, linha 293) lança 7826
`"Eleição não encontrada {}"`.

## 3. Eleitores: `CEleitores` / `CEleitorDetalhe`

### 3.1 Ciclo de vida

```
votaInit -> 7787 (start-up)
   CEleitores::StaticLoad(estatico/)          [lines 395/407 = errors 7840/7841, inlined in 7787]
       GetEleitoresEstaticos: *-el.dat + *-tte.dat  (ASN.1 ModuloEleitores::EntidadeEleitores)
       GetEleitoresImpedidos: *-imp.dat            (ModuloImpedidos::EntidadeImpedidos)
       LoadEleitorDetalhe(decorator, impedidos, idx, nullptr) per voter (5764, observed)
   -> if EstadoVota >= '3': CInformacaoEleitor::CarregarDadosDinamicos (6734), see 3.4
votaInit / vota::CGeraDadosDinamicos::StartState  ("gera base dinâmica", EstadoVota 1 -> 3)
   6737 (CInformacaoEleitor::GerarDadosDinamicos, u02's name): unless fase '3' + treinamento-do-eleitor:
       CEleitores::DynamicCreate(GetPathTrab(interna, turno)/uenux.db)   i.e. /dsk/fi/dinamico/trabN/  (5761)
       CEleitores::DynamicCreate(GetPathTrab(externa, turno)/uenux.db)   i.e. /dsk/fe/dinamico/trabN/
   6734: if NOT treinamento-do-eleitor: CompleteLoad(estatico/, GetPathTrab(interna, turno)/uenux.db)
poll worker enables the voter -> CEleitores::MarcaEleitorFoiHabilitado (2825)
voter finishes                -> CEleitores::MarcaVotou (lines 456/460, inlined in 10210)
end of day                    -> GetQtdAptos (2824), QtdVotaramPorTipoHabilitacao (6034) feed BU / QR / RDV
```

### 3.2 Abrangência de um eleitor (func 3721, corpo de `GetCurrentAbrangenciaEleitor`, linha 497)

Eleitores com *transferência temporária de eleitor* (TTE) votam em uma seção fora do seu domicílio. Exemplo:
o *voto em trânsito*. O tipo de TTE fica em `decorator +48`. A regra:

* sem TTE, ou município do domicílio = município da urna → `MUNICIPAL`: todos os cargos;
* mesma UF da urna → `ESTADUAL`: cargos estaduais e federais;
* caso contrário → `FEDERAL`: apenas cargos federais (por exemplo, Presidente).

`CCargos::FiltraPorAbrangencia` aplica essa regra à lista de cargos para cada eleitor.

### 3.3 `DynamicCreate` (5761): criando a tabela SQLite de estado dos eleitores

1. Lança 7837 `"Não há informações carregadas"` se o cadastro está vazio, e 7838 `"Informações dinâmicas já carregadas"` se o primeiro eleitor já tem dados dinâmicos.
2. Calcula quais abrangências têm cargos nesta urna (`find` linear sobre o set em `+52`).
3. Para cada eleitor, monta `CEleitorDinamico(título, título, estado)` (2800). `estado` = **0 (faltou)** se o eleitor tem pelo menos um cargo em que pode votar (§3.2), **1 (sem cargo para votar)** caso contrário. Os campos biométricos são zerados, áudio = 2, erro = 0.
4. Abre `dao::CEleitorDinamicoDAO(uenux.db)`, que cria a tabela `eleitor_dinamico` (`titulo BIGINT PRIMARY KEY…`). Se a tabela está vazia, insere todas as linhas em uma única transação. Se não está vazia, ordena as duas listas e exige que sejam **iguais** (`operator==` campo a campo; os campos biométricos só são comparados quando `estado ∈ {2,3}` e `tipo == 1`). Caso contrário, lança 7839 `"Já existem informações dinâmicas no banco {}"`.

### 3.4 `CompleteLoad` (6734) e seu wrapper

A func 6734 carrega os srclocs de `CEleitores::CompleteLoad(const std::string&, const std::string&)` (linhas
205/219/238), mas a função é um **wrapper**. Seu `this` é o singleton de 12 bytes retornado pela func 509,
que u02/u08 chamam de `vota::CInformacaoEleitor`; a u02 chama este método de `CarregarDadosDinamicos()` (byte `+1`
= "dados dinâmicos carregados"). O wrapper faz cinco coisas:

1. carrega o **`rdv.dat`** (cifrado, `api::CEncryptedFile`) em `CRdvVota` (`Desconverte`, slot 12);
2. **somente se `CEstadoGeralVota.treinamentoEleitor` for false** (`+72`, o sexto componente ASN.1, confirmado no conversor 11390), executa `CEleitores::CompleteLoad`:
   * 7834 `"Não existe o arquivo {}"` se `uenux.db` não existe;
   * lê os impedimentos (lista de arquivos em `+88`), o cadastro estático e todas as linhas de `eleitor_dinamico`, e ordena impedimentos e linhas por identidade (comparadores via ponteiro de função 11532/11531);
   * exige o mesmo número de registros estáticos e dinâmicos, com identidades iguais par a par; senão, 7835 `"Havia divergência entre os títulos dinâmicos e os estáticos"`;
   * monta um novo map `identity → CEleitorDetalhe` com `ValidaIdentidadesEleitor` (5765: 7852 `"Identificador principal ({}) ausente para o eleitor ({})"`) e `LoadEleitorDetalhe` (5764), e lança 7836 `"Identificador principal {} duplicado"` em caso de chave duplicada;
   * preenche `map<any identifier → principal identifier>` (título, CPF ou número livre levam todos ao eleitor), conta as linhas `VOTOU` em `m_qtdVotaram` e liga `m_dinamicosCarregados`;
3. carrega as **justificativas** (SQLite `registro_justificativa`) em `CJustificador`, através do seu membro `comum::servico::CJustificadorServico` (+28);
4. carrega o **comparecimento dos mesários** (SQLite `comparecimento_mesario`) em `CDAORepositorio<CComparecimentoMesario>`, através do seu membro `comum::servico::CComparecimentoMesarioServico` (+40). As duas chamadas `SelectAll` passam pelo mesmo corpo mesclado por ICF 5723 (`m_dao->SelectAll()`, slot 8 do DAO);
5. executa a verificação de integridade referencial (`comum_f2543` → `CIntegridadeReferencial::Lanca` 2263: 7871 `"Integridade referencial: {}"`) e então liga a flag de carregado.

### 3.5 `LoadEleitorDetalhe` (5764): merge-join com os impedimentos

Eleitores e impedimentos são percorridos em ordem de identidade. `idx` aponta para o próximo impedimento:

* identidade igual → pega P1 (`+20`) e P2 (`+24`, se presente; senão 0), `++idx`;
* identidade do eleitor **maior** → o impedimento não corresponde a nenhum eleitor → 7851 `"CEleitores::LoadEleitorDetalhe - ({}-{} > {}-{}). {} com marcação de impedido não encontrado nas listas de eleitores e de tte"`;
* menor → sem impedimento.

Em seguida monta o `CEleitorDetalhe`. Com dados dinâmicos, o construtor (celeitordetalhe.cpp:49) verifica se o
título da linha é igual à identidade principal do eleitor; senão, 7829 `"CEleitorDetalhe - identidades divergentes"`.
`CEleitorDetalhe::PodeVotar` (comum_f2266) usa P2 quando o *turno* do CDadoCarga é `'2'`, senão P1.

### 3.6 Outras funções de `CEleitores` / `CEleitorDetalhe`

* `GetQtdAptos` (2824): `map{MUNICIPAL, ESTADUAL, FEDERAL} → SQtdeAptos{uint16 seção, uint16 TTE}`. Todo eleitor sem impedimento no turno corrente é contado na sua própria abrangência **e em todas as mais amplas**, no contador `TTE` se `decorator +48 != 0`. O QR code do BU (`APTA/APTS/APTT`), a linha "eleitores aptos" do BU (11968) e `CGravaResultado` o usam.
* `MarcaEleitorFoiHabilitado` (2825): 7842 `"Item inexistente"`, 7843 `"Dados dinâmicos não carregados"`, 7844 `"Eleitor já votou"`. Em seguida executa o `CEleitorDetalhe::MarcaHabilitado` inlinado (um `br_table` sobre o tipo de habilitação): para os tipos 0/1/2, define estado ← 2 e tipo, e dedo/score/tentativas/erro/título do mesário conforme o tipo (o tipo 2 mantém ou limpa o título do mesário dependendo de os dados de habilitação trazerem um). **Qualquer outro valor cai no default do `br_table`, que pula tudo isso**: estado, tipo e os campos biométricos mantêm os valores anteriores (estado continua `0` faltou), e o eleitor não é registrado como habilitado. Em todos os casos, o tipo de áudio, a identidade de habilitação e o resultado da foto são copiados.
* `MarcaVotou` (linhas 456/460, inlinada em 10210 `CSincronismoOperador::vf2`): 7845 `"Item inexistente"`, 7846 `"Dados dinâmicos não carregados"`; se a linha ainda não é `VOTOU`, define estado ← 3 e incrementa o contador de 32 bits em `+104`; caso contrário, não faz nada. A 10210 a pula inteiramente na fase `'3'` + treinamento-do-eleitor.
* `QtdVotaramPorTipoHabilitacao(tipo)` (6034, via 2821 = tipo 1, 1935 = tipo 2): eleitores com dados dinâmicos, desse tipo e com `estado == VOTOU`. São os campos de QR `HBBM` / `HBBG`; `HBSB` = tipo 0 vem da gêmea comum_f2822.
* `GetDinamico` (1271) / `ConfereDadosDinamicos` (3770): 7832 `"{}: Dados dinâmicos não carregados"`.
* `GetBiometria` (1937): a biometria cifrada é **lida sob demanda**. O arquivo do cadastro (`<fase><pe><uf><mun><zona><seção>-el.dat`, ou `-tte.dat` quando o eleitor está em TTE diferente do tipo 4 *acessibilidade*) é aberto com `"rb"`, posicionado no offset armazenado, e `tamanho` bytes são lidos. Os bytes são decodificados em BER como `ModuloEleitores::BiometriaEleitorCifrada` e decifrados por `asn::CConversorBiometriaEleitorCifrada` (slot 3 da vtable) com a identidade do eleitor. Erros: 7830 `"Eleitor corrente não tem biometria."`, 7831 `"Arquivo {} não existe"`, 7669/7670 `"CLeitorASN::LeBiometriaEleitor(<id>) - não foi possível …"`, 5953/5954 erros de decodificação, 7658 `"Entidade está inválida: {}"`.
* Fontes de texto para a tela do mesário: `CEleitorDadoNomeParaUrna::Text(fmt)` (nome social se definido, senão nome, cortado em 40), `CEleitorDadoSequencial`, `CEleitorDadoSecao`, `CEleitorDadoTTE::Text()` → `"Transferência Temporária"` ou `""`. Cada uma lança 785x `"Não posicionado no eleitor corretamente"` sem eleitor corrente. Elas usam `std::vformat` porque a string de formato vem do layout da tela.

## 4. `CLocal`, `CHV`, `CPE`, `CIntegridadeReferencial`

* `CLocal` (singleton lazy, nunca lança em `GetInst`). `VerificaLido(func)` (782) lança 7874 `"{}: o arquivo de locais ainda não foi carregado"`. `GetZonaID` (1003, passa `"GetZona"`) obtém a zona da seção ou da identificação de contingência. `GetSecaoID` (1078) retorna **0 para uma urna de contingência**. Fora desses casos, ambas lançam 7872/7873 `"Tipo de local inválido"`. `VerificaEhSecao` (5742) lança 7875 `"{}: o local não era de seção"`.
* `CHV::GetHorarioVerao` (5745, nome inferido: ela passa `"GetHorarioVerao"`) lança 7870 `"{} - não entra em horário de verão"`, e então o `CHorarioVeraoMunicipio::GetHorarioVerao` inlinado lança 8005. Só o relatório de autoteste da impressora a chama.
* `CPE::CreateInst` (2787) monta seu caminho a partir dos dados da seção, por exemplo `estatico/t02400-cp.dat` (`FormataFase + FormataNumero(pe,5) + "-cp.dat"`). Lê `ModuloProcessoEleitoral::EntidadeProcessoEleitoral` e guarda o pleito do turno corrente (turno `'1'` → pleito1, senão `GetPleito2()`). Lança 7876 `"Instância já criada"`. Os chamadores a criam sob demanda (testam comum_f2788 antes). O novo objeto recebe uma **cópia** do processo eleitoral lido (o temporário é destruído com 2830 depois). 5598 é `~CPE`, 2830 é `~CProcessoEleitoral`, e 11226 libera o singleton (provavelmente na saída; nenhum código referencia seu slot na tabela).

## 5. Dados lidos e escritos

| dados | formato | onde |
|---|---|---|
| `estatico/<fase><pe><uf><mun5><zona4><sec4>-el.dat`, `-tte.dat` | ASN.1 `ModuloEleitores` (cadastro de eleitores, blob de biometria cifrada por eleitor) | loaders de StaticLoad / CompleteLoad, `GetBiometria` (leitura parcial no offset) |
| `estatico/…-imp.dat` | ASN.1 `ModuloImpedidos` (`impedimentoP1/P2`) | `GetEleitoresImpedidos` |
| `estatico/<fase><pe>-cp.dat` | ASN.1 `ModuloProcessoEleitoral` | `CPE::CreateInst` |
| `dinamico/trab{1,2}/uenux.db` tabela `eleitor_dinamico` | SQLite: `titulo, tipo_identificador, identidade_habilitacao, tipo_identidade_habilitacao, estado_comparecimento, tipo_habilitacao, dedo_habilitacao, score_habilitacao, numero_tentativa, erro_decifrar_biometria, tipo_ativacao_audio, estado_apresentacao_foto, resultado_decifracao_foto, titulo_mesario, tipo_identificador_mesario` | `DynamicCreate` (INSERT em uma transação), `CompleteLoad` (SELECT) |
| `uenux.db` tabelas `registro_justificativa`, `comparecimento_mesario` | SQLite | o wrapper 6734 |
| `dinamico/…/rdv.dat` | RDV cifrado | o wrapper 6734 |
| `vota.bin` `treinamentoEleitor` | ASN.1 `ModuloEstadoGeralVota` | decide se os dados dinâmicos dos eleitores sequer existem |

## 6. Boletim de Urna: os QR codes (func 5604 e auxiliares)

O **BU** impresso (*Boletim de Urna*, a fita com os resultados da seção) termina com um ou mais QR codes.
`vota::CMostraQRCodeBU` também os mostra na tela. Os payloads de texto deles são produzidos por
**`comum::CGeradorBUQRCode::GeraQRCodes(size_t tamanhoMaximo)`**. O nome do método é inferido; a classe e o
arquivo são certos pelo srcloc `cgeradorbuqrcode.cpp:405 RetornaBlocoAssinado`. Reconstrução:
`src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp`.

**Chamador.** `vota::CGeraBU::StartState` (12110, geração do BU no *encerramento*) faz quatro coisas:

1. Monta o cabeçalho com os setters de `CCabecalhoQRCodeBuilder` (ZONA 5618, SECA 5620, IDUE 5621, IDCA 5623, HICA/HIQT 5622).
2. Constrói `CGeradorBUQRCodeVota(cabecalho, comparecimento, dataEmissao)` (5603). O construtor da base copia o cabeçalho (5608, 33 strings), guarda `CRdvVota` como fonte dos votos e armazena `CEleitores::GetQtdAptos()`. `comparecimento` = máximo, entre as eleições, de `CVotosEleicoesVota::Comparecimento`.
3. Chama `GeraQRCodes(1100)`. O resultado é `{vector<string> conteudos; string assinatura}`.
4. Para cada payload, acrescenta uma legenda `i/n` e `CQRCodeImagePaper::MontaImagem(payload)` ao BU impresso.

**Passo 1: cabeçalho.** Ele é montado a partir de uma cópia dos 33 campos do builder. Cada campo é uma string `"TAG:value "`.

| tag | valor | origem |
|---|---|---|
| `ORIG` | `VOTA`, ou `RED` quando a flag `+418` do gerador está ligada (outros aplicativos: `SA` = *sistema de apuração*; `RED` = *recuperador de dados*) | `PreencheCabecalho` virtual (11242, u35) |
| `ORLC` | `LEG` ou `COM` (origem da configuração oficial/comunitária) | `CConfiguracaoEleicao +20 == 2` |
| `PROC` | id do processo eleitoral | config `+0` |
| `DTPL` | data do pleito `YYYYMMDD` | data do `CPleito` via `CDate::Format` |
| `PLEI` | id do pleito | config `+28` |
| `TURN` | `1`/`2` (char) | dado de carga do `CEstadoGeral` |
| `FASE` | `{:c}` caractere da fase em maiúscula (O/S/T) | `CDadoCarga::GetFaseChar` |
| `UNFE` | `{:2s}` UF, convertida para maiúsculas com mapa de acentos (comum_f3509) | `CLocal::GetUF` |
| `MUNI`, `ZONA`, `SECA`, `AGRE` | município, zona, seção, seções agregadas (`AGRE` só se não vazio) | 11242 / builder |
| `IDUE`, `IDCA`, `HICA` (`HIQT`) | id da urna, código de carga (`{:.24s}`), histórico de cargas | builder |
| `VERS` | `GetVersionNumber("10.23.0.1 - DESENVOLVIMENTO")` → `10.23.0.1` neste build | 5604 |
| `LOCA`, `APTO`, `APTS`, `APTT`, `COMP`, `FALT` | local de votação, aptos (total, seção, TTE), comparecimento, faltosos (= aptos − comparecimento) | 11242 |
| `HBBM`, `HBBG`, `HBSB` | eleitores que votaram, habilitados por biometria / pelo código do mesário / sem biometria (escritos só quando comum_f820 é true, provavelmente "a eleição usa biometria"; não obrigatórios) | 11242 via 6034 (§3.6) |
| `DTAB`, `HRAB`, `DTFC`, `HRFC` | data/hora de abertura e de encerramento (dhIni/dhFimAquisicao de `CEstadoGeralVota`; senão `"Início/Fim da aquisição não marcado"`) | 11242 |
| `DTEM`, `HREM` | data/hora de emissão: **apenas nos payloads de RED/SA** (e `JUNT`/`TURM` para o SA) | 11242 |

A validação (lambda 2791 de `CCabecalhoQRCodeBuilder::preBuild`) verifica todos os campos obrigatórios. Um campo vazio lança
`EUeComumRelatoriosError` 9050 `"Campo ({}) não informado."`. Para o VOTA, os campos obrigatórios são Origem,
OrigemProcessoEleitoral, ProcessoEleitoral, DataPleito, Pleito, Turno, Fase, Uf, Municipio, Zona, Secao, IdUrna,
CodigoCarga, HistoricoCarga, VersaoSoftware, Local, QtdeAptos, QtdeAptosDaSecao, QtdeAptosTTE, QtdeCompareceram
e QtdeFaltosos. Ordem de concatenação: `ORIG ORLC PROC DTPL PLEI TURN FASE UNFE MUNI ZONA SECA [AGRE] IDUE IDCA
HICA VERS LOCA APTO APTS APTT COMP FALT HBBM HBBG HBSB DTAB HRAB DTFC HRFC` (o RED acrescenta `DTEM HREM`; o SA usa
`JUNT TURM DTEM HREM` em vez de `LOCA…HRFC`).

**Passo 2: corpo.** Os cargos são percorridos na ordem de impressão (`OrdenaPorOrdemImpressao`, `First/IsEnd/Next`):

* quando a eleição muda: `IDEL:<id> ` + slot virtual 3 (vazio para o VOTA);
* por cargo: `CARG:<código> TIPO:<0 maj|1 prop|2 consulta> `, depois `VERC:<versão do pacote>` (cargos com candidatos: `CCandidaturas::RecuperaVersaoPacote`; consultas: `CCargos::GetCurrentEleicaoVersaoPacote` → `CPleito::GetVersaoPacoteEleicao`, 8163 `"Versão de pacote não encontrada"`);
* cargo com candidatos, mas **sem nenhum candidato apto**: `APTA:<n> [APTS APTT] CSEC:<total de votos do cargo> `;
* **proporcional**: para cada partido com votos: `PART:<nº> ` + `<candidato>:<votos> ` para cada candidato apto com votos + `LEGP:<legenda> TOTP:<total do partido> `. Depois `APTA… NOMI:<nominais> LEGC:<legendas> BRAN:<brancos> NULO:<nulos> TOTC:<total>`;
* **majoritário**: `<candidato>:<votos> ` para cada candidatura do cargo com votos, depois `APTA… NOMI BRAN NULO TOTC` (5606);
* **consulta**: as respostas ordenadas por número (5605 é a instanciação de `std::sort`), `<resposta>:<votos> ` para as que têm votos, depois os mesmos totais.

`APTA:` (3696, u25) é a contagem de aptos da abrangência do cargo (`APTS`/`APTT` são acrescentados quando há eleitores
em TTE). As contagens de votos vêm dos slots 3–10 de `CRdvVota` (`Candidato`, `Legenda`, `Partido`, `Nominais`,
`Legendas`, `Nulos`, `Brancos`, `Cargo`).

**Passo 3: divisão.** O texto `header + body` é cortado em partes de no máximo `tamanhoMaximo − 277` = **823**
caracteres. Uma parte que preenche o limite e não termina em espaço é cortada no seu último espaço. Toda parte passa
por trim.

**Passo 4: cadeia de hashes e assinatura (`RetornaBlocoAssinado`, linha 405).**

* `hash₀ = SHA-512(parte₀)`; `hashᵢ = SHA-512(join(bloco₀…bloco_{i−1}, " ") + " " + parteᵢ)`, onde `blocoᵢ = "parteᵢ HASH:hashᵢ"`. Os hashes têm 128 caracteres hexadecimais maiúsculos (`ecourna::api::security::CSha("SHA2-512")`), alimentados em fatias de 512 bytes.
* payload `i` = `QRBU:<i+1>:<n> VRQR:6.0 parteᵢ HASH:hashᵢ`. A constante de versão é a string de 3 caracteres `"6.0"`, decodificada a partir do argumento `string_view` empacotado.
* o **último** payload também recebe ` ASSI:<hex>`: os bytes brutos do último hash são assinados através do poly-singleton `api::pkcs11::IPkcs11` (slots da vtable 23 → 6 → 24). A assinatura em hex também é retornada separadamente.

Payload ilustrativo do VOTA (a estrutura segue o código; os valores são inventados):

```
QRBU:1:1 VRQR:6.0 ORIG:VOTA ORLC:LEG PROC:2400 DTPL:20261004 PLEI:2410 TURN:1 FASE:T UNFE:AC MUNI:1 ZONA:1
SECA:1 IDUE:... IDCA:... HICA:... VERS:10.23.0.1 LOCA:... APTO:1 APTS:1 APTT:0 COMP:0 FALT:1 HBBM:0 HBBG:0
HBSB:0 DTAB:... HRAB:... DTFC:... HRFC:... IDEL:... CARG:13 TIPO:1 VERC:... PART:91 91001:1 LEGP:0 TOTP:1
APTA:1 NOMI:1 LEGC:0 BRAN:0 NULO:0 TOTC:1 CARG:11 TIPO:0 VERC:... APTA:1 NOMI:0 BRAN:0 NULO:1 TOTC:1
HASH:<128 hex> ASSI:<hex>
```

**No build web**, nada registra um `api::pkcs11::IPkcs11`: a classe não tem RTTI, e seu nome mangled
`N3api6pkcs117IPkcs11E` (@517073) é referenciado apenas pela busca `CPolySingleton<IPkcs11>::instance` (3704),
que também atende a assinatura do arquivo do BU (10273) e `CEstadoGeral::RecuperarCertificado` (5635). Sua primeira verificação
(cpolysingleton.h:78) lança `EPatternErr` 1301 `"PolySingleton - solicitada uma instancia nao criada
N3api6pkcs117IPkcs11E…"` (o caminho posterior `"{}: solicitada uma instância não criada de {}"` não é alcançado). Assim,
os payloads de QR não podem ser concluídos pelo módulo sem modificações. De qualquer forma, a página web nunca chega à
geração do BU (docs/10-boletim-de-urna.md §4.4/§5.1); o harness do BU usa no lugar um assinante de zero bytes.

## 7. Particularidades do build web

* **Modo "Treinamento do eleitor".** O `vota.bin` do simulador tem `treinamentoEleitor = TRUE`; `comum_f697` = `fase == '3' && treinamentoEleitor`. Duas consequências:
  * `DynamicCreate` nunca é executada. Isso é confirmado por `analysis/runtime/memfs-after-init`: `uenux.db` não tem a tabela `eleitor_dinamico`.
  * A parte `CompleteLoad` da 6734 é pulada. Só o cadastro estático é carregado (`StaticLoad` → `LoadEleitorDetalhe` foi executada, amostrada uma vez). Nenhum eleitor tem dados dinâmicos, então `MarcaEleitorFoiHabilitado`, `GetDinamico` e `QtdVotaramPorTipoHabilitacao` lançariam exceção ou contariam 0.
* O cadastro de cada cenário (`…0000100010001-el.dat`) tem **um** registro de eleitor (um diferente para os cenários de 1º e de 2º turno); os cadastros `-tte.dat` estão vazios. Em municipal-t1, `-imp.dat` marca esse eleitor com P2 = `suspenso`.
* Nenhum assinante PKCS#11 é registrado (§6).
* `DynamicCreate` gravaria a tabela no `uenux.db` das **duas flashes** do turno corrente
  (`/dsk/fi/dinamico/trabN/` e `/dsk/fe/dinamico/trabN/`, via `CPath::GetPathTrab(interna|externa, turno)`).
  As duas cópias em `memfs-after-init` só têm `comparecimento_mesario` e `registro_justificativa`.
* Os mutexes dos singletons são no-ops: as chamadas de lock sumiram, e só resta um resíduo de `std::mutex::unlock` (func 150) em cada `GetInst`.

## 8. Notas sobre Wasm / Emscripten

* **O inlining engana a nomeação.** O pipeline nomeou a 5604 com base em um método de 6 linhas que ela inlina. A 6734 carrega os srclocs de CompleteLoad, mas é um wrapper. A 5745 carrega `VerificaEntraEmHorarioVerao`, mas retorna `this + 4` (`GetHorarioVerao`). A 5764 carrega tanto `CEleitorDetalhe::CEleitorDetalhe` quanto `CEleitores::LoadEleitorDetalhe`. A 11556 carrega `CCargos::GetInst`, mas é uma lambda do resumo da zerésima.
* **Eliminação de argumentos mortos.** `LoadEleitorDetalhe` perdeu o `this`. `CPE::CreateInst` perdeu seu `EFlashOrigem`. `CEleitores::GetInst`, inlinada nas funções `Text`, lê a global diretamente.
* **merge-similar-functions.** 5616 (`"RED"`) e 5617 (`"SA"`) são thunks de 13 bytes que passam uma constante `string_view` empacotada (`len<<32 | ptr`) ao corpo compartilhado 6028.
* **Argumentos de formatação.** O `std::format` da libc++ armazena os tipos dos argumentos como códigos de 5 bits: 6 = unsigned, 13 = string_view, 15 = handle. Decodificá-los deu os tipos dos argumentos das mensagens, por exemplo `QRBU:{}:{} VRQR:{} {}` = (unsigned, unsigned, string_view, string_view).
* No wasm-decompile, `a[N]:T@k` significa **offset em bytes N·k** (por exemplo, `c[10]:int@2` = offset 20). Isso foi necessário para `SQtdeAptos` (dois `uint16` em +20/+22 do nó do map).
* As funções `Text` chamam `std::vformat` porque as strings de formato são dados de tempo de execução (layout da tela).

## 9. Código suspeito ou digno de nota

Veja a lista `suspicious` no resultado da unidade. Resumo:

1. A assinatura do QR do BU precisa de `IPkcs11`, que não existe no build web (§6). Chegar à geração do QR do BU no simulador lançaria exceção.
2. O laço de divisão do QR (5604) não tem proteção para uma janela cheia de 823 caracteres sem um espaço utilizável. A busca pelo último espaço retorna `npos` quando não há nenhum: todo o restante vira uma parte e `pos += npos` move o cursor um caractere para trás (com `pos == 0`, `substr` lança `out_of_range` em vez disso). Ela retorna `0` quando o único espaço é o primeiro caractere da janela, que é exatamente o que segue cada corte: uma parte vazia é inserida e `pos` não se move. Em ambos os casos o laço nunca termina (o caso `npos` volta para trás até chegar ao caso `0`), e o vector de partes cresce até a memória acabar. Dispará-lo exige uma sequência de cerca de 822 caracteres sem espaço depois de um corte; dados normais não conseguem produzir isso, porque os valores dos campos são curtos e truncados.
3. O merge-join de impedimentos (5764) nunca verifica os impedimentos que sobram depois do último eleitor: um impedimento cuja identidade fica, na ordenação, depois de todos os eleitores é ignorado silenciosamente. Nenhum dos chamadores (StaticLoad em 7787, CompleteLoad em 6734) compara o índice final com o tamanho da lista.
4. `CEleitorDetalhe::MarcaHabilitado` ignora silenciosamente tipos de habilitação desconhecidos (≥ 3): o default do `br_table` pula a atualização de estado, então estado e tipo mantêm os valores anteriores (normalmente `0` faltou, ou seja, o eleitor não é registrado como habilitado), enquanto os campos de áudio/identidade/foto ainda são sobrescritos. Não lança exceção.
5. `CParteCargos::Imprime` (11209) inicia sua "eleição anterior" com a eleição do **último** cargo. Com uma única eleição, o cabeçalho da eleição nunca é impresso; com duas ou mais, é. Isso parece intencional.
6. `CCargos::FiltraPorAbrangencia` com uma abrangência inválida só lança exceção quando a lista de cargos não está vazia.
7. Privacidade: o cadastro de treinamento distribuído com o simulador contém um nome completo, número de título e data de nascimento (provavelmente dados de teste fictícios). Este código o carrega na memória na inicialização.

## 10. Tabela de mapeamento completa (83 funções)

"arquivo u04" = reconstruído nos arquivos desta unidade. "fragmento" = reconstruído em um arquivo de fragmento para um arquivo
de outra unidade. "biblioteca" = instanciação de template resumida em um comentário.

| idx | símbolo reconstruído | arquivo original | onde / observação |
|---:|---|---|---|
| 187 | `comum::CConfiguracaoEleicao::GetInst` | comum/dados/cconfiguracaoeleicao.cpp:36 | arquivo u04 |
| 273 | `comum::CCargos::GetInst` | comum/dados/ccargos.cpp:24 | arquivo u04 |
| 282 | `std::swap<comum::md::CEleitorDinamico>` | (instanciação em celeitores.cpp) | biblioteca (move-swap) |
| 326 | `comum::CEleitores::GetInst` | comum/dados/celeitores.cpp:44 | arquivo u04 |
| 332 | `comum::CCargos::GetCurrent` | comum/dados/ccargos.cpp:59 | arquivo u04 |
| 470 | `comum::md::CEleitorDinamico::operator=(CEleitorDinamico&&)` | comum/dados/md/eleitor/celeitordinamico.h | biblioteca/implícita (atribuição por movimento) |
| 782 | `comum::CLocal::VerificaLido` | comum/dados/clocal.cpp:345 | arquivo u04 |
| 861 | `comum::CConfiguracaoEleicao::GetCargo` | comum/dados/cconfiguracaoeleicao.cpp:316 | arquivo u04 |
| 1003 | `comum::CLocal::GetZonaID` | comum/dados/clocal.cpp:86 | arquivo u04 |
| 1073 | `comum::CPE::GetInst` | comum/dados/cpe.cpp:21 | arquivo u04 |
| 1078 | `comum::CLocal::GetSecaoID` | comum/dados/clocal.cpp:110 | arquivo u04 |
| 1271 | `comum::CEleitorDetalhe::GetDinamico` | comum/dados/celeitordetalhe.cpp | arquivo u04 |
| 1392 | `std::__sort3<…, comum::md::CEleitorDinamico*>` | (celeitores.cpp) | biblioteca (std::sort) |
| 1708 | `comum::CCargos::Next` | comum/dados/ccargos.cpp:46 | arquivo u04 |
| 1937 | `comum::CEleitorDetalhe::GetBiometria` | comum/dados/celeitordetalhe.cpp:94/101 | arquivo u04 |
| 1938 | `comum::CCargos::GetCurrentCargoID` | comum/dados/ccargos.cpp:68 | arquivo u04 |
| 2263 | `comum::CIntegridadeReferencial::Lanca` | comum/dados/cintegridadereferencial.cpp:344 | arquivo u04 |
| 2269 | `comum::CCargos::First` | comum/dados/ccargos.cpp | arquivo u04 (nome inferido) |
| 2272 | `comum::CCandidaturas::GetNumerosCandidatosAptos` | comum/dados/ccandidaturas.cpp (u03) | fragmento (nome inferido) |
| 2728 | `std::__tree<map<CComparecimentoMesarioPK,CComparecimentoMesario>>::destroy` | (celeitores.cpp) | biblioteca |
| 2787 | `comum::CPE::CreateInst` | comum/dados/cpe.cpp:26 | arquivo u04 |
| 2800 | `comum::md::CEleitorDinamico::CEleitorDinamico(id, id, estado)` | comum/dados/md/eleitor/celeitordinamico.cpp (u05) | fragmento |
| 2809 | `std::__tree<map<CNumeroInscricaoEleitoral,CJustificadorDetalhe>>::destroy` | (celeitores.cpp) | biblioteca |
| 2817 | `comum::CHV::GetInst` | comum/dados/chv.cpp:27 | arquivo u04 |
| 2820 | `std::__sort4<…, fn-ptr cmp, CEleitorDinamico*>` | (celeitores.cpp) | biblioteca |
| 2824 | `comum::CEleitores::GetQtdAptos` (slot 0 de IDataSourceAptos) | comum/dados/celeitores.cpp | arquivo u04 (nome inferido) |
| 2825 | `comum::CEleitores::MarcaEleitorFoiHabilitado` | comum/dados/celeitores.cpp:435/439/447 | arquivo u04 (+ `CEleitorDetalhe::MarcaHabilitado` inlinado) |
| 2830 | `comum::md::CProcessoEleitoral::~CProcessoEleitoral` | comum/dados/md/processoeleitoral/cprocessoeleitoral.h | biblioteca/implícita (anotado em cpe.cpp) |
| 2835 | `comum::CCargos::GetCurrentEleicaoID` | comum/dados/ccargos.cpp:85 | arquivo u04 |
| 2836 | `comum::CCargos::GetCurrentEleicao` (+ `CConfiguracaoEleicao::GetEleicao` inlinado) | comum/dados/ccargos.cpp:76 (+ cconfiguracaoeleicao.cpp:293) | arquivo u04 |
| 3721 | `comum::(anon)::GetAbrangenciaEleitor` | comum/dados/celeitores.cpp | arquivo u04 (nome inferido; corpo de GetCurrentAbrangenciaEleitor:497) |
| 3757 | `std::__sort4<…, CEleitorDinamico*>` | (celeitores.cpp) | biblioteca |
| 3758 | `std::__split_buffer<CEleitorDinamico>::~__split_buffer` | (celeitores.cpp) | biblioteca |
| 3759 | `std::__uninitialized_allocator_relocate<CEleitorDinamico>` | (celeitores.cpp) | biblioteca |
| 3762 | `std::__introsort<…, CEleitorDinamico*>` | (celeitores.cpp) | biblioteca (sort em DynamicCreate) |
| 3763 | `std::vector<CEleitorDinamico>::__destroy_vector::operator()` | (celeitores.cpp) | biblioteca |
| 3770 | `comum::CEleitorDetalhe::ConfereDadosDinamicos` | comum/dados/celeitordetalhe.cpp:224 | arquivo u04 |
| 5404 | `vota::CControlaReconhecimento::LimiteVerificacoesAtingido` | vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp (u10) | fragmento (nome inferido) |
| 5598 | `comum::CPE::~CPE` | comum/dados/cpe.cpp | arquivo u04 |
| 5603 | `comum::CGeradorBUQRCodeVota::CGeradorBUQRCodeVota` (+ ctor da base) | comum/relatorios/cgeradorbuqrcodevota.cpp (caminho inferido) | fragmento QR |
| 5604 | `comum::CGeradorBUQRCode::GeraQRCodes` | comum/relatorios/cgeradorbuqrcode.cpp:405 (u25) | fragmento QR (nome inferido; inlina CCargos::GetCurrentEleicaoVersaoPacote:93, CPleito::GetVersaoPacoteEleicao:158, RetornaBlocoAssinado:405) |
| 5605 | `std::__introsort<…, md::CRespostaConsulta*>` (sort por número) | (cgeradorbuqrcode.cpp) | biblioteca |
| 5606 | `comum::CGeradorBUQRCode::TotaisVotosCargo` | comum/relatorios/cgeradorbuqrcode.cpp | fragmento QR (nome inferido) |
| 5608 | `comum::CCabecalhoQRCode::CCabecalhoQRCode(const CCabecalhoQRCode&)` | comum/relatorios/ccabecalhoqrcodebuilder.h (caminho inferido) | biblioteca/implícita (33 strings) |
| 5616 | `comum::(anon)::EhOrigemRED` | comum/relatorios/cgeradorbuqrcode.cpp | fragmento QR (nome inferido) |
| 5617 | `comum::(anon)::EhOrigemSA` | comum/relatorios/cgeradorbuqrcode.cpp | fragmento QR (nome inferido) |
| 5723 | `m_dao->SelectAll()` (slot 8 do DAO): corpo compartilhado por ICF de `comum::servico::CJustificadorServico::SelectAll` / `CComparecimentoMesarioServico::SelectAll` | comum/servico (caminho inferido) | anotado em celeitores.cpp (chamado com CJustificador+28 e o repositório de mesários+40, ambos objetos `*Servico`; não é um método de CDAORepositorio) |
| 5742 | `comum::CLocal::VerificaEhSecao` | comum/dados/clocal.cpp:354 | arquivo u04 |
| 5745 | `comum::CHV::GetHorarioVerao` (+ VerificaEntraEmHorarioVerao:117 inlinado) | comum/dados/chv.cpp | arquivo u04 (nome inferido) |
| 5748 | `std::__insertion_sort_incomplete<…, CEleitorDinamico*>` | (celeitores.cpp) | biblioteca |
| 5749 | `std::__sort5<…, CEleitorDinamico*>` | (celeitores.cpp) | biblioteca |
| 5751 | `std::__insertion_sort_incomplete<…, fn-ptr cmp, CEleitorDinamico*>` | (celeitores.cpp) | biblioteca |
| 5761 | `comum::CEleitores::DynamicCreate` | comum/dados/celeitores.cpp:301/305/358 | arquivo u04 |
| 5764 | `comum::CEleitores::LoadEleitorDetalhe` (+ ctor de `CEleitorDetalhe`) | comum/dados/celeitores.cpp:719 (+ celeitordetalhe.cpp:49) | arquivo u04 |
| 5765 | `comum::CEleitores::ValidaIdentidadesEleitor` | comum/dados/celeitores.cpp:738 | arquivo u04 |
| 5766 | `std::__introsort<…, fn-ptr cmp, CEleitorDinamico*>` | (celeitores.cpp) | biblioteca (sort em CompleteLoad) |
| 6028 | `comum::(anon)::OrigemIgual` (corpo compartilhado de 5616/5617) | comum/relatorios/cgeradorbuqrcode.cpp | fragmento QR |
| 6034 | `comum::CEleitores::QtdVotaramPorTipoHabilitacao` | comum/dados/celeitores.cpp | arquivo u04 (nome inferido) |
| 6734 | `vota::CInformacaoEleitor::CarregarDadosDinamicos` (nome da u02) com `comum::CEleitores::CompleteLoad` inlinado | comum/dados/celeitores.cpp:205/219/238 (+ vota/eleitor/comum/cinformacaoeleitor.cpp) | arquivo u04 (CompleteLoad) + fragmento (wrapper) |
| 10447 | `vota::CDadoEleitorNaoConfere::ProcessInput` | vota/operador/confirmaidentidade/cdadoeleitornaoconfere.cpp (caminho inferido) | fragmento |
| 10448 | `vota::CDadoEleitorNaoConfere::StartState` | idem | fragmento |
| 11203 | `comum::CParteRdv::Imprime` | comum/relatorios/cpartecandidatos.cpp (u25) | fragmento |
| 11209 | `comum::CParteCargos::Imprime` | comum/relatorios/cpartecandidatos.cpp (u25) | fragmento |
| 11213 | `comum::CParteCandidatosProporcionais::Imprime` | comum/relatorios/cpartecandidatos.cpp (u25) | fragmento |
| 11226 | liberação de `CPE::s_inst` (`delete exchange(s_inst, nullptr)`, argumento `void*` não usado; papel de at-exit inferido, nenhum código referencia seu slot 2984 na tabela) | comum/dados/cpe.cpp | arquivo u04 (nome inferido) |
| 11259 | `comum::CDataSourcesRelatorio<CRdvVota,CEleitores>::HeaderDetalheSeHouverVotos` | comum/relatorios/cdatasourcesrelatorio.h (u25) | fragmento (nome inferido) |
| 11526 | `comum::CEleitorDadoTTE::Text` | comum/dados/celeitores.cpp:796 | arquivo u04 |
| 11527 | `comum::CEleitorDadoSecao::Text` | comum/dados/celeitores.cpp:784 | arquivo u04 |
| 11528 | `comum::CEleitorDadoSequencial::Text` | comum/dados/celeitores.cpp:772 | arquivo u04 |
| 11529 | `comum::CEleitorDadoNomeParaUrna::Text` | comum/dados/celeitores.cpp:756 | arquivo u04 |
| 11556 | lambda "nome da eleição corrente" (ponteiro de função, slot 3049) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (u09) | fragmento (inlina CCargos::GetInst:24) |
| 11687 | `vota::CMajoritarioValido::GetMensagemAudio` | vota/eleitor/votamajoritario/cconfirmamajoritario.cpp (caminho inferido) | fragmento (nome inferido) |
| 11690 | `vota::CMajoritarioRepetido::GetMensagemAudio` | idem | fragmento (nome inferido) |
| 11693 | `vota::CMajoritarioNulo::GetMensagemAudio` | idem | fragmento (nome inferido) |
| 11696 | `vota::CMajoritarioBranco::GetMensagemAudio` | idem | fragmento (nome inferido) |
| 11724 | `vota::CPedeNominal::GetProximoEstado` | vota/eleitor/votaproporcional/cpedenominal.cpp (caminho inferido) | fragmento |
| 11968 | `CDataSourcesRelatorio<CRdvVota,CEleitores>::GetLinhasEleitoresAptos()::lambda::operator()` | comum/relatorios/cdatasourcesrelatorio.h:212 (u25) | fragmento |
| 11972 | `CDataSourcesRelatorio<CRdvVota,CEleitores>::DetalheCandidato` | comum/relatorios/cdatasourcesrelatorio.h (u25) | fragmento (nome inferido) |
| 11975 | `CDataSourcesRelatorio<CRdvVota,CEleitores>::TrailerComparecimento` | comum/relatorios/cdatasourcesrelatorio.h (u25) | fragmento (nome inferido) |
| 11976 | `vota::PartidoTemCandidatos` (predicado da zerésima, slot 1631) | vota/eleitor/iniciovotacao/cgerazeresimabase.cpp (caminho inferido) | fragmento (nome inferido) |
| 12742 | `api::CDataText<comum::CCandidaturasDSNumero>::GetText` (+ `CCandidaturasDSNumero::operator()`) | comum/dados/ccandidaturas.cpp (u03) / api CDataText | fragmento |
| 12811 | `comum::NomeCargoComEscolha` (fonte de texto, slot 1088) | comum/dados/ccargods.cpp (u03, caminho inferido) | fragmento (nome inferido) |
| 13550 | `vota::CPreShowProgressBar::PreShow` | vota/eleitor/comum/cpreshowprogressbar.cpp (caminho inferido) | fragmento (descrito) |

Mapa de códigos de erro (`EUeComumDadosError`) deste diretório: 7815–7821 CCargos, 7826/7827 CConfiguracaoEleicao,
7829–7832 CEleitorDetalhe, 7834–7839 CompleteLoad/DynamicCreate, 7840/7841 StaticLoad, 7842–7844
MarcaEleitorFoiHabilitado, 7845/7846 MarcaVotou, 7847 GetCurrentAbrangenciaEleitor, 7851/7852 LoadEleitorDetalhe/ValidaIdentidadesEleitor, 7854–7857 fontes
Text, 7866… CHV::CreateInst, 7870 CHV, 7871 CIntegridadeReferencial, 7872–7875 CLocal, 7876 CPE.
