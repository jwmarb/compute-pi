import os
import csv

def parse_time(line: str):
    return line[len("value:"):]

NTASKS          = 0
CPUS_PER_TASK   = 1
N_CPUS          = 2
WALL_TIME       = 3
EFFICIENCY      = 4

if __name__ == '__main__':
    print(parse_time("value:0.003"))
    files = os.listdir("logs")
    rows: list[list[str]] = []
    with open("out/perf.csv", "w", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["ntasks(#nodes)", "cpus-per-task",  "N CPUs", "execution time (seconds)", "parallel efficiency"])
        for file_name in files:
            with open(f"logs/{file_name}") as file:
                ntasks, cpus_per_task = file_name.split("_")
                for line in file:
                    if line.startswith("value:"):
                        rows.append([ntasks, cpus_per_task, str(int(ntasks) * int(cpus_per_task)), parse_time(line), None])
                        break
        baseline = None
        for row in rows:
            if row[N_CPUS] == "1":
                baseline = row
                break

        baseline_walltime = float(baseline[WALL_TIME])

        for row in rows:
            n_cpus, wall_time = int(row[N_CPUS]), float(row[WALL_TIME])
            row[EFFICIENCY] = f"{(baseline_walltime/(n_cpus * wall_time)) * 100}%"
            writer.writerow(row)
    print("Written to out/perf.csv")
