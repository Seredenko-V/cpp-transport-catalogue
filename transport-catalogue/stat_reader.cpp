#include "stat_reader.h"

using namespace std;

namespace st {

	StatReader::StatReader(tc::TransportCatalogue& transport_catalogue)
		: transport_catalogue_(transport_catalogue) {
	}

	void StatReader::PrintInformBus(const tc::BusInfo& stat_bus) {
		if (stat_bus.is_empty) {
			cout << "Bus "s << stat_bus.name << ": not found"s << endl;
		} else {
			cout << "Bus "s << stat_bus.name << ": "s << stat_bus.number_stops << " stops on route, "s <<
				stat_bus.number_unique_stops << " unique stops, "s << setprecision(6) << stat_bus.road_distance
				<< " route length, "s << stat_bus.tortuosity << " curvature" << endl;
		}
	}

	string_view StatReader::GetNameBus(string_view text_query) {
		return text_query.substr(text_query.find_first_of(' ') + 1);
	}

	TypeQuery StatReader::GetTypeQuery(string_view text_query) const {
		text_query.remove_suffix(text_query.size() - text_query.find_first_of(' '));
		return text_query == "Stop"sv ? TypeQuery::STOP : TypeQuery::BUS;
	}

	string_view StatReader::GetNameStop(string_view text_query) {
		return text_query.substr(text_query.find_first_of(' ') + 1);
	}

	void StatReader::PrintBusesStop(const tc::StopBuses& stop_buses) {
		cout << "Stop "sv << stop_buses.name << ": "sv;
		if (stop_buses.is_empty) {
			cout << "not found"sv << endl;
			return;
		}
		if (stop_buses.buses_stop.empty()) {
			cout << "no buses"sv << endl;
		} else {
			cout << "buses "sv;
			for (const tc::Bus* const& bus : stop_buses.buses_stop) {
				cout << bus->name << " "sv;
			}
			cout << endl;
		}
	}

	void StatReader::GetStat(istream& input) {
		size_t num_string;
		string text_query;

		input >> num_string; // кол-во запросов
		getline(input, text_query); // извлекается '\n'
		vector<string> output_queries(num_string);

		for (size_t i = 0; i < num_string; ++i) {
			getline(input, text_query);
			output_queries[i] = move(text_query);
		}

		for (const string& query : output_queries) {
			switch (GetTypeQuery(query)) {
			case TypeQuery::BUS:
				PrintInformBus(transport_catalogue_.GetBusInfo(GetNameBus(query)));
				break;
			case TypeQuery::STOP:
				PrintBusesStop(transport_catalogue_.GetListBusesStop(GetNameStop(query)));
				break;
			default:
				break;
			}
		}
	}

}