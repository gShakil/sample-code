#pragma once

#pragma comment(lib, "setupapi.lib")
#include <setupapi.h>
#include <devguid.h>

struct ComPortInfo
{
	std::string portName;     // COM3
	std::string friendlyName; // USB Serial Port (COM3)
	std::string hardwareId;   // FTDI Á¤º¸
};

class vcpscanner
{
public:
	std::vector<ComPortInfo> GetComPortList();

private:
	void printdebug(std::string_view vs);

};

