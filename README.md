# VOTA, reconstruído a partir do simulador de votação público do TSE

*[English version](README_EN.md)*

Reconstrução em C++ legível do código do TSE que está compilado em `vota_web_wasm.wasm`, o módulo
WebAssembly por trás do simulador de votação que o Tribunal Superior Eleitoral publica em
<https://www.justicaeleitoral.jus.br/simulador-votacao/>.

| | |
|---|---|
| módulo | `vota_web_wasm.wasm`, 7.713.699 bytes |
| SHA-256 | `8579ef8d1a93819152cecdbf52906d2e4dd0f57999cb2e1e8c0d1161cd1007e2` |
| versão gravada no binário | `10.23.0.1 - DESENVOLVIMENTO` (uma build de desenvolvimento) |

## O que tem aqui

| caminho | conteúdo |
|---|---|
| `src/uenux2/src/app/vota/` | o aplicativo de votação: terminal do eleitor, terminal do mesário, abertura e encerramento, geração do BU |
| `src/uenux2/src/app/comum/` | código comum aos aplicativos da urna: dados da eleição, RDV, relatórios, gravação dos arquivos de resultado, conversores ASN.1, logs |
| `src/uenux2/src/api/` | a camada de plataforma da urna: interface gráfica, registro de serviços, threads e mensagens, arquivos, timers |
| `src/uenux2/mock/`, `src/uenux2/wasm/` | a pequena camada web: as funções `vota*` chamadas pela página e as versões para navegador da tela, do teclado e do som |
| `src/ecourna/` | a biblioteca `ecourna` do TSE: criptografia, acesso ao SQLite, ZIP, modelo de dados e seus conversores ASN.1 |
| `src/asn1/` | os 32 módulos ASN.1 (formatos de dados) recuperados do binário |
| `docs/modules/` | um capítulo por unidade de reconstrução (41), cada um terminando com a tabela função → símbolo → arquivo; comece por `docs/modules/README.md` |
| `docs/data-model/asn1-schemas.md` | como os módulos ASN.1 foram recuperados e o que cada um descreve |
| `docs/glossary.md` | termos em português e do TSE |

São 1.184 arquivos C++ e 83.183 linhas. Os caminhos dos arquivos são os originais, recuperados do
binário. O módulo tem 3.463 funções do TSE, e todas estão reconstruídas aqui, menos 4 triviais: três
destrutores de string gerados pelo compilador e uma função auxiliar sem nome. Não há código de
bibliotecas de terceiros (OpenSSL, SQLite, RHVoice, Boost, zlib, 7-Zip, libc++).

A documentação está em `docs/` (em português) e em `docs_en/` (o original em inglês).

## O que isto não é

* **Não é o código-fonte original do TSE.** É uma reconstrução feita a partir do binário compilado.
  Os nomes vêm de dados que o compilador deixou no módulo (registros `std::source_location` com
  arquivo, função e linha; nomes de classe do RTTI; vtables), e os nomes deduzidos estão marcados.
  Um segundo revisor conferiu cada função contra o código descompilado, mas pode haver erros.
* **Não compila.** O código é para ser lido ao lado do binário. Dos 815 headers do projeto que os
  arquivos incluem, 325 nunca foram reconstruídos, e os headers das bibliotecas de terceiros não
  estão aqui.
* **Não é o programa das urnas lacradas.** O simulador é uma build de desenvolvimento da mesma árvore
  de código, com versões web no lugar do hardware e assinaturas de mentira
  (`assinatura simulada para vota_web_wasm`). As urnas reais da eleição de 2026 rodaram
  `10.23.0.0 - Praia da Barra do Cahy`. Os arquivos de resultado e os logs que elas gravaram batem
  com este código, o que indica a mesma árvore de código, mas não prova que os binários são iguais.

## Como ler o código

* `func N`: a função de índice N no módulo, contando as importações. É o número que o `wasm2wat`
  mostra como `(func (;N;)`, então dá para comparar qualquer função reconstruída com as instruções
  originais:

  ```sh
  wasm2wat vota_web_wasm.wasm -o vota.wat     # wabt: https://github.com/WebAssembly/wabt
  grep -n '(func (;7840;)' vota.wat           # votaInit, exportada como "Eb"
  ```
* `@N`: um endereço na memória linear do módulo (textos, vtables, variáveis globais).
* `srcloc arquivo.cpp:L`: um registro `std::source_location`, que dá o arquivo e a linha originais.
* `uNN §x`: uma seção do capítulo `docs/modules/uNN-*.md`.
* Menções a `analysis/`, `decompiled/`, `tools/`, `work/`, `upstream/`, `samples/`, `q.py` e aos
  capítulos `00`–`12`, `bu/` ou `libraries/` se referem ao projeto de análise de onde este
  repositório saiu. Esses arquivos não estão incluídos, e os links para eles viraram texto simples.

## Dados

* Os caminhos de código-fonte gravados no binário foram mantidos exatamente como aparecem nele.
* Os cenários de treinamento do simulador trazem um registro de eleitor (nome, título e data de
  nascimento). Esses valores foram trocados aqui por `NOME OMITIDO`, `XXXXXXXXXXXX` e `AAAA`.
  Entradas de teste com dígito verificador válido (um CPF e um título) foram mascaradas do mesmo
  jeito.

## Aviso

Feito só com material público: o módulo e os arquivos de dados servidos pela página do simulador do
TSE, e arquivos que o próprio TSE publica. Feito para estudo (WebAssembly e o funcionamento do
simulador publicado). Sem vínculo com o TSE e sem endosso dele. Os direitos sobre o software
original pertencem aos seus titulares, e este repositório não concede licença sobre ele.

**Pedido de remoção.** Se o TSE ou outra autoridade competente solicitar, este repositório será
retirado do ar prontamente. Para pedir, basta abrir uma issue aqui ou entrar em contato pelo perfil
do GitHub.
