import argparse
import os
from pathlib import Path
import subprocess
import logging 
from datetime import datetime
import sys
import traceback
import json

_script_dir_path = Path(__file__).parent.resolve()

def get_git_info():
    try:
        repo_root = subprocess.check_output(
            ["git", "rev-parse", "--show-toplevel"],
            cwd=_script_dir_path,
            text=True
        ).strip()

        commit = subprocess.check_output(
            ["git", "rev-parse", "HEAD"],
            cwd=repo_root,
            text=True
        ).strip()

        branch = subprocess.check_output(
            ["git", "branch", "--show-current"],
            cwd=repo_root,
            text=True
        ).strip()

        status = subprocess.check_output(
            ["git", "status", "--porcelain"],
            cwd=repo_root,
            text=True
        ).strip()
        dirty = "DIRTY" if status else "clean"

        return commit, branch, dirty

    except (subprocess.CalledProcessError, FileNotFoundError):
        return "unknown", "unknown", None

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


def run_subprocess(logger,cmd):
    try:
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
            encoding="utf-8",
            errors="replace"
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
    
    except UnicodeDecodeError:
        logger.exception("Subprocess ended with unexpected error!")
        logger.exception(traceback.print_exc())
        return False, -1

    except Exception as e:
        logger.exception("Subprocess ended with unexpected error!")
        return False, -1

def config_logger(sim_root=None):
    if sim_root is None:
        sim_root = Path(__file__).parent.resolve()
    now = datetime.now()
    date_and_hour = now.strftime("%Y_%m_%d_%H_%M_%S")
    date_and_hour_logger = now.strftime("%Y-%m-%d %H:%M:%S")
    new_log_dir = os.path.join(sim_root,"log",f"{date_and_hour}")
    os.makedirs(new_log_dir, exist_ok=True)

    terminal_log_file_path = os.path.join(new_log_dir,'term.log')
    app_log_file_path = os.path.join(new_log_dir,'app.log')

    logger = logging.getLogger(__name__)
    logger.setLevel(logging.DEBUG)
    logger.propagate = False
    logger.handlers.clear()

    formatter = logging.Formatter('[%(asctime)s] %(levelname)s [%(filename)s:%(lineno)d] - %(message)s')

    # Save original streams
    original_stdout = sys.stdout
    original_stderr = sys.stderr

    # Redirect stdout/stderr first (term.log)
    sys.stdout = TerminalTee(original_stdout, terminal_log_file_path)
    sys.stderr = TerminalTee(original_stderr, terminal_log_file_path)

    # app.log
    file_handler = logging.FileHandler(filename=app_log_file_path, mode='a', encoding='utf-8')
    file_handler.setLevel(logging.DEBUG)
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)

    # Now logger output goes through TerminalTee
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(logging.DEBUG)
    console_handler.setFormatter(formatter)
    logger.addHandler(console_handler)

    #detecta os crashes do Python
    def handle_exception(exc_type, exc_value, exc_traceback):
        # Ignora o KeyboardInterrupt (Ctrl+C) para o usuário conseguir fechar o terminal normalmente
        if issubclass(exc_type, KeyboardInterrupt):
            sys.__excepthook__(exc_type, exc_value, exc_traceback)
            return
        # grava o erro completo com o traceback no log
        logger.critical("Unhandled critical error!", exc_info=(exc_type, exc_value, exc_traceback))

    sys.excepthook = handle_exception
    return logger, date_and_hour_logger, new_log_dir


def main(logger,target,ttasim_flags,adf_file,tpef_file,simulator,sim_root,x86_64_program=None,x86_64_program_args=None):
    if target == 'tta':
        cmd = [simulator]
        if ttasim_flags:
            for flag in ttasim_flags:
                cmd.append(fr"{flag}")
        cmd += ["-a",fr"{adf_file}"]
        cmd += ["-p",fr"{tpef_file}"] 
        logger.info(fr"Starting {simulator}...")
    else: # x86_64
        cmd = [fr"{x86_64_program}"]
        if x86_64_program_args:
            for arg in x86_64_program_args:
                cmd.append(fr"{arg}")
        logger.info(fr"Starting {x86_64_program}")

    success, ret_code = run_subprocess(logger,cmd)
    
    if target == 'tta':
        if not success:
            logger.error(f"{simulator} failed with return code %d", ret_code)
    else: # x86_64
        if not success:
            logger.error(f"{x86_64_program} failed with return code %d", ret_code)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-t','--target',choices=['tta','x86_64'],required=True,help='Architecture target for simulated program.')
    parser.add_argument('-s','--simulator',choices=['ttasim','proxim'],default='ttasim')
    parser.add_argument('-f','--ttasim_flags',nargs="+",type=str,help="Spaced list of flags/options to pass to ttasim. See 'ttasim --help' for more info.")
    parser.add_argument('-a','--adf_file',type=str,help="*.adf file")
    parser.add_argument('-p','--tpef_file',type=str,help="*.tpef file.")
    parser.add_argument('-d','--sim_root',type=str,help="simulation root dir (implicitly defaults, in code, to open_baseband/openasip/sim)")
    parser.add_argument('-e','--x86_64_program',type=str,help="x86_64 program for simulation.")
    parser.add_argument('-r','--x86_64_program_args',nargs="+",type=str,help="Spaced list of arguments to pass to x86_64 program.")
    args = parser.parse_args()
    
    target = args.target
    simulator = args.simulator
    ttasim_flags = args.ttasim_flags
    adf_file = args.adf_file
    tpef_file = args.tpef_file
    sim_root = args.sim_root
    x86_64_program = args.x86_64_program
    x86_64_program_args = args.x86_64_program_args

    logger, date_and_hour_logger, new_log_dir = config_logger(sim_root)

    if target == "tta":
        incompatible_options = []

        if x86_64_program:
            incompatible_options.append(f"--x86_64_program={x86_64_program}")

        if x86_64_program_args:
            incompatible_options.append("--x86_64_program_args={" ".join(x86_64_program_args)}")

    elif target == "x86_64":
        incompatible_options = []

        if adf_file:
            incompatible_options.append(f"--adf_file={adf_file}")

        if tpef_file:
            incompatible_options.append(f"--tpef_file={tpef_file}")


        if ttasim_flags:
            incompatible_options.append("--ttasim_flags={" ".join(ttasim_flags)}")

    if incompatible_options:
        logger.warning(
            "User selected target '%s' with incompatible options: %s. "
            "They will be ignored.",
            target,
            incompatible_options
        )
    
    simulator = args.simulator
    ttasim_flags = args.ttasim_flags
    if simulator == 'proxim' and ttasim_flags:
        logger.warning(fr"User selected both 'proxim' simulator and 'ttasim' flags: {[flag for flag in ttasim_flags]}. Ignoring 'ttasim' flags...")
        ttasim_flags = []
    
    commit_hash, branch, working_tree = get_git_info()

    run_info_obj = {
        "timestamp": date_and_hour_logger,
        "commit_hash": commit_hash,
        "branch": branch,
        "git_working_tree": working_tree,
        "simulation_target": target,
        "adf": 'N/A' if target == 'x86_64' else str(Path(adf_file).resolve()),
        "tpef": 'N/A' if target == 'x86_64' else str(Path(tpef_file).resolve()),
        "compiler": 'g++' if target == 'x86_64' else 'clang',
        "simulator": 'N/A' if target == 'x86_64' else simulator,
        "ttasim_flags": ['N/A'] if target == 'x86_64' else [ttasim_flags],
        "program": str(Path(x86_64_program).resolve()) if target == 'x86_64' else 'N/A',
        "program_args": [x86_64_program_args] if target == 'x86_64' else ['N/A']
    }
    
    print("\n")
    logger.info(fr""" 
    =========================================
    Simulation started
    =========================================
    Timestamp:         '{run_info_obj['timestamp']}'
    
    Git commit:        '{run_info_obj['commit_hash']}'
    Git branch:        '{run_info_obj['branch']}'
    Git working tree:  '{run_info_obj['git_working_tree']}'
    
    Simulation target: '{run_info_obj['simulation_target']}'
    ADF:               '{run_info_obj['adf']}'
    TPEF:              '{run_info_obj['tpef']}'
    Compiler:          '{run_info_obj['compiler']}'
    Simulator:         '{run_info_obj['simulator']}'
    ttasim flags:      {[flag for flag in run_info_obj['ttasim_flags']]}
    Program:           '{run_info_obj['program']}'
    Program arguments: {[arg for arg in run_info_obj['program_args']]}
    =========================================
    """)
    
    run_info_path = os.path.join(new_log_dir,"run_info.json")
    with open(run_info_path, "w", encoding="utf-8") as json_file:
        json.dump(run_info_obj, json_file, indent=4)
    
    main(logger,target,ttasim_flags,adf_file,tpef_file,simulator,sim_root,x86_64_program,x86_64_program_args)
