#include <fstream>
#include <iostream>
#include <sstream>

#include "geometry.h"


namespace TFDEMO {

void _split_words(const std::string& line, std::vector<std::string>& words, char delim) {
    std::stringstream ss(line);
    std::string word = "";
    while (getline(ss, word, delim)) {
        words.emplace_back(word);
    }
}

void load_obj(
    const std::string& path, Vertices& vertices, Faces& faces
) {
    Vertices().swap(vertices);
    Faces().swap(faces);

    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << path << std::endl;
    }

    std::string line;
    try {
        while (getline(file, line)) {
            if (line.empty()) {
                continue;
            }
            if (line[0] == '#') {
                continue;
            }
            std::vector<std::string> words;
            _split_words(line, words, ' ');

            if (words[0] == "v") {
                vertices.emplace_back(Vertex{
                    glm::vec3(stof(words[1]), stof(words[2]), stof(words[3]))
                });
            }
            else if (words[0] == "f") {
                faces.emplace_back(Face{
                    glm::uvec3(stoi(words[1]) - 1, stoi(words[2]) - 1, stoi(words[3]) - 1)
                });
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << "load mesh err: " << e.what() << std::endl;
    }
    file.close();

    std::clog << vertices.size() << " vertices, " << faces.size() << " faces" << std::endl;
}

}
