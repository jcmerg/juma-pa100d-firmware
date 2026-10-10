#!/usr/bin/env python3
# Builds the PDF manuals from manual-en.md and manual-de.md (pandoc + weasyprint).
# Usage: python3 docs/manual/build-manual.py
import os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
DOCS = [("manual-en.md", "JUMA PA-100D Operating Manual v5.00.pdf", "en", "JUMA PA-100D Operating Manual · Firmware v5.00"),
        ("manual-de.md", "JUMA PA-100D Bedienungsanleitung v5.00.pdf", "de", "JUMA PA-100D Bedienungsanleitung · Firmware v5.00")]

def run(args, inp=None):
    return subprocess.run(args, input=inp, capture_output=True, text=True, check=True, cwd=HERE).stdout

for src, pdf, lang, title in DOCS:
    path = os.path.join(HERE, src)
    if not os.path.exists(path):
        print("skip", src); continue
    md = open(path, encoding="utf-8").read()
    opts = ["pandoc", "-f", "gfm+attributes", "--toc", "--toc-depth=2", "-t", "html5"]
    body = run(opts, md)
    toc = run(opts + ["-s", "--template", "toc.tpl"], md)
    body = body.replace("<p>[[TOC]]</p>", toc)
    html = f"""<!doctype html><html lang="{lang}"><head><meta charset="utf-8"><title>{title}</title>
<link rel="stylesheet" href="style.css"></head><body><div class="doctitle">{title}</div>{body}</body></html>"""
    tmp = os.path.join(HERE, src.replace(".md", ".html"))
    open(tmp, "w", encoding="utf-8").write(html)
    subprocess.run(["weasyprint", tmp, os.path.join(HERE, pdf)], check=True, cwd=HERE)
    os.remove(tmp)
    print("OK:", pdf)
