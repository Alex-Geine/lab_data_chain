import os
import subprocess
import signal
import time

b              = "/build"
coderPath      = "Coder" + b
decoderPath    = "Decoder" + b
scoreboardPath = "Scoreboard" + b
mapperPath     = "Mapper" + b
demapperPath   = "Demapper" + b

def build (path:str):
    root = os.getcwd()
    os.chdir(path)
    os.system("cmake ..")
    os.system("make")
    os.chdir(root)

# all chain
pathList = [scoreboardPath, decoderPath, demapperPath, mapperPath, coderPath]

# coder_decoder chain
pathListCoderDecoder = [scoreboardPath, decoderPath, coderPath]

# mapper_demapper chain
pathListMapperDemapper = [scoreboardPath, demapperPath, mapperPath]

for path in pathList:
    build(path)

coderBin      = "coder"
decoderBin    = "decoder"
scoreboardBin = "scoreboard"
mapperBin     = "mapper"
demapperBin   = "demapper"

def run(path:str, cmd:str):
    root = os.getcwd()
    os.chdir(path)
    proc = subprocess.Popen(['bash', '-c', cmd])
    time.sleep(1)
    os.chdir(root)
    return proc

# commands to run coder -> decoder -> scoreboard
port_coder_decoder      = " 5555 "
port_decoder_scoreboard = " 5556 "

cmd     = "./" + scoreboardBin + port_decoder_scoreboard
cmd1    = "./" + decoderBin    + port_decoder_scoreboard + port_coder_decoder
cmd2    = "./" + coderBin      + port_coder_decoder

cmdListCoder = [cmd, cmd1, cmd2]

# commands to run coder -> mapper -> demapper -> decoder -> scoreboard
port_coder_mapper     = " 5553 "
port_mapper_demapper  = " 5554 "
port_demapper_decoder = " 5555 "

cmd_scoreboard = "./" + scoreboardBin + port_decoder_scoreboard
cmd_decoder    = "./" + decoderBin    + port_decoder_scoreboard + port_demapper_decoder
cmd_demapper   = "./" + demapperBin   + port_demapper_decoder   + port_mapper_demapper
cmd_mapper     = "./" + mapperBin     + port_mapper_demapper    + port_coder_mapper
cmd_coder      = "./" + coderBin      + port_coder_mapper

cmdListAll = [cmd_scoreboard, cmd_decoder, cmd_demapper, cmd_mapper, cmd_coder]

# commands to run mapper -> demapper -> scoreboard
port_demapper_scoreboard = "5557"

cmd_mapper_1     = "./" + mapperBin     + port_mapper_demapper
cmd_demapper_1   = "./" + demapperBin   + port_demapper_scoreboard + port_mapper_demapper
cmd_scoreboard_1 = "./" + scoreboardBin + port_demapper_scoreboard

cmdListMapper = [cmd_scoreboard_1, cmd_demapper_1, cmd_mapper_1]

run_flag = int(input("Type "0" to run all chain, "1" to run coder-decoder, "2" to run mapper-demapper. "))

runPathList
runCmdList

if (run_flag == 0):       # All chain
    runPathList = pathList
    runCmdList  = cmdListAll
else if (run_flag == 1):  # Coder-decoder chain
    runPathList = pathListCoderDecoder
    runCmdList  = cmdListCoder
else if (run_flag == 2):  # Mapper-demapper chain
    runPathList = pathListMapperDemapper
    runCmdList  = cmdListMapper

runList = list(zip(runPathList, runCmdList))

proclist: list[subprocess.Popen] = []

for path, cmd in runList:
    print(path + '   ' + cmd)
    proclist.append(run(path, cmd))

print("Any input to kill processes")
input()

for proc in proclist:
    os.killpg(os.getpgid(proc.pid), signal.SIGKILL)  # или signal.SIGTERM