// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/cqrcodeguard.cpp
// (srcloc record :23).
//
// wasm func 5502 is 13.8 KB because libqrencode's QRcode_encodeString -> QRcode_encodeStringReal ->
// QRinput_new2 / Split_splitStringToQRinput / QRcode_encodeInput / QRcode_encodeMask were inlined into it
// (the out-of-line pieces it still calls: QRspec_lengthIndicator 1147, QRinput_estimateBitsMode{Num,An,8}
// 2719/1899/1534, Split_eat8 5348, QRinput_append 3597, QRinput_getByteStream 5351, RSECC_encode 3595,
// MMask_writeFormatInformation 1253, MQRspec_* 5346/5347/10241, QRcode_free 1254). The constants visible in
// the inlined code fix the arguments:
//   * QRinput_new2: the 28-byte QRinput is zero-initialised -> version 0 (smallest that fits) and
//     level 0 = QR_ECLEVEL_L;
//   * Split_splitString never tests for Kanji and no upper-casing copy is made -> hint QR_MODE_8,
//     casesensitive 1;
//   * errno is set to 28 (EINVAL in Emscripten's WASI numbering) for a NULL/empty string.
#include "api/gui/cqrcodeguard.h"

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// wasm func 5502 (srcloc line 23)
CQRCodeGuard::CQRCodeGuard(const std::string& texto)
    : m_qrcode(QRcode_encodeString(texto.c_str(), 0, QR_ECLEVEL_L, QR_MODE_8, 1))
{
    if (m_qrcode == nullptr)                                                              // :23
        throw CUeGuiError(EUeGuiError{4947}, "Não foi possível criar o QR code");
}

} // namespace api
