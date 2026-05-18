#include "pch.h"
#include "velopack_usage.h"
#include "velopack/Velopack.hpp"
#include <string>
#include <format>
#include <algorithm>

void update()
{
	Velopack::VelopackApp::Build().Run();

}


void velopack_usage::get_entry()
{
	Velopack::VelopackApp::Build().Run();

	CString updatePath = _T("D:\\\\publish");
	char utf8[260] = { NULL, };
	WideCharToMultiByte(CP_UTF8, NULL, updatePath, -1, utf8, sizeof(utf8), NULL, NULL);

	Velopack::UpdateManager mgr(utf8);

	std::optional<Velopack::UpdateInfo> updInfo = mgr.CheckForUpdates();

	if (!updInfo.has_value())
		return;

	mgr.DownloadUpdates(updInfo.value()
		, [](void* p_user_data, size_t progress)
		{
			static_cast<velopack_usage*>(p_user_data)->progress_callback(progress);
		}
		, this
	);
	mgr.WaitExitThenApplyUpdates(updInfo.value());
	exit(0);
}

bool velopack_usage::update()
{
	OutputDebugString(L"init #1");

	CString s = _T("D:\\예제프로젝트\\samplecode\\c++\\velopack(autoupdate)\\src\\velopack_sample\\outputPackage");
	//CString s = _T("\\\\192.168.1.11\\rex_SW1팀공용\\002.무인단속\\publish");
	char utf8[260] = { NULL, };
	WideCharToMultiByte(CP_UTF8, NULL, s, -1, utf8, sizeof(utf8), NULL, NULL);

	OutputDebugString(L"init #2");
	Velopack::UpdateManager mgr(utf8);
	try
	{
		OutputDebugString(L"init #3");
		std::optional<Velopack::UpdateInfo> updInfo = mgr.CheckForUpdates();

		OutputDebugString(L"init #4");
		if (!updInfo.has_value())
		{
			OutputDebugString(L"updinfo has no value. return.");
			return false;
		}

		OutputDebugString(L"init #5");
		// download the update, optionally providing progress callbacks
		mgr.DownloadUpdates(updInfo.value()
			, [](void* p_user_data, size_t progress)
			{
				static_cast<velopack_usage*>(p_user_data)->progress_callback(progress);
			}
			, this
		);
		OutputDebugString(L"init #6");

		// prepare the Updater in a new process, and wait 60 seconds for this process to exit
		mgr.WaitExitThenApplyUpdates(updInfo.value());
		OutputDebugString(L"init #7");
		exit(99);
		OutputDebugString(L"init #8");
	}
	catch (const std::runtime_error& e)
	{
		const char* utf8Str = e.what();
		// 1. 필요한 버퍼 크기 계산 (WideChar 기준)
		int nLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);

		CStringW strResult;
		LPWSTR pBuf = strResult.GetBuffer(nLen);

		// 2. 실제 변환 수행
		MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, pBuf, nLen);
		strResult.ReleaseBuffer();

		OutputDebugString(strResult);
		AfxMessageBox(strResult);
	}
	catch (const std::exception& e)
	{
		const char* utf8Str = e.what();
		// 1. 필요한 버퍼 크기 계산 (WideChar 기준)
		int nLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);

		CStringW strResult;
		LPWSTR pBuf = strResult.GetBuffer(nLen);

		// 2. 실제 변환 수행
		MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, pBuf, nLen);
		strResult.ReleaseBuffer();
		
		OutputDebugString(strResult);
		AfxMessageBox(strResult);
	}

	return true;
}

bool velopack_usage::downgrade()
{
	OutputDebugString(L"init #1");

	CString s = _T("D:\\예제프로젝트\\samplecode\\c++\\velopack(autoupdate)\\src\\velopack_sample\\outputPackage");
	//CString s = _T("\\\\192.168.1.11\\rex_SW1팀공용\\002.무인단속\\publish");
	char utf8[260] = { NULL, };
	WideCharToMultiByte(CP_UTF8, NULL, s, -1, utf8, sizeof(utf8), NULL, NULL);

	Velopack::UpdateOptions options{};
	options.AllowVersionDowngrade = true;
	options.ExplicitChannel = std::nullopt;
	options.MaximumDeltasBeforeFallback = -1; // downgrade는 full update가 필요하므로 델타 비활성화 권장

	OutputDebugString(L"init #2");
	Velopack::UpdateManager mgr(utf8, &options);
	try
	{
		OutputDebugString(L"init #3");

		OutputDebugString(L"init #4");
		
		Velopack::UpdateInfo updInfo;

		updInfo.IsDowngrade = true;

		OutputDebugString(L"init #5");
		// download the update, optionally providing progress callbacks
		mgr.DownloadUpdates(updInfo
			, [](void* p_user_data, size_t progress)
			{
				static_cast<velopack_usage*>(p_user_data)->progress_callback(progress);
			}
			, this
				);
		OutputDebugString(L"init #6");

		// prepare the Updater in a new process, and wait 60 seconds for this process to exit
		mgr.WaitExitThenApplyUpdates(updInfo);
		OutputDebugString(L"init #7");
		exit(99);
		OutputDebugString(L"init #8");
	}
	catch (const std::runtime_error& e)
	{
		const char* utf8Str = e.what();
		// 1. 필요한 버퍼 크기 계산 (WideChar 기준)
		int nLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);

		CStringW strResult;
		LPWSTR pBuf = strResult.GetBuffer(nLen);

		// 2. 실제 변환 수행
		MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, pBuf, nLen);
		strResult.ReleaseBuffer();

		OutputDebugString(strResult);
		AfxMessageBox(strResult);
	}
	catch (const std::exception& e)
	{
		const char* utf8Str = e.what();
		// 1. 필요한 버퍼 크기 계산 (WideChar 기준)
		int nLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);

		CStringW strResult;
		LPWSTR pBuf = strResult.GetBuffer(nLen);

		// 2. 실제 변환 수행
		MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, pBuf, nLen);
		strResult.ReleaseBuffer();

		OutputDebugString(strResult);
		AfxMessageBox(strResult);
	}

	return true;
}

void velopack_usage::printversion()
{
	OutputDebugString(L"init #1");

	CString s = _T("D:\\예제프로젝트\\samplecode\\c++\\velopack(autoupdate)\\src\\velopack_sample\\outputPackage");
	//CString s = _T("\\\\192.168.1.11\\rex_SW1팀공용\\002.무인단속\\publish");
	char utf8[260] = { NULL, };
	WideCharToMultiByte(CP_UTF8, NULL, s, -1, utf8, sizeof(utf8), NULL, NULL);

	OutputDebugString(L"init #2");
	Velopack::UpdateManager mgr(utf8);
	try
	{
		OutputDebugString(L"init #3");
		std::optional<Velopack::UpdateInfo> updInfo = mgr.CheckForUpdates();

		OutputDebugString(L"init #4");
		if (!updInfo.has_value())
		{
			OutputDebugString(L"updinfo has no value. return.");
			return;
		}

		OutputDebugStringA(std::format("TargetFullRelease: packageID [{}] Verstion [{}] Type [{}] FileName [{}]"
			, updInfo->TargetFullRelease.PackageId
			, updInfo->TargetFullRelease.Version
			, updInfo->TargetFullRelease.Type
			, updInfo->TargetFullRelease.FileName
		).c_str());

		if (updInfo->BaseRelease.has_value())
		{
			OutputDebugStringA(std::format("BaseRelease: packageID [{}] Verstion [{}] Type [{}] FileName [{}]"
				, updInfo->BaseRelease->PackageId
				, updInfo->BaseRelease->Version
				, updInfo->BaseRelease->Type
				, updInfo->BaseRelease->FileName
			).c_str());
		}

		for_each(updInfo->DeltasToTarget.begin(), updInfo->DeltasToTarget.end(),
			[](Velopack::VelopackAsset& asset)
			{
				OutputDebugStringA(std::format("Deltas To Target asset version: {}", asset.Version).c_str());
			}
		);

		OutputDebugStringA(std::format("IsDowngrade: {}", updInfo->IsDowngrade).c_str());
	}
	catch (const std::runtime_error& e)
	{
		const char* utf8Str = e.what();
		// 1. 필요한 버퍼 크기 계산 (WideChar 기준)
		int nLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);

		CStringW strResult;
		LPWSTR pBuf = strResult.GetBuffer(nLen);

		// 2. 실제 변환 수행
		MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, pBuf, nLen);
		strResult.ReleaseBuffer();

		OutputDebugString(strResult);
		AfxMessageBox(strResult);
	}
	catch (const std::exception& e)
	{
		const char* utf8Str = e.what();
		// 1. 필요한 버퍼 크기 계산 (WideChar 기준)
		int nLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);

		CStringW strResult;
		LPWSTR pBuf = strResult.GetBuffer(nLen);

		// 2. 실제 변환 수행
		MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, pBuf, nLen);
		strResult.ReleaseBuffer();

		OutputDebugString(strResult);
		AfxMessageBox(strResult);
	}
}

void velopack_usage::progress_callback(size_t progress)
{
	OutputDebugStringA(std::format("progress callback {}", progress).c_str());
}
