# common rules
#
# to be included at the end of each makefile

makedirs:
        -@mkdir $(OUTDIR)

{$(SOURCEDIR)}.c{$(OUTDIR)}.obj::
	$(CC) $(CFLAGS) /FpCpch /Fd$(OUTDIR)\vc90.pdb /Fo$(OUTDIR)/ -c $< 

{$(SOURCEDIR)}.cpp{$(OUTDIR)}.obj::
	$(CC) $(CFLAGS) /FpCpch /Fd$(OUTDIR)\vc90.pdb /Fo$(OUTDIR)/ -c $< 

{$(OUTDIR)}.c{$(OUTDIR)}.obj::
	$(CC) $(CFLAGS) /Fd$(OUTDIR)\vc90.pdb  /Fo$(OUTDIR)/ -I$(SOURCEDIR) -c $< 

{$(SOURCEDIR)}.rc{$(OUTDIR)}.res:
        $(RC) /fo $@ /i$(SOURCEDIR) $**

clean :
        @$(RM) $(OUTDIR)\*
        @$(RMDIR) $(OUTDIR)
