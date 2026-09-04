import argparse
import os
import glob
from pathlib import Path
import subprocess
import logging 
from datetime import datetime
import sys
import traceback

sim_dir = Path(__file__).parent.resolve()
now = datetime.now()
date_and_hour = now.strftime("%Y_%m_%d_%H_%M_%S")
new_log_dir = os.path.join(sim_dir,"log",f"{date_and_hour}")
os.makedirs(new_log_dir, exist_ok=True)

terminal_log_file_path = os.path.join(new_log_dir,'term.log')
app_log_file_path = os.path.join(new_log_dir,'app.log')

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

# app.log
formatter = logging.Formatter('[%(asctime)s] %(levelname)s [%(filename)s:%(lineno)d] - %(message)s')
file_handler = logging.FileHandler(filename=app_log_file_path, mode='a', encoding='utf-8')
file_handler.setLevel(logging.DEBUG)
file_handler.setFormatter(formatter)
logger.addHandler(file_handler)

# term.log
class TerminalTee:
    #duplicate stream (stdout/stderr) for screen and file
    def __init__(self, original_stream, log_path):
        self.original_stream = original_stream
        self.log_file = open(log_path, mode='a', encoding='utf-8', buffering=1)

    def write(self, data):
        self.original_stream.write(data) #show in terminal
        self.log_file.write(data) #save in raw file

    def flush(self):
        self.original_stream.flush()
        self.log_file.flush()


# save original stream before alterations
original_stdout = sys.stdout
original_stderr = sys.stderr

# redirect SO to save in terminal.log
sys.stdout = TerminalTee(original_stdout, terminal_log_file_path)
sys.stderr = TerminalTee(original_stderr, terminal_log_file_path)

#detecta os crashes do Python
def handle_exception(exc_type, exc_value, exc_traceback):
    # Ignora o KeyboardInterrupt (Ctrl+C) para o usuário conseguir fechar o terminal normalmente
    if issubclass(exc_type, KeyboardInterrupt):
        sys.__excepthook__(exc_type, exc_value, exc_traceback)
        return
    # grava o erro completo com o traceback no log
    logger.critical("Unhandled critical error!", exc_info=(exc_type, exc_value, exc_traceback))

sys.excepthook = handle_exception

def run_subprocess(cmd,status=False):
    try:
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1
        )

        for line in process.stdout:
            sys.stdout.write(line)
            sys.stdout.flush()

        ret_code = process.wait()

        if ret_code == 0:
            logger.info(fr"Subprocess ended successfully! (Code {ret_code})")
            return True, ret_code

        logger.warning(fr"Subprocess ended with error! (Code {ret_code}")
        return False, ret_code

    except Exception as e:
        logger.exception("Subprocess ended with unexpected error!")
        return False, -1


def main(ttasim_flags,adf_file,tpef_file,simulator):
    simulator_cmd = [simulator]
    if ttasim_flags:
        for flag in ttasim_flags:
            simulator_cmd.append(fr"{flag}")
    simulator_cmd += ["-a",fr"{adf_file}"]
    simulator_cmd += ["-p",fr"{tpef_file}"] 
    logger.info(fr"Starting {simulator}...")
    success, ret_code = run_subprocess(simulator_cmd,status=True)
    if not success:
        logger.error(f"{simulator} failed with return code %d", ret_code)

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-s','--simulator',choices=['ttasim','proxim'],default='ttasim')
    parser.add_argument('-t','--ttasim-flags',nargs="+",type=str,help="Spaced list of flags/options to pass to ttasim. See 'ttasim --help' for more info.")
    parser.add_argument('-a','--adf-file',type=str,help="*.adf file")
    parser.add_argument('-p','--tpef-file',type=str,help="*.tpef file")
    args = parser.parse_args()

    simulator = args.simulator
    ttasim_flags = args.ttasim_flags
    if simulator == 'proxim' and ttasim_flags:
        logger.warning(fr"User selected both 'proxim' simulator and 'ttasim' flags: {[flag for flag in ttasim_flags]}. Ignoring 'ttasim' flags...")
        ttasim_flags = []

    sim_dir = Path(__file__).parent.resolve()
    openasip_root = sim_dir.parent.resolve()
    arch_root = os.path.join(openasip_root,"arch")
    tta_kernels_root = os.path.join(openasip_root,"../build/tta/openasip/targets/tta/kernels")

    adf_file = args.adf_file
    if adf_file is None:
        adf_file_pattern = os.path.join(arch_root,"*.adf")
        if adf_file is None:
            sys.exit("No .adf file found in '{arch_root}'.")
        adf_file = glob.glob(adf_file_pattern)[0]

    tpef_file = args.tpef_file
    if tpef_file is None:
        tpef_file_pattern = os.path.join(tta_kernels_root,"*.tpef")
        if tpef_file is None:
            sys.exit("No .tpef file found in '{tta_kernels_root}'.")
        tpef_file = glob.glob(tpef_file_pattern)[0]

    main(ttasim_flags,adf_file,tpef_file,simulator)
