// uenux2/src/app/vota/comum/cassinadorvota.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35). The tools assigned the function to app:comum (comum_f1501)
// because comum::CAssinador's constructor is inlined into it; the last vptr store (@1532312) makes it the
// vota::CAssinadorVota constructor.
#include "vota/comum/cassinadorvota.h"

#include <string>

#include "comum/carquivossavd.h"     // comum::CArquivosSavd (GetInst = func 1164, operator[] = func 275)

namespace comum {

namespace {
// wasm func 5462 (tools: comum_f5462) - everything before the last '/', or the whole text when there is no '/'.
std::string Diretorio(const std::string& caminho);             // name inferred (other unit)
// wasm func 2763 (tools: api_f2763) - everything after the last '/', or the whole text when there is no '/'.
std::string NomeArquivo(const std::string& caminho);           // name inferred (other unit)
} // namespace

// Out-of-line body of the base constructor as it appears inlined in func 1501 (declared in cassinador.h, u23).
// The fields of the object (layout from u23): +4 m_aplicacao, +8 m_pacote, +12 m_arquivo, +24 m_diretorio,
// +36 m_local. m_local stays EMPTY here: only vota::CGravaResultado fills it (15 characters "uf mun zona seção")
// before calling AssinaArquivosResultado; the objects built by this constructor only use Assina() (func 1277).
CAssinador::CAssinador(ESavdAplic aplicacao, ESavdPacote pacote)
    : m_aplicacao(aplicacao),
      m_pacote(pacote)
{
    // Path of the package, e.g. "<GetPathTrab(MI)>/vota.vsu" for pacotes 122/123 (1st/2nd turno) or
    // "<GetPathResult(MI)>/<prefixo>-vota.vsc" for 158/159 (carquivossavd.cpp:49 formats it).
    const std::string caminho = CArquivosSavd::GetInst()[pacote];
    m_diretorio = Diretorio(caminho) + "/";                    // +24
    m_arquivo = NomeArquivo(caminho);                          // +12
}

} // namespace comum

namespace vota {

// wasm func 1501 - observed executing, called from comum_f491 (the state save "Gravando o estado da urna"; 88
// profiler samples in the recorded session). Other callers: the urna's vote synchronisation
// (vota::impl::CSincronismoVotoEleitor, func 7174; the web build uses its own policy), CSincronizaVota::
// SincronizaRelatorios (1836), CPedeAnoNascimento (10590) and helpers 3333 / 4657 / 4668. Callers pass pacote 122/123 (vota.vsu of the
// state files) or 158/159 (vota.vsc of the result files, inlined copy in CGravaResultado).
CAssinadorVota::CAssinadorVota(comum::ESavdPacote pacote)
    : comum::CAssinador(comum::ESavdAplic(1) /* VOTA */, pacote)
{
}

} // namespace vota
