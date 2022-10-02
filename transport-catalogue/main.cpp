#include "transport_catalogue.h"
#include "json_reader.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <fstream>

using namespace std;
using namespace input;

int main() {
    try {
        transport_catalogue::TransportCatalogue t_catalogue;
        input::JsonReader reader(t_catalogue);
        reader.ProcessingQueries(LoadJSON(cin), cout);
    } catch (const exception& except) {
        cout << except.what();
    }
    return 0;
}