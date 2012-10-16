ASYNC filter README
--------------------

This sample set contains three separate projects that work together.

	Base 	- Base library with I/O functionality
	Filter	- Source for Async Filter (which uses the Base library)
	Memfile	- Sample application that uses the async filter

Since these projects build on each other, you have two options when building
an individual project:

1) Use the Async.DSW Visual C++ workspace in the Async directory.  This
	workspace provides dependency information, such that building one
	project will also build any project on which it depends.  For example,
	if you build Memfile, it will also build the Base and Filter
	directories if they are out of date or have not yet been built.


2) Build each project individually by opening its Visual C++ workspace.

   To prevent linker errors, build the projects in the following order: 
		Base	(with asynbase.dsw)
		Filter	(with asyncflt.dsw)
		MemFile (with memfile.dsw)
