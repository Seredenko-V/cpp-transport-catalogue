#include "input_reader.h"

using namespace std;

namespace input {

	namespace read {
		string ReadLine(istream& input) {
			string s;
			getline(input, s);
			return s;
		}

		int ReadLineWithNumber(istream& input) {
			int result;
			input >> result;
			ReadLine(input);
			return result;
		}

		vector<string> ReadQuery(istream& input) {
			size_t num_string = ReadLineWithNumber(input);
			vector<string> input_queries(num_string);
			for (size_t i = 0; i < num_string; ++i) {
				input_queries[i] = move(ReadLine(input));
			}
			return input_queries;
		}
	}

	namespace parse {
		vector<string> SplitIntoInformBlocks(string&& text, const char symbol) {
			vector<string> inform_blocks;
			size_t begin_block = 0;
			while (begin_block != text.npos) {
				inform_blocks.emplace_back(move(text.substr(begin_block, text.find_first_of(symbol, begin_block) - begin_block)));
				begin_block = text.find_first_of(symbol, begin_block) != text.npos ? text.find_first_of(symbol, begin_block) + 2 : text.npos;
			}
			return inform_blocks;
		}

		pair<size_t, string> GetDistanceToStop(string&& text) {
			size_t end_pos_distance_value = text.find_first_of('m');
			size_t distance_value = static_cast<size_t>(atof(text.substr(0, end_pos_distance_value).c_str()));
			return { distance_value, move(text.substr(end_pos_distance_value + 5)) };
		}

		vector<string> SplitIntoStops(string&& text, const char separator_symbol) {
			vector<string> stops_bus;

			size_t begin_position = 0;
			size_t position_separator = text.find_first_of(separator_symbol, begin_position);
			while (position_separator != text.npos) {
				stops_bus.emplace_back(text.substr(begin_position, position_separator - 1 - begin_position));
				begin_position = position_separator + 2;
				position_separator = text.find_first_of(separator_symbol, begin_position);
			}
			stops_bus.emplace_back(text.substr(begin_position, position_separator - 1 - begin_position));

			return stops_bus;
		}
	}

	InputReader::InputReader(tc::TransportCatalogue& transport_catalogue)
		: transport_catalogue_(transport_catalogue) {
	}

	void InputReader::FillDistanceStops(unordered_map<string, vector<string>>&& inform_blocks_stops) {
		for (auto& [stop, inform_blocks] : inform_blocks_stops) {
			if (inform_blocks.size() <= 2) {
				continue;
			}
			for (size_t i = 2; i < inform_blocks.size(); ++i) {
				pair<size_t, string> distance_to_other_stop = parse::GetDistanceToStop(move(inform_blocks[i]));
				transport_catalogue_.AddDistanceBetweenStops(stop, move(distance_to_other_stop.second), distance_to_other_stop.first);
			}
		}
	}

	void InputReader::FillBuses(vector<string>&& text_buses) {
		for (string& text : text_buses) {
			string name = text.substr(0, text.find_first_of(':'));
			bool bus_is_ring = text.find_first_of('-') != text.npos ? false : true;
			char separator_symbol = bus_is_ring ? '>' : '-';

			vector<string> names_stops = parse::SplitIntoStops(move(text.substr(text.find_first_of(':') + 2)), separator_symbol);
			transport_catalogue_.AddBus(move(name), move(names_stops), bus_is_ring);
		}
	}

	void InputReader::FillCatalogue(vector<string>&& queries) {
		using namespace parse;
		unordered_map<string, vector<string>> inform_blocks_stops;
		inform_blocks_stops.reserve(queries.size() / 2);
		vector<string> text_buses; // для обработки маршрутов ПОСЛЕ обработки остановок

		for (string& text_query : queries) {
			if (text_query.substr(0, text_query.find_first_of(' ')) == "Stop"s) {
				string name_stop = move(text_query.substr(text_query.find_first_of(' ') + 1, text_query.find_first_of(':') - (text_query.find_first_of(' ') + 1)));
				inform_blocks_stops[name_stop] = (SplitIntoInformBlocks(move(text_query.substr(text_query.find_first_of(':') + 1)), ','));
				transport_catalogue_.AddStop(move(name_stop), atof(inform_blocks_stops.at(name_stop)[0].c_str()), atof(inform_blocks_stops.at(name_stop)[1].c_str()));
			} else {
				// добавление перечня остановок в вектор строк без ключевого слова "Bus"
				text_buses.emplace_back(move(text_query.substr(text_query.find_first_of(' ') + 1)));
			}
		}

		FillDistanceStops(move(inform_blocks_stops));
		FillBuses(move(text_buses));		
	}
}