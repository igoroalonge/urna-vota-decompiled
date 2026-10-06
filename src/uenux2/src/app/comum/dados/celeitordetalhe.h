// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/celeitordetalhe.h
//
// CEleitorDetalhe = everything the urna knows about ONE voter of the section:
//   * the static data from the voter roll ("cadastro": *-el.dat / *-tte.dat, ASN.1 ModuloEleitores),
//     wrapped by md::CEleitorDecorator;
//   * the dynamic data ("dados dinâmicos": attendance / enabling state kept in the SQLite table
//     eleitor_dinamico of uenux.db), md::CEleitorDinamico, present only after CEleitores::CompleteLoad;
//   * the impediments ("impedimentos") for 1st and 2nd round from the *-imp.dat file.
// It is the value type of CEleitores' map (CDataMap<CEleitorIdentidade, CEleitorDetalhe>); a map node
// is 236 bytes = 16 (tree header) + 16 (key) + 204 (this class).
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "md/eleitor/cbiometriaeleitor.h"
#include "md/eleitor/celeitordecorator.h"
#include "md/eleitor/celeitordinamico.h"

namespace comum {

namespace md {
// ASN.1 ModuloImpedidos::TipoImpedimento is 1-based (semImpedimento = 1); the md enum used here is
// 0-based: 0 = no impediment (CEleitorDetalhe::PodeVotar tests == 0).            // values inferred
enum class ETipoImpedimento : int { NENHUM = 0 /*, VOTA_NA_SECAO_ORIGINAL, SOLICITOU_VOTO_EM_TRANSITO, ...*/ };

// Layout of the 108-byte md::CEleitorDecorator as used by u04 (celeitordecorator.h, unit u05):
//   +0   int      sequencial               (CEleitorDadoSequencial::Text)
//   +4   uint16   seção of the voter       (CEleitorDadoSecao::Text; file name of the biometrics)
//   +8   vector<CEleitorIdentidade> identificadores (16 bytes each: string numero + int tipo)
//   +24  string   nome;  +36 string nomeSocial (used when not empty)   (CEleitorDadoNomeParaUrna)
//   +48  int      tipo de transferência temporária (0 = none, 4 = acessibilidade, other = TTE)
//   +52  string   UF of the voter's domicile;  +64 int município of the domicile
//   +80  optional<{ vector<uebyte>; uint32 offset; uint32 tamanho }> localização da biometria
//        (engaged flag +100) - where the encrypted biometrics sit inside the roll file
//   +104 ecourna::app::dados::ETipoIdentificadorEleitor tipo do identificador principal
//
// Layout of the 84-byte md::CEleitorDinamico (celeitordinamico.h, unit u05) - one row of the
// SQLite table eleitor_dinamico:
//   +0  CEleitorIdentidade titulo;  +16 CEleitorIdentidade identidadeHabilitacao
//   +32 EEstadoComparecimento estado;  +36 ETipoHabilitacao tipoHabilitacao
//   +40 dedo;  +44 uint16 score;  +46 uint8 tentativas
//   +48 tipoAtivacaoAudio (default 2);  +52 erroDecifrarBiometria (default 0)
//   +56 optional<CEleitorIdentidade> tituloMesario (flag +72)
//   +76 estadoApresentacaoFoto;  +80 resultadoDecifracaoFoto
enum class EEstadoComparecimento : int {        // values inferred from the code; ASN.1 is 1-based
    FALTOU = 0,                                 // "faltou" (initial state of a voter with cargos)
    SEM_CARGO_PARA_VOTAR = 1,                   // voter em trânsito with no cargo in this urna
    NAO_VOTOU = 2,                              // enabled ("habilitado") but did not finish
    VOTOU = 3,
};
enum class ETipoHabilitacao : int {             // names inferred
    SEM_BIOMETRIA = 0,                          // QR "HBSB"
    BIOMETRIA = 1,                              // fingerprint recognised, QR "HBBM"
    CODIGO_MESARIO = 2,                         // enabled by the poll worker's code, QR "HBBG"
};
}  // namespace md

// Data collected when the poll worker enables the voter (md, unit u05). Layout used by u04:
//   +0  CEleitorIdentidade identidade usada na habilitação;  +16 ETipoHabilitacao tipo
//   +20 tipoAtivacaoAudio
//   +24 md::CEleitorDadosHabilitacaoBiometrica { +24 dedo, +28 uint16 score, +30 uint8 tentativas,
//       +32 erroDecifrarBiometria, +36 optional<CEleitorIdentidade> tituloMesario (flag +52) }
//   +56 apresentação da foto { estado +56, resultado +60 }
class CEleitorDadosHabilitacao;

class CEleitorDetalhe {
public:
    // celeitordetalhe.cpp:49. Exists only inlined into CEleitores::LoadEleitorDetalhe (wasm 5764).
    CEleitorDetalhe(const md::CEleitorDecorator& eleitor, const md::CEleitorDinamico& dinamico,
                    const md::ETipoImpedimento impedimentoP1, const md::ETipoImpedimento impedimentoP2);
    // same without dynamic data (used by CEleitores::StaticLoad)                    // ?
    CEleitorDetalhe(const md::CEleitorDecorator& eleitor,
                    const md::ETipoImpedimento impedimentoP1, const md::ETipoImpedimento impedimentoP2);

    const md::CEleitorDecorator& GetEleitor() const { return m_eleitor; }
    bool TemDinamico() const { return m_dinamico.has_value(); }
    const md::CEleitorDinamico& GetDinamico() const;                    // wasm 1271
    md::CEleitorDinamico& GetDinamico();                                 // (same body)
    md::CBiometriaEleitor GetBiometria() const;                         // lines 94/101 (wasm 1937)
    void ConfereDadosDinamicos(const std::string& funcao) const;         // line 224 (wasm 3770)
    // Only inlined (into CEleitores::MarcaEleitorFoiHabilitado, wasm 2825). Name inferred from the
    // string "MarcaHabilitado" it passes to ConfereDadosDinamicos.
    void MarcaHabilitado(const CEleitorDadosHabilitacao& dados);

    // comum_f2266 (not in u04): impediment of the current round (CDadoCarga turno '2' -> P2)
    bool PodeVotar() const;                                              // name inferred
    md::ETipoImpedimento GetImpedimentoP1() const { return m_impedimentoP1; }
    md::ETipoImpedimento GetImpedimentoP2() const { return m_impedimentoP2; }

private:
    md::CEleitorDecorator                m_eleitor;         // +0   (108 bytes)
    std::optional<md::CEleitorDinamico>  m_dinamico;        // +108 (84 bytes, engaged flag +192)
    md::ETipoImpedimento                 m_impedimentoP1;   // +196
    md::ETipoImpedimento                 m_impedimentoP2;   // +200
};

}  // namespace comum
