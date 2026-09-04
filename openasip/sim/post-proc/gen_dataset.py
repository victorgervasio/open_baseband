import re
import csv
import argparse
from pathlib import Path


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
START_END_RE = re.compile(
    r"\[t_sim \[s\] = ([0-9.]+)\]"
    r"\[(SNR_\d{2})\]"
    r"(?:\[Block (\d+)\])?"
    r"(?:\[decode\])?"
    r"(?:\[iIter (\d+) ; iLayer (\d+)\])?"
    r"\s+(Started|Ended) (encode|rate matching|rate recover|decode|checkNodeOperation)"
)


# ------------------------------------------------------------
# Parsing
# ------------------------------------------------------------

def parse_log(log_file):
    """
    Parse a simulation log.

    Returns:
        dict:
            operation -> list of measurements

    Each measurement contains:
        snr
        start_time
        end_time
        elapsed
        block
        iteration
        layer
    """

    # operation -> list of records
    measurements = {
        operation: []
        for operation in OPERATIONS
    }

    # Used to pair Started -> Ended messages.
    active = {}

    with open(log_file, "r", encoding="utf-8", errors="replace") as f:

        for line in f:

            match = START_END_RE.search(line)

            if not match:
                continue

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

            # Convert optional fields
            block = int(block) if block is not None else None
            iteration = int(iteration) if iteration is not None else None
            layer = int(layer) if layer is not None else None

            # Unique key for matching Started and Ended.
            key = (
                snr,
                operation,
                block,
                iteration,
                layer,
            )

            if event == "Started":

                active[key] = timestamp

            elif event == "Ended":

                if key not in active:
                    print(
                        f"WARNING: Ended without Started: "
                        f"{snr} {operation} "
                        f"block={block} iter={iteration} layer={layer}"
                    )
                    continue

                start_time = active.pop(key)
                elapsed = timestamp - start_time

                measurements[operation].append({
                    "snr": snr,
                    "start": start_time,
                    "end": timestamp,
                    "elapsed": elapsed,
                    "block": block,
                    "iteration": iteration,
                    "layer": layer,
                })

    return measurements


# ------------------------------------------------------------
# CSV generation
# ------------------------------------------------------------

def write_operation_csv(operation, measurements, output_dir):

    output_dir.mkdir(parents=True, exist_ok=True)

    filename = operation.replace(" ", "_") + ".csv"
    output_file = output_dir / filename

    records = measurements[operation]

    # --------------------------------------------------------
    # Normal operations
    # --------------------------------------------------------

    if operation != "checkNodeOperation":

        # Each occurrence gets its own row.
        #
        # occurrence | SNR_00 | SNR_01 | ...
        #
        # This is useful because encode/rate matching/etc.
        # occur once per block.

        max_occurrences = max(
            (
                sum(1 for r in records if r["snr"] == snr)
                for snr in SNR_GROUPS
            ),
            default=0
        )

        rows = []

        for occurrence in range(max_occurrences):

            row = {
                "occurrence": occurrence
            }

            for snr in SNR_GROUPS:

                snr_records = [
                    r for r in records
                    if r["snr"] == snr
                ]

                if occurrence < len(snr_records):
                    row[snr] = snr_records[occurrence]["elapsed"]
                else:
                    row[snr] = ""

            rows.append(row)

        fieldnames = ["occurrence"] + SNR_GROUPS

    # --------------------------------------------------------
    # checkNodeOperation
    # --------------------------------------------------------

    else:

        # For checkNodeOperation we want to preserve:
        #
        # Block
        # Iteration
        # Layer
        #
        # while making SNR the columns.
        #
        # Example:
        #
        # Block | Iter | Layer | SNR_00 | SNR_01 | ...
        #   0   |   0  |   0   | 0.0296 | 0.0312 | ...
        #

        keys = set()

        for r in records:
            keys.add(
                (
                    r["block"],
                    r["iteration"],
                    r["layer"],
                )
            )

        keys = sorted(keys)

        rows = []

        for block, iteration, layer in keys:

            row = {
                "block": block,
                "iteration": iteration,
                "layer": layer,
            }

            for snr in SNR_GROUPS:

                matching = [
                    r for r in records
                    if (
                        r["snr"] == snr
                        and r["block"] == block
                        and r["iteration"] == iteration
                        and r["layer"] == layer
                    )
                ]

                if matching:
                    row[snr] = matching[0]["elapsed"]
                else:
                    row[snr] = ""

            rows.append(row)

        fieldnames = [
            "block",
            "iteration",
            "layer",
        ] + SNR_GROUPS

    # --------------------------------------------------------
    # Write CSV
    # --------------------------------------------------------

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
        writer.writerows(rows)

    print(f"Generated: {output_file}")


# ------------------------------------------------------------
# Main
# ------------------------------------------------------------

def main():

    parser = argparse.ArgumentParser(
        description="Extract TTA simulation operation timings from term.log"
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
        default=Path("csv"),
        help="Directory where CSV files will be created"
    )

    args = parser.parse_args()

    measurements = parse_log(args.log_file)

    for operation in OPERATIONS:

        write_operation_csv(
            operation,
            measurements,
            args.output_dir
        )


if __name__ == "__main__":
    main()
