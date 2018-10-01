#ifndef __COMMON_HPP_2017_02_06__
#define __COMMON_HPP_2017_02_06__
#include <algorithm>
#include <vector>
#include <map>
#include <chrono>
#include <stdint.h>
#include <string>
#include <functional>
#include <setupapi.h>
#include <devguid.h>

using params_t = std::map<std::string, std::string>;
using multi_params_t = std::vector<params_t>;
using error_handler_t = std::function<void(const std::string &)>;
using response_handler_t = std::function<void(const params_t &)>;
using response_multi_handler_t = std::function<void(const multi_params_t &)>;
//////////////////////////////////////////////////////////////////////////

static std::pair<std::string, std::string> get_mac_ip()
{
	std::pair<std::string, std::string> ret;

	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS)
	{
		GlobalFree(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		int cnt = 0;
		while (pAdapter)
		{
			u_char local_mac[6] = { 0 };
			DWORD local_iP = 0;
			memcpy(local_mac, pAdapter->Address, 6);

			local_iP = ::inet_addr(pAdapter->IpAddressList.IpAddress.String);

			in_addr in = { 0 };
			in.S_un.S_addr = local_iP;

			char mac_buffer[32] = { 0 };
			sprintf_s(mac_buffer, "%02X-%02X-%02X-%02X-%02X-%02X", local_mac[0], local_mac[1], local_mac[2],
				local_mac[3], local_mac[4], local_mac[5]);

			if (std::string(inet_ntoa(in)) != "0.0.0.0")
				return{ inet_ntoa(in), mac_buffer };

			ret = std::make_pair(inet_ntoa(in), mac_buffer);
			pAdapter = pAdapter->Next;
			cnt++;
		}
	}
	else
	{
		printf("CalltoGetAdaptersInfofailed.n");
	}

	return ret;
}

static std::string file_version(const std::wstring& file)
{
	if (file.empty()) return "0.0.0.0";

	int ver[4] = {};
	DWORD dwVerInfoSize = 0;
	DWORD dwVerHnd;
	VS_FIXEDFILEINFO* pFileInfo;

	dwVerInfoSize = GetFileVersionInfoSize(file.c_str(), &dwVerHnd);
	if (dwVerInfoSize)
	{
		HANDLE  hMem;
		LPVOID  lpvMem;
		unsigned int uInfoSize = 0;

		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpvMem = GlobalLock(hMem);
		GetFileVersionInfo(file.c_str(), dwVerHnd, dwVerInfoSize, lpvMem);
		::VerQueryValue(lpvMem, (LPTSTR)_T("\\"), (void**)&pFileInfo, &uInfoSize);

		ver[0] = HIWORD(pFileInfo->dwProductVersionMS);
		ver[1] = LOWORD(pFileInfo->dwProductVersionMS);
		ver[2] = HIWORD(pFileInfo->dwProductVersionLS);
		ver[3] = LOWORD(pFileInfo->dwProductVersionLS);

		GlobalUnlock(hMem);
		GlobalFree(hMem);
	}
	else
	{
		ver[0] = 0;
		ver[1] = 0;
		ver[2] = 0;
		ver[3] = 0;
	}

	return std::to_string(ver[0]) + "." +
		std::to_string(ver[1]) + "." +
		std::to_string(ver[2]) + "." +
		std::to_string(ver[3]);
}

/*
static std::string file_version(LPCWSTR pFileName)
{
	std::wstring Version(L"0.0.0.0");
	std::wstring FileName(pFileName);
	if (::PathIsRelative(pFileName))
	{
		wchar_t szFileName[MAX_PATH] = { 0 };
		::GetModuleFileName(NULL, szFileName, _countof(szFileName));
		::PathRemoveFileSpec(szFileName);
		::PathAddBackslash(szFileName);
		lstrcat(szFileName, pFileName);
		FileName = szFileName;
	}
	DWORD dwSize = ::GetFileVersionInfoSize(FileName.c_str(), NULL);
	if (dwSize)
	{
		LPTSTR pblock = new TCHAR[dwSize + 1];
		::GetFileVersionInfo(pFileName, 0, dwSize, pblock);
		UINT nQuerySize;
		DWORD* pTransTable = NULL;
		::VerQueryValue(pblock, L"\\VarFileInfo\\Translation", (void **)&pTransTable, &nQuerySize);
		LONG m_dwLangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));
		TCHAR SubBlock[MAX_PATH] = { 0 };
		_stprintf_s(SubBlock, MAX_PATH, TEXT("\\StringFileInfo\\%08lx\\FileVersion"), m_dwLangCharset);
		LPTSTR lpData;
		if (::VerQueryValue(pblock, SubBlock, (PVOID*)&lpData, &nQuerySize))
			Version = lpData;
		delete[] pblock;
	}

	// replace ',' to '.'
	std::replace_if(Version.begin(), Version.end(),
		std::bind2nd(std::equal_to<std::wstring::value_type>(), L','), L'.');

	// delete [Space]
	std::wstring::iterator iter = std::remove_if(Version.begin(), Version.end(),
		std::bind2nd(std::equal_to<std::wstring::value_type>(), L' '));

	// remove redundant character
	if (iter != Version.end())
		Version.erase(iter, Version.end());

	std::string val = CW2A(Version.c_str());

	val.erase(0, val.find_first_not_of((" \n\r\t")));
	val.erase(val.find_last_not_of((" \n\r\t")) + 1);
	return val;
}
*/

static std::vector<char> capture_screen()
{
	using namespace Gdiplus;

	struct init_gdi_plus_t
	{
		ULONG_PTR m_gdiplusToken_ = 0;
		init_gdi_plus_t()
		{
			//	GdiplusStartupInput StartupInput = {};
			//	GdiplusStartup(&m_gdiplusToken_, &StartupInput, NULL);
		}

		~init_gdi_plus_t()
		{
			//	GdiplusShutdown(m_gdiplusToken_);
		}
	};
	static init_gdi_plus_t init;

	auto GetEncoderClsid = [](const WCHAR* format, CLSID* pClsid)
	{
		UINT num = 0;                     // number of image encoders   
		UINT size = 0;                   // size of the image encoder array in bytes   
		ImageCodecInfo* pImageCodecInfo = NULL;
		GetImageEncodersSize(&num, &size);
		if (size == 0)
			return false;     //   Failure   

		pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == NULL)
			return false;     //   Failure   

		GetImageEncoders(num, size, pImageCodecInfo);
		for (UINT j = 0; j < num; ++j)
		{
			if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return true;     //   Success   
			}
		}
		free(pImageCodecInfo);
		return false;     //   Failure   
	};

	auto stream_to_mem = [](IStream *stream, std::vector<char> &buffer)
	{
		ULARGE_INTEGER ulnSize;
		LARGE_INTEGER lnOffset;
		lnOffset.QuadPart = 0;
		/* get the stream size */
		if (stream->Seek(lnOffset, STREAM_SEEK_END, &ulnSize) != S_OK)
		{
			return false;
		}
		if (stream->Seek(lnOffset, STREAM_SEEK_SET, NULL) != S_OK)
		{
			return false;
		}

		/* read it */
		buffer.resize((uint32_t)ulnSize.QuadPart);
		ULONG bytesRead = 0;
		if (stream->Read((void *)buffer.data(), buffer.size(), &bytesRead) != S_OK)
		{
			return false;
		}

		return true;
	};

	HWND hwnd = ::GetDesktopWindow();
	HDC hdc = GetWindowDC(NULL);
	int x = GetDeviceCaps(hdc, HORZRES);
	int y = GetDeviceCaps(hdc, VERTRES);
	HBITMAP hbmp = ::CreateCompatibleBitmap(hdc, x, y), hold;
	HDC hmemdc = ::CreateCompatibleDC(hdc);
	hold = (HBITMAP)::SelectObject(hmemdc, hbmp);
	BitBlt(hmemdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
	SelectObject(hmemdc, hold);

	Bitmap bit(x, y), bit2(hbmp, NULL);
	Graphics g(&bit);
	g.ScaleTransform((float)x / x, (float)y / y);
	g.DrawImage(&bit2, 0, 0);

	::DeleteObject(hbmp);
	::DeleteObject(hmemdc);


	CLSID encoderClsid;
	EncoderParameters encoderParameters;

	int quality = 40;
	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = EncoderQuality;
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;
	encoderParameters.Parameter[0].Value = &quality;

	try
	{
		GetEncoderClsid(L"image/jpeg", &encoderClsid);

		IStreamPtr stream;
		if (::CreateStreamOnHGlobal(NULL, TRUE, &stream) != S_OK)
		{
			return{};
		}

		Gdiplus::Status save_s = bit.Save(stream, &encoderClsid, &encoderParameters);
		if (save_s != Gdiplus::Ok)
			return{};

		std::vector<char> buffer;
		if (!stream_to_mem(stream, buffer))
		{
			return{};
		}

		return buffer;
	}
	catch (_com_error &)
	{
		return{};
	}
}

static std::string url_decode(const std::string &uri)
{
	const unsigned char *ptr = (const unsigned char *)uri.c_str();
	std::string ret;
	ret.reserve(uri.length());
	for (; *ptr; ++ptr)
	{
		if (*ptr == '%')
		{
			if (*(ptr + 1))
			{
				char a = *(ptr + 1);
				char b = *(ptr + 2);
				if (!((a >= 0x30 && a < 0x40) || (a >= 0x41 && a < 0x47))) continue;
				if (!((b >= 0x30 && b < 0x40) || (b >= 0x41 && b < 0x47))) continue;
				char buf[3];
				buf[0] = a;
				buf[1] = b;
				buf[2] = 0;
				ret += (char)strtoul(buf, NULL, 16);
				ptr += 2;
				continue;
			}
		}
		if (*ptr == '+')
		{
			ret += ' ';
			continue;
		}
		ret += *ptr;
	}
	return ret;
}

static BOOL system_restart()
{
	HANDLE				hToken;
	TOKEN_PRIVILEGES	tkp;
	OSVERSIONINFO		osvi;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osvi) == 0) return FALSE;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) return FALSE;
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, 0);
	return TRUE;
}

template < typename ValueType, typename CharT >
static void split(std::vector<ValueType> &seq, const std::basic_string<CharT> &str, CharT separator)
{
	if (str.empty())
		return;

	std::basic_istringstream<CharT> iss(str);
	for (std::basic_string<CharT> s; std::getline(iss, s, separator);)
	{
		ValueType val;
		std::basic_istringstream<CharT> isss(s);

		isss >> val;

		assert(isss.good() || isss.eof());
		seq.push_back(std::move(val));
	}

	return;
}

static std::chrono::system_clock::time_point string_2_time(const std::string &val)
{
	return std::chrono::system_clock::from_time_t(std::stoull(val));
}

static LPCTSTR camera_name(LPTSTR szDevName, DWORD dwSize)
{
	auto GUID = GUID_DEVCLASS_IMAGE;
	DWORD DeviceIndex = 0;
	SP_DEVINFO_DATA DevData = { sizeof(SP_DEVINFO_DATA) };
	HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID, NULL, NULL, DIGCF_PRESENT);
	while (SetupDiEnumDeviceInfo(hDevInfo, DeviceIndex, &DevData))
	{
		TCHAR szBufferId[MAX_PATH] = { 0 };
		SetupDiGetDeviceRegistryProperty(hDevInfo, &DevData, SPDRP_HARDWAREID, NULL, (PBYTE)szBufferId, MAX_PATH*sizeof(TCHAR), NULL);

		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DevData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)szDevName, dwSize*sizeof(TCHAR), NULL) || szDevName[0] == 0)
		{
			SetupDiGetDeviceRegistryProperty(hDevInfo, &DevData, SPDRP_DEVICEDESC, 0, (PBYTE)szDevName, dwSize*sizeof(TCHAR), NULL);
		}
		if (szDevName[0] != 0)
		{
			if (IsEqualIID(GUID, GUID_DEVCLASS_DISKDRIVE))
			{
				std::wstring sDevName = szDevName;
				if (sDevName.find(_T("CGM Virtual")) == std::wstring::npos)
				{
					std::wstring(sDevName);
					lstrcpy(szDevName, sDevName.c_str());
					SetupDiDestroyDeviceInfoList(hDevInfo);
					break;
				}
				else
				{
					memset(szDevName, 0, dwSize);
				}
			}
			else
			{
				std::wstring sDevName = szDevName;

				lstrcpy(szDevName, sDevName.c_str());
				SetupDiDestroyDeviceInfoList(hDevInfo);
				break;
			}
		}
		++DeviceIndex;
	}

	if (hDevInfo)
		SetupDiDestroyDeviceInfoList(hDevInfo);
	return szDevName;
}

#endif
