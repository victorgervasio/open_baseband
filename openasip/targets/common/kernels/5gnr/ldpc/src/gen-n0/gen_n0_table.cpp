// generate_n0_table.cpp
#include <defines.h>
#include <cmath>
#include <fstream>
#include <iomanip>

int main(int argc, char** argv)
{
    if (argc != 2)
        return 1;

    // Put the actual Es/N0 values used by your LDPC program here.
    constexpr float ESN0_START = -4.0f;
    constexpr float ESN0_STEP  = 0.25f;
    constexpr unsigned ESN0_SIZE = LIN_SPACE_NUM;

    std::ofstream out(argv[1]);

    out << "#ifndef N0_TABLE_H\n";
    out << "#define N0_TABLE_H\n\n";

    out << "constexpr unsigned N0_TABLE_SIZE = "
        << ESN0_SIZE << ";\n";

    // Es/N0 values in dB
    out << "constexpr float ESN0_DB_TABLE[N0_TABLE_SIZE] = {\n";

    for (unsigned i = 0; i < ESN0_SIZE; ++i) {
        float esn0_db = ESN0_START + i * ESN0_STEP;

        out << "    "
            << std::fixed
            << std::setprecision(2)
            << esn0_db;

        if (i + 1 != ESN0_SIZE)
            out << ",";

        out << "\n";
    }

    out << "};\n\n";

    // N0 = 10^(-Es/N0[dB] / 10)
    out << "constexpr float N0_TABLE[N0_TABLE_SIZE] = {\n";

    for (unsigned i = 0; i < ESN0_SIZE; ++i) {
        float esn0_db = ESN0_START + i * ESN0_STEP;

        // This is evaluated by the HOST compiler.
        float n0 = std::pow(10.0f, -esn0_db / 10.0f);

        out << "    "
            << std::setprecision(9)
            << n0;

        if (i + 1 != ESN0_SIZE)
            out << ",";

        out << "\n";
    }

    out << "};\n\n";
    out << "#endif\n";
}
