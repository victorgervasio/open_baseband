#include <random>
#include <fstream>

int main(int argc, char** argv)
{
    if (argc != 2)
        return 1;

    std::default_random_engine engine(12345);
    std::normal_distribution<float> dist(0.0, 1.0);

    std::ofstream out(argv[1]);

    out << "#ifndef NORMAL_SAMPLES_H\n";
    out << "#define NORMAL_SAMPLES_H\n\n";
    out << "constexpr unsigned NORMAL_SAMPLES_SIZE = 10000;\n";
    out << "constexpr float NORMAL_SAMPLES[NORMAL_SAMPLES_SIZE] = {\n";

    for (unsigned i = 0; i < 10000; ++i) {
        out << "    " << dist(engine);
        if (i != 9999)
            out << ",";
        out << "\n";
    }

    out << "};\n\n";
    out << "#endif\n";
}
