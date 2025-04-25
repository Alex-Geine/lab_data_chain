import os
import subprocess
import signal
import time

b = "/build"
coderPath = "Coder" + b
decoderPath = "Decoder" + b
scoreboardPath = "Scoreboard" + b

def build (path:str):
    root = os.getcwd()
    os.chdir(path)
    os.system("cmake ..")
    os.system("make")
    os.chdir(root)

pathList = [scoreboardPath, decoderPath, coderPath]

for path in pathList:
    build(path)

coderBin = "coder"
decoderBin = "decoder"
scoreboardBin = "scoreboard"

def run(path:str, cmd:str):
    root = os.getcwd()
    os.chdir(path)
    proc = subprocess.Popen(['bash', '-c', cmd])
    time.sleep(1)
    os.chdir(root)
    return proc

port_coder_decoder = " 5555 "
port_decoder_scoreboard = " 5556 "
cmd = "./" + scoreboardBin + port_decoder_scoreboard
cmd1 = "./" + decoderBin + port_decoder_scoreboard + port_coder_decoder
cmd2 = "./" + coderBin + port_coder_decoder
cmdList = [cmd, cmd1, cmd2]

print(cmdList)
print(pathList)

runList = list(zip(pathList, cmdList))
print(runList)
proclist: list[subprocess.Popen]= []
for path, cmd in runList:
    print(path + '   ' + cmd)
    proclist.append(run(path, cmd))

print("Any input to kill processes")
input()

for proc in proclist:
    os.killpg(os.getpgid(proc.pid), signal.SIGKILL)  # или signal.SIGTERM
