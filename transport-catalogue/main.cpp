#include "transport_catalogue.h"
#include "json_reader.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <fstream>

using namespace std;
using namespace input;

int main() {

	istringstream input_JSON {"{\"render_settings\": {\"width\": 32107.28117719066, \"height\": 71936.98012090003, \"padding\": 53.98914803751499, \"stop_radius\": 42038.64102039122, \"line_width\": 86086.78648467388, \"stop_label_font_size\": 7597, \"stop_label_offset\": [-32278.039724323884, 18486.788689490102], \"underlayer_color\": \"tan\", \"underlayer_width\": 7741.009226568677, \"color_palette\": [[103, 135, 214], [101, 99, 86, 0.747283216592306], [94, 220, 111], [126, 42, 128, 0.13710542817098337], \"black\", [152, 87, 195], [48, 200, 247], [187, 136, 15], [24, 184, 213], [5, 27, 162], [224, 196, 57, 0.0190199476263343], [133, 18, 82], [220, 178, 51], \"black\", [86, 113, 127], \"teal\", \"orchid\", \"pink\", [108, 110, 78, 0.32184669570841495], \"yellow\", [214, 108, 203], [190, 133, 116], \"sienna\", \"sienna\", [178, 165, 92], \"bisque\", [40, 188, 159], \"purple\", \"white\", \"yellow\", \"olive\", \"orange\", \"peru\", [130, 51, 157], [18, 150, 242, 0.9594513931198525], [31, 40, 8], \"khaki\", \"bisque\", [94, 250, 155, 0.4039707643645265], \"purple\", [253, 255, 79], [214, 53, 228, 0.5463664997714781], [229, 149, 20], [251, 119, 117], [48, 138, 94], [14, 177, 230], \"magenta\", [32, 241, 32], [89, 63, 237], \"gray\", [143, 204, 8, 0.26035436190357264], [77, 251, 241], [22, 201, 241, 0.2902943412037535], \"orchid\", [0, 67, 130], [161, 1, 29], \"lavender\", [217, 39, 158], \"indigo\", \"thistle\", [145, 91, 201], [44, 143, 176], \"lime\", [174, 149, 29, 0.31475581042865963], \"khaki\", [11, 138, 115, 0.3218710946422525], [143, 94, 106, 0.6201153897184918], [111, 180, 40], \"wheat\", \"teal\", \"white\", [201, 193, 77], \"cyan\", \"bisque\", [201, 108, 38], [85, 98, 227], [170, 41, 170], \"orange\", [208, 54, 82], \"blue\", \"white\", [3, 34, 192], [31, 39, 169, 0.7603500253499714], [209, 250, 12], \"aqua\", [238, 233, 134, 0.18853011813535747], \"lavender\", \"wheat\", [214, 113, 144, 0.9197251498905594], [31, 211, 158], \"fuchsia\", [146, 114, 144], \"khaki\", \"wheat\", \"orchid\", \"cornsilk\"], \"bus_label_font_size\": 87702, \"bus_label_offset\": [14992.124856072842, -70426.57766719483]}, \"base_requests\": [{\"type\": \"Stop\", \"name\": \"A ASTANOVKA AFNA\", \"latitude\": 55.611087, \"longitude\": 37.20829, \"road_distances\": {}}, {\"type\": \"Stop\", \"name\": \"B G\", \"latitude\": 55.595884, \"longitude\": 37.209755, \"road_distances\": {\"A ASTANOVKA AFNA\": 1000, \"C\": 1000}}, {\"type\": \"Stop\", \"name\": \"C\", \"latitude\": 55.632761, \"longitude\": 37.333324, \"road_distances\": {\"A ASTANOVKA AFNA\": 1000}}, {\"type\": \"Bus\", \"name\": \"ABC\", \"stops\": [\"A ASTANOVKA AFNA\", \"B G\", \"C\", \"A ASTANOVKA AFNA\"], \"is_roundtrip\": true}], \"stat_requests\": [{\"id\": 445724299, \"type\": \"Bus\", \"name\": \"ABC\"}]}"
	};
    ifstream fin("TestJSON.txt");
    //ofstream fout("OutSVG.svg");

    try {
        transport_catalogue::TransportCatalogue t_catalogue;

        input::JsonReader reader(t_catalogue);
        reader.ProcessingQueries(LoadJSON(fin), cout);
    } catch (const exception& except) {
        cout << except.what();
    }
	return 0;
}