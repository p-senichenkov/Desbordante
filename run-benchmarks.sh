set -e

echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
sudo systemctl stop unattended-upgrades apt-daily apt-daily.timer
sync && echo 3 | sudo tee /proc/sys/vm/drop_caches
taskset -c 1 uv run ~/Desbordante/src/tests/benchmark/benchmarker/main.py results -n50 -d'domain-pac-medium-iowas'
