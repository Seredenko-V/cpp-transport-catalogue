#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>
#include <sstream>

using namespace std;

int main() {
	tc::TransportCatalogue transport_catalogue;

	input::InputReader input(transport_catalogue);
	input.ReadQuery(cin);

	st::StatReader output(transport_catalogue);
	output.GetStat(cin);
	return 0;
}