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
    cs_pin: str
    args: str

class CommandBatch(BaseModel):
    commands: List[Command]

class StepperCommand(BaseModel):
    direction: str  # e.g., "left" or "right"
    steps: int

class StepperCommandBatch(BaseModel):
    commands: List[StepperCommand]

# Persistent process variables
command_queue = None
stepper_queue = None
shutdown_flag = False

# Path to the control executable
current_file_path = Path(__file__).resolve()
control_executable = current_file_path.parent.parent / "build" / "control"
stepper_executable = current_file_path.parent.parent / "build" / "stepperMotor"

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
            
            logging.info("Persistent subprocess started.")

            while not shutdown_flag:
                try:
                    if process.returncode is not None:
                        raise RuntimeError("Subprocess terminated unexpectedly.")
                    
                    command = await asyncio.wait_for(command_queue.get(), timeout=.1)
                    command_input = f"{command['cs_pin']} {command['args']}\n"
                    
                    if process.returncode is not None:
                        raise RuntimeError("Subprocess terminated before executing command.")
                    
                    process.stdin.write(command_input.encode())
                    await process.stdin.drain()
                    
                    output_lines = []
                    while True:
                        line = await process.stdout.readline()
                        if line:
                            line = line.decode().strip()
                            if line == "END":
                                break
                            output_lines.append(line)
                    
                    full_output = "\n".join(output_lines)
                    command['result'].set_result(full_output)
                except asyncio.TimeoutError:
                    pass
                except Exception as e:
                    logging.error(f"Error processing command: {e}")
                    break

            if process.returncode is not None:
                logging.error(f"Subprocess terminated with code {process.returncode}. Restarting...")
            
            if process.stdin:
                process.stdin.close()
            if process.stdout:
                process.stdout.close()
            await process.wait()

    except Exception as e:
        logging.error(f"Error in command_processor: {e}")
    finally:
        if process:
            process.kill()
            await process.wait()
        logging.info("Shutting down command processor.")

async def stepper_processor():
    global shutdown_flag
    process = None
    logging.info("stepper_procesor starts")
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
                    if process.returncode is not None:
                        raise RuntimeError("Stepper subprocess terminated unexpectedly.")
                    
                    command = await asyncio.wait_for(stepper_queue.get(), timeout=1.0)
                    command_input = f"{command['direction']} {command['steps']}\n"

                    if process.returncode is not None:
                        raise RuntimeError("Stepper subprocess terminated before executing command.")

                    process.stdin.write(command_input.encode())
                    await process.stdin.drain()
                except asyncio.TimeoutError:
                    pass
                except Exception as e:
                    logging.error(f"Error processing stepper command: {e}")
                    break
    finally:
        if process:
            try:
                process.kill()
                await process.wait()
            except ProcessLookupError:
                logging.warning("Stepper process already terminated.")
        logging.info("Shutting down stepper processor.")

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

@app.on_event("shutdown")
async def shutdown_event():
    global shutdown_flag
    shutdown_flag = True
    logging.info("Shutdown signal received. Cleaning up resources...")
    # Ensure subprocesses are terminated
    await os.system(f"sudo pkill -f {control_executable}")
    await os.system(f"sudo pkill -f {stepper_executable}")

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
        await stepper_queue.put({"direction": cmd.direction, "steps": cmd.steps})
    return {"status": True, "results": results}

# Serve the static files (Svelte app)
# Ensure these files are mounted last, otherwise POST requests may fail
app.mount("/", StaticFiles(directory=svelte_frontend, html=True), name="build")

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=7070, log_level="info")
