// uenux2/src/app/comum/dados/asn/eleitor/cconversordedo.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// One fingerprint template of a voter:
//   Dedo ::= SEQUENCE { tipo TipoDedo, qtdMinucias INTEGER, minucias OCTET STRING }
// The OCTET STRING is a bit-packed list of minutiae (see DescompactaMinucias below). This is the only
// place in the binary that documents the format: bits are read most-significant first, groups share
// one angle, and x/y are stored as offsets from a base with a per-template bit width.
#include "comum/dados/asn/eleitor/cconversordedo.h"

#include <cmath>
#include <cstdint>
#include <format>

#include "ecourna/api/util/cbitarray.h"

namespace comum::asn {

namespace {

// Raw element of the unpacked stream: either a group header {angulo, quantidade} or a point {x, y}.
struct SParMinucia {   // 4 bytes   // name inferred
    std::uint16_t a;
    std::uint16_t b;
};

// Helper object built from the unpacked stream (inlined; layout from the stack frame of func 5702):
//   +0 xMin (init 999) +2 bitsX +4 yMin (init 999) +6 bitsY +8 qtdAngulos
//   +12 std::vector<api::SXYT> minucias   +24 std::vector<SParMinucia> compactado
// It computes the bounding box and the bit widths ceil(log2(max - min)) that a re-compression would need;
// only the minutiae list is used here.
class CMinuciasCompactadas   // name inferred
{
public:
    explicit CMinuciasCompactadas(const std::vector<SParMinucia>& pares) : m_compactado(pares)
    {
        std::uint16_t xMax = 0;
        std::uint16_t yMax = 0;
        for (std::size_t i = 0; i < pares.size(); ++i) {
            const auto [angulo, quantidade] = pares[i];
            ++m_qtdAngulos;
            for (const std::size_t fim = i + quantidade; i != fim;) {
                ++i;
                const auto [x, y] = pares.at(i);   // std::out_of_range if a group claims more points than exist
                m_minucias.push_back(api::SXYT{x, y, angulo, 0});
                m_xMin = std::min(m_xMin, x);
                xMax = std::max(xMax, x);
                m_yMin = std::min(m_yMin, y);
                yMax = std::max(yMax, y);
            }
        }
        if (!pares.empty()) {
            // Saturating float->int conversion: NaN/-inf (empty or single-valued range) give 0.
            m_bitsY = static_cast<std::uint16_t>(std::ceil(std::log2(static_cast<double>(yMax - m_yMin))));
            m_bitsX = static_cast<std::uint16_t>(std::ceil(std::log2(static_cast<double>(xMax - m_xMin))));
        }
    }

    const std::vector<api::SXYT>& GetMinucias() const { return m_minucias; }

private:
    std::uint16_t m_xMin = 999;
    std::uint16_t m_bitsX = 0;
    std::uint16_t m_yMin = 999;
    std::uint16_t m_bitsY = 0;
    std::uint16_t m_qtdAngulos = 0;
    std::vector<api::SXYT> m_minucias;
    std::vector<SParMinucia> m_compactado;
};

// wasm func 5702   // name inferred
// Packed format (MSB first):
//   xBase:10  bitsX:4  yBase:10  bitsY:4  qtdGrupos:8
//   qtdGrupos x { angulo:9  quantidade:8  quantidade x { dx:bitsX  dy:bitsY } }      x = xBase+dx, y = yBase+dy
std::vector<api::SXYT> DescompactaMinucias(const ASN1::OCTET_STRING& minucias)
{
    const std::vector<uebyte> bytes(minucias.begin(), minucias.end());   // vector::assign (funcs 5493/5494/5496)

    ecourna::api::util::CBitArray bits;   // wraps a shared boost::dynamic_bitset<unsigned char>
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        for (int bit = 7; bit >= 0; --bit) {
            bits.push_back((bytes.at(i) & (1u << bit)) != 0);
        }
    }

    const auto xBase = bits.GetValor(10);
    const auto bitsX = static_cast<uebyte>(bits.GetValor(4));
    const auto yBase = bits.GetValor(10);
    const auto bitsY = static_cast<uebyte>(bits.GetValor(4));
    const auto qtdGrupos = static_cast<uebyte>(bits.GetValor(8));

    std::vector<SParMinucia> pares;
    for (uebyte g = 0; g < qtdGrupos; ++g) {
        const auto angulo = static_cast<std::uint16_t>(bits.GetValor(9));
        const auto quantidade = static_cast<uebyte>(bits.GetValor(8));
        pares.push_back({angulo, quantidade});
        for (uebyte n = 0; n < quantidade; ++n) {
            const auto x = static_cast<std::uint16_t>(bits.GetValor(bitsX) + xBase);
            const auto y = static_cast<std::uint16_t>(bits.GetValor(bitsY) + yBase);
            pares.push_back({x, y});
        }
    }
    return CMinuciasCompactadas(pares).GetMinucias();
}

} // namespace

// Inlined into wasm func 11416 (srcloc line 159)
md::CDedo::TipoDedo CConversorDedo::DesconverteTipo(const ModuloTiposEleitorais::TipoDedo::NamedNumber tipo)
{
    const int valor = static_cast<int>(tipo);
    if (valor < 1 || valor > 10) {   // naoIdentificado(0) is refused too
        throw CDadosError(7896, std::format("Tipo inválido: {}", tipo));   // line 159
    }
    return static_cast<md::CDedo::TipoDedo>(valor);
}

// wasm func 11416 (vtable slot 3; srcloc line 169, plus the inlined md::CDedo constructor cdedo.cpp:28/33)
md::CDedo CConversorDedo::DoDesconverte(const TEntidade& dedo) const
{
    if (static_cast<std::size_t>(dedo.get_qtdMinucias()) != DescompactaMinucias(dedo.get_minucias()).size()) {
        throw CDadosError(7897, "Campo qtdMinucias não confere com quantidade de minúcias enviadas.");   // line 169
    }
    const auto tipo = DesconverteTipo(dedo.get_tipo());
    const auto qtdMinucias = static_cast<uedword>(dedo.get_qtdMinucias());
    // The template is unpacked a SECOND time for the constructor.
    // md::CDedo::CDedo(TipoDedo, uedword, const std::vector<api::SXYT>&) (inlined):
    //   qtd >= 201                       -> CDadosError(8061, "Quantidade de minúcias menor que 0 ou maior que o maior valor aceito: {}")
    //   empty or more than 200 minutiae  -> CDadosError(8062, "Conjunto de minúcias vazia ou maior que o maior valor aceito: {}")
    return md::CDedo(tipo, qtdMinucias, DescompactaMinucias(dedo.get_minucias()));
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Library code emitted in this TU: wasm funcs 5493 (std::vector<uebyte>::__vdeallocate), 5494
// (__construct_at_end(first, last), wrapped in the invoke_iiiii try protocol) and 5496 (__assign_with_size), i.e.
// std::vector<unsigned char>::assign(first, last). 5496 is also called by (anonymous)::CWasmSavd::RecebeMensagem,
// which is why these three were observed executing during a normal vote.
