#pragma once

#include "tracer.h"
#include <wincodec.h>
#include <wrl.h>
#include <string>
#include <sstream>

#pragma comment(lib, "Windowscodecs.lib")

namespace img_processing
{
	namespace wrl = Microsoft::WRL;

	inline void HR(HRESULT res)
	{
		if (res != S_OK)
		{
			throw std::exception();
		}
	}

	struct ComInitialize
	{
		ComInitialize()
		{
			HR(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
		}

		~ComInitialize()
		{
			::CoUninitialize();
		}
	};


	template<typename T>
	inline void CreateInstance(REFCLSID clsid, wrl::ComPtr<T>& ptr) {

		ASSERT(!ptr);
		HR(::CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, __uuidof(T), reinterpret_cast<void**>(ptr.GetAddressOf())));
	}

	struct img_info {
		UINT width;
		UINT height;
		UINT channel_count;
	};

	class imager
	{
		wrl::ComPtr <IWICImagingFactory> _cp_wicfactory;

		UINT get_stride(const UINT width, const UINT bitCount)  // bits per pixel
		{
			ASSERT(0 == bitCount % 8);

			const UINT byteCount = bitCount / 8;
			const UINT stride = (width * byteCount + 3) & ~3;

			ASSERT(0 == stride % sizeof(DWORD));
			return stride;
		}

		std::wstring filename_current_time(const std::wstring& directoryName) {

			auto static counter = 0u;
			auto const fileExt = std::wstring{ L".jpg" };
			auto oss = std::wostringstream{};

			auto time = SYSTEMTIME{};
			::GetLocalTime(&time);

			oss << directoryName << L"img" <<
				time.wDay << L"-" << std::to_wstring(time.wMonth) << L"-" << std::to_wstring(time.wYear) << L"  " << std::to_wstring(time.wHour) << L"-" << std::to_wstring(time.wMinute)
				<< L"-" << std::to_wstring(time.wSecond) << std::to_wstring(counter++) << fileExt;

			return oss.str();
		}

	public:

		imager()
		{
			CreateInstance(CLSID_WICImagingFactory, _cp_wicfactory);
		}

		void save_image(std::wstring directoryName, BYTE* img_src, const int width, const int height)
		{

			directoryName += L"\\";
			auto wfilename = filename_current_time(directoryName);


			auto str_filename = wfilename.c_str();

			/*auto cp_file = wrl::ComPtr < IStream >{};
			HR(SHCreateStreamOnFileEx(str_filename, STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, FILE_ATTRIBUTE_NORMAL, TRUE, nullptr, cp_file.GetAddressOf()));*/


			wrl::ComPtr<IWICStream> cp_file;
			HR(_cp_wicfactory->CreateStream(&cp_file));
			HR(cp_file->InitializeFromFilename(str_filename, GENERIC_WRITE));


			auto imageFormatGuid = GUID(GUID_ContainerFormatJpeg);

			auto cp_encoder = wrl::ComPtr < IWICBitmapEncoder >{};
			HR(_cp_wicfactory->CreateEncoder(imageFormatGuid, nullptr, cp_encoder.GetAddressOf()));

			HR(cp_encoder->Initialize(cp_file.Get(), WICBitmapEncoderNoCache));

			auto cp_frame = wrl::ComPtr < IWICBitmapFrameEncode >{};
			auto cp_properties = wrl::ComPtr < IPropertyBag2 >{};
			HR(cp_encoder->CreateNewFrame(cp_frame.GetAddressOf(), cp_properties.GetAddressOf()));
			HR(cp_frame->Initialize(cp_properties.Get()));


			HR(cp_frame->SetSize(width, height));

			auto pixelFormat = GUID(GUID_WICPixelFormat32bppRGBA);

			cp_frame->SetPixelFormat(&pixelFormat);

			wrl::ComPtr<IWICBitmap> cp_bitmap;

			HR(_cp_wicfactory->CreateBitmapFromMemory(width, height, GUID_WICPixelFormat32bppRGBA, get_stride(width, 32), width*height * 4, img_src, cp_bitmap.GetAddressOf()));
			HR(cp_frame->WriteSource(cp_bitmap.Get(), nullptr));

			//HR(cp_frame->WritePixels(height, get_stride(width, 32), width * height * 4, img_src));
			HR(cp_frame->Commit());
			HR(cp_encoder->Commit());
		}


		auto load_image(_In_ const std::wstring& filepath, _Out_ BYTE** img_result)
		{
			ASSERT(*img_result != nullptr);

			auto cp_decoder = wrl::ComPtr < IWICBitmapDecoder >{};
			HR(_cp_wicfactory->CreateDecoderFromFilename(filepath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, cp_decoder.GetAddressOf()));

			auto cp_decoder_info = wrl::ComPtr<IWICBitmapDecoderInfo>{};
			HR(cp_decoder->GetDecoderInfo(cp_decoder_info.GetAddressOf()));



			auto cp_frame = wrl::ComPtr < IWICBitmapFrameDecode >{};
			HR(cp_decoder->GetFrame(0, cp_frame.GetAddressOf()));

			auto width = UINT{};
			auto height = UINT{};
			HR(cp_frame->GetSize(&width, &height));


			auto cp_format_converter = wrl::ComPtr < IWICFormatConverter >{};
			HR(_cp_wicfactory->CreateFormatConverter(cp_format_converter.GetAddressOf()));

			HR(cp_format_converter->Initialize(cp_frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom));

			//auto rect = WICRect{ 0, 0, static_cast<int>(width), static_cast<int>(height) };
			auto const channel_count = 4;
			auto stride = get_stride(width, channel_count * 8);
			auto buffer_size = width*height * channel_count;
			*img_result = new BYTE[buffer_size];
			HR(cp_format_converter->CopyPixels(0, stride, buffer_size, *img_result));

			//save_image(LR"(N:\)", *img_result, width, height);
			return img_info{ width, height, channel_count};

			
		}
	};
}