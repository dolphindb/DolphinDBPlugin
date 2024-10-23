## Process this file with automake to produce Makefile.in -*-Makefile-*-
AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = README Makefile.am.in driver.h impcodes.h \
fig_defs.h post.h $(CORETERM) PostScript lua js write_png_image.c sixel.c \
latex_old.h

postscriptdir = $(pkgdatadir)/$(VERSION_MAJOR)/PostScript

luadir = $(pkgdatadir)/$(VERSION_MAJOR)/lua
jsdir = $(pkgdatadir)/$(VERSION_MAJOR)/js

# List of terminal drivers, and list of postscript installation files
# must be created prior to running automake
##trm-files-begin
CORETERM = ai.trm aquaterm.trm be.trm caca.trm cairo.trm canvas.trm \
cgm.trm context.trm corel.trm debug.trm djsvga.trm dumb.trm dxf.trm \
dxy.trm eepic.trm emf.trm emxvga.trm epson.trm estimate.trm excl.trm \
fig.trm gd.trm ggi.trm gpic.trm grass.trm hp2648.trm hp26.trm hp500c.trm \
hpgl.trm hpljii.trm hppj.trm imagen.trm kyo.trm latex.trm linux-vgagl.trm \
lua.trm metafont.trm metapost.trm mif.trm pbm.trm pc.trm pdf.trm \
pict2e.trm pm.trm post.trm pslatex.trm pstricks.trm qms.trm qt.trm \
regis.trm svg.trm t410x.trm tek.trm texdraw.trm tgif.trm tkcanvas.trm \
tpic.trm vws.trm win.trm wxt.trm x11.trm xlib.trm 
#
postscript_DATA = PostScript/8859-15.ps PostScript/8859-1.ps \
PostScript/8859-2.ps PostScript/8859-9.ps PostScript/cp1250.ps \
PostScript/cp1251.ps PostScript/cp1252.ps PostScript/cp437.ps \
PostScript/cp850.ps PostScript/cp852.ps PostScript/koi8r.ps \
PostScript/koi8u.ps PostScript/prologue.ps PostScript/utf-8.ps \
PostScript/aglfn.txt 
js_DATA = js/canvasmath.js js/canvastext.js js/gnuplot_common.js \
js/gnuplot_dashedlines.js js/gnuplot_mouse.js js/gnuplot_svg_2018.js \
js/gnuplot_svg.js js/grid.png js/help.png js/nextzoom.png \
js/previouszoom.png js/return.png js/textzoom.png js/gnuplot_mouse.css \
js/README 
lua_DATA = lua/gnuplot-tikz.lua 
##trm-files-end

Makefile.am: Makefile.am.in
	rm -f $@ $@t
	sed -n '1,/^##trm-files-begin/p' $< > $@t
	echo CORETERM = *.trm | fmt | (tr '\012' @; echo) \
	  |sed 's/@$$/%/;s/@/ \\@/g' | tr @% '\012 ' \
	  >> $@t
	echo '#' >> $@t
	echo postscript_DATA = PostScript/*.ps PostScript/*.txt | fmt \
	  | (tr '\012' @; echo) |sed 's/@$$/%/;s/@/ \\@/g' \
	  | tr @% '\012 ' >> $@t
	echo js_DATA = js/*.js js/*.png js/*.css js/README | fmt \
	  | (tr '\012' @; echo) |sed 's/@$$/%/;s/@/ \\@/g' \
	  | tr @% '\012 ' >> $@t
	echo lua_DATA = lua/gnuplot-tikz.lua | fmt \
	  | (tr '\012' @; echo) |sed 's/@$$/%/;s/@/ \\@/g' \
	  | tr @% '\012 ' >> $@t
	sed -n '/^##trm-files-end/,$$p' $< >> $@t
	chmod og-w $@t
	mv $@t $@

