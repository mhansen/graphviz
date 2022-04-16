## Process this file with automake to produce Makefile.in

SUBDIRS = info schema

pdf = dotguide.pdf neatoguide.pdf libgraph/Agraph.pdf libguide/libguide.pdf
html = build.html FAQ.html index.html winbuild.html pspdf.png char.html gdtclft.entities.example.png todo.html internal_todo.html

if WITH_SMYRNA
pdf += smyrna/smyrna.pdf
endif

pdf_DATA = $(pdf)
html_DATA = $(html)
dist_doc_DATA = Dot.ref latex_suggestions.txt fontfaq.txt addingLayout.txt

EXTRA_DIST = $(pdf) $(html) schema/*.xml schema/*.xslt infosrc/*
