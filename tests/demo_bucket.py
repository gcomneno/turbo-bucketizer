import subprocess, sys

ips = ["1.2.3.4","10.0.0.1","192.168.0.1","8.8.8.8"]
for ip in ips:
    out = subprocess.check_output(["../turbo-bucketizer","--ip",ip,"--k","12"], text=True)
    print(out.strip())
