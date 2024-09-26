from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import subprocess
import uvicorn


app = FastAPI()


class Command(BaseModel):
    cs_pin: str
    args: str


@app.get("/")
def status():
    return {"status": True}


@app.post("/execute")
def execute_command(command: Command):
    """
    Endpoint to execute a given command via command line.
    """
    command_args = ["./build/control", command.cs_pin + command.args]

    try:
        subprocess.run(command_args, check=True)
        return {"status": True}
    except subprocess.CalledProcessError as e:
        raise HTTPException(status_code=400, detail=str(e))


if __name__ == "__main__":
    # Use uvicorn to run the application.
    # By default, it will run on http://127.0.0.1:8000
    # The command `reload=True` enables the server to reload on code changes.
    uvicorn.run(app, host="0.0.0.0", port=7070, log_level="info")
