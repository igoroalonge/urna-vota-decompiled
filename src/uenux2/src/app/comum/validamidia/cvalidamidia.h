// uenux2/src/app/comum/validamidia/cvalidamidia.h   (path inferred from cvalidamidia.cpp, attested by the
// std::source_location record :182 of IValidaMidia::GetInst)
// Reconstructed from vota_web_wasm.wasm (unit u26 = owner; GetInst was reconstructed by unit u19 in
// cvalidamidia.u19.cpp).
//
// comum::impl::IValidaMidia checks the content of the result medium (MR, "mídia de resultado": the USB
// stick that receives the result files at the end of the day, mounted at /dsk/mr/). Before VOTA writes the
// BU/RDV/log files to it (vota::CCopiaResultadoParaMR, wasm func 12134) the stick must be empty, or contain
// only the files the TSE puts on an initialised stick ("infomidia.dat/.vsc", SA packages, keys...), and
// must not already contain result files of an urna.
//
// RTTI: comum::impl::IValidaMidia (typeinfo @1577084, no vtable of its own: every slot is pure) <-
//       comum::impl::CValidaMidia (typeinfo @1577072, vtable @1577032, 4 bytes = vptr only).
// Slot names are inferred (u09 names slots 3 and 6 after their use in CCopiaResultadoParaMR).
#pragma once

#include <string>
#include <vector>

#include "comum/comumtypes.h"          // EUrnaTurno ('1'/'2'), TPleito, TMunicipio, TZona, TSecao (header ?)

namespace comum {

/// Applications of the urna ecosystem (values 1..20). The web build's CWasmInit reports 10 for VOTA
/// (IInterfaceInit slot 4). Enumerator names unknown; the values used by the validator are:
///   7..10, 15..17: need a valid turno ('1'/'2');  11..14, 18: expect infomidia.dat + infomidia.vsc;
///   15 / 16 / 17: + SA package "t/o/s<5 digits><uf>-pkgsa.jez/.vsc" (treinamento/oficial/simulado);
///   20: "##.pub", "##.id", "##ue", "###ue##.vpe";  7..10: nothing;  19 and 1..6 (valid turno): error 9200.
enum class EAplicativosDeUrna : int;

namespace impl {

/// Result of a validation. Values of `motivo` (enumerator names inferred):
enum class EMotivoValidacao : int {
    MIDIA_VAZIA = 1,              // MR missing or empty, nothing expected                   (valida = true)
    MIDIA_VALIDA = 2,             // only permitted files, every expected file present       (valida = true)
    APLICATIVO_INVALIDO = 4,      // EAplicativosDeUrna outside 1..20
    TURNO_INVALIDO = 5,           // the application needs turno '1' or '2'
    MIDIA_VAZIA_ESPERAVA_ARQUIVOS = 6,
    CONTEM_DIRETORIO = 7,         // a sub-directory other than "." / ".."
    ARQUIVOS_INSUFICIENTES = 8,   // fewer files than expected names
    ARQUIVO_NAO_PERMITIDO = 9,    // a file matches none of the 14 permitted names/patterns
    ARQUIVO_ESPERADO_AUSENTE = 10,
};

struct SResultadoValidacao {       // 8 bytes {bool, int} returned in memory       name as in unit u09
    bool valida;
    EMotivoValidacao motivo;
};

class IValidaMidia {
public:
    static IValidaMidia& GetInst();                                                         // func 5575 (u19)
    virtual ~IValidaMidia() = default;                                                      // [0] 174 / [1] 144

    virtual bool MidiaResultadoValida(EAplicativosDeUrna aplicativo, EUrnaTurno turno) = 0;               // [2]
    virtual SResultadoValidacao ValidaMidiaResultado(EAplicativosDeUrna aplicativo, EUrnaTurno turno) = 0; // [3]
    virtual bool MidiaSemArquivosIndevidos() = 0;                                                        // [4]
    virtual bool MidiaContemResultadosSecao(TPleito pleito, const std::string& uf, TMunicipio municipio,
                                       TZona zona, TSecao secao, char fase) = 0;                          // [5]
    virtual bool MidiaContemResultados() = 0;                                                            // [6]
    virtual bool MidiaContemArquivo(const std::string& arquivo) = 0;                                     // [7]
    virtual bool MidiaContemSomenteResultados() = 0;                                                     // [8]
    virtual void RemoveArquivosSistemaOperacional() = 0;                                                 // [9]
};

class CValidaMidia : public IValidaMidia {
public:
    bool MidiaResultadoValida(EAplicativosDeUrna aplicativo, EUrnaTurno turno) override;                 // func 11173
    SResultadoValidacao ValidaMidiaResultado(EAplicativosDeUrna aplicativo, EUrnaTurno turno) override;  // func 11172
    bool MidiaSemArquivosIndevidos() override;                                                           // func 11171
    bool MidiaContemResultadosSecao(TPleito pleito, const std::string& uf, TMunicipio municipio,
                               TZona zona, TSecao secao, char fase) override;                             // func 11170
    bool MidiaContemResultados() override;                                                               // func 11169
    bool MidiaContemArquivo(const std::string& arquivo) override;                                        // func 11168
    bool MidiaContemSomenteResultados() override;                                                        // func 11167
    void RemoveArquivosSistemaOperacional() override;                                                    // func 11166
};

} // namespace impl
} // namespace comum
