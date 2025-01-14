#pragma once
namespace ns_library
{
	class library;
}
class compatible
{
public:
	void to_library(ns_library::library& lib, compatible& comp);
	void from_library(ns_library::library& lib, compatible& comp);
};

