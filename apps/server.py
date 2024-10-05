from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import subprocess
import uvicorn
from pathlib import Path  # Import Path from pathlib
import asyncio

app = FastAPI()


class Command(BaseModel):
    cs_pin: str
    args: str


@app.get("/")
def status():
    return {"status": True}


@app.post("/execute")
async def execute_command(command: Command):
    current_file_path = Path(__file__).resolve()
    control_executable = current_file_path.parent.parent / "build" / "control"
    command_args = [str(control_executable), command.cs_pin, command.args]

    process = await asyncio.create_subprocess_exec(
        *command_args, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE
    )

    stdout, stderr = await process.communicate()

    if process.returncode != 0:
        raise HTTPException(status_code=400, detail=stderr.decode().strip())

    return {"status": True, "output": stdout.decode().strip()}


if __name__ == "__main__":
    # Use uvicorn to run the application.
    # By default, it will run on http://127.0.0.1:8000
    # The command `reload=True` enables the server to reload on code changes.
    uvicorn.run(app, host="0.0.0.0", port=7070, log_level="info")
