import glob
import subprocess

for f in glob.glob("*.1"):
    #cmd = ["F:\\pandoc-3.9.0.2\\pandoc.exe", "-s", "-f", "man", "-t", "rst", f]
    #with open(f.replace(".1", ".rst"), "w") as x:
    #    subprocess.run(cmd, stdout=x, text=True)
    print(f"tal_src/{f.replace('.1','')}")
    