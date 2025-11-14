#define NOMINMAX
#include "ImageCodecs.h"
#include <algorithm>

#pragma comment(lib, "windowscodecs.lib")

bool ImageCodecs::LoadImage(const wchar_t* filepath, ImageData& out) {
    out.valid = false;

    IWICImagingFactory* factory = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
    );
    if (FAILED(hr)) return false;

    IWICBitmapDecoder* decoder = nullptr;
    hr = factory->CreateDecoderFromFilename(
        filepath,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );
    if (FAILED(hr)) {
        factory->Release();
        return false;
    }

    IWICBitmapFrameDecode* frame = nullptr;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        decoder->Release();
        factory->Release();
        return false;
    }

    UINT width, height;
    frame->GetSize(&width, &height);
    out.width = width;
    out.height = height;

    IWICFormatConverter* converter = nullptr;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        frame->Release();
        decoder->Release();
        factory->Release();
        return false;
    }

    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );
    if (FAILED(hr)) {
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return false;
    }

    out.pixels.resize(width * height * 4);
    hr = converter->CopyPixels(
        nullptr,
        width * 4,
        width * height * 4,
        out.pixels.data()
    );

    converter->Release();
    frame->Release();
    decoder->Release();
    factory->Release();

    if (FAILED(hr)) return false;

    out.valid = true;
    return true;
}

bool ImageCodecs::SaveImage(const wchar_t* filepath, const ImageData& img) {
    // TODO: Implement save functionality
    return false;
}
