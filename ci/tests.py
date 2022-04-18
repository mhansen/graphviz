#!/usr/bin/env python3

"""
test cases that are only relevant to run in CI
"""

import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
from typing import Dict
import pytest

sys.path.append(os.path.join(os.path.dirname(__file__), "../rtest"))
from gvtest import dot, is_cmake, is_mingw \
  #pylint: disable=wrong-import-position

def _freedesktop_os_release() -> Dict[str, str]:
  """
  polyfill for `platform.freedesktop_os_release`
  """
  release = {}
  os_release = Path("/etc/os-release")
  if os_release.exists():
    with open(os_release, "rt", encoding="utf-8") as f:
      for line in f.readlines():
        if line.startswith("#") or "=" not in line:
          continue
        key, _, value = (x.strip() for x in line.partition("="))
        # remove quotes
        if len(value) >= 2 and value[0] == '"' and value[-1] == '"':
          value = value[1:-1]
        release[key] = value
  return release

def is_centos() -> bool:
  """
  is the current CI environment CentOS-based?
  """
  return _freedesktop_os_release().get("ID") == "centos"

def is_win64() -> bool:
  """
  is the Graphviz under test targeting the x64 Windows API?
  """
  if platform.system() != "Windows":
    return False
  if os.getenv("project_platform") != "x64":
    return False
  return True

@pytest.mark.parametrize("binary", [
  "acyclic",
  "bcomps",
  "ccomps",
  "circo",
  "cluster",
  "diffimg",
  "dijkstra",
  "dot",
  "dot2gxl",
  "dot_builtins",
  "edgepaint",
  "fdp",
  "gc",
  "gml2gv",
  "graphml2gv",
  "gv2gml",
  "gv2gxl",
  "gvcolor",
  "gvedit",
  "gvgen",
  "gvmap",
  "gvmap.sh",
  "gvpack",
  "gvpr",
  "gxl2dot",
  "gxl2gv",
  "mingle",
  "mm2gv",
  "neato",
  "nop",
  "osage",
  "patchwork",
  "prune",
  "sccmap",
  "sfdp",
  "smyrna",
  "tred",
  "twopi",
  "unflatten",
  "vimdot",
])
def test_existence(binary: str):
  """
  check that a given binary was built and is on $PATH
  """

  tools_not_built_with_cmake = [
    "cluster",
    "dot_builtins",
    "gv2gxl",
    "gvedit",
    "gvmap",
    "gvmap.sh",
    "gxl2dot",
    "prune",
  ]

  tools_not_built_with_msbuild = [
    "cluster",
    "dot2gxl",
    "dot_builtins",
    "gv2gxl",
    "gvedit",
    "gvmap.sh",
    "gxl2dot",
    "vimdot",
  ]

  tools_not_built_with_autotools_on_macos = [
    "mingle",
    "smyrna",
  ]

  os_id = _freedesktop_os_release().get("ID")

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1835 is fixed
  if os_id == "ubuntu" and binary == "mingle" and not is_cmake():
    check_that_tool_does_not_exist(binary, os_id)
    pytest.skip(f"mingle is not built for {os_id} (#1835)")

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1834 is fixed
  if os_id == "centos" and binary == "smyrna":
    check_that_tool_does_not_exist(binary, os_id)
    pytest.skip("smyrna is not built for Centos (#1834)")

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1753 and
  # https://gitlab.com/graphviz/graphviz/-/issues/1836 is fixed
  if os.getenv("build_system") == "cmake":
    if binary in tools_not_built_with_cmake:
      check_that_tool_does_not_exist(binary, os_id)
      pytest.skip(f"{binary} is not built with CMake (#1753 & #1836)")

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1837 is fixed
  if os.getenv("build_system") == "msbuild":
    if binary in tools_not_built_with_msbuild:
      check_that_tool_does_not_exist(binary, os_id)
      pytest.skip(f"{binary} is not built with MSBuild (#1837)")

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1854 is fixed
  if os.getenv("build_system") == "autotools":
    if platform.system() == "Darwin":
      if binary in tools_not_built_with_autotools_on_macos:
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip(f"{binary} is not built with autotools on macOS (#1854)")

  if binary == "mingle" and is_cmake() and (is_win64() or is_mingw()):
    check_that_tool_does_not_exist(binary, os_id)
    pytest.skip(f"{binary} is not built on some Windows due to lacking libANN")

  if binary == "diffimg" and is_win64():
    check_that_tool_does_not_exist(binary, os_id)
    pytest.skip(f"{binary} is not built on 64-bit Windows due to lacking libgd")

  # FIXME: Smyrna dependencies are not avaiable in other jobs
  if binary == "smyrna" and is_cmake() and platform.system() != "Linux":
    check_that_tool_does_not_exist(binary, os_id)
    pytest.skip("smyrna is not built on non-Linux due to lacking dependencies")

  if binary == "vimdot" and platform.system() == "Windows":
    check_that_tool_does_not_exist(binary, os_id)
    pytest.skip("vimdot is not installed on Windows")

  assert shutil.which(binary) is not None

def check_that_tool_does_not_exist(tool, os_id):
  """
  validate that the given tool does *not* exist
  """
  assert shutil.which(tool) is None, f"{tool} has been resurrected in the " \
    f'{os.getenv("build_system")} build on {os_id}. Please remove skip.'

@pytest.mark.xfail(is_cmake() and not is_centos()
                   and not platform.system() == "Darwin",
                   reason="png:gd unavailable when built with CMake",
                   strict=True) # FIXME
def test_1786():
  """
  png:gd format should be supported
  https://gitlab.com/graphviz/graphviz/-/issues/1786
  """

  # run a trivial graph through Graphviz
  dot("png:gd", source="digraph { a -> b; }")

def test_installation():
  """
  check that Graphviz reports the expected version number
  """

  expected_version = os.environ["GV_VERSION"]

  actual_version_string = subprocess.check_output(
      [
          "dot",
          "-V",
      ],
      universal_newlines=True,
      stderr=subprocess.STDOUT,
  )
  try:
    actual_version = actual_version_string.split()[4]
  except IndexError:
    pytest.fail(f"Malformed version string: {actual_version_string}")
  assert actual_version == expected_version
