#!/usr/bin/python3

import json
from sys import argv
import argparse
from dataclasses import dataclass
from typing import Any, Tuple


@dataclass
class InfoState:
    count: int
    sum_latency: float
    errors: int
    size: int

@dataclass
class Info:
    count: int
    error_rate: float
    latency: float
    size: int
    iops: float


def filter(req, sizes, sectors):
    return (
        (req["size"] >= sizes["min"]) and (req["size"] <= sizes["max"])
        and (req["sector"] >= sectors["min"]) and (req["sector"] <= sectors["max"])
    )

def filter_reqs(reqs: list, sizes, sectors):
    return [req for req in reqs if filter(req, sizes, sectors)]


def req_latency(req):
    return req["end_ts"] - req["start_ts"]

def req_err(req):
    return 1 if req["status"] != 0 else 0

def req_collect_info(state, req):
    size = req["size"]
    if state.get(size) == None:
        state[size] = InfoState(0, 0, 0, 0)

    state[size].count += 1
    state[size].sum_latency += req_latency(req)
    state[size].errors += req_err(req)
    state[size].size += size

def state_to_info(state: InfoState, time: int):
    time_s = time / 10 ** 9
    return Info(
        state.count,
        state.errors / state.count,
        state.sum_latency / state.count,
        state.size,
        state.count / time_s,
    )

def collect_info(reqs: list):
    states: dict[Any, InfoState] = {}
    start = None
    end = None
    for req in reqs:
        req_collect_info(states, req)
        if (start == None) or (start > req["start_ts"]):
            start = req["start_ts"]

        if (end == None) or (end < req["end_ts"]):
            end = req["end_ts"]

    total = InfoState(0, 0, 0, 0)
    for state in states.values():
        total.count += state.count
        total.errors += state.errors
        total.size += state.size
        total.sum_latency += state.sum_latency

    states["total"] = total

    info: dict[Any, Info] = {}
    for key, state, in states.items():
        info[key] = state_to_info(state, end - start)

    return info


MAX_INT = 2 ** 63 - 1

def parse_min_max(s: str) -> Tuple[str, str]:
    mins, maxs = s.split('-')
    min = 0 if mins == "min" else int(mins)
    max = MAX_INT if maxs == "max" else int(maxs)
    return (min, max)

def format_info(tag, info: Info, format: str):
    counts = f"count: {info.count};"
    error_rates = f"error_rate: {info.error_rate} %;"
    iopss = f"iops: {info.iops};"
    latency = f"latency: {info.latency} ns;"
    sizes = f"total_size: {info.size} b;"

    format = format.replace("%c", counts).replace("%l", latency).replace("%e", error_rates).replace("%s", sizes).replace("%i", iopss)
    return f"{tag}[{format}];"

def format_output(info, format):
    print(format_info("total", info["total"], format))
    info.pop("total")
    keys = sorted(info.keys())
    for key in keys:
        print(format_info(key, info[key], format))

parser = argparse.ArgumentParser()
parser.add_argument("dump")
parser.add_argument("-f", "--format", type=str, default="%c %l %e %s %i")
parser.add_argument("-s", "--sizes", type=str, default="min-max")
parser.add_argument("-S", "--sectors", type=str, default="min-max")

args = parser.parse_args()

f = open(args.dump, 'r')
reqs = json.load(f)["requests"]

sizes = {}
sizes["min"], sizes["max"] = parse_min_max(args.sizes)

sectors = {}
sectors["min"], sectors["max"] = parse_min_max(args.sectors)

filetered_reqs = filter_reqs(reqs, sizes, sectors)
info = collect_info(filetered_reqs)
format_output(info, args.format)
