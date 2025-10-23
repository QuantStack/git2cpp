# Modify the git2cpp emscripten-forge recipe to build from the local repo.
# This can be called repeatedly and will produce the same output.

from pathlib import Path
import shutil
import sys
import yaml


def quit(msg):
    print(msg)
    exit(1)

if len(sys.argv) < 2:
    quit(f'Usage: {sys.argv[0]} <recipe directory containing yaml file to modify>')

input_dir = Path(sys.argv[1])
if not input_dir.is_dir():
    quit(f'{input_dir} should exist and be a directory')

input_filename = input_dir / 'recipe.yaml'
if not input_filename.is_file():
    quit(f'{input_filename} should exist and be a file')

# If backup does not exist create it.
input_backup = input_dir / 'recipe_original.yaml'
backup_exists = input_backup.exists()
if not backup_exists:
    shutil.copy(input_filename, input_backup)

# Read and parse input backup file which is the original recipe file.
with open(input_backup) as f:
    recipe = yaml.safe_load(f)

build_number = recipe['build']['number']
print(f'  Changing build number from {build_number} to {build_number+1}')
recipe['build']['number'] = build_number+1

source = recipe['source']
if not ('sha256' in source and 'url' in source):
    raise RuntimeError('Expected recipe to have both a source sha256 and url')
del source['sha256']
del source['url']
print('  Changing source to point to local git2cpp repo')
source['path'] = '../../../../../../'

# Overwrite recipe file.
with open(input_filename, 'w') as f:
    yaml.dump(recipe, f)
