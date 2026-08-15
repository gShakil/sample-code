#include "pch.h"
#include "ftdideviceConnector.h"
#include <assert.h>
#include <memory>
#include <format>
#include "d2xx/ftd2xx.h"

namespace __ENF::nsconnector_v2
{
	ftdideviceConnector::ftdideviceConnector(const char* szPortName, DWORD dwBaud /*= 115200*/, BYTE byData /*= 8*/, BYTE byStop /*= 0*/, BYTE byParity /*= 0*/)
		:m_port(szPortName)
		, m_baudrate(dwBaud)
		, m_deviceHandle(nullptr)
		, m_bprintdebug(false)
		, m_recvEvent(INVALID_HANDLE_VALUE)
	{
		if (byData == 7)
			m_databit = 7;
		else
			m_databit = 8;

		if (byStop == 2)
			m_stopbit = 2;
		else
			m_stopbit = 1;

		if (byParity == 4)
			m_parity = 4;
		else if (byParity == 3)
			m_parity = 3;
		else if (byParity == 2)
			m_parity = 2;
		else if (byParity == 1)
			m_parity = 1;
		else
			m_parity = 0;
	}

	ftdideviceConnector::~ftdideviceConnector()
	{

	}

	bool ftdideviceConnector::connect()
	{
		// 		vcpscanner vcpscan;
		// 		auto ftdiList = GetFtdiDevices();
		// 		std::vector<ComPortInfo> vcpList = vcpscan.GetComPortList();
		// 
		// 		for (const auto& dev : ftdiList)
		// 		{
		// 			std::string com = FindComPortBySerial(dev.serial, vcpList);
		// 
		// 			printdebug(std::format("FTDI Serial: {}", dev.serial));
		// 			printdebug(std::format("Matched COM: {}", (com.empty() ? "NOT FOUND" : com)));
		// // 			std::cout << "FTDI Serial: " << dev.serial << std::endl;
		// // 			std::cout << "Matched COM: " << (com.empty() ? "NOT FOUND" : com) << std::endl;
		// // 			std::cout << "-----------------------------\n";
		// 		}
		// 
		// 		return 0;
		// 
		// 


		DWORD outNum = 0;
		FT_STATUS stat = FT_CreateDeviceInfoList(&outNum);
		printdebug(std::format("FT_CreateDeviceInfoList. stat: {}, outNum: {}", stat, outNum));
		assert(stat == FT_OK);
		if (stat != FT_OK && outNum <= 0)
			return false;

		// get dev list .. 
		auto pdevList = std::make_unique< FT_DEVICE_LIST_INFO_NODE[]>(outNum);
		//std::unique_ptr<FT_DEVICE_LIST_INFO_NODE> pdevList(new FT_DEVICE_LIST_INFO_NODE[outNum]);
		DWORD count = 0;
		stat = FT_GetDeviceInfoList(pdevList.get(), &count);
		printdebug(std::format("FT_GetDeviceInfoList. stat: {}, count: {}", stat, count));
		assert(stat == FT_OK);
		if (stat != FT_OK)
			return false;

		std::string comNum;
		for (char c : m_port) {
			if (std::isdigit(c)) {
				comNum += c;
			}
		}
		const int toconNum = atoi(comNum.c_str());
		bool con_status = false;

		FT_HANDLE outHandle = nullptr;
		for (DWORD i = 0; i < outNum; i++)
		{
			stat = FT_Open(i, &outHandle);
			printdebug(std::format("FT_Open. stat: {}, outHandle: {}", stat, outHandle));
			if (stat == FT_OK)
			{
				LONG outVCPNum = -1;
				stat = FT_GetComPortNumber(outHandle, &outVCPNum);
				if (stat == FT_OK)
				{
					printdebug(std::format("FT_GetComPortNumber. stat: {}, outVCPNum: {}, find comNUM: {}", stat, outVCPNum, comNum));
					if (toconNum == outVCPNum)
					{
						con_status = true;
						break;
					}
					FT_Close(outHandle);
				}
			}
		}
		if (!con_status)
			return false;

		assert(stat == FT_OK);
		if (stat == FT_OK && outHandle != nullptr)
		{
			// set char
			stat = FT_SetDataCharacteristics(outHandle
				, m_databit
				, m_stopbit == 2 ? FT_STOP_BITS_2 : FT_STOP_BITS_1
				, m_parity
			);
			printdebug(std::format("FT_SetDataCharacteristics. stat: {}. data: {} stop: {} parity: {}"
				, stat
				, m_databit
				, m_stopbit
				, m_parity
			));

			// baud rate ..
			stat = FT_SetBaudRate(outHandle, m_baudrate);
			printdebug(std::format("FT_SetBaudRate - {}. stat: {}", m_baudrate, stat));
			assert(stat == FT_OK);

			// set timeout
			stat = FT_SetTimeouts(outHandle, 10, 10); // write까지 1로 설정하는게 맞을까 .. ? 
			printdebug(std::format("FT_SetTimeouts. stat: {}", stat));
			assert(stat == FT_OK);

			// set latency
			stat = FT_SetLatencyTimer(outHandle, 2);
			UCHAR outlatency = 255;
			FT_GetLatencyTimer(outHandle, &outlatency);
			printdebug(std::format("FT_SetLatencyTimer set 2 ... stat: {}, get: {}", stat, outlatency));
			assert(stat == FT_OK);

			// set recv event
			m_recvEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			FT_SetEventNotification(outHandle, FT_EVENT_RXCHAR, m_recvEvent);

			// 			stat = FT_SetLatencyTimer(outHandle, 2);
			// 			outlatency = 255;
			// 			FT_GetLatencyTimer(outHandle, &outlatency);
			// 			printdebug(std::format("FT_SetLatencyTimer set 2... stat: {}, get: {}", stat, outlatency));
			// 			assert(stat == FT_OK);

			m_deviceHandle = outHandle;
			m_bisconnect.store(true);
			return true;
		}

		return false;
	}

	void ftdideviceConnector::disconnect()
	{
		if (m_deviceHandle != nullptr)
		{
			printdebug("close .. #1");
			FT_HANDLE* dev = (FT_HANDLE*)m_deviceHandle;
			m_deviceHandle = nullptr;
			FT_STATUS stat = FT_Close(dev);
			assert(stat == FT_OK);
			printdebug("close .. #2");

			if (m_recvEvent != INVALID_HANDLE_VALUE)
			{
				printdebug("close set evt .. #1");
				SetEvent(m_recvEvent);
				printdebug("close set evt .. #2");
				CloseHandle(m_recvEvent);
				printdebug("close set evt .. #3");
				m_recvEvent = INVALID_HANDLE_VALUE;
			}
		}
	}

	bool ftdideviceConnector::isconnect()
	{
		return m_deviceHandle != nullptr;
	}

	bool ftdideviceConnector::waitrecvsignal()
	{
		if (!isconnect())
			return false;

		const DWORD waitResult = WaitForSingleObject(m_recvEvent, INFINITE);
		if (waitResult != WAIT_OBJECT_0) 
			return false;

		return true;
	}


	size_t ftdideviceConnector::send(std::vector<unsigned char> vdata, int param /*= 0*/)
	{
		if (isconnect())
		{
			DWORD write = 0;
			const FT_STATUS stat = FT_Write(
				m_deviceHandle
				, vdata.data()
				, (DWORD)vdata.size()
				, &write
			);
			assert(stat == FT_OK);
			if (stat == FT_OK && write > 0)
				return write;
		}
		return 0;
	}

	std::int64_t ftdideviceConnector::recv(unsigned char* buffer, size_t buflen)
	{
		// buflen의 크기가 실제 수신된 크기보다 작은 경우
		// ftdideviceConnector::recv 함수 다시 호출하는 경우 수신됨.

		DWORD rxQueue;
		const FT_STATUS stat = FT_GetQueueStatus(m_deviceHandle, &rxQueue);
		if (stat != FT_OK || rxQueue == 0) {
			return 0;
		}

		if (rxQueue > 0)
		{
			const DWORD toRead = std::min<DWORD>(rxQueue, static_cast<DWORD>(buflen));

			DWORD read = 0;
			const FT_STATUS stat = FT_Read(m_deviceHandle
				, buffer
				, toRead
				, &read
			);
			// 			else
			// 				printdebug(std::format("FT_Read ret: {} read size: 0", stat));
			assert(stat == FT_OK);
			if (stat == FT_OK && read > 0)
				return read;
		}

		return 0;
	}

	void ftdideviceConnector::printdebug(std::string_view vs)
	{
		if (vs.empty() || !m_bprintdebug)
			return;

		OutputDebugStringA(vs.data());
	}

	std::string ftdideviceConnector::vec2string(std::vector<unsigned char> vdat)
	{
		std::string s;
		for (size_t i = 0; i < vdat.size(); ++i)
		{
			s += std::format("0x{:02X}", static_cast<unsigned int>(vdat[i]));
			if (i != vdat.size() - 1)
				s += " ";
		}
		return s;
	}

	void ftdideviceConnector::setprintdebug(bool value)
	{
		m_bprintdebug = value;
	}

}