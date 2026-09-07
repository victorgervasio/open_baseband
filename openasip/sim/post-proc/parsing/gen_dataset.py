import re
import csv
import argparse
import time
from pathlib import Path

import matplotlib.pyplot as plt


SNR_GROUPS = [f"SNR_{i:02d}" for i in range(10)]


OPERATIONS = [
    "encode",
    "rate matching",
    "rate recover",
    "decode",
    "checkNodeOperation",
]


# ------------------------------------------------------------
# Regexes
# ------------------------------------------------------------

# Example:
# [t_sim [s] = 0.000534][SNR_00] Started encode
#
# Example:
# [t_sim [s] = 0.0108][SNR_00][Block 0][decode]
# [iIter 7 ; iLayer 15] Ended checkNodeOperation

START_END_RE = re.compile(
    r"\[t_sim \[s\] = ([0-9.]+)\]"
    r"\[(SNR_\d{2})\]"
    r"(?:\[Block (\d+)\])?"
    r"(?:\[decode\])?"
    r"(?:\[iIter (\d+) ; iLayer (\d+)\])?"
    r"\s+(Started|Ended) "
    r"(encode|rate matching|rate recover|decode|checkNodeOperation)"
)


# Example:
# Modulation: BPSK[M,R] = [9000,0.000000]

MODULATION_RE = re.compile(
    r"Modulation:\s*(\S+)\[M,R\]\s*=\s*\["
    r"([0-9.+-]+),([0-9.eE+-]+)\]"
)


# Example:
# EsN0_db = [-4.000000-3.750000-3.500000-3.250000...]

ESN0_RE = re.compile(
    r"EsN0_db\s*=\s*\[([^\]]+)\]"
)


BER_RE = re.compile(
    r"BER\s*=\s*\[([^\]]+)\]"
)


BLER_RE = re.compile(
    r"BLER\s*=\s*\[([^\]]+)\]"
)


# More specific regex used only for EsN0 values.
FLOAT_RE = re.compile(
    r"[+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?"
)


# ------------------------------------------------------------
# Helpers
# ------------------------------------------------------------

def parse_float_list(text):
    """
    Parse a whitespace-separated list of floating-point values.

    Example:
        "0.005222 0.000111 0.000000"

    -> [0.005222, 0.000111, 0.0]
    """

    return [
        float(value)
        for value in text.split()
    ]


def parse_esn0_list(text):
    """
    Parse EsN0 values.

    The simulation output may contain negative values without
    commas or whitespace:

        [-4.000000-3.750000-3.500000-3.250000]

    Signed floating-point values are therefore extracted
    explicitly.
    """

    return [
        float(value)
        for value in FLOAT_RE.findall(text)
    ]


# ------------------------------------------------------------
# Parsing
# ------------------------------------------------------------

def parse_log(log_file):
    """
    Parse the simulation log in a single pass.

    The data structures are organized specifically to avoid
    repeatedly scanning large lists during CSV generation.

    Returns:
        timing_data:
            {
                operation:
                    {
                        snr:
                            [measurement, measurement, ...]
                    }
            }

        check_node_data:
            {
                (block, iteration, layer):
                    {
                        snr: elapsed
                    }
            }

        link_performance:
            dict containing:
                modulation
                modulation_M
                modulation_R
                esn0_db
                ber
                bler
    """

    # --------------------------------------------------------
    # Normal operation data
    #
    # timing_data[operation][snr] -> list of records
    #
    # This replaces the need to repeatedly search:
    #
    #     [r for r in records if r["snr"] == snr]
    #
    # --------------------------------------------------------

    timing_data = {
        operation: {
            snr: []
            for snr in SNR_GROUPS
        }
        for operation in OPERATIONS
        if operation != "checkNodeOperation"
    }


    # --------------------------------------------------------
    # checkNodeOperation data
    #
    # check_node_data[(block, iteration, layer)][snr]
    #     -> elapsed
    #
    # This is the critical optimization.
    #
    # The old implementation repeatedly searched the complete
    # checkNodeOperation record list.
    # --------------------------------------------------------

    check_node_data = {}


    # --------------------------------------------------------
    # Active Started operations
    #
    # key -> start timestamp
    # --------------------------------------------------------

    active = {}


    # --------------------------------------------------------
    # Link-performance results
    # --------------------------------------------------------

    link_performance = {
        "modulation": None,
        "modulation_M": None,
        "modulation_R": None,
        "esn0_db": [],
        "ber": [],
        "bler": [],
    }


    # --------------------------------------------------------
    # Progress information
    # --------------------------------------------------------

    line_count = 0
    timing_count = 0
    check_node_count = 0

    progress_interval = 100_000

    parse_start = time.perf_counter()


    # --------------------------------------------------------
    # Read log
    # --------------------------------------------------------

    with open(
        log_file,
        "r",
        encoding="utf-8",
        errors="replace"
    ) as f:

        for line in f:

            line_count += 1

            # ------------------------------------------------
            # Operation timing
            #
            # This is by far the most common line type, so
            # process it first.
            # ------------------------------------------------

            match = START_END_RE.search(line)

            if match:

                (
                    timestamp,
                    snr,
                    block,
                    iteration,
                    layer,
                    event,
                    operation,
                ) = match.groups()

                timestamp = float(timestamp)

                block = (
                    int(block)
                    if block is not None
                    else None
                )

                iteration = (
                    int(iteration)
                    if iteration is not None
                    else None
                )

                layer = (
                    int(layer)
                    if layer is not None
                    else None
                )

                key = (
                    snr,
                    operation,
                    block,
                    iteration,
                    layer,
                )

                if event == "Started":

                    active[key] = timestamp

                else:  # Ended

                    start_time = active.pop(key, None)

                    if start_time is None:

                        print(
                            f"WARNING: Ended without Started: "
                            f"{snr} {operation} "
                            f"block={block} "
                            f"iter={iteration} "
                            f"layer={layer}"
                        )

                    else:

                        elapsed = timestamp - start_time

                        # ------------------------------------
                        # checkNodeOperation
                        # ------------------------------------

                        if operation == "checkNodeOperation":

                            check_key = (
                                block,
                                iteration,
                                layer,
                            )

                            snr_data = check_node_data.setdefault(
                                check_key,
                                {}
                            )

                            # Preserve the behavior of the
                            # original implementation:
                            #
                            # matching[0]
                            #
                            # Therefore don't overwrite an
                            # existing value.
                            if snr not in snr_data:
                                snr_data[snr] = elapsed

                            check_node_count += 1

                        # ------------------------------------
                        # Normal operation
                        # ------------------------------------

                        else:

                            timing_data[
                                operation
                            ][snr].append({
                                "start": start_time,
                                "end": timestamp,
                                "elapsed": elapsed,
                                "block": block,
                                "iteration": iteration,
                                "layer": layer,
                            })

                            timing_count += 1

                # No need to test the other regexes for an
                # operation line.
                continue


            # ------------------------------------------------
            # Modulation
            # ------------------------------------------------

            match = MODULATION_RE.search(line)

            if match:

                (
                    modulation,
                    modulation_M,
                    modulation_R,
                ) = match.groups()

                link_performance["modulation"] = modulation
                link_performance["modulation_M"] = float(
                    modulation_M
                )
                link_performance["modulation_R"] = float(
                    modulation_R
                )

                continue


            # ------------------------------------------------
            # EsN0
            # ------------------------------------------------

            match = ESN0_RE.search(line)

            if match:

                link_performance["esn0_db"] = (
                    parse_esn0_list(
                        match.group(1)
                    )
                )

                continue


            # ------------------------------------------------
            # BER
            # ------------------------------------------------

            match = BER_RE.search(line)

            if match:

                link_performance["ber"] = (
                    parse_float_list(
                        match.group(1)
                    )
                )

                continue


            # ------------------------------------------------
            # BLER
            # ------------------------------------------------

            match = BLER_RE.search(line)

            if match:

                link_performance["bler"] = (
                    parse_float_list(
                        match.group(1)
                    )
                )


            # ------------------------------------------------
            # Progress
            # ------------------------------------------------

            if line_count % progress_interval == 0:

                elapsed = (
                    time.perf_counter() - parse_start
                )

                print(
                    f"Parsed {line_count:,} lines "
                    f"({elapsed:.1f} s)"
                )


    parse_elapsed = (
        time.perf_counter() - parse_start
    )


    # --------------------------------------------------------
    # Report parsing statistics
    # --------------------------------------------------------

    print()
    print(
        f"Finished parsing {line_count:,} lines "
        f"in {parse_elapsed:.2f} s"
    )

    print(
        f"Timing measurements: {timing_count:,}"
    )

    print(
        f"checkNodeOperation measurements: "
        f"{check_node_count:,}"
    )

    print(
        f"checkNodeOperation unique "
        f"(block, iteration, layer) keys: "
        f"{len(check_node_data):,}"
    )


    return (
        timing_data,
        check_node_data,
        link_performance,
    )


# ------------------------------------------------------------
# Link-performance CSV
# ------------------------------------------------------------

def write_link_performance_csv(
    link_performance,
    output_dir
):
    """
    Write EsN0 / BER / BLER results to CSV.
    """

    output_dir.mkdir(
        parents=True,
        exist_ok=True
    )

    output_file = (
        output_dir / "link_performance.csv"
    )

    esn0 = link_performance["esn0_db"]
    ber = link_performance["ber"]
    bler = link_performance["bler"]


    if not (
        len(esn0) ==
        len(ber) ==
        len(bler)
    ):
        raise ValueError(
            "EsN0, BER and BLER arrays have "
            "different lengths: "
            f"EsN0={len(esn0)}, "
            f"BER={len(ber)}, "
            f"BLER={len(bler)}"
        )


    with open(
        output_file,
        "w",
        newline="",
        encoding="utf-8"
    ) as f:

        writer = csv.writer(f)

        writer.writerow([
            "EsN0_dB",
            "BER",
            "BLER",
        ])

        writer.writerows(
            zip(esn0, ber, bler)
        )


    print(
        f"Generated: {output_file}"
    )


# ------------------------------------------------------------
# Link-performance plots
# ------------------------------------------------------------

def plot_link_performance(
    link_performance,
    output_dir
):
    """
    Generate high-resolution BER and BLER versus Es/N0 plots.

    The channel is assumed to be AWGN.

    Zero BER/BLER values cannot be represented on a
    logarithmic y-axis, so they are omitted from the plotted
    curve but remain in link_performance.csv.
    """

    output_dir.mkdir(
        parents=True,
        exist_ok=True
    )

    esn0 = link_performance["esn0_db"]
    ber = link_performance["ber"]
    bler = link_performance["bler"]

    modulation = link_performance["modulation"]
    modulation_M = link_performance["modulation_M"]
    modulation_R = link_performance["modulation_R"]


    if not (
        len(esn0) ==
        len(ber) ==
        len(bler)
    ):
        raise ValueError(
            "EsN0, BER and BLER arrays have "
            "different lengths."
        )


    # --------------------------------------------------------
    # Plot helper
    # --------------------------------------------------------

    def make_plot(
        y_values,
        ylabel,
        filename,
        title
    ):
        
        plots_output_dir = Path(output_dir,"img")
        plots_output_dir.mkdir(
            parents=True,
            exist_ok=True
        )

        valid = [
            (x, y)
            for x, y in zip(
                esn0,
                y_values
            )
            if y > 0
        ]


        if not valid:

            print(
                f"WARNING: No positive values available "
                f"for {ylabel}; plot not generated."
            )

            return


        x_plot, y_plot = zip(*valid)


        fig, ax = plt.subplots(
            figsize=(8, 6),
            dpi=300
        )


        ax.semilogy(
            x_plot,
            y_plot,
            marker="o",
            linewidth=1.8,
            markersize=6,
        )


        ax.set_xlabel(
            r"$E_s/N_0$ (dB)",
            fontsize=12
        )


        ax.set_ylabel(
            ylabel,
            fontsize=12
        )


        ax.set_title(
            title,
            fontsize=13
        )


        ax.grid(
            True,
            which="both",
            linestyle="--",
            linewidth=0.6,
            alpha=0.7
        )


        ax.tick_params(
            axis="both",
            labelsize=10
        )


        fig.tight_layout()


        output_file = (
            plots_output_dir / filename
        )


        # 600 DPI for thesis-quality raster output.
        fig.savefig(
            output_file,
            dpi=600,
            bbox_inches="tight"
        )


        plt.close(fig)


        print(
            f"Generated: {output_file}"
        )


    # --------------------------------------------------------
    # Caption
    # --------------------------------------------------------

    if modulation is not None:

        modulation_description = (
            f"{modulation}, "
            f"M={modulation_M:g}, "
            f"R={modulation_R:g}"
        )

    else:

        modulation_description = "BPSK"


    # --------------------------------------------------------
    # BER
    # --------------------------------------------------------

    make_plot(
        ber,
        "Bit Error Rate (BER)",
        "ber_vs_esn0.png",
        (
            f"BER vs. $E_s/N_0$ for "
            f"{modulation_description} "
            f"over an AWGN channel"
        )
    )


    # --------------------------------------------------------
    # BLER
    # --------------------------------------------------------

    make_plot(
        bler,
        "Block Error Rate (BLER)",
        "bler_vs_esn0.png",
        (
            f"BLER vs. $E_s/N_0$ for "
            f"{modulation_description} "
            f"over an AWGN channel"
        )
    )


# ------------------------------------------------------------
# Operation CSV generation
# ------------------------------------------------------------

def write_operation_csv(
    operation,
    timing_data,
    check_node_data,
    output_dir
):
    """
    Generate the CSV for one operation.

    This implementation performs only direct dictionary/list
    access. It does NOT repeatedly scan the complete set of
    measurements.
    """

    output_dir.mkdir(
        parents=True,
        exist_ok=True
    )

    filename = (
        operation.replace(" ", "_")
        + ".csv"
    )

    output_file = (
        output_dir / filename
    )


    # --------------------------------------------------------
    # Normal operations
    # --------------------------------------------------------

    if operation != "checkNodeOperation":

        snr_records = timing_data[operation]


        max_occurrences = max(
            (
                len(snr_records[snr])
                for snr in SNR_GROUPS
            ),
            default=0
        )


        fieldnames = [
            "occurrence"
        ] + SNR_GROUPS


        with open(
            output_file,
            "w",
            newline="",
            encoding="utf-8"
        ) as f:

            writer = csv.DictWriter(
                f,
                fieldnames=fieldnames
            )

            writer.writeheader()


            for occurrence in range(
                max_occurrences
            ):

                row = {
                    "occurrence": occurrence
                }


                for snr in SNR_GROUPS:

                    records = snr_records[snr]


                    if occurrence < len(records):

                        row[snr] = (
                            records[
                                occurrence
                            ]["elapsed"]
                        )

                    else:

                        row[snr] = ""


                writer.writerow(row)


    # --------------------------------------------------------
    # checkNodeOperation
    # --------------------------------------------------------

    else:

        fieldnames = [
            "block",
            "iteration",
            "layer",
        ] + SNR_GROUPS


        # Sorting is performed once.
        keys = sorted(
            check_node_data.keys()
        )


        with open(
            output_file,
            "w",
            newline="",
            encoding="utf-8"
        ) as f:

            writer = csv.DictWriter(
                f,
                fieldnames=fieldnames
            )

            writer.writeheader()


            for (
                block,
                iteration,
                layer
            ) in keys:

                snr_data = check_node_data[
                    (
                        block,
                        iteration,
                        layer
                    )
                ]


                row = {
                    "block": block,
                    "iteration": iteration,
                    "layer": layer,
                }


                # Direct dictionary lookup.
                for snr in SNR_GROUPS:

                    row[snr] = snr_data.get(
                        snr,
                        ""
                    )


                writer.writerow(row)


    print(
        f"Generated: {output_file}"
    )


def plot_average_operation_times(timing_data, output_dir):
    """
    Generate one plot per operation showing average elapsed time
    as a function of SNR.

    timing_data:
        {
            operation: {
                snr: [record, record, ...]
            }
        }

    Each record must contain an "elapsed" field.
    """
    output_dir.mkdir(parents=True, exist_ok=True)

    for operation in OPERATIONS:
        snr_data = timing_data.get(operation, {})

        if not snr_data:
            continue

        snr_values = []
        average_times = []

        # Keep SNR_00, SNR_01, ... in numerical order
        for snr in SNR_GROUPS:
            records = snr_data.get(snr, [])

            if not records:
                continue

            elapsed_times = [
                record["elapsed"]
                for record in records
            ]

            average_time = sum(elapsed_times) / len(elapsed_times)

            snr_values.append(snr)
            average_times.append(average_time)

        if not average_times:
            continue

        fig, ax = plt.subplots(figsize=(8, 6))

        ax.plot(
            snr_values,
            average_times,
            marker="o",
            linewidth=2,
        )

        ax.set_xlabel("SNR")
        ax.set_ylabel("Average elapsed time [s]")
        ax.set_title(
            f"Average elapsed time of {operation} per SNR"
        )

        ax.grid(True, which="both", linestyle="--", alpha=0.5)

        fig.tight_layout()

        filename = (
            operation.replace(" ", "_")
            + "_average_time_vs_snr.png"
        )

        fig.savefig(
            output_dir / filename,
            dpi=600,
            bbox_inches="tight",
        )

        plt.close(fig)

# ------------------------------------------------------------
# Main
# ------------------------------------------------------------

def main():

    parser = argparse.ArgumentParser(
        description=(
            "Extract TTA/x86_64 simulation operation "
            "timings and LDPC link-performance results "
            "from term.log"
        )
    )


    parser.add_argument(
        "log_file",
        type=Path,
        help="Path to term.log"
    )


    parser.add_argument(
        "-o",
        "--output-dir",
        type=Path,
        required=True,
        help=(
            "Directory where CSV files and plots "
            "will be created"
        )
    )


    args = parser.parse_args()


    # --------------------------------------------------------
    # Parse
    # --------------------------------------------------------

    total_start = time.perf_counter()


    (
        timing_data,
        check_node_data,
        link_performance,
    ) = parse_log(
        args.log_file
    )


    parse_finished = time.perf_counter()

    plot_average_operation_times(timing_data, Path(args.output_dir,"img"))

    # --------------------------------------------------------
    # Operation timing CSVs
    # --------------------------------------------------------

    for operation in OPERATIONS:

        write_operation_csv(
            operation,
            timing_data,
            check_node_data,
            args.output_dir
        )


    # --------------------------------------------------------
    # Link-performance CSV and plots
    # --------------------------------------------------------

    if (
        link_performance["esn0_db"]
        and link_performance["ber"]
        and link_performance["bler"]
    ):

        write_link_performance_csv(
            link_performance,
            args.output_dir
        )


        plot_link_performance(
            link_performance,
            args.output_dir
        )

    else:

        print(
            "WARNING: Link-performance data not found "
            "or incomplete. No BER/BLER plots generated."
        )


    # --------------------------------------------------------
    # Timing summary
    # --------------------------------------------------------

    total_elapsed = (
        time.perf_counter() - total_start
    )

    csv_elapsed = (
        time.perf_counter() - parse_finished
    )


    print()
    print("=" * 60)
    print("Dataset generation completed")
    print("=" * 60)
    print(
        f"Parsing time:       "
        f"{parse_finished - total_start:.2f} s"
    )
    print(
        f"CSV/plot time:      "
        f"{csv_elapsed:.2f} s"
    )
    print(
        f"Total time:         "
        f"{total_elapsed:.2f} s"
    )
    print("=" * 60)


if __name__ == "__main__":
    main()
