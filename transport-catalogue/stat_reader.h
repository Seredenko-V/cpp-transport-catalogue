#pragma once

#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>
#include <istream>
#include <string_view>
#include <vector>
#include <string>

using namespace std::literals;

namespace st {

	enum TypeQuery {
		BUS,
		STOP
	};

	class StatReader {
	public:
		StatReader(tc::TransportCatalogue& transport_catalogue);

		void GetStat(std::istream& input);

	private:
		TypeQuery GetTypeQuery(std::string_view text_query) const;

		std::string_view GetNameBus(std::string_view text_query);
		void PrintInformBus(const tc::BusInfo& stat_bus);

		std::string_view GetNameStop(std::string_view text_query);
		void PrintBusesStop(const tc::StopBuses& stop_buses);

		tc::TransportCatalogue& transport_catalogue_;
	};
}