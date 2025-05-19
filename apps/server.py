from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from asyncio import Queue, create_task
import asyncio
from pathlib import Path
import logging
import uvicorn
from typing import List
import os
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse
import subprocess
import json
from fastapi.middleware.cors import CORSMiddleware
from typing import Optional
import signal


logging.basicConfig(level=logging.INFO)

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Change "*" to your frontend URL for better security
    allow_credentials=True,
    allow_methods=["GET", "POST", "PUT", "DELETE"],  # Ensure POST is included
    allow_headers=["*"],
)

# Command models
class Command(BaseModel):
    cs_pin: int
    args: str

class CommandBatch(BaseModel):
    commands: List[Command]

class StepperCommand(BaseModel):
    direction: str  # e.g., "left" or "right"
    steps: int

class StepperCommandBatch(BaseModel):
    commands: List[StepperCommand]

# Persistent process variables
command_queue: Optional[Queue] = None
stepper_queue: Optional[Queue] = None

shutdown_flag = False

# Path to the control executable
current_file_path = Path(__file__).resolve()
control_executable = current_file_path.parent.parent / "build" / "control"
stepper_executable = current_file_path.parent.parent / "build" / "stepperMotorGPIO"

class DotData(BaseModel):
    index: int
    number: int

class DotBatch(BaseModel):
    dots: List[DotData]

# Global lock to prevent overlapping /send_dots executions
send_dots_lock = asyncio.Lock()

@app.post("/send_dots")
async def receive_dots(batch: DotBatch):
    if send_dots_lock.locked():
        raise HTTPException(status_code=429, detail="Previous /send_dots still in progress")

    async with send_dots_lock:
        # Define chip wiring (12 indexes each, ordered by physical layout)
        cs1_array = [23, 22, 21, 28, 27, 26, 32, 31, 30, 35, 34, 33]
        cs2_array = [29, 25, 20, 24, 19, 13, 18, 12, 6, 11, 5, 0]
        cs3_array = [1, 7, 14, 2, 8, 15, 3, 9, 16, 4, 10, 17]

        chip_for_index = {idx: 1 for idx in cs1_array}
        chip_for_index.update({idx: 2 for idx in cs2_array})
        chip_for_index.update({idx: 3 for idx in cs3_array})

        try:
            dot_list = json.dumps([dot.model_dump() for dot in batch.dots])
            logging.info(f"Received dot list {dot_list}.")

            chip_dots = {1: {}, 2: {}, 3: {}}
            for dot in batch.dots:
                chip = chip_for_index.get(dot.index)
                if chip == 1:
                    dot.number += 2
                elif chip == 2:
                    dot.number += 4
                if dot.number > 6:
                    dot.number -= 6
                if chip:
                    chip_dots[chip][dot.index] = dot.number

            processed_list = [dot.model_dump() for dot in batch.dots]
            logging.info(f"Processed dot list {json.dumps(processed_list)}.")

            # Stepper move BACKWARD
            result_future1 = asyncio.get_running_loop().create_future()
            await stepper_queue.put({
                "direction": "BACKWARD"[:],
                "steps": int(20000),
                "result": result_future1
            })
            await result_future1
            logging.info("Stepper BACKWARD complete.")

            # Generate dynamic args
            chip_map = {1: cs1_array, 2: cs2_array, 3: cs3_array}
            chip_args = {1: {}, 2: {}, 3: {}}

            for chip in (1, 2, 3):
                index_list = chip_map[chip]
                for num in range(1, 7):
                    arg_parts = []
                    for pos, idx in enumerate(index_list, 1):
                        value = 1 if chip_dots[chip].get(idx) == num else 0
                        arg_parts.append(f"{value},{pos}")
                    chip_args[chip][num] = " ".join(arg_parts)

            # # Send programming commands
            # for chip in (1, 2, 3):
            #     cs_pin = str(chip)
            #     for num in range(1, 7):
            #         args = chip_args[chip][num]
            #         result_future = asyncio.get_running_loop().create_future()
            #         await command_queue.put({
            #             "cs_pin": cs_pin,
            #             "args": args,
            #             "result": result_future
            #         })
            #         await result_future

            # Send setup
            for chip in (1, 2, 3):
                cs_pin = str(chip)
                result_future = asyncio.get_running_loop().create_future()
                await command_queue.put({
                    "cs_pin": cs_pin,
                    "args": "1,1 1,2 1,3 1,4 1,5 1,6 1,7 1,8 1,9 1,10 1,11 1,12",
                    "result": result_future
                })
                await result_future            
            for chip in (4,4):
                cs_pin = str(chip)
                result_future = asyncio.get_running_loop().create_future()
                await command_queue.put({
                    "cs_pin": cs_pin,
                    "args": "2,1 2,2 2,3 2,4 2,5 2,6 2,7 2,8 2,9 2,10 2,11 2,12",
                    "result": result_future
                })
                await result_future

            # Stepper move FORWARD
            result_future2 = asyncio.get_running_loop().create_future()
            await stepper_queue.put({
                "direction": "FORWARD"[:],
                "steps": int(20000),
                "result": result_future2
            })
            await result_future2
            logging.info("Stepper FORWARD complete.")

            # # Send cleanup
            # for chip in (1, 2, 3):
            #     cs_pin = str(chip)
            #     result_future = asyncio.get_running_loop().create_future()
            #     await command_queue.put({
            #         "cs_pin": cs_pin,
            #         "args": "0,1 0,2 0,3 0,4 0,5 0,6 0,7 0,8 0,9 0,10 0,11 0,12",
            #         "result": result_future
            #     })
            #     await result_future

            return {"status": "success", "dots": processed_list}

        except Exception as e:
            logging.error(f"Error in /send_dots: {e}")
            return {"status": "error", "message": str(e)}
    
async def command_processor():
    global shutdown_flag
    process = None

    try:
        while not shutdown_flag:
            process = await asyncio.create_subprocess_exec(
                "sudo", "-S", str(control_executable),
                stdin=asyncio.subprocess.PIPE, stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )

            if not process.stdin or not process.stdout:
                raise RuntimeError("Subprocess pipes not initialized.")
            
            logging.info("Persistent control subprocess started.")

            while not shutdown_flag:
                try:
                    command = await asyncio.wait_for(command_queue.get(), timeout=0.1)
                    
                    if process.returncode is not None:
                        raise RuntimeError("Subprocess terminated before executing command.")

                    command_input = f"{command['cs_pin']} {command['args']}\n"
                    process.stdin.write(command_input.encode())
                    await process.stdin.drain()

                    output_lines = []
                    while True:
                        line = await process.stdout.readline()
                        if not line:
                            break
                        decoded = line.decode().strip()
                        if decoded == "END":
                            break
                        output_lines.append(decoded)

                    full_output = "\n".join(output_lines)
                    command['result'].set_result(full_output)
                except asyncio.TimeoutError:
                    continue
                except Exception as e:
                    logging.error(f"Error processing control command: {e}")
                    break

            # Log shutdown cause
            if process.returncode is not None:
                if process.returncode < 0:
                    sig = -process.returncode
                    logging.warning(f"Control subprocess terminated by signal {sig} ({signal.Signals(sig).name})")
                else:
                    logging.warning(f"Control subprocess exited with code {process.returncode}")

    except Exception as e:
        logging.error(f"Error in command_processor: {e}")
    finally:
        if process:
            logging.info("Shutting down control subprocess...")
            try:
                process.terminate()
                await asyncio.wait_for(process.wait(), timeout=3.0)
            except asyncio.TimeoutError:
                logging.warning("Control subprocess did not terminate. Killing it.")
                process.kill()
                await process.wait()
        logging.info("Command processor fully shut down.")

async def stepper_processor():
    global shutdown_flag
    process = None

    logging.info("Stepper processor starts.")
    try:
        while not shutdown_flag:
            process = await asyncio.create_subprocess_exec(
                "sudo", "-S", str(stepper_executable),
                stdin=asyncio.subprocess.PIPE, stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )

            if not process.stdin or not process.stdout:
                raise RuntimeError("Stepper subprocess pipes not initialized.")

            logging.info("Persistent stepper subprocess started.")

            while not shutdown_flag:
                try:
                    command = await asyncio.wait_for(stepper_queue.get(), timeout=1.0)
                    
                    if process.returncode is not None:
                        raise RuntimeError("Stepper subprocess terminated before executing command.")

                    command_input = f"{command['direction']} {command['steps']}\n"
                    logging.info(f"Sent to stepper: {command_input.strip()}")

                    process.stdin.write(command_input.encode())
                    await process.stdin.drain()

                    # Wait for DONE
                    while True:
                        line = await process.stdout.readline()
                        if not line:
                            break
                        decoded = line.decode().strip()
                        logging.info(f"Stepper subprocess output: {decoded}")
                        if decoded == "DONE":
                            break

                    if command.get("result"):
                        command["result"].set_result(True)

                except asyncio.TimeoutError:
                    continue
                except Exception as e:
                    logging.error(f"Error processing stepper command: {e}")
                    break

            if process.returncode is not None:
                if process.returncode < 0:
                    sig = -process.returncode
                    logging.warning(f"Stepper subprocess terminated by signal {sig} ({signal.Signals(sig).name})")
                else:
                    logging.warning(f"Stepper subprocess exited with code {process.returncode}")

    finally:
        if process:
            logging.info("Shutting down stepper subprocess...")
            try:
                process.terminate()
                await asyncio.wait_for(process.wait(), timeout=3.0)
            except asyncio.TimeoutError:
                logging.warning("Stepper subprocess did not terminate. Killing it.")
                process.kill()
                await process.wait()
        logging.info("Stepper processor fully shut down.")

@app.on_event("startup")
async def startup_event():
    global command_queue, stepper_queue, shutdown_flag
    shutdown_flag = False
    command_queue = Queue()
    stepper_queue = Queue()

    # Function to check if process is running
    def is_running(executable):
        return os.system(f"pgrep -f {executable} > /dev/null") == 0

    # Kill only if running
    if is_running(control_executable):
        os.system(f"sudo pkill -f {control_executable}")

    if is_running(stepper_executable):
        os.system(f"sudo pkill -f {stepper_executable}")

    # Reversing the order of the two create tasks below causes CS1 to become unresponsive for unknown reasons
    asyncio.create_task(stepper_processor())
    asyncio.create_task(command_processor())
    
    

    logging.info("Startup event complete. Command and stepper processors initialized.")

async def shutdown_event():
    global shutdown_flag
    shutdown_flag = True
    logging.info("Shutdown signal received. Cleaning up resources...")

    proc1 = await asyncio.create_subprocess_shell(f"sudo pkill -f {control_executable}")
    await proc1.wait()

    proc2 = await asyncio.create_subprocess_shell(f"sudo pkill -f {stepper_executable}")
    await proc2.wait()

    logging.info("Subprocesses terminated successfully.")



# Define the path to the frontend folder correctly
svelte_frontend = current_file_path.parent.parent / "frontend" / "build"

# Serve the Svelte index.html for the root route
@app.get("/")
async def serve_svelte():
    return FileResponse(svelte_frontend / "index.html")

@app.post("/execute_batch")
async def execute_batch_commands(batch: CommandBatch):
    results = []
    for cmd in batch.commands:
        result_future = asyncio.get_running_loop().create_future()
        await command_queue.put({"cs_pin": cmd.cs_pin, "args": cmd.args, "result": result_future})
        output = await result_future
        results.append({"cs_pin": cmd.cs_pin, "output": output})
    return {"status": True, "results": results}

@app.post("/stepper")
async def control_stepper(batch: StepperCommandBatch):
    results = []
    for cmd in batch.commands:
        result_future = asyncio.get_running_loop().create_future()
        await stepper_queue.put({
            "direction": str(cmd.direction),  # force string copy
            "steps": int(cmd.steps),          # force int copy
            "result": result_future
        })
        results.append(result_future)
    await asyncio.gather(*results)
    return {"status": True, "results": results}


# Serve the static files (Svelte app)
# Ensure these files are mounted last, otherwise POST requests may fail
app.mount("/", StaticFiles(directory=svelte_frontend, html=True), name="build")

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=7070, log_level="info")
