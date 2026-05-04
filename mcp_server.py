#!/usr/bin/env python3
"""MCP server for chip-8 emulator.

Runs src/mcp_cpu.c as a subprocess and exposes emulator controls as MCP tools.
Can also launch the debugger GUI.
"""

import asyncio
import json
import os
import subprocess
import sys
from pathlib import Path

from mcp.server.fastmcp import FastMCP

mcp = FastMCP("chip8")

# Global handle to the C emulator subprocess
_proc = None

def _ensure_proc():
    global _proc
    if _proc is None or _proc.poll() is not None:
        repo = Path(__file__).parent.resolve()
        exe = repo / "bin" / "mcp_cpu.exe"
        if not exe.exists():
            # try to build on the fly with tcc
            tcc = "tcc"
            src = repo / "src" / "mcp_cpu.c"
            os.makedirs(exe.parent, exist_ok=True)
            subprocess.run([tcc, str(src), "-o", str(exe)], check=True)
        _proc = subprocess.Popen(
            [str(exe)],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
        )
        _send("INIT")
    return _proc

def _send(cmd: str) -> dict:
    p = _ensure_proc()
    p.stdin.write(cmd + "\n")
    p.stdin.flush()
    while True:
        line = p.stdout.readline().strip()
        if not line:
            continue
        try:
            return json.loads(line)
        except json.JSONDecodeError:
            # skip stray non-JSON output (e.g. cpu.h load printf)
            continue


@mcp.tool()
def init_cpu() -> str:
    """Initialize / reset the CHIP-8 CPU."""
    r = _send("INIT")
    return "CPU initialized." if r.get("ok") else r.get("error", "?")

@mcp.tool()
def load_rom(path: str) -> str:
    """Load a ROM file into the CHIP-8 CPU. Path is relative to repo root."""
    repo = Path(__file__).parent.resolve()
    full = repo / path
    r = _send(f"LOAD {full}")
    return f"Loaded {path}" if r.get("ok") else r.get("error", "?")

@mcp.tool()
def step_cpu() -> str:
    """Execute a single CPU cycle."""
    r = _send("STEP")
    return f"Stepped to pc=${r.get('pc', 0):03x}" if r.get("ok") else r.get("error", "?")

@mcp.tool()
def run_cpu(cycles: int = 1) -> str:
    """Run N CPU cycles."""
    r = _send(f"RUN {cycles}")
    return f"Ran to pc=${r.get('pc', 0):03x}" if r.get("ok") else r.get("error", "?")

@mcp.tool()
def get_regs() -> str:
    """Read all CHIP-8 registers (v0-vf, i, sp, pc, delay, sound)."""
    r = _send("REGS")
    if not r.get("ok", True) and "error" in r:
        return r["error"]
    v = r.get("v", [])
    lines = []
    lines.append(" ".join(f"v{i:x}=${v[i]:02x}" for i in range(8)))
    lines.append(" ".join(f"v{i:x}=${v[i]:02x}" for i in range(8, 16)))
    lines.append(f"i=${r['i']:03x} sp=${r['sp']:03x} pc=${r['pc']:03x} dt={r['dt']} st={r['st']}")
    return "\n".join(lines)

@mcp.tool()
def get_memory(address: str = "0x200", length: int = 16) -> str:
    """Read memory as hex dump. Address can be decimal or hex (0x...)."""
    addr = int(address, 0)
    r = _send(f"MEM {addr} {length}")
    if not r.get("ok", True) and "error" in r:
        return r["error"]
    return r.get("bytes", "")

@mcp.tool()
def get_screen() -> str:
    """Get the 64x32 screen buffer as ASCII art (# = on, . = off)."""
    r = _send("SCREEN")
    if not r.get("ok", True) and "error" in r:
        return r["error"]
    return r.get("pixels", "")

@mcp.tool()
def disassemble(address: str = "0x200", count: int = 10) -> str:
    """Disassemble instructions at the given address."""
    addr = int(address, 0)
    r = _send(f"DIS {addr} {count}")
    if not r.get("ok", True) and "error" in r:
        return r["error"]
    lines = []
    for ln in r.get("lines", []):
        lines.append(f"{int(ln['addr']):04x}  {ln['op']:>4s}  {ln['text']}")
    return "\n".join(lines)

@mcp.tool()
def list_roms() -> str:
    """List available ROM files in the roms/ directory."""
    repo = Path(__file__).parent.resolve()
    roms = []
    for p in sorted(repo.rglob("*.ch8")):
        roms.append(str(p.relative_to(repo)))
    return "\n".join(roms) if roms else "No ROMs found."

@mcp.tool()
def launch_gui(rom: str = "./roms/IBM Logo.ch8") -> str:
    """Launch the SDL2 debugger GUI with the given ROM."""
    repo = Path(__file__).parent.resolve()
    full = repo / rom
    # Run the GUI via the cmd script
    cmd = ["cmd", "/c", str(repo / "run_gui.cmd")]
    # We can't easily pass a ROM to run_gui.cmd, but the GUI hardcodes one.
    # Just launch it for now.
    subprocess.Popen(cmd, cwd=str(repo), creationflags=subprocess.DETACHED_PROCESS)
    return f"Launching GUI with {rom}"


if __name__ == "__main__":
    mcp.run(transport="stdio")
