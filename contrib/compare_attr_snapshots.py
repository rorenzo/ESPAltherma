#!/usr/bin/env python3
"""Compare two snapshots of the ESPAltherma payload (before/after a firmware update).

Each snapshot can be either the raw `espaltherma/ATTR` JSON payload or the
Home Assistant state of `sensor.althermasensors`, for example saved from the
Terminal & SSH app with:

  curl -s -H "Authorization: Bearer $SUPERVISOR_TOKEN" \
    http://supervisor/core/api/states/sensor.althermasensors > before.json

The report lists removed, added and type-changed values, and shows numeric
values side by side so drifts or unit changes stand out. Exit code 1 when a
value disappeared that is not listed with --expect-removed.
"""
from __future__ import annotations

import argparse
import json
import sys

# Attributes added by Home Assistant, not by the firmware.
HA_ATTRIBUTES = {'friendly_name', 'icon', 'unit_of_measurement', 'device_class', 'state_class'}


def load(path):
    with open(path, encoding='utf-8') as stream:
        data = json.load(stream)
    if isinstance(data, dict) and isinstance(data.get('attributes'), dict):
        data = data['attributes']
    if not isinstance(data, dict):
        raise ValueError(f'{path}: expected a JSON object')
    return {key: value for key, value in data.items() if key not in HA_ATTRIBUTES}


def kind(value):
    if isinstance(value, bool):
        return 'bool'
    if isinstance(value, (int, float)):
        return 'number'
    if isinstance(value, str):
        try:
            float(value)
            return 'number'
        except ValueError:
            return 'text'
    return type(value).__name__


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('before')
    parser.add_argument('after')
    parser.add_argument('--expect-removed', action='append', default=[], metavar='KEY',
                        help='value removed on purpose (repeatable)')
    args = parser.parse_args()
    try:
        before, after = load(args.before), load(args.after)
    except (OSError, ValueError) as error:
        parser.exit(2, f'{error}\n')

    removed = sorted(set(before) - set(after))
    unexpected = [key for key in removed if key not in args.expect_removed]
    added = sorted(set(after) - set(before))
    common = sorted(set(before) & set(after))

    print(f'Values: {len(before)} before, {len(after)} after, {len(common)} in common\n')
    for key in removed:
        print(f'REMOVED{"" if key in args.expect_removed else " (unexpected)"}: {key} = {before[key]!r}')
    for key in added:
        print(f'ADDED: {key} = {after[key]!r}')
    for key in common:
        old, new = before[key], after[key]
        if kind(old) != kind(new):
            print(f'TYPE CHANGED: {key}: {old!r} ({kind(old)}) -> {new!r} ({kind(new)})')
        elif old != new:
            print(f'           {key}: {old!r} -> {new!r}')
    for key in args.expect_removed:
        if key not in before:
            print(f'NOTE: --expect-removed {key!r} was not present before')
    return 1 if unexpected else 0


if __name__ == '__main__':
    sys.exit(main())
