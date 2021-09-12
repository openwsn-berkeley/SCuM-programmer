# run from source

- `cd 01_software`
- `pip install -r requirements.txt`
- `python -m scumprogrammer`

# build

- `pip install --upgrade pip setuptools build`
- `cd 01_software`
- `python -m build`

# install locally

- `cd 01_software`
- `pip install .`

After installation, verify you can the `scumprogrammer` commmand from any directory.

# uninstall locally

- `pip uninstall scumprogrammer`

After installation, verify you can no longer run the `scumprogrammer` commmand.

# upload to PyPI

- create an API token after logging in at https://pypi.org/ (it's a long string starting with `pypi-`)
- `pip install --upgrade twine`
- `cd 01_software`
- `twine upload dist/*`
    - username: `__token__`
    - password: the entire token above, including the `pypi-` prefix
- update appears at https://pypi.org/project/scumprogrammer/