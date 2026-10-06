// uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp  (+ .h below as a comment)
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// CInformacaoEleicao is a thin, copyable view over the urna parameters (-pu.dat, ecourna::app::dados::
// CParametrosUrna, at CConfiguracaoEleicao +88). Every getter answers a FIXED value when the urna runs in
// demonstration mode (modo demonstração, IInterfaceInit::GetDemoMode()) and the configured parameter otherwise.
// The only srcloc of the file (cinformacaoeleicao.cpp:40) is the one of EhModoDemonstracao(), which LTO inlined
// into every getter; this is why the tools named all eight functions "EhModoDemonstracao".
//
//   class CInformacaoEleicao {
//   public:
//       explicit CInformacaoEleicao(const CConfiguracaoEleicao& cfg);    // vota_f603: m_parametros = &cfg +88
//       static bool EhModoDemonstracao();                                // func 2286
//       int    GetNumBUVotaObrigatorios() const;       // func 3847   (tools: EhModoDemonstracao@3847)
//       int    GetNumBUVotaAdicionais() const;         // func 5914
//       int    GetNumRelatorioEstado() const;          // func 2865   ("Estado da urna" report copies)
//       int    GetNumRelatorioEleitores() const;       // func 5917   ("Lista de eleitores")
//       int    GetNumRelatorioVersoesDados() const;    // func 5916   ("Versões de pacotes")
//       int    GetNumRelatorioPU() const;              // func 5915   ("Parâmetros de urna")
//       bool   IdentificaMesarios() const;             // func 1950   (registrarMesarios)
//       bool   ImprimeBoletimJustificativa() const;    // func 5918   (aceitarJustificativa; vota_f5918, other unit)
//   private:
//       const ecourna::app::dados::CParametrosUrna* m_parametros;   // +0
//   };
//
// In the web build IInterfaceInit is simulador::CWasmInit, whose command 67 (demo mode) answers 0, so
// GetDemoMode() is false and the getters return the -pu.dat values.
#include "comum/informacao/cinformacaoeleicao.h"

#include "api/pattern/cpolysingletonlist.h"
#include "comum/iinterfaceinit.h"

namespace comum {

// wasm func 2286 (srcloc cinformacaoeleicao.cpp:40)
bool CInformacaoEleicao::EhModoDemonstracao()
{
    return api::CPolySingletonList::instance<IInterfaceInit>().GetDemoMode();   // func 611 + func 729
}

// wasm func 3847 (name inferred). Mandatory BU copies printed at the encerramento.
int CInformacaoEleicao::GetNumBUVotaObrigatorios() const
{
    return EhModoDemonstracao() ? 1 : m_parametros->GetNumBUVotaObrigatorios();       // CParametrosUrna +12
}

// wasm func 5914 (name inferred). Extra BU copies the mesário may ask for.
int CInformacaoEleicao::GetNumBUVotaAdicionais() const
{
    return EhModoDemonstracao() ? 1 : m_parametros->GetNumBUVotaAdicionais();         // +16
}

// wasm func 2865 (name inferred; observed executing: CVerificaHorarioZeresima / "Mais informações" menu).
int CInformacaoEleicao::GetNumRelatorioEstado() const
{
    return EhModoDemonstracao() ? 1 : m_parametros->GetNumRelatorioEstado();          // +56
}

// wasm func 5917 (name inferred)
int CInformacaoEleicao::GetNumRelatorioEleitores() const
{
    return EhModoDemonstracao() ? 1 : m_parametros->GetNumRelatorioEleitores();       // +60
}

// wasm func 5916 (name inferred)
int CInformacaoEleicao::GetNumRelatorioVersoesDados() const
{
    return EhModoDemonstracao() ? 1 : m_parametros->GetNumRelatorioVersoesDados();    // +64
}

// wasm func 5915 (name inferred)
int CInformacaoEleicao::GetNumRelatorioPU() const
{
    return EhModoDemonstracao() ? 1 : m_parametros->GetNumRelatorioPU();              // +68
}

// wasm func 1950 (name inferred; other units call it IdentificaMesarios). Mesário registration (and the
// BIM report) is never done in demonstration mode.
bool CInformacaoEleicao::IdentificaMesarios() const
{
    return !EhModoDemonstracao() && m_parametros->GetRegistrarMesarios();               // +400
}

}  // namespace comum
