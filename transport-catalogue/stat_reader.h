#pragma once

#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>
#include <istream>
#include <string_view>
#include <vector>
#include <string>

using namespace std::literals;

namespace statr {
	enum TypeQuery {
		BUS,
		STOP
	};

	namespace getinf {
		TypeQuery GetTypeQuery(std::string_view text_query);
		std::string_view GetNameBus(std::string_view text_query);
		std::string_view GetNameStop(std::string_view text_query);

	}

	namespace print {
		void PrintInformBus(const tc::BusInfo& stat_bus);
		void PrintBusesStop(const tc::StopBuses& stop_buses);
	}

	class StatReader {
	public:
		StatReader(tc::TransportCatalogue& transport_catalogue);
		void GetStat(std::istream& input);

	private:
		tc::TransportCatalogue& transport_catalogue_;
	};
}