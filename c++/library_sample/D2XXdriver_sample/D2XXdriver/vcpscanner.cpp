#include "pch.h"
#include "vcpscanner.h"
#include <format>

std::vector<ComPortInfo> vcpscanner::GetComPortList()
{
	std::vector<ComPortInfo> list;

	HDEVINFO hDevInfo = SetupDiGetClassDevs(
		&GUID_DEVCLASS_PORTS,
		NULL,
		NULL,
		DIGCF_PRESENT);

	if (hDevInfo == INVALID_HANDLE_VALUE)
		return list;

	SP_DEVINFO_DATA devInfo = {};
	devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

	for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfo); i++)
	{
		char buffer[256];

		// Friendly Name
		if (SetupDiGetDeviceRegistryPropertyA(
			hDevInfo, &devInfo, SPDRP_FRIENDLYNAME,
			NULL, (PBYTE)buffer, sizeof(buffer), NULL))
		{
			/*
			장치 관리자를 통해 표시되는 포트(COM & LPT)가 다음과 같을 때
			USB Serial Port(COM11)
			USB Serial Port(COM13)
			USB Serial Port(COM7)
			USB Serial Port(COM8)
			통신 포트(COM1)
			통신 포트(COM3)
			통신 포트(COM4)
			통신 포트(COM5)
			통신 포트(COM6)
			*
			friendly:
			USB Serial Port(COM8)
			USB Serial Port(COM7)
			통신 포트(COM1)
			통신 포트(COM3)
			통신 포트(COM4)
			통신 포트(COM5)
			통신 포트(COM6)
			USB Serial Port(COM13)
			USB Serial Port(COM11)
			*/
			std::string friendly = buffer;
			printdebug(std::format("SetupDiGetDeviceRegistryPropertyA friendly: {}", friendly));

			size_t start = friendly.find("(COM");
			if (start != std::string::npos)
			{
				size_t end = friendly.find(")", start);
				std::string com = friendly.substr(start + 1, end - start - 1);

				ComPortInfo info;
				info.portName = com;
				printdebug(std::format("SetupDiGetDeviceRegistryPropertyA info.portName: {}", info.portName));
				info.friendlyName = friendly;
				printdebug(std::format("SetupDiGetDeviceRegistryPropertyA info.friendlyName: {}", info.friendlyName));

				// Hardware ID
				if (SetupDiGetDeviceRegistryPropertyA(
					hDevInfo, &devInfo, SPDRP_LOCATION_INFORMATION/*SPDRP_HARDWAREID*/,
					NULL, (PBYTE)buffer, sizeof(buffer), NULL))
				{
					info.hardwareId = buffer;
					//printdebug(std::format("SetupDiGetDeviceRegistryPropertyA info.hardwareId: {}\n", info.hardwareId));
				}
				// debug
				for( int i=0; i <= 36; i++)
				{
					if (SetupDiGetDeviceRegistryPropertyA(
						hDevInfo, &devInfo, i/*SPDRP_HARDWAREID*/,
						NULL, (PBYTE)buffer, sizeof(buffer), NULL))
					{
						info.hardwareId = buffer;
						printdebug(std::format("SetupDiGetDeviceRegistryPropertyA[debug] i: {}, info.hardwareId: {}\n",i, info.hardwareId));
					}
				}


				list.push_back(info);
			}
		}
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);
	return list;
}

void vcpscanner::printdebug(std::string_view vs)
{
	if (vs.empty())
		return;

	OutputDebugStringA(vs.data());
}
