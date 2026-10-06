// Reconstructed from vota_web_wasm.wasm (unit u05). Original: uenux2/src/app/comum/dados/crdvvota.h
//
// CRdvVota = the VOTA application's in-memory RDV (Registro Digital do Voto, "digital record of the
// vote"): per election (TEleicaoID) and per office (TCargoID) the list of votes cast, in the order
// defined by an RDV "posicionador": comum::CRdvPosicionadorVota (func 11496) keeps each list SORTED by
// (tipo, digitado) with std::upper_bound, so the order of the votes is not kept (not a random shuffle).
// It is a process-wide singleton created during start-up (CreateInst, crdvvota.cpp:92, fully inlined into
// the giant init function 7787, vota::CInformacaoEleitor::Inicializar) and read by
// the BU/zeresima generators (CGeradorBUBase<CRdvVota, CEleitores>, CDataSourcesRelatorio<...>).
//
// Class hierarchy (RTTI):  comum::CRdv (abstract, 14 pure virtuals, NO virtual destructor)
//                            └── comum::CRdvVota
//
// Error type used here: CUeRdvError = ecourna::api::exception::CBaseError<api::EUeRdvError,
//                                     SErrorLimits{4650, 4850}>  (typeinfo @1559972)
#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "md/rdv/cvotoseleicoesvota.h"                 // comum::md::CVotosEleicoesVota (unit u22)
#include "asn/rdv/cconversoreleicoesvota.h"            // comum::asn::CConversorEleicoesVota (unit u03)
#include "../gravadores/asn/cconversorregistrodigitalvoto.h"  // ? CConversorRegistroDigitalVoto<>

namespace comum {

using TEleicaoID   = std::uint32_t;   // ?
using TCargoID     = std::uint8_t;
using TPartidoID   = std::uint16_t;
using TCandidatoID = std::uint32_t;
using TQtdVoto     = std::uint16_t;   // the lambdas return "unsigned short"
using uebyte       = std::uint8_t;

// Declared in crdv.h (crdv.cpp has no function of its own in the binary: its constructor is inlined
// into the start-up function 7787). Slot order recovered from CRdvVota's vtable @1560172.
class CRdv {
public:
    // NOTE: the vtable of CRdv (@1559900) has 14 pure slots and no destructor slot: the singleton is
    // destroyed through a non-virtual call (api_f5734 + free) on the concrete CRdvVota.
    virtual std::map<TCargoID, md::CVotos> GetVotos() const = 0;                        // slot 0
    virtual std::map<TCargoID, md::CVotos> GetVotos(TEleicaoID eleicao) const = 0;     // slot 1
    virtual TQtdVoto Comparecimento(TEleicaoID eleicao) const = 0;                      // slot 2
    virtual TQtdVoto Candidato(TCargoID cargo, TCandidatoID numero, uebyte digitos) const = 0; // slot 3
    virtual TQtdVoto Legenda(TCargoID cargo, TPartidoID partido) const = 0;             // slot 4
    virtual TQtdVoto Partido(TCargoID cargo, TPartidoID partido) const = 0;             // slot 5
    virtual TQtdVoto Nominais(TCargoID cargo) const = 0;                                // slot 6
    virtual TQtdVoto Legendas(TCargoID cargo) const = 0;                                // slot 7
    virtual TQtdVoto Nulos(TCargoID cargo) const = 0;                                   // slot 8
    virtual TQtdVoto Brancos(TCargoID cargo) const = 0;                                 // slot 9
    virtual TQtdVoto Cargo(TCargoID cargo) const = 0;                                   // slot 10
    virtual std::vector<uebyte> Converte() const = 0;                                   // slot 11 name inferred
    virtual void Desconverte(const std::vector<uebyte>& conteudo) = 0;                  // slot 12
    virtual bool ConfereConteudo(const std::vector<uebyte>& conteudo) const = 0;        // slot 13 name inferred

protected:
    // CRdv::CRdv(md::RdvPosicionadorPtr, const std::vector<uebyte>& cargos, uebyte digitosPartido)
    //   (reconstructed by unit u02 in src/uenux2/src/app/comum/dados/crdv.cpp)
    //   crdv.cpp:90  "Número de dígitos do partido nulo"                          (EUeRdvError 4650)
    //   crdv.cpp:93  "Número de dígitos do partido ({}) supera o limite (5)"     (EUeRdvError 4651)
    //   crdv.cpp:60/78 (anonymous)::GetCifradorCryptoTable(cargos): SHA-512 of the sorted office
    //   codes + 32 bytes picked from the 128-byte IUrna crypto table (simulator: all 0x03) ->
    //   HKDF-SHA512(info "RDV") -> symmetric cipher used by CEncryptedFile for rdv.dat.
    CRdv(md::RdvPosicionadorPtr posicionador, const std::vector<uebyte>& cargos, uebyte digitosPartido);

    md::RdvPosicionadorPtr m_posicionador;                               // +4  (CRdvPosicionadorVota)
    std::shared_ptr<ecourna::api::security::ISymmetricCipher> m_cifrador; // +8 ? used by CEncryptedFile
    uebyte m_digitosPartido;                                             // +16 (1..5, from CConfiguracaoEleicao +164)
};

class CRdvVota final : public CRdv {
public:
    static CRdvVota& GetInst();     // crdvvota.cpp:87
    static void CreateInst();       // crdvvota.cpp:92 (inlined into func 7787)

    std::map<TCargoID, md::CVotos> GetVotos() const override;
    std::map<TCargoID, md::CVotos> GetVotos(TEleicaoID eleicao) const override;
    TQtdVoto Comparecimento(TEleicaoID eleicao) const override;
    TQtdVoto Candidato(TCargoID cargo, TCandidatoID numero, uebyte digitos) const override;
    TQtdVoto Legenda(TCargoID cargo, TPartidoID partido) const override;
    TQtdVoto Partido(TCargoID cargo, TPartidoID partido) const override;
    TQtdVoto Nominais(TCargoID cargo) const override;
    TQtdVoto Legendas(TCargoID cargo) const override;
    TQtdVoto Nulos(TCargoID cargo) const override;
    TQtdVoto Brancos(TCargoID cargo) const override;
    TQtdVoto Cargo(TCargoID cargo) const override;
    std::vector<uebyte> Converte() const override;
    void Desconverte(const std::vector<uebyte>& conteudo) override;
    bool ConfereConteudo(const std::vector<uebyte>& conteudo) const override;

    TQtdVoto Comparecimento() const;   // func 1269 (not in unit u05): maximum over the elections, name inferred

private:
    CRdvVota();   // inlined into 7787 (see crdvvota.u02.cpp): CRdv(make_unique<CRdvPosicionadorVota>(),
                  //   sorted office codes, CConfiguracaoEleicao::GetQtdDigitosPartido())

    // +20  24 bytes: { std::map<TEleicaoID, CVotosCargos> (+20), std::map<TCargoID, TEleicaoID> (+32) }
    md::CVotosEleicoesVota m_votos;
    // +44  vptr CConversorRegistroDigitalVoto<...>, +48 embedded CConversorEleicoesVota, +64.. the
    //      identification fields copied into the RDV header (zona, seção, município, eleições, ...)
    asn::CConversorRegistroDigitalVoto<asn::CConversorEleicoesVota> m_conversor;
};   // sizeof == 100 (operator_new(100) in 7787)

}  // namespace comum
