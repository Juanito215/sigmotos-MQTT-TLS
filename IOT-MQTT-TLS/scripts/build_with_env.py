#!/usr/bin/env python3

import subprocess
import sys
import os
from pathlib import Path

project_dir = Path(__file__).parent.parent
os.chdir(project_dir)

env_vars = []

env_file = project_dir / '.env'

if env_file.exists():

    with open(env_file, 'r', encoding='utf-8') as f:

        for line in f:

            line = line.strip()

            if line and not line.startswith('#') and '=' in line:

                key, value = line.split('=', 1)

                key = key.strip()
                value = value.strip()

                if (
                    (value.startswith('"') and value.endswith('"'))
                    or
                    (value.startswith("'") and value.endswith("'"))
                ):
                    value = value[1:-1]

                env_vars.append(f'-D{key}="{value}"')

# comando base
cmd = ['pio', 'run']

# upload opcional
if len(sys.argv) > 1 and sys.argv[1] == 'upload':
    cmd += ['-t', 'upload']

# agregar defines
cmd += ['--project-option', f'build_flags={" ".join(env_vars)}']

print("Ejecutando:", " ".join(cmd))

subprocess.run(cmd, check=True)