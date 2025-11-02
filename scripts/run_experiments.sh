#!/usr/bin/env bash
set -euo pipefail

# ==== Paths for the project ====
SEARCH_BIN="./search"
MNIST_DATA="./imgs/train-images.idx3-ubyte"
MNIST_QUERY="./imgs/t10k-images.idx3-ubyte"
SIFT_DATA="./imgs/sift_base.fvecs"
SIFT_QUERY="./imgs/sift_query.fvecs"
OUT_DIR="./out/experiments"

# Seed to keep runs reproducible
SEED=1

# --- CLI options ---
ALGO="all"              # lsh | hypercube | ivfflat | ivfpq | all
DATASETS="mnist,sift"   # "mnist" | "sift" | "mnist,sift"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --algo)      ALGO="$2"; shift 2;;
    --datasets)  DATASETS="$2"; shift 2;;
    *) echo "Unknown arg: $1"; exit 1;;
  esac
done


mkdir -p "${OUT_DIR}"

# --- Append Silhouette to out file (if present) ---
append_silhouette() {
  # Usage: append_silhouette <out_file> <cmd> [args...]
  local out_file="$1"; shift
  local cmd_output sil_line sil_val
  # Run the command, capturing BOTH stdout and stderr
  if ! cmd_output="$("$@" 2>&1)"; then
    # even if the command fails, try to append any silhouette line it produced and then rethrow
    sil_line="$(printf "%s\n" "$cmd_output" | grep -i 'silhouette' | tail -n1 || true)"
    if [[ -n "${sil_line}" ]]; then
      {
        echo ""
        echo "${sil_line}"
      } >> "${out_file}" || true
    fi
    echo "$cmd_output" 1>&2
    return 1
  fi
  # Extract last line containing 'Silhouette'
  sil_line="$(printf "%s\n" "$cmd_output" | grep -i 'silhouette' | tail -n1 || true)"
  if [[ -n "${sil_line}" ]]; then
    # Try to extract a numeric value from that line; fallback to raw line
    sil_val="$(printf "%s\n" "$sil_line" | sed -n 's/.*\([+-]\?[0-9]\+\(\.[0-9]\+\)\?\([eE][+-]\?[0-9]\+\)\?\).*/\1/p' | tail -n1)"
    {
      echo ""
      if [[ -n "${sil_val}" ]]; then
        echo "Silhouette: ${sil_val}"
      else
        echo "${sil_line}"
      fi
    } >> "${out_file}"
  fi
}

run_lsh () { # args: dataset_type (mnist|sift)
  local dtype="$1"
  local data_file query_file
  if [[ "$dtype" == "mnist" ]]; then
    data_file="${MNIST_DATA}"; query_file="${MNIST_QUERY}"
  else
    data_file="${SIFT_DATA}";  query_file="${SIFT_QUERY}"
  fi

  for k in 4 8; do
    for L in 5 10; do
      for w in 4.0 40.0 400.0 1000.0; do
        for N in 10; do
          local tag="LSH__${dtype}__k-${k}__L-${L}__w-${w}__N-${N}__seed-${SEED}"
          "${SEARCH_BIN}" -d "${data_file}" -q "${query_file}" \
            -k ${k} -L ${L} -w ${w} -o "${OUT_DIR}/${tag}.txt" \
            -N ${N} -type "${dtype}" -lsh -range false -seed ${SEED}
        done
      done
    done
  done
}

run_hypercube () {
  local dtype="$1"
  local data_file query_file
  if [[ "$dtype" == "mnist" ]]; then
    data_file="${MNIST_DATA}"; query_file="${MNIST_QUERY}"
  else
    data_file="${SIFT_DATA}";  query_file="${SIFT_QUERY}"
  fi

  for kproj in 8 14 20; do
    for w in 4.0 40.0 400.0 1000.0; do
      for M in 10 100 1000; do
        for probes in 2 4 8; do
          for N in 1 10; do
            local tag="Hypercube__${dtype}__kproj-${kproj}__w-${w}__M-${M}__probes-${probes}__N-${N}__seed-${SEED}"
            "${SEARCH_BIN}" -d "${data_file}" -q "${query_file}" \
              -kproj ${kproj} -w ${w} -M ${M} -probes ${probes} \
              -o "${OUT_DIR}/${tag}.txt" -N ${N} -type "${dtype}" \
              -range false -hypercube -seed ${SEED}
          done
        done
      done
    done
  done
}

run_ivfflat () {
  local dtype="$1"
  local data_file query_file
  if [[ "$dtype" == "mnist" ]]; then
    data_file="${MNIST_DATA}"; query_file="${MNIST_QUERY}"
  else
    data_file="${SIFT_DATA}";  query_file="${SIFT_QUERY}"
  fi

  for kclusters in 200 900; do
    for nprobe in 20; do
      for N in 10; do
        local tag="IVFFlat__${dtype}__kclusters-${kclusters}__nprobe-${nprobe}__N-${N}__seed-${SEED}"
        local out="${OUT_DIR}/${tag}.txt"
        # Run and append silhouette (if printed to stdout/stderr)
        append_silhouette "${out}" \
          "${SEARCH_BIN}" -d "${data_file}" -q "${query_file}" \
            -kclusters ${kclusters} -nprobe ${nprobe} \
            -o "${out}" -N ${N} -type "${dtype}" \
            -range false -ivfflat -seed ${SEED}
      done
    done
  done
}

run_ivfpq () {
  local dtype="$1"
  local data_file query_file
  if [[ "$dtype" == "mnist" ]]; then
    data_file="${MNIST_DATA}"; query_file="${MNIST_QUERY}"
  else
    data_file="${SIFT_DATA}";  query_file="${SIFT_QUERY}"
  fi

  for kclusters in 512; do
    for nprobe in 10 20; do
      for M in 16; do
        for nbits in 10 8; do
          for N in 10; do
            local tag="IVFPQ__${dtype}__kclusters-${kclusters}__nprobe-${nprobe}__M-${M}__nbits-${nbits}__N-${N}__seed-${SEED}"
            local out="${OUT_DIR}/${tag}.txt"
            # Run and append silhouette (if printed to stdout/stderr)
            append_silhouette "${out}" \
              "${SEARCH_BIN}" -d "${data_file}" -q "${query_file}" \
                -kclusters ${kclusters} -nprobe ${nprobe} -M ${M} -nbits ${nbits} \
                -o "${out}" -N ${N} -type "${dtype}" \
                -range false -ivfpq -seed ${SEED}
          done
        done
      done
    done
  done
}

# === Choose which datasets/algorithms to run ===
IFS=',' read -r -a DATA_ARR <<< "$DATASETS"

for dtype in "${DATA_ARR[@]}"; do
  case "$ALGO" in
    lsh)
      run_lsh "$dtype"
      ;;
    hypercube)
      run_hypercube "$dtype"
      ;;
    ivfflat)
      run_ivfflat "$dtype"
      ;;
    ivfpq)
      run_ivfpq "$dtype"
      ;;
    all)
      run_lsh "$dtype"
      run_hypercube "$dtype"
      run_ivfflat "$dtype"
      run_ivfpq "$dtype"
      ;;
    *)
      echo "Unknown --algo '$ALGO'"; exit 1;;
  esac
done

echo "All runs completed. Outputs in: ${OUT_DIR}"
