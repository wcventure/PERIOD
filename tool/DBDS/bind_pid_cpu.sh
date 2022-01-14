if [ $1 = "-h" ] || [ $1 = "--help" ]; then
    echo "Use 'sudo ./bind_pid_cpu PID CPU_id' to bind a process to a certain cpu."
    echo "E.g., 'sudo ./bind_pid_cpu \$\$ 0' binds the current shell process to cpu0."
    exit 0
fi

mkdir /dev/cpuset
mount -t cgroup -o cpuset cpuset /dev/cpuset
cd /dev/cpuset
mkdir cpu$2
echo $2 | sudo tee cpu$2/cpuset.cpus
echo 0  | sudo tee cpu$2/cpuset.mems
echo 1  | sudo tee cpuset.cpu_exclusive
echo 0  | sudo tee cpuset.sched_load_balance
echo 1  | sudo tee cpu$2/cpuset.cpu_exclusive
echo 1  | sudo tee cpu$2/cpuset.mem_exclusive
echo $1 | sudo tee cpu$2/tasks

echo "Process $1 is now bound to CPU $2. For more info about cpusets, check:"
echo "https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v1/cpusets.html"
