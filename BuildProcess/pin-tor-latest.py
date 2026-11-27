#!/usr/bin/env python3
"""pin-tor-latest.py — rewrite the tor expert bundle source in flatpak
manifests to the latest stable release on dist.torproject.org.

  BuildProcess/pin-tor-latest.py <manifest.yaml> [<manifest.yaml>...]

dist.torproject.org rotates old versions off the server within weeks
of a new stable release, so a hardcoded manifest pin eventually 404s
(the v1.4.3 CI windows legs died to this).  This rewrites the tor
module's url+sha256 to the newest stable X.Y.Z — alpha directories
(16.0a*) never match.  The committed pin stays the reproducible
baseline for local builds; CI rewrites at packaging time.  Exits
nonzero when resolution fails or a manifest carries no tor block.
"""
import re
import sys
import urllib.request

INDEX = "https://dist.torproject.org/torbrowser/"
ARTIFACT = "tor-expert-bundle-linux-x86_64-{v}.tar.gz"
STABLE = re.compile(r'href="(\d+\.\d+\.\d+)/"')


def fetch(url):
    with urllib.request.urlopen(url, timeout=30) as r:
        return r.read().decode()


def latest_stable():
    versions = set(STABLE.findall(fetch(INDEX)))
    if not versions:
        sys.exit("no stable tor browser release found on dist.torproject.org")
    return max(versions, key=lambda v: [int(x) for x in v.split(".")])


def rewrite(path, version, sha):
    lines = open(path).read().splitlines(keepends=True)
    out, pending, hits = [], False, 0
    for ln in lines:
        if "url:" in ln and "tor-expert-bundle-linux" in ln:
            indent = ln[: len(ln) - len(ln.lstrip())]
            url = f"https://dist.torproject.org/torbrowser/{version}/{ARTIFACT.format(v=version)}"
            ln = f"{indent}url: {url}\n"
            pending = True
            hits += 1
        elif pending and re.match(r"\s*sha256: [0-9a-f]{64}\s*$", ln):
            indent = ln[: len(ln) - len(ln.lstrip())]
            ln = f"{indent}sha256: {sha}\n"
            pending = False
        out.append(ln)
    if pending:
        sys.exit(f"{path}: tor url has no adjacent sha256 line — layout changed?")
    if not hits:
        sys.exit(f"{path}: no tor-expert-bundle source found")
    open(path, "w").write("".join(out))
    print(f"{path}: tor expert bundle -> {version} ({sha[:16]}...)")


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    version = latest_stable()
    sums = fetch(f"https://dist.torproject.org/torbrowser/{version}/sha256sums-unsigned-build.txt")
    artifact = ARTIFACT.format(v=version)
    sha = next((l.split()[0] for l in sums.splitlines() if l.endswith(f" {artifact}")), None)
    if not sha:
        sys.exit(f"no sha256 entry for {artifact}")
    for path in sys.argv[1:]:
        rewrite(path, version, sha)


if __name__ == "__main__":
    main()
