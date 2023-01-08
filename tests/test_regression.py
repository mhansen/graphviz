"""
Graphviz regression tests

The test cases in this file relate to previously observed bugs. A failure of one
of these indicates that a past bug has been reintroduced.
"""

import json
import os
from pathlib import Path
import platform
import re
import signal
import stat
import subprocess
import sys
import tempfile
from typing import List
import xml.etree.ElementTree as ET
import pytest

sys.path.append(os.path.dirname(__file__))
from gvtest import dot, gvpr, ROOT, remove_xtype_warnings, run_c, which \
  #pylint: disable=wrong-import-position

def is_ndebug_defined() -> bool:
  """
  are assertions disabled in the Graphviz build under test?
  """

  # the Windows release builds set NDEBUG
  if os.environ.get("configuration") == "Release":
    return True

  return False

# The terminology used in rtest.py is a little inconsistent. At the
# end it reports the total number of tests, the number of "failures"
# (crashes) and the number of "changes" (which is the number of tests
# where the output file did not match the reference file). However,
# for each test that detects "changes", it prints an error message
# saying "== Failed ==" which thus is not counted as a failure at the
# end.

def test_regression_failure():
  """
  Run all tests but ignore differences and fail the test only if there is a
  crash. This will leave the differences for png output in
  tests/nhtml/index.html for review.
  """

  os.chdir(Path(__file__).resolve().parent)
  with subprocess.Popen([sys.executable, "rtest.py"], stderr=subprocess.PIPE,
                        universal_newlines=True) as result:
    text = result.communicate()[1]
  print(text)
  assert "Layout failures: 0" in text
# FIXME: re-enable when all tests pass on all platforms
#    assert result.returncode == 0

@pytest.mark.xfail(platform.system() == "Windows",
        reason="#56",
        strict=not is_ndebug_defined()) # FIXME
def test_14():
  """
  using ortho and twopi in combination should not cause an assertion failure
  https://gitlab.com/graphviz/graphviz/-/issues/14
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "14.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  dot("svg", input)

@pytest.mark.skipif(which("neato") is None, reason="neato not available")
def test_42():
  """
  check for a former crash in neatogen
  https://gitlab.com/graphviz/graphviz/-/issues/42
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "42.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  subprocess.check_call(["neato", "-n2", "-Tpng", input],
                        stdout=subprocess.DEVNULL)

def test_56():
  """
  parsing a particular graph should not cause a Trapezoid-table overflow
  assertion failure
  https://gitlab.com/graphviz/graphviz/-/issues/56
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "56.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  dot("svg", input)

def test_121():
  """
  test a graph that previously caused an assertion failure in `merge_chain`
  https://gitlab.com/graphviz/graphviz/-/issues/121
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "121.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  dot("pdf", input)

def test_131():
  """
  PIC back end should produce valid output
  https://gitlab.com/graphviz/graphviz/-/issues/131
  """

  # a basic graph
  src = "digraph { a -> b; c -> d; }"

  # ask Graphviz to process this to PIC
  pic = dot("pic", source=src)

  if which("gpic") is None:
    pytest.skip("GNU PIC not available")

  # ask GNU PIC to process the Graphviz output
  subprocess.run(["gpic"], input=pic, stdout=subprocess.DEVNULL, check=True,
    universal_newlines=True)

@pytest.mark.parametrize("testcase", ("144_no_ortho.dot", "144_ortho.dot"))
def test_144(testcase: str):
  """
  using ortho should not result in head/tail confusion
  https://gitlab.com/graphviz/graphviz/-/issues/144
  """

  # locate our associated test cases in this directory
  input = Path(__file__).parent / testcase
  assert input.exists(), "unexpectedly missing test case"

  # process the non-ortho one into JSON
  out = dot("json", input)
  data = json.loads(out)

  # find the two nodes, “A” and “B”
  A = [x for x in data["objects"] if x["name"] == "A"][0]
  B = [x for x in data["objects"] if x["name"] == "B"][0]

  # find the edge between them
  edge = [x for x in data["edges"]
          if x["tail"] == A["_gvid"] and x["head"] == B["_gvid"]][0]

  # the edge between them should have been routed vertically
  points = edge["_draw_"][1]["points"]
  xs = [x for x, _ in points]
  assert all(x == xs[0] for x in xs), "A->B not routed vertically"

  # determine whether it is routed down or up
  ys = [y for _, y in points]
  if ys == sorted(ys):
    routed_up = True
  elif list(reversed(ys)) == sorted(ys):
    routed_up = False
  else:
    pytest.fail("A->B seems routed neither straight up nor down")

  # determine Graphviz’ idea of which end is the head and which is the tail
  head_point = edge["_hldraw_"][2]["pt"]
  tail_point = edge["_tldraw_"][2]["pt"]
  head_is_top = head_point[1] > tail_point[1]

  # FIXME: remove when #144 is fixed
  if testcase == "144_ortho.dot":
    assert routed_up != head_is_top, "#144 fixed?"
    return

  # this should be consistent with the direction the edge is drawn
  assert routed_up == head_is_top, "heap/tail confusion"

def test_146():
  """
  dot should respect an alpha channel value of 0 when writing SVG
  https://gitlab.com/graphviz/graphviz/-/issues/146
  """

  # a graph using white text but with 0 alpha
  source = 'graph {\n'                                                         \
           '  n[style="filled", fontcolor="#FFFFFF00", label="hello world"];\n'\
           '}'

  # ask Graphviz to process this
  svg = dot("svg", source=source)

  # the SVG should be setting opacity
  opacity = re.search(r'\bfill-opacity="(\d+(\.\d+)?)"', svg)
  assert opacity is not None, "transparency not set for alpha=0 color"

  # it should be zeroed
  assert float(opacity.group(1)) == 0, \
    "alpha=0 color set to something non-transparent"

def test_165():
  """
  dot should be able to produce properly escaped xdot output
  https://gitlab.com/graphviz/graphviz/-/issues/165
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "165.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to translate it to xdot
  output = dot("xdot", input)

  # find the line containing the _ldraw_ attribute
  ldraw = re.search(r"^\s*_ldraw_\s*=(?P<value>.*?)$", output, re.MULTILINE)
  assert ldraw is not None, "no _ldraw_ attribute in graph"

  # this should contain the label correctly escaped
  assert r'hello \\\" world' in ldraw.group("value"), \
    "unexpected ldraw contents"

def test_165_2():
  """
  variant of test_165() that checks a similar problem for edges
  https://gitlab.com/graphviz/graphviz/-/issues/165
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "165_2.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to translate it to xdot
  output = dot("xdot", input)

  # find the lines containing _ldraw_ attributes
  ldraw = re.findall(r"^\s*_ldraw_\s*=(.*?)$", output, re.MULTILINE)
  assert ldraw is not None, "no _ldraw_ attributes in graph"

  # one of these should contain the label correctly escaped
  assert any(r'hello \\\" world' in l for l in ldraw), \
    "unexpected ldraw contents"

def test_165_3():
  """
  variant of test_165() that checks a similar problem for graph labels
  https://gitlab.com/graphviz/graphviz/-/issues/165
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "165_3.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to translate it to xdot
  output = dot("xdot", input)

  # find the lines containing _ldraw_ attributes
  ldraw = re.findall(r"^\s*_ldraw_\s*=(.*?)$", output, re.MULTILINE)
  assert ldraw is not None, "no _ldraw_ attributes in graph"

  # one of these should contain the label correctly escaped
  assert any(r'hello \\\" world' in l for l in ldraw), \
    "unexpected ldraw contents"

def test_167():
  """
  using concentrate=true should not result in a segfault
  https://gitlab.com/graphviz/graphviz/-/issues/167
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent /  "167.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process this with dot
  ret = subprocess.call(["dot", "-Tpdf", "-o", os.devnull, input])

  # Graphviz should not have caused a segfault
  assert ret != -signal.SIGSEGV, "Graphviz segfaulted"

def test_191():
  """
  a comma-separated list without quotes should cause a hard error, not a warning
  https://gitlab.com/graphviz/graphviz/-/issues/191
  """

  source = 'graph {\n' \
           '  "Trackable" [fontcolor=grey45,labelloc=c,fontname=Vera Sans, ' \
           'DejaVu Sans, Liberation Sans, Arial, Helvetica, sans,shape=box,' \
           'height=0.3,align=center,fontsize=10,style="setlinewidth(0.5)"];\n' \
           '}'

  with subprocess.Popen(["dot", "-Tdot"], stdin=subprocess.PIPE,
                        stderr=subprocess.PIPE, universal_newlines=True) as p:
    _, stderr = p.communicate(source)

    assert "syntax error" in stderr, "missing error message for unquoted list"

    assert p.returncode != 0, "syntax error was only a warning, not an error"

def test_358():
  """
  setting xdot version to 1.7 should enable font characteristics
  https://gitlab.com/graphviz/graphviz/-/issues/358
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent /  "358.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process this with dot
  xdot = dot("xdot", input)

  for i in range(6):
    m = re.search(f"\\bt {1 << i}\\b", xdot)
    assert m is not None, \
      f"font characteristic {1 << i} not enabled in xdot 1.7"

@pytest.mark.skipif(which("gv2gxl") is None or which("gxl2gv") is None,
                    reason="GXL tools not available")
def test_517():
  """
  round tripping a graph through gv2gxl should not lose HTML labels
  https://gitlab.com/graphviz/graphviz/-/issues/517
  """

  # our test case input
  input = \
    'digraph{\n' \
    '  A[label=<<TABLE><TR><TD>(</TD><TD>A</TD><TD>)</TD></TR></TABLE>>]\n' \
    '  B[label="<TABLE><TR><TD>(</TD><TD>B</TD><TD>)</TD></TR></TABLE>"]\n' \
    '}'

  # translate it to GXL
  gxl = subprocess.check_output(["gv2gxl"], input=input,
    universal_newlines=True)

  # translate this back to Dot
  dot_output = subprocess.check_output(["gxl2gv"], input=gxl,
    universal_newlines=True)

  # the result should have both expected labels somewhere
  assert \
    "label=<<TABLE><TR><TD>(</TD><TD>A</TD><TD>)</TD></TR></TABLE>>" in \
    dot_output, "HTML label missing"
  assert \
    'label="<TABLE><TR><TD>(</TD><TD>B</TD><TD>)</TD></TR></TABLE>"' in \
    dot_output, "regular label missing"

def test_793():
  """
  Graphviz should not crash when using VRML output with a non-writable current
  directory
  https://gitlab.com/graphviz/graphviz/-/issues/793
  """

  # create a non-writable directory
  with tempfile.TemporaryDirectory() as tmp:
    t = Path(tmp)
    t.chmod(t.stat().st_mode & ~stat.S_IWRITE)

    # ask the VRML back end to handle a simple graph, using the above as the
    # current working directory
    with subprocess.Popen(["dot", "-Tvrml", "-o", os.devnull], cwd=t) as p:
      p.communicate("digraph { a -> b; }")

      # Graphviz should not have caused a segfault
      assert p.returncode != -signal.SIGSEGV, "Graphviz segfaulted"

def test_797():
  """
  “&;” should not be considered an XML escape sequence
  https://gitlab.com/graphviz/graphviz/-/issues/797
  """

  # some input containing the invalid escape
  input = 'digraph tree {\n' \
          '"1" [shape="box", label="&amp; &amp;;", URL="a"];\n' \
          '}'

  # process this with the client-side imagemap back end
  output = dot("cmapx", source=input)

  # the escape sequences should have been preserved
  assert "&amp; &amp;" in output

def test_827():
  """
  Graphviz should not crash when processing the b15.gv example
  https://gitlab.com/graphviz/graphviz/-/issues/827
  """

  b15gv = Path(__file__).parent / "graphs/b15.gv"
  assert b15gv.exists(), "missing test case file"

  ret = subprocess.call(["dot", "-Tsvg", "-o", os.devnull, b15gv])

  assert ret == 1, "Graphviz crashed when processing b15.gv"

def test_925():
  """
  spaces should be handled correctly in UTF-8-containing labels in record shapes
  https://gitlab.com/graphviz/graphviz/-/issues/925
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "925.dot"
  assert input.exists(), "unexpectedly mising test case"

  # process this with dot
  svg = dot("svg", input)

  # The output should include the correctly spaced UTF-8 label. Note that these
  # are not ASCII capital As in this string, but rather UTF-8 Cyrillic Capital
  # Letter As.
  assert "ААА ААА ААА" in svg, "incorrect spacing in UTF-8 label"

def test_1221():
  """
  assigning a node to two clusters with newrank should not cause a crash
  https://gitlab.com/graphviz/graphviz/-/issues/1221
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1221.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process this with dot
  dot("svg", input)

@pytest.mark.skipif(which("gv2gml") is None, reason="gv2gml not available")
def test_1276():
  """
  quotes within a label should be escaped in translation to GML
  https://gitlab.com/graphviz/graphviz/-/issues/1276
  """

  # DOT input containing a label with quotes
  src = 'digraph test {\n' \
        '  x[label=<"Label">];\n' \
        '}'

  # process this to GML
  gml = subprocess.check_output(["gv2gml"], input=src, universal_newlines=True)

  # the unescaped label should not appear in the output
  assert '""Label""' not in gml, "quotes not escaped in label"

  # the escaped label should appear in the output
  assert '"&quot;Label&quot;"' in gml or '"&#34;Label&#34;"' in gml, \
    "escaped label not found in GML output"

def test_1314():
  """
  test that a large font size that produces an overflow in Pango is rejected
  https://gitlab.com/graphviz/graphviz/-/issues/1314
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1314.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it, which should fail
  try:
    dot("svg", input)
  except subprocess.CalledProcessError:
    return

  # the execution did not fail as expected
  pytest.fail("dot incorrectly exited with success")

def test_1408():
  """
  parsing particular ortho layouts should not cause an assertion failure
  https://gitlab.com/graphviz/graphviz/-/issues/1408
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1408.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  dot("svg", input)

def test_1411():
  """
  parsing strings containing newlines should not disrupt line number tracking
  https://gitlab.com/graphviz/graphviz/-/issues/1411
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1411.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz (should fail)
  with subprocess.Popen(["dot", "-Tsvg", "-o", os.devnull, input],
                        stderr=subprocess.PIPE, universal_newlines=True) as p:
    _, output = p.communicate()

    assert p.returncode != 0, "Graphviz accepted broken input"

  assert "syntax error in line 17 near '\\'" in output, \
    'error message did not identify correct location'

def test_1436():
  """
  test a segfault from https://gitlab.com/graphviz/graphviz/-/issues/1436 has
  not reappeared
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1436.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it, which should generate a segfault if this bug
  # has been reintroduced
  dot("svg", input)

def test_1444():
  """
  specifying 'headport' as an edge attribute should work regardless of what
  order attributes appear in
  https://gitlab.com/graphviz/graphviz/-/issues/1444
  """

  # locate the first of our associated tests
  input1 = Path(__file__).parent / "1444.dot"
  assert input1.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it
  with subprocess.Popen(["dot", "-Tsvg", input1], stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE, universal_newlines=True) as p:
    stdout1, stderr = p.communicate()

    assert p.returncode == 0, "failed to process a headport edge"

  stderr = remove_xtype_warnings(stderr).strip()
  assert stderr == "", "emitted an error for a legal graph"

  # now locate our second variant, that simply has the attributes swapped
  input2 = Path(__file__).parent / "1444-2.dot"
  assert input2.exists(), "unexpectedly missing test case"

  # process it identically
  with subprocess.Popen(["dot", "-Tsvg", input2], stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE, universal_newlines=True) as p:
    stdout2, stderr = p.communicate()

    assert p.returncode == 0, "failed to process a headport edge"

  stderr = remove_xtype_warnings(stderr).strip()
  assert stderr == "", "emitted an error for a legal graph"

  assert stdout1 == stdout2, \
    "swapping edge attributes altered the output graph"

def test_1449():
  """
  using the SVG color scheme should not cause warnings
  https://gitlab.com/graphviz/graphviz/-/issues/1449
  """

  # start Graphviz
  with subprocess.Popen(["dot", "-Tsvg", "-o", os.devnull],
                        stdin=subprocess.PIPE,
                        stderr=subprocess.PIPE, universal_newlines=True) as p:

    # pass it some input that uses the SVG color scheme
    _, stderr = p.communicate('graph g { colorscheme="svg"; }')

    assert p.returncode == 0, "Graphviz exited with non-zero status"

  assert stderr.strip() == "", "SVG color scheme use caused warnings"

@pytest.mark.skipif(which("gvpr") is None, reason="GVPR not available")
def test_1594():
  """
  GVPR should give accurate line numbers in error messages
  https://gitlab.com/graphviz/graphviz/-/issues/1594
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1594.gvpr"

  # run GVPR with our (malformed) input program
  with subprocess.Popen(["gvpr", "-f", input], stdin=subprocess.PIPE,
                        stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                        universal_newlines=True) as p:
    _, stderr = p.communicate()

    assert p.returncode != 0, "GVPR did not reject malformed program"

  assert "line 3:" in stderr, \
    "GVPR did not identify correct line of syntax error"

@pytest.mark.parametrize("long,short", (("--help", "-?"),
                                        ("--version", "-V")))
def test_1618(long: str, short: str):
  """
  Graphviz should understand `--help` and `--version`
  https://gitlab.com/graphviz/graphviz/-/issues/1618
  """

  # run Graphviz with the short form of the argument
  p1 = subprocess.run(["dot", short], stdout=subprocess.PIPE,
                      stderr=subprocess.PIPE, check=True)

  # run it with the long form of the argument
  p2 = subprocess.run(["dot", long], stdout=subprocess.PIPE,
                      stderr=subprocess.PIPE, check=True)

  # output from both should match
  assert p1.stdout == p2.stdout, f"`dot {long}` wrote output than `dot {short}`"
  assert p1.stderr == p2.stderr, f"`dot {long}` wrote output than `dot {short}`"

@pytest.mark.xfail(strict=True)
def test_1624():
  """
  record shapes should be usable
  https://gitlab.com/graphviz/graphviz/-/issues/1624
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1624.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  dot("svg", input)

def test_1658():
  """
  the graph associated with this test case should not crash Graphviz
  https://gitlab.com/graphviz/graphviz/-/issues/1658
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1658.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  dot("png", input)

def test_1676():
  """
  https://gitlab.com/graphviz/graphviz/-/issues/1676
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1676.dot"
  assert input.exists(), "unexpectedly missing test case"

  # run Graphviz with this input
  ret = subprocess.call(["dot", "-Tsvg", "-o", os.devnull, input])

  # this malformed input should not have caused Graphviz to crash
  assert ret != -signal.SIGSEGV, "Graphviz segfaulted"

def test_1724():
  """
  passing malformed node and newrank should not cause segfaults
  https://gitlab.com/graphviz/graphviz/-/issues/1724
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1724.dot"
  assert input.exists(), "unexpectedly missing test case"

  # run Graphviz with this input
  ret = subprocess.call(["dot", "-Tsvg", "-o", os.devnull, input])

  assert ret != -signal.SIGSEGV, "Graphviz segfaulted"

def test_1767():
  """
  using the Pango plugin multiple times should produce consistent results
  https://gitlab.com/graphviz/graphviz/-/issues/1767
  """

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1777 is fixed
  if os.getenv("build_system") == "msbuild":
    pytest.skip("Windows MSBuild release does not contain any header files (#1777)")

  # find co-located test source
  c_src = (Path(__file__).parent / "1767.c").resolve()
  assert c_src.exists(), "missing test case"

  # find our co-located dot input
  src = (Path(__file__).parent / "1767.dot").resolve()
  assert src.exists(), "missing test case"

  _, _ = run_c(c_src, [src], link=["cgraph", "gvc"])

  # FIXME: uncomment this when #1767 is fixed
  # assert stdout == "Loaded graph:clusters\n" \
  #                  "cluster_0 contains 5 nodes\n" \
  #                  "cluster_1 contains 1 nodes\n" \
  #                  "cluster_2 contains 3 nodes\n" \
  #                  "cluster_3 contains 3 nodes\n" \
  #                  "Loaded graph:clusters\n" \
  #                  "cluster_0 contains 5 nodes\n" \
  #                  "cluster_1 contains 1 nodes\n" \
  #                  "cluster_2 contains 3 nodes\n" \
  #                  "cluster_3 contains 3 nodes\n"

@pytest.mark.skipif(which("gvpr") is None, reason="GVPR not available")
@pytest.mark.skipif(platform.system() != "Windows",
  reason="only relevant on Windows")
def test_1780():
  """
  GVPR should accept programs at absolute paths
  https://gitlab.com/graphviz/graphviz/-/issues/1780
  """

  # get absolute path to an arbitrary GVPR program
  clustg = Path(__file__).resolve().parent.parent / "cmd/gvpr/lib/clustg"

  # GVPR should not fail when given this path
  gvpr(clustg)

def test_1783():
  """
  Graphviz should not segfault when passed large edge weights
  https://gitlab.com/graphviz/graphviz/-/issues/1783
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1783.dot"
  assert input.exists(), "unexpectedly missing test case"

  # run Graphviz with this input
  ret = subprocess.call(["dot", "-Tsvg", "-o", os.devnull, input])

  assert ret != 0, "Graphviz accepted illegal edge weight"

  assert ret != -signal.SIGSEGV, "Graphviz segfaulted"

@pytest.mark.skipif(which("gvedit") is None, reason="Gvedit not available")
def test_1813():
  """
  gvedit -? should show usage
  https://gitlab.com/graphviz/graphviz/-/issues/1813
  """

  environ_copy = os.environ.copy()
  environ_copy.pop("DISPLAY", None)
  output = subprocess.check_output(["gvedit", "-?"],
    env=environ_copy,
    universal_newlines=True)

  assert "Usage" in output, "gvedit -? did not show usage"

def test_1845():
  """
  rendering sequential graphs to PS should not segfault
  https://gitlab.com/graphviz/graphviz/-/issues/1845
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1845.dot"
  assert input.exists(), "unexpectedly missing test case"

  # generate a multipage PS file from this input
  dot("ps", input)

@pytest.mark.xfail(strict=True) # FIXME
def test_1856():
  """
  headports and tailports should be respected
  https://gitlab.com/graphviz/graphviz/-/issues/1856
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1856.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it into JSON
  out = dot("json", input)
  data = json.loads(out)

  # find the two nodes, “3” and “5”
  three = [x for x in data["objects"] if x["name"] == "3"][0]
  five  = [x for x in data["objects"] if x["name"] == "5"][0]

  # find the edge from “3” to “5”
  edge = [x for x in data["edges"]
          if x["tail"] == three["_gvid"] and x["head"] == five["_gvid"]][0]

  # The edge should look something like:
  #
  #        ┌─┐
  #        │3│
  #        └┬┘
  #    ┌────┘
  #   ┌┴┐
  #   │5│
  #   └─┘
  #
  # but a bug causes port constraints to not be respected and the edge comes out
  # more like:
  #
  #        ┌─┐
  #        │3│
  #        └┬┘
  #         │
  #   ┌─┐   │
  #   ├5̶┼───┘
  #   └─┘
  #
  # So validate that the edge’s path does not dip below the top of the “5” node.

  top_of_five = max(y for _, y in five["_draw_"][1]["points"])

  waypoints_y = [y for _, y in edge["_draw_"][1]["points"]]

  assert all(y >= top_of_five for y in waypoints_y), "edge dips below 5"

@pytest.mark.skipif(which("fdp") is None, reason="fdp not available")
def test_1865():
  """
  fdp should not read out of bounds when processing node names
  https://gitlab.com/graphviz/graphviz/-/issues/1865
  Note, the crash this test tries to provoke may only occur when run under
  Address Sanitizer or Valgrind
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1865.dot"
  assert input.exists(), "unexpectedly missing test case"

  # fdp should not crash when processing this file
  subprocess.check_call(["fdp", "-o", os.devnull, input])

@pytest.mark.skipif(which("gv2gml") is None, reason="gv2gml not available")
@pytest.mark.skipif(which("gml2gv") is None, reason="gml2gv not available")
@pytest.mark.parametrize("penwidth", ("1.0", "1"))
def test_1871(penwidth: str):
  """
  round tripping something with either an integer or real `penwidth` through
  gv2gml→gml2gv should return the correct `penwidth`
  """

  # a trivial graph
  input = f"graph {{ a [penwidth={penwidth}] }}"

  # pass it through gv2gml
  gv = subprocess.check_output(["gv2gml"], input=input, universal_newlines=True)

  # pass this through gml2gv
  gml = subprocess.check_output(["gml2gv"], input=gv, universal_newlines=True)

  # the result should have a `penwidth` of 1
  has_1 = re.search(r"\bpenwidth\s*=\s*1[^\.]", gml) is not None
  has_1_0 = re.search(r"\bpenwidth\s*=\s*1\.0\b", gml) is not None
  assert has_1 or has_1_0, \
    f"incorrect penwidth from round tripping through GML (output {gml})"

@pytest.mark.skipif(which("fdp") is None, reason="fdp not available")
def test_1876():
  """
  fdp should not rename nodes with internal names
  https://gitlab.com/graphviz/graphviz/-/issues/1876
  """

  # a trivial graph to provoke this issue
  input = "graph { a }"

  # process this with fdp
  try:
    output = subprocess.check_output(["fdp"], input=input,
      universal_newlines=True)
  except subprocess.CalledProcessError as e:
    raise RuntimeError("fdp failed to process trivial graph") from e

  # we should not see any internal names like "%3"
  assert "%" not in output, "internal name in fdp output"

@pytest.mark.skipif(which("fdp") is None, reason="fdp not available")
def test_1877():
  """
  fdp should not fail an assertion when processing cluster edges
  https://gitlab.com/graphviz/graphviz/-/issues/1877
  """

  # simple input with a cluster edge
  input = "graph {subgraph cluster_a {}; cluster_a -- b}"

  # fdp should be able to process this
  subprocess.run(["fdp", "-o", os.devnull], input=input, check=True,
    universal_newlines=True)

def test_1880():
  """
  parsing a particular graph should not cause a Trapezoid-table overflow
  assertion failure
  https://gitlab.com/graphviz/graphviz/-/issues/1880
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1880.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  dot("png", input)

def test_1898():
  """
  test a segfault from https://gitlab.com/graphviz/graphviz/-/issues/1898 has
  not reappeared
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1898.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it, which should generate a segfault if this bug
  # has been reintroduced
  dot("svg", input)

def test_1902():
  """
  test a segfault from https://gitlab.com/graphviz/graphviz/-/issues/1902 has
  not reappeared
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1902.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it, which should generate a segfault if this bug
  # has been reintroduced
  dot("svg", input)

# root directory of this checkout
ROOT = Path(__file__).parent.parent.resolve()

def test_1855():
  """
  SVGs should have a scale with sufficient precision
  https://gitlab.com/graphviz/graphviz/-/issues/1855
  """

  # locate our associated test case in this directory
  src = Path(__file__).parent / "1855.dot"
  assert src.exists(), "unexpectedly missing test case"

  # run it through Graphviz
  svg = dot("svg", src)

  # find the graph element
  root = ET.fromstring(svg)
  graph = root[0]
  assert graph.get("class") == "graph", "could not find graph element"

  # extract its `transform` attribute
  transform = graph.get("transform")

  # this should begin with a scale directive
  m = re.match(r"scale\((?P<x>\d+(\.\d*)?) (?P<y>\d+(\.\d*))\)", transform)
  assert m is not None, f"failed to find 'scale' in '{transform}'"

  x = m.group("x")
  y = m.group("y")

  # the scale should be somewhere in reasonable range of what is expected
  assert float(x) >= 0.32 and float(x) <= 0.34, "inaccurate x scale"
  assert float(y) >= 0.32 and float(y) <= 0.34, "inaccurate y scale"

  # two digits of precision are insufficient for this example, so require a
  # greater number of digits in both scale components
  assert len(x) > 4, "insufficient precision in x scale"
  assert len(y) > 4, "insufficient precision in y scale"

@pytest.mark.parametrize("variant", [1, 2])
@pytest.mark.skipif(which("gml2gv") is None, reason="gml2gv not available")
def test_1869(variant: int):
  """
  gml2gv should be able to parse the style, outlineStyle, width and
  outlineWidth GML attributes and map them to the DOT attributes
  style and penwidth respectively
  https://gitlab.com/graphviz/graphviz/-/issues/1869
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / f"1869-{variant}.gml"
  assert input.exists(), "unexpectedly missing test case"

  # ask gml2gv to translate it to DOT
  output = subprocess.check_output(["gml2gv", input],
    universal_newlines=True)

  assert "style=dashed" in output, "style=dashed not found in DOT output"
  assert "penwidth=2" in output, "penwidth=2 not found in DOT output"

@pytest.mark.xfail() # FIXME
def test_1879():
  """https://gitlab.com/graphviz/graphviz/-/issues/1879"""

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1879.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with DOT
  stdout = subprocess.check_output(["dot", "-Tsvg", "-o", os.devnull, input],
                                   cwd=Path(__file__).parent,
                                   stderr=subprocess.STDOUT,
                                   universal_newlines=True)

  # check we did not trigger an assertion failure
  print(stdout)
  assert re.search(r"\bAssertion\b.*\bfailed\b", stdout) is None

def test_1893():
  """
  an HTML label containing just a ] should work
  https://gitlab.com/graphviz/graphviz/-/issues/1893
  """

  # a graph containing a node with an HTML label with a ] in a table cell
  input = "digraph { 0 [label=<<TABLE><TR><TD>]</TD></TR></TABLE>>] }"

  # ask Graphviz to process this
  dot("svg", source=input)

  # we should be able to do the same with an escaped ]
  input = "digraph { 0 [label=<<TABLE><TR><TD>&#93;</TD></TR></TABLE>>] }"

  dot("svg", source=input)

def test_1906():
  """
  graphs that cause an overflow during rectangle calculation should result in
  a layout error
  https://gitlab.com/graphviz/graphviz/-/issues/1906
  """

  # one of the rtest graphs is sufficient to provoke this
  input = Path(__file__).parent / "graphs/root.gv"
  assert input.exists(), "unexpectedly missing test case"

  # use Circo to translate it to DOT
  with subprocess.Popen(["dot", "-Kcirco", "-Tgv", "-o", os.devnull, input],
                        stderr=subprocess.PIPE, universal_newlines=True) as p:
    _, stderr = p.communicate()

    assert p.returncode != 0, "graph that generates overflow was accepted"

  assert "area too large" in stderr, "missing/incorrect error message"

@pytest.mark.skipif(which("twopi") is None, reason="twopi not available")
def test_1907():
  """
  SVG edges should have title elements that match their names
  https://gitlab.com/graphviz/graphviz/-/issues/1907
  """

  # a trivial graph to provoke this issue
  input = "digraph { A -> B -> C }"

  # generate an SVG from this input with twopi
  output = subprocess.check_output(["twopi", "-Tsvg"], input=input,
    universal_newlines=True)

  assert "<title>A&#45;&gt;B</title>" in output, \
    "element title not found in SVG"

@pytest.mark.skipif(which("gvpr") is None, reason="gvpr not available")
def test_1909():
  """
  GVPR should not output internal names
  https://gitlab.com/graphviz/graphviz/-/issues/1909
  """

  # locate our associated test case in this directory
  prog = Path(__file__).parent / "1909.gvpr"
  graph = Path(__file__).parent / "1909.dot"

  # run GVPR with the given input
  output = subprocess.check_output(["gvpr", "-c", "-f", prog, graph],
    universal_newlines=True)

  # we should have produced this graph without names like "%2" in it
  assert output == "// begin\n" \
                   "digraph bug {\n" \
                   "	a -> b;\n" \
                   "	b -> c;\n" \
                   "}\n"

def test_1910():
  """
  Repeatedly using agmemread() should have consistent results
  https://gitlab.com/graphviz/graphviz/-/issues/1910
  """

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1777 is fixed
  if os.getenv("build_system") == "msbuild":
    pytest.skip("Windows MSBuild release does not contain any header files (#1777)")

  # find co-located test source
  c_src = (Path(__file__).parent / "1910.c").resolve()
  assert c_src.exists(), "missing test case"

  # run the test
  _, _ = run_c(c_src, link=["cgraph", "gvc"])

def test_1913():
  """
  ALIGN attributes in <BR> tags should be parsed correctly
  https://gitlab.com/graphviz/graphviz/-/issues/1913
  """

  # a template of a trivial graph using an ALIGN attribute
  graph = 'digraph {{\n' \
          '  table1[label=<<table><tr><td align="text">hello world' \
          '<br align="{}"/></td></tr></table>>];\n' \
          '}}'

  def run(input):
    """
    run Dot with the given input and return its exit status and stderr
    """
    with subprocess.Popen(["dot", "-Tsvg", "-o", os.devnull],
                          stdin=subprocess.PIPE, stderr=subprocess.PIPE,
                          universal_newlines=True) as p:
      _, stderr = p.communicate(input)
      return p.returncode, remove_xtype_warnings(stderr)

  # Graphviz should accept all legal values for this attribute
  for align in ("left", "right", "center"):

    input = align
    ret, stderr = run(graph.format(input))
    assert ret == 0
    assert stderr.strip() == ""

    # these attributes should also be valid when title cased
    input = f"{align[0].upper()}{align[1:]}"
    ret, stderr = run(graph.format(input))
    assert ret == 0
    assert stderr.strip() == ""

    # similarly, they should be valid when upper cased
    input = align.upper()
    ret, stderr = run(graph.format(input))
    assert ret == 0
    assert stderr.strip() == ""

  # various invalid things that have the same prefix or suffix as a valid
  # alignment should be rejected
  for align in ("lamp", "deft", "round", "might", "circle", "venter"):

    input = align
    _, stderr = run(graph.format(input))
    assert f"Warning: Illegal value {input} for ALIGN - ignored" in stderr

    # these attributes should also fail when title cased
    input = f"{align[0].upper()}{align[1:]}"
    _, stderr = run(graph.format(input))
    assert f"Warning: Illegal value {input} for ALIGN - ignored" in stderr

    # similarly, they should fail when upper cased
    input = align.upper()
    _, stderr = run(graph.format(input))
    assert f"Warning: Illegal value {input} for ALIGN - ignored" in stderr

def test_1931():
  """
  New lines within strings should not be discarded during parsing

  """

  # a graph with \n inside of strings
  graph = 'graph {\n'                 \
          '  node1 [label="line 1\n'  \
          'line 2\n'                  \
          '"];\n'                     \
          '  node2 [label="line 3\n'  \
          'line 4"];\n'                \
          '  node1 -- node2\n'        \
          '  node2 -- "line 5\n'      \
          'line 6"\n'                 \
          '}'

  # ask Graphviz to process this to dot output
  xdot = dot("xdot", source=graph)

  # all new lines in strings should have been preserved
  assert "line 1\nline 2\n" in xdot
  assert "line 3\nline 4" in xdot
  assert "line 5\nline 6" in xdot

@pytest.mark.skipif(which("edgepaint") is None,
                    reason="edgepaint not available")
def test_1971():
  """
  edgepaint should reject invalid command line options
  https://gitlab.com/graphviz/graphviz/-/issues/1971
  """

  # a basic graph that edgepaint can process
  input =                                                                      \
    'digraph {\n'                                                              \
    '  graph [bb="0,0,54,108"];\n'                                             \
    '  node [label="\\N"];\n'                                                  \
    '  a       [height=0.5,\n'                                                 \
    '           pos="27,90",\n'                                                \
    '           width=0.75];\n'                                                \
    '  b       [height=0.5,\n'                                                 \
    '           pos="27,18",\n'                                                \
    '           width=0.75];\n'                                                \
    '  a -> b  [pos="e,27,36.104 27,71.697 27,63.983 27,54.712 27,46.112"];\n' \
    '}'

  # run edgepaint with an invalid option, `-rabbit`, that happens to have the
  # same first character as valid options
  args = ["edgepaint", "-rabbit"]
  with subprocess.Popen(args, stdin=subprocess.PIPE,
                        universal_newlines=True) as p:
    p.communicate(input)

    assert p.returncode != 0, "edgepaint incorrectly accepted '-rabbit'"

@pytest.mark.xfail(platform.system() == "Windows",
        reason="#56",
        strict=not is_ndebug_defined()) # FIXME
def test_1990():
  """
  using ortho and circo in combination should not cause an assertion failure
  https://gitlab.com/graphviz/graphviz/-/issues/14
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1990.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  subprocess.check_call(["circo", "-Tsvg", "-o", os.devnull, input])

def test_2057():
  """
  gvToolTred should be usable by user code
  https://gitlab.com/graphviz/graphviz/-/issues/2057
  """

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1777 is fixed
  if os.getenv("build_system") == "msbuild":
    pytest.skip("Windows MSBuild release does not contain any header files (#1777)")

  # find co-located test source
  c_src = (Path(__file__).parent / "2057.c").resolve()
  assert c_src.exists(), "missing test case"

  # run the test
  _, _ = run_c(c_src, link=["gvc"])

def test_2078():
  """
  Incorrectly using the "layout" attribute on a subgraph should result in a
  sensible error.
  https://gitlab.com/graphviz/graphviz/-/issues/2078
  """

  # our sample graph that incorrectly uses layout
  input = "graph {\n"          \
          "  subgraph {\n"     \
          "    layout=osage\n" \
          "  }\n"              \
          "}"

  # run it through Graphviz
  with subprocess.Popen(["dot", "-Tcanon", "-o", os.devnull],
                        stdin=subprocess.PIPE, stderr=subprocess.PIPE,
                        universal_newlines=True) as p:
    _, stderr = p.communicate(input)

    assert p.returncode != 0, "layout on subgraph was incorrectly accepted"

  assert "layout attribute is invalid except on the root graph" in stderr, \
    "expected warning not found"

  # a graph that correctly uses layout
  input = "graph {\n"          \
          "  layout=osage\n" \
          "  subgraph {\n"     \
          "  }\n"              \
          "}"

  # ensure this one does not trigger warnings
  with subprocess.Popen(["dot", "-Tcanon", "-o", os.devnull],
                        stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE, universal_newlines=True) as p:
    stdout, stderr = p.communicate(input)

    assert p.returncode == 0, "correct layout use was rejected"

  assert stdout.strip() == "", "unexpected output"
  assert "layout attribute is invalid except on the root graph" not in stderr, \
    "incorrect warning output"

def test_2082():
  """
  Check a bug in inside_polygon has not been reintroduced.
  https://gitlab.com/graphviz/graphviz/-/issues/2082
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "2082.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it, which should generate an assertion failure if
  # this bug has been reintroduced
  dot("png", input)

@pytest.mark.xfail(strict=True)
@pytest.mark.parametrize("html_like_first", (False, True))
def test_2089(html_like_first: bool): # FIXME
  """
  HTML-like and non-HTML-like strings should peacefully coexist
  https://gitlab.com/graphviz/graphviz/-/issues/2089
  """

  # a graph using an HTML-like string and a non-HTML-like string
  if html_like_first:
    graph = 'graph {\n'           \
            '  a[label=<foo>];\n' \
            '  b[label="foo"];\n' \
            '}'
  else:
    graph = 'graph {\n'           \
            '  a[label="foo"];\n' \
            '  b[label=<foo>];\n' \
            '}'

  # normalize the graph
  canonical = dot("dot", source=graph)

  assert 'label=foo' in canonical, "non-HTML-like label not found"
  assert "label=<foo>" in canonical, "HTML-like label not found"

@pytest.mark.xfail(strict=True) # FIXME
def test_2089_2():
  """
  HTML-like and non-HTML-like strings should peacefully coexist
  https://gitlab.com/graphviz/graphviz/-/issues/2089
  """

  # find co-located test source
  c_src = (Path(__file__).parent / "2089.c").resolve()
  assert c_src.exists(), "missing test case"

  # run it
  _, _ = run_c(c_src, link=["cgraph"])

@pytest.mark.skipif(which("dot2gxl") is None, reason="dot2gxl not available")
def test_2092():
  """
  an empty node ID should not cause a dot2gxl NULL pointer dereference
  https://gitlab.com/graphviz/graphviz/-/issues/2092
  """
  p = subprocess.run(["dot2gxl", "-d"], input='<node id="">',
                     universal_newlines=True)

  assert p.returncode != 0, "dot2gxl accepted invalid input"

  assert p.returncode == 1, "dot2gxl crashed"

@pytest.mark.skipif(which("dot2gxl") is None, reason="dot2gxl not available")
def test_2093():
  """
  dot2gxl should handle elements with no ID
  https://gitlab.com/graphviz/graphviz/-/issues/2093
  """
  with subprocess.Popen(["dot2gxl", "-d"], stdin=subprocess.PIPE,
                        universal_newlines=True) as p:
    p.communicate('<graph x="">')

    assert p.returncode == 1, "dot2gxl did not reject missing ID"

@pytest.mark.skipif(os.environ.get("build_system") == "msbuild" and
                    os.environ.get("configuration") == "Debug",
                    reason="Graphviz built with MSBuild in Debug mode has an "
                           "insufficient stack size for this test")
def test_2095():
  """
  Exceeding 1000 boxes during computation should not cause a crash
  https://gitlab.com/graphviz/graphviz/-/issues/2095
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "2095.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it
  dot("pdf", input)

@pytest.mark.skipif(which("gv2gml") is None, reason="gv2gml not available")
def test_2131():
  """
  gv2gml should be able to process basic Graphviz input
  https://gitlab.com/graphviz/graphviz/-/issues/2131
  """

  # a trivial graph
  input = "digraph { a -> b; }"

  # ask gv2gml what it thinks of this
  try:
    subprocess.run(["gv2gml"], input=input, check=True,
                   universal_newlines=True)
  except subprocess.CalledProcessError as e:
    raise RuntimeError("gv2gml rejected a basic graph") from e

@pytest.mark.skipif(which("gvpr") is None, reason="gvpr not available")
@pytest.mark.parametrize("examine", ("indices", "tokens"))
def test_2138(examine: str):
  """
  gvpr splitting and tokenizing should not result in trailing garbage
  https://gitlab.com/graphviz/graphviz/-/issues/2138
  """

  # find our co-located GVPR program
  script = (Path(__file__).parent / "2138.gvpr").resolve()
  assert script.exists(), "missing test case"

  # run it with NUL input
  out = subprocess.check_output(["gvpr", "-f", script],
                                stdin=subprocess.DEVNULL)

  # Decode into text. We do this instead of `universal_newlines=True` above
  # because the trailing garbage can contain invalid UTF-8 data causing cryptic
  # failures. We want to correctly surface this as trailing garbage, not an
  # obscure UTF-8 decoding error.
  result = out.decode("utf-8", "replace")

  if examine == "indices":
    # check no indices are miscalculated
    index_re = r"^// index of space \(st\) :\s*(?P<index>-?\d+)\s*<< must " \
               r"NOT be less than -1$"
    for m in re.finditer(index_re, result, flags=re.MULTILINE):
      index = int(m.group("index"))
      assert index >= -1, "illegal index computed"

  if examine == "tokens":
    # check for text the author of 2138.gvpr expected to find
    assert "// tok[3]    >>3456789<<   should NOT include trailing spaces or " \
      "junk chars" in result, "token 3456789 not found or has trailing garbage"
    assert "// tok[7]    >>012<<   should NOT include trailing spaces or "     \
      "junk chars" in result, "token 012 not found or has trailing garbage"

def test_2179():
  """
  processing a label with an empty line should not yield a warning
  https://gitlab.com/graphviz/graphviz/-/issues/2179
  """

  # a graph containing a label with an empty line
  input = 'digraph "" {\n' \
          '  0 -> 1 [fontname="Lato",label=<<br/>1>]\n' \
          '}'

  # run a graph with an empty label through Graphviz
  with subprocess.Popen(["dot", "-Tsvg", "-o", os.devnull],
                        stdin=subprocess.PIPE, stderr=subprocess.PIPE,
                        universal_newlines=True) as p:
    _, stderr = p.communicate(input)

    assert p.returncode == 0

  assert "Warning: no hard-coded metrics for" not in stderr, \
    "incorrect warning triggered"

def test_2179_1():
  """
  processing a label with a line containing only a space should not yield a
  warning
  https://gitlab.com/graphviz/graphviz/-/issues/2179
  """

  # a graph containing a label with a line containing only a space
  input = 'digraph "" {\n' \
          '  0 -> 1 [fontname="Lato",label=< <br/>1>]\n' \
          '}'

  # run a graph with an empty label through Graphviz
  with subprocess.Popen(["dot", "-Tsvg", "-o", os.devnull],
                        stdin=subprocess.PIPE, stderr=subprocess.PIPE,
                        universal_newlines=True) as p:
    _, stderr = p.communicate(input)

    assert p.returncode == 0

  assert "Warning: no hard-coded metrics for" not in stderr, \
    "incorrect warning triggered"

@pytest.mark.skipif(which("nop") is None, reason="nop not available")
def test_2184_1():
  """
  nop should not reposition labelled graph nodes
  https://gitlab.com/graphviz/graphviz/-/issues/2184
  """

  # run `nop` on a sample with a labelled graph node at the end
  source = Path(__file__).parent / "2184.dot"
  assert source.exists(), "missing test case"
  nopped = subprocess.check_output(["nop", source], universal_newlines=True)

  # the normalized output should have a graph with no label within
  # `clusterSurround1`
  m = re.search(r'\bclusterSurround1\b.*\bgraph\b.*\bcluster1\b', nopped,
                flags=re.DOTALL)
  assert m is not None, \
    "nop rearranged a graph in a not-semantically-preserving way"

def test_2184_2():
  """
  canonicalization should not reposition labelled graph nodes
  https://gitlab.com/graphviz/graphviz/-/issues/2184
  """

  # canonicalize a sample with a labelled graph node at the end
  source = Path(__file__).parent / "2184.dot"
  assert source.exists(), "missing test case"
  canonicalized = dot("canon", source)

  # the canonicalized output should have a graph with no label within
  # `clusterSurround1`
  m = re.search(r'\bclusterSurround1\b.*\bgraph\b.*\bcluster1\b', canonicalized,
                flags=re.DOTALL)
  assert m is not None, \
    "`dot -Tcanon` rearranged a graph in a not-semantically-preserving way"

def test_2185_1():
  """
  GVPR should deal with strings correctly
  https://gitlab.com/graphviz/graphviz/-/issues/2185
  """

  # find our collocated GVPR program
  script = Path(__file__).parent / "2185.gvpr"
  assert script.exists(), "missing test case"

  # run this with NUL input, checking output is valid UTF-8
  gvpr(script)

def test_2185_2():
  """
  GVPR should deal with strings correctly
  https://gitlab.com/graphviz/graphviz/-/issues/2185
  """

  # find our collocated GVPR program
  script = Path(__file__).parent / "2185.gvpr"
  assert script.exists(), "missing test case"

  # run this with NUL input
  out = subprocess.check_output(["gvpr", "-f", script],
                                stdin=subprocess.DEVNULL)

  # decode output in a separate step to gracefully cope with garbage unicode
  out = out.decode("utf-8", "replace")

  # deal with Windows eccentricities
  eol = "\r\n" if platform.system() == "Windows" else "\n"
  expected = f"one two three{eol}"

  # check the first line is as expected
  assert out.startswith(expected), "incorrect GVPR interpretation"

def test_2185_3():
  """
  GVPR should deal with strings correctly
  https://gitlab.com/graphviz/graphviz/-/issues/2185
  """

  # find our collocated GVPR program
  script = Path(__file__).parent / "2185.gvpr"
  assert script.exists(), "missing test case"

  # run this with NUL input
  out = subprocess.check_output(["gvpr", "-f", script],
                                stdin=subprocess.DEVNULL)

  # decode output in a separate step to gracefully cope with garbage unicode
  out = out.decode("utf-8", "replace")

  # deal with Windows eccentricities
  eol = "\r\n" if platform.system() == "Windows" else "\n"
  expected = f"one two three{eol}one  five three{eol}"

  # check the first two lines are as expected
  assert out.startswith(expected), "incorrect GVPR interpretation"

def test_2185_4():
  """
  GVPR should deal with strings correctly
  https://gitlab.com/graphviz/graphviz/-/issues/2185
  """

  # find our collocated GVPR program
  script = Path(__file__).parent / "2185.gvpr"
  assert script.exists(), "missing test case"

  # run this with NUL input
  out = subprocess.check_output(["gvpr", "-f", script],
                                stdin=subprocess.DEVNULL)

  # decode output in a separate step to gracefully cope with garbage unicode
  out = out.decode("utf-8", "replace")

  # deal with Windows eccentricities
  eol = "\r\n" if platform.system() == "Windows" else "\n"
  expected = f"one two three{eol}one  five three{eol}99{eol}"

  # check the first three lines are as expected
  assert out.startswith(expected), "incorrect GVPR interpretation"

def test_2185_5():
  """
  GVPR should deal with strings correctly
  https://gitlab.com/graphviz/graphviz/-/issues/2185
  """

  # find our collocated GVPR program
  script = Path(__file__).parent / "2185.gvpr"
  assert script.exists(), "missing test case"

  # run this with NUL input
  out = subprocess.check_output(["gvpr", "-f", script],
                                stdin=subprocess.DEVNULL)

  # decode output in a separate step to gracefully cope with garbage unicode
  out = out.decode("utf-8", "replace")

  # deal with Windows eccentricities
  eol = "\r\n" if platform.system() == "Windows" else "\n"
  expected = f"one two three{eol}one  five three{eol}99{eol}Constant{eol}"

  # check the first four lines are as expected
  assert out.startswith(expected), "incorrect GVPR interpretation"

@pytest.mark.xfail(strict=True) # FIXME
def test_2193():
  """
  the canonical format should be stable
  https://gitlab.com/graphviz/graphviz/-/issues/2193
  """

  # find our collocated test case
  input = Path(__file__).parent / "2193.dot"
  assert input.exists(), "unexpectedly missing test case"

  # derive the initial canonicalization
  canonical = dot("canon", input)

  # now canonicalize this again to see if it changes
  new = dot("canon", source=canonical)
  assert canonical == new, "canonical translation is not stable"

@pytest.mark.skipif(which("gvpr") is None, reason="GVPR not available")
def test_2211():
  """
  GVPR’s `index` function should return correct results
  https://gitlab.com/graphviz/graphviz/-/issues/2211
  """

  # find our collocated test case
  program = Path(__file__).parent / "2211.gvpr"
  assert program.exists(), "unexpectedly missing test case"

  # run it through GVPR
  output = gvpr(program)

  # it should have found the right string indices for characters
  assert output == "index: 9  should be 9\n" \
                   "index: 3  should be 3\n" \
                   "index: -1  should be -1\n"

def test_2215():
  """
  Graphviz should not crash with `-v`
  https://gitlab.com/graphviz/graphviz/-/issues/2215
  """

  # try it on a simple graph
  input = "graph g { a -- b; }"
  subprocess.run(["dot", "-v"], input=input, check=True,
                 universal_newlines=True)

  # try the same on a labelled version of this graph
  input = 'graph g { node[label=""] a -- b; }'
  subprocess.run(["dot", "-v"], input=input, check=True,
                 universal_newlines=True)

def test_package_version():
  """
  The graphviz_version.h header should define a non-empty PACKAGE_VERSION
  """

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1777 is fixed
  if os.getenv("build_system") == "msbuild":
    pytest.skip("Windows MSBuild release does not contain any header files (#1777)")

  # find co-located test source
  c_src = (Path(__file__).parent / "check-package-version.c").resolve()
  assert c_src.exists(), "missing test case"

  # run the test
  _, _ = run_c(c_src)

def test_user_shapes():
  """
  Graphviz should understand how to embed a custom SVG image as a node’s shape
  """

  # find our collocated test case
  input = Path(__file__).parent / "usershape.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to translate this to SVG
  output = subprocess.check_output(["dot", "-Tsvg", input],
    cwd=os.path.dirname(__file__), universal_newlines=True)

  # the external SVG should have been parsed and is now referenced
  assert '<image xlink:href="usershape.svg" width="62px" height="44px" ' in \
    output

def test_xdot_json():
  """
  check the output of xdot’s JSON API
  """

  # find our collocated C helper
  c_src = Path(__file__).parent / "xdot2json.c"

  # some valid xdot commands to process
  input = "c 9 -#fffffe00 C 7 -#ffffff P 4 0 0 0 36 54 36 54 0"

  # ask our C helper to process this
  try:
    output, err = run_c(c_src, input=input, link=["xdot"])
  except subprocess.CalledProcessError:
    # FIXME: Remove this try-catch when
    # https://gitlab.com/graphviz/graphviz/-/issues/1777 is fixed
    if os.getenv("build_system") == "msbuild":
      pytest.skip("Windows MSBuild release does not contain any header "
                  "files (#1777)")
    raise
  assert err == ""

  if os.getenv("build_system") == "msbuild":
    pytest.fail("Windows MSBuild unexpectedly passed compilation of a "
                  "Graphviz header. Remove the above try-catch? (#1777)")

  # confirm the output was what we expected
  data = json.loads(output)
  assert data == [
                  {"c" : "#fffffe00"},
                  {"C" : "#ffffff"},
                  {"P" : [0.0, 0.0, 0.0, 36.0, 54.0, 36.0, 54.0, 0.0]}
                 ]

# find all .vcxproj files
VCXPROJS: List[Path] = []
for repo_root, _, files in os.walk(ROOT):
  for stem in files:
    # skip files generated by MSBuild itself
    if stem in ("VCTargetsPath.vcxproj", "CompilerIdC.vcxproj",
                "CompilerIdCXX.vcxproj"):
      continue
    full_path = Path(repo_root) / stem
    if full_path.suffix != ".vcxproj":
      continue
    VCXPROJS.append(full_path)

@pytest.mark.parametrize("vcxproj", VCXPROJS)
def test_vcxproj_inclusive(vcxproj: Path):
  """check .vcxproj files correspond to .vcxproj.filters files"""

  def fix_sep(path: str) -> str:
    """translate Windows path separators to ease running this on non-Windows"""
    return path.replace("\\", os.sep)

  # FIXME: files missing a filters file
  FILTERS_WAIVERS = (Path("lib/version/version.vcxproj"),)

  filters = Path(f"{str(vcxproj)}.filters")
  if vcxproj.relative_to(ROOT) not in FILTERS_WAIVERS:
    assert filters.exists(), \
      f"no {str(filters)} corresponding to {str(vcxproj)}"

  # namespace the MSBuild elements live in
  ns = "http://schemas.microsoft.com/developer/msbuild/2003"

  # parse XML out of the .vcxproj file
  srcs1 = set()
  tree = ET.parse(vcxproj)
  root = tree.getroot()
  for elem in root:
    if elem.tag == f"{{{ns}}}ItemGroup":
      for child in elem:
        if child.tag in (f"{{{ns}}}ClInclude", f"{{{ns}}}ClCompile"):
          filename = fix_sep(child.attrib["Include"])
          assert filename not in srcs1, \
            f"duplicate source {filename} in {str(vcxproj)}"
          srcs1.add(filename)

  # parse XML out of the .vcxproj.filters file
  if filters.exists():
    srcs2 = set()
    tree = ET.parse(filters)
    root = tree.getroot()
    for elem in root:
      if elem.tag == f"{{{ns}}}ItemGroup":
        for child in elem:
          if child.tag in (f"{{{ns}}}ClInclude", f"{{{ns}}}ClCompile"):
            filename = fix_sep(child.attrib["Include"])
            assert filename not in srcs2, \
              f"duplicate source {filename} in {str(filters)}"
            srcs2.add(filename)

    assert srcs1 == srcs2, \
      "mismatch between sources in {str(vcxproj)} and {str(filters)}"

@pytest.mark.xfail() # FIXME: fails on CentOS 7/8, macOS Autotools, MSBuild
@pytest.mark.skipif(which("gvmap") is None, reason="gvmap not available")
def test_gvmap_fclose():
  """
  gvmap should not attempt to fclose(NULL). This example will trigger a crash if
  this bug has been reintroduced and Graphviz is built with ASan support.
  """

  # a reasonable input graph
  input = 'graph "Alík: Na vlastní oči" {\n'                                  \
          '	graph [bb="0,0,128.9,36",\n'                                      \
          '		concentrate=true,\n'                                            \
          '		overlap=prism,\n'                                               \
          '		start=3\n'                                                      \
          '	];\n'                                                             \
          '	node [label="\\N"];\n'                                            \
          '	{\n'                                                              \
          '		bob	[height=0.5,\n'                                             \
          '			pos="100.95,18",\n'                                           \
          '			width=0.77632];\n'                                            \
          '	}\n'                                                              \
          '	{\n'                                                              \
          '		alice	[height=0.5,\n'                                           \
          '			pos="32.497,18",\n'                                           \
          '			width=0.9027];\n'                                             \
          '	}\n'                                                              \
          '	alice -- bob	[pos="65.119,18 67.736,18 70.366,18 72.946,18"];\n' \
          '	bob -- alice;\n'                                                  \
          '}'

  # pass this through gvmap
  subprocess.run(["gvmap"], input=input.encode("utf-8"), check=True)

@pytest.mark.skipif(which("gvpr") is None, reason="gvpr not available")
def test_gvpr_usage():
  """
  gvpr usage information should be included when erroring on a malformed command
  """

  # create a temporary directory, under which we know no files will exist
  with tempfile.TemporaryDirectory() as tmp:

    # ask GVPR to process a non-existent file
    with subprocess.Popen(["gvpr", "-f", "nofile"], stderr=subprocess.PIPE,
                          cwd=tmp, universal_newlines=True) as p:
      _, stderr = p.communicate()

      assert p.returncode != 0, "GVPR accepted a non-existent file"

  # the stderr output should have contained full usage instructions
  assert "-o <ofile> - write output to <ofile>; stdout by default" in stderr, \
    "truncated or malformed GVPR usage information"

def test_2225():
  """
  sfdp should not segfault with curved splines
  https://gitlab.com/graphviz/graphviz/-/issues/2225
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "2225.dot"
  assert input.exists(), "unexpectedly missing test case"

  # run this through sfdp
  p = subprocess.run(["sfdp", "-Gsplines=curved", "-o", os.devnull, input],
                     stderr=subprocess.PIPE, universal_newlines=True)

  # if sfdp was built without libgts, it will not handle anything non-trivial
  no_gts_error = "remove_overlap: Graphviz not built with triangulation library"
  if no_gts_error in p.stderr:
    assert p.returncode != 0, "sfdp returned success after an error message"
    return

  p.check_returncode()

def test_2257():
  """
  `$GV_FILE_PATH` being set should prevent Graphviz from running

  `$GV_FILE_PATH` was an environment variable formerly used to implement a file
  system sandboxing policy when Graphviz was exposed to the internet via a web
  server. These days, there are safer and more robust techniques to sandbox
  Graphviz and so `$GV_FILE_PATH` usage has been removed. But if someone
  attempts to use this legacy mechanism, we do not want Graphviz to
  “fail-open,” starting anyway and silently ignoring `$GV_FILE_PATH` giving
  the user the false impression the sandboxing is in force.

  https://gitlab.com/graphviz/graphviz/-/issues/2257
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "2257.dot"
  assert input.exists(), "unexpectedly missing test case"

  env = os.environ.copy()
  env["GV_FILE_PATH"] = "/tmp"

  # Graphviz should refuse to process an input file
  with pytest.raises(subprocess.CalledProcessError):
    subprocess.check_call(["dot", "-Tsvg", input, "-o", os.devnull], env=env)

def test_2258():
  """
  'id' attribute should be propagated to all graph children in output
  https://gitlab.com/graphviz/graphviz/-/issues/2258
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "2258.dot"
  assert input.exists(), "unexpectedly missing test case"

  # translate this to SVG
  svg = dot("svg", input)

  # load this as XML
  root = ET.fromstring(svg)

  # the output is expected to contain a number of linear gradients, all of which
  # are semantic children of graph marked `id = "G2"`
  gradients = root.findall(".//{http://www.w3.org/2000/svg}linearGradient")
  assert len(gradients) > 0, "no gradients in output"

  for gradient in gradients:
    assert "G2" in gradient.get("id"), "ID was not applied to linear gradients"

def test_2270(tmp_path: Path):
  """
  `-O` should result in the expected output filename
  https://gitlab.com/graphviz/graphviz/-/issues/2270
  """

  # write a simple graph
  input = tmp_path / "hello.gv"
  input.write_text("digraph { hello -> world }", encoding="utf-8")

  # process it with Graphviz
  subprocess.check_call(["dot", "-T", "plain:dot:core", "-O", "hello.gv"],
                        cwd=tmp_path)

  # it should have produced output in the expected location
  output = tmp_path / "hello.gv.core.dot.plain"
  assert output.exists(), "-O resulted in an unexpected output filename"

def test_2272():
  """
  using `agmemread` with an unterminated string should not fail assertions
  https://gitlab.com/graphviz/graphviz/-/issues/2272
  """

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1777 is fixed
  if os.getenv("build_system") == "msbuild":
    pytest.skip("Windows MSBuild release does not contain any header files (#1777)")

  # find co-located test source
  c_src = (Path(__file__).parent / "2272.c").resolve()
  assert c_src.exists(), "missing test case"

  # run the test
  run_c(c_src, link=["cgraph", "gvc"])

def test_2272_2():
  """
  An unterminated string in the source should not crash Graphviz. Variant of
  `test_2272`.
  """

  # a graph with an open string
  graph = 'graph { a[label="abc'

  # process it with Graphviz, which should not crash
  p = subprocess.run(["dot", "-o", os.devnull], input=graph,
                     universal_newlines=True)
  assert p.returncode != 0, "dot accepted invalid input"
  assert p.returncode == 1, "dot crashed"

def test_2282():
  """
  using the `fdp` layout with JSON output should result in valid JSON
  https://gitlab.com/graphviz/graphviz/-/issues/2282
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "2282.dot"
  assert input.exists(), "unexpectedly missing test case"

  # translate this to JSON
  output = dot("json", input)

  # confirm this is valid JSON
  json.loads(output)

@pytest.mark.skipif(which("gxl2gv") is None, reason="gxl2gv not available")
@pytest.mark.xfail()
def test_2300_1():
  """
  translating GXL with an attribute `name` should not crash
  https://gitlab.com/graphviz/graphviz/-/issues/2300
  """

  # locate our associated test case containing a node attribute `name`
  input = Path(__file__).parent / "2300.gxl"
  assert input.exists(), "unexpectedly missing test case"

  # ask `gxl2gv` to process this
  subprocess.check_call(["gxl2gv", input])

def test_2307():
  """
  'id' attribute should be propagated to 'url' links in SVG output
  https://gitlab.com/graphviz/graphviz/-/issues/2307
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "2258.dot"
  assert input.exists(), "unexpectedly missing test case"

  # translate this to SVG
  svg = dot("svg", input)

  # load this as XML
  root = ET.fromstring(svg)

  # the output is expected to contain a number of polygons, any of which have
  # `url` fills should include the ID “G2”
  polygons = root.findall(".//{http://www.w3.org/2000/svg}polygon")
  assert len(polygons) > 0, "no polygons in output"

  for polygon in polygons:
    m = re.match(r"url\((?P<url>.*)\)$", polygon.get("fill"))
    if m is None:
      continue
    assert re.search(r"\bG2_", m.group("url")) is not None, \
      "ID G2 was not applied to polygon fill url"

def test_2325():
  """
  using more than 63 styles and/or more than 128 style bytes should not trigger
  an out-of-bounds memory read
  https://gitlab.com/graphviz/graphviz/-/issues/2325
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "2325.dot"
  assert input.exists(), "unexpectedly missing test case"

  # run it through Graphviz
  dot("svg", input)

def test_changelog_dates():
  """
  Check the dates of releases in the changelog are correctly formatted
  """
  changelog = Path(__file__).parent / "../CHANGELOG.md"
  with open(changelog, "rt", encoding="utf-8") as f:
    for lineno, line in enumerate(f, 1):
      m = re.match(r"## \[\d+\.\d+\.\d+\] [-–] (?P<date>.*)$", line)
      if m is None:
        continue
      d = re.match(r"\d{4}-\d{2}-\d{2}", m.group("date"))
      assert d is not None, \
        f"CHANGELOG.md:{lineno}: date in incorrect format: {line}"
