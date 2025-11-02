import re, json, os
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt

OUT_DIR = Path("./out/experiments")
SUMMARY_CSV = Path("./out/experiments") / "results_summary.csv"

def parse_metrics(text: str):
    #Return dict with metrics per the assignment's required template
    def grab(key):
        m = re.search(rf"^{key}:\s*([0-9eE\.\-]+)\s*$", text, flags=re.MULTILINE)
        return float(m.group(1)) if m else None
    return {
        "AverageAF": grab("Average AF"),
        "RecallAtN": grab("Recall@N"),
        "QPS": grab("QPS"),
        "tApproximateAverage_ms": grab("tApproximateAverage"),
        "tTrueAverage_ms": grab("tTrueAverage"),
    }

def parse_filename(fname: str):
    #Filename encodes algo, dataset and params like KEY-VAL separated by __
    #Example: IVFPQ__mnist__kclusters-64__nprobe-5__M-16__nbits-8__N-10__R-2000__seed-1.txt
    stem = Path(fname).stem
    parts = stem.split("__")
    algo = parts[0]
    dataset = parts[1] if len(parts) > 1 else "unknown"
    params = {"algo": algo, "dataset": dataset}
    for part in parts[2:]:
        if "-" in part:
            k, v = part.split("-", 1)
            params[k] = v
    return params

rows = []
files = sorted([p for p in OUT_DIR.glob("*.txt")])
for p in files:
    try:
        txt = p.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        continue
    metrics = parse_metrics(txt)
    params = parse_filename(p.name)
    row = {**params, **metrics, "file": p.name}
    rows.append(row)

df = pd.DataFrame(rows)

#Save CSV
df.to_csv(SUMMARY_CSV, index=False)

#Display table to user
try:
    from caas_jupyter_tools import display_dataframe_to_user
except ImportError:
    display_dataframe_to_user = None

if display_dataframe_to_user:
    display_dataframe_to_user("ANN Results Summary", df)
else:
    print(df.head().to_string())

#Plots (only if we have data)
if not df.empty:
    #1) AF vs Recall
    plt.figure()
    for algo, dfa in df.groupby("algo"):
        plt.scatter(dfa["AverageAF"], dfa["RecallAtN"], label=algo)
    plt.xlabel("Average AF")
    plt.ylabel("Recall@N")
    plt.title("AF vs Recall@N by Algorithm")
    plt.legend()
    plt.tight_layout()
    plt.savefig(OUT_DIR / "plot_af_vs_recall.png")
    plt.close()

    #2) QPS vs Recall
    plt.figure()
    for algo, dfa in df.groupby("algo"):
        plt.scatter(dfa["QPS"], dfa["RecallAtN"], label=algo)
    plt.xlabel("QPS")
    plt.ylabel("Recall@N")
    plt.title("QPS vs Recall@N")
    plt.legend()
    plt.tight_layout()
    plt.savefig(OUT_DIR / "plot_qps_vs_recall.png")
    plt.close()

print(f"Summary saved to: {SUMMARY_CSV}")
print("If plots were generated, they are saved next to the CSV.")
