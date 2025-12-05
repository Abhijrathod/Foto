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
    if (!img.valid || img.pixels.empty() || img.width == 0 || img.height == 0) {
        return false;
    }

    // Determine file format from extension
    const wchar_t* ext = wcsrchr(filepath, L'.');
    if (!ext) return false;

    GUID containerFormat = GUID_ContainerFormatPng; // Default to PNG
    if (_wcsicmp(ext, L".jpg") == 0 || _wcsicmp(ext, L".jpeg") == 0) {
        containerFormat = GUID_ContainerFormatJpeg;
    } else if (_wcsicmp(ext, L".bmp") == 0) {
        containerFormat = GUID_ContainerFormatBmp;
    } else if (_wcsicmp(ext, L".tiff") == 0 || _wcsicmp(ext, L".tif") == 0) {
        containerFormat = GUID_ContainerFormatTiff;
    }

    IWICImagingFactory* factory = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
    );
    if (FAILED(hr)) return false;

    // Create stream
    IWICStream* stream = nullptr;
    hr = factory->CreateStream(&stream);
    if (FAILED(hr)) {
        factory->Release();
        return false;
    }

    hr = stream->InitializeFromFilename(filepath, GENERIC_WRITE);
    if (FAILED(hr)) {
        stream->Release();
        factory->Release();
        return false;
    }

    // Create encoder
    IWICBitmapEncoder* encoder = nullptr;
    hr = factory->CreateEncoder(containerFormat, nullptr, &encoder);
    if (FAILED(hr)) {
        stream->Release();
        factory->Release();
        return false;
    }

    hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
    if (FAILED(hr)) {
        encoder->Release();
        stream->Release();
        factory->Release();
        return false;
    }

    // Create frame
    IWICBitmapFrameEncode* frame = nullptr;
    IPropertyBag2* props = nullptr;
    hr = encoder->CreateNewFrame(&frame, &props);
    if (FAILED(hr)) {
        encoder->Release();
        stream->Release();
        factory->Release();
        return false;
    }

    // Set JPEG quality if applicable
    if (containerFormat == GUID_ContainerFormatJpeg && props) {
        PROPBAG2 option = {};
        option.pstrName = const_cast<LPOLESTR>(L"ImageQuality");
        VARIANT varValue;
        VariantInit(&varValue);
        varValue.vt = VT_R4;
        varValue.fltVal = 0.9f; // 90% quality
        props->Write(1, &option, &varValue);
    }

    hr = frame->Initialize(props);
    if (props) props->Release();
    if (FAILED(hr)) {
        frame->Release();
        encoder->Release();
        stream->Release();
        factory->Release();
        return false;
    }

    hr = frame->SetSize(img.width, img.height);
    if (FAILED(hr)) {
        frame->Release();
        encoder->Release();
        stream->Release();
        factory->Release();
        return false;
    }

    WICPixelFormatGUID format = GUID_WICPixelFormat32bppRGBA;
    hr = frame->SetPixelFormat(&format);
    if (FAILED(hr)) {
        frame->Release();
        encoder->Release();
        stream->Release();
        factory->Release();
        return false;
    }

    // Write pixels
    hr = frame->WritePixels(
        img.height,
        img.width * 4,
        img.width * img.height * 4,
        const_cast<uint8_t*>(img.pixels.data())
    );
    if (FAILED(hr)) {
        frame->Release();
        encoder->Release();
        stream->Release();
        factory->Release();
        return false;
    }

    hr = frame->Commit();
    if (FAILED(hr)) {
        frame->Release();
        encoder->Release();
        stream->Release();
        factory->Release();
        return false;
    }

    hr = encoder->Commit();
    if (FAILED(hr)) {
        frame->Release();
        encoder->Release();
        stream->Release();
        factory->Release();
        return false;
    }

    frame->Release();
    encoder->Release();
    stream->Release();
    factory->Release();

    return true;
}
