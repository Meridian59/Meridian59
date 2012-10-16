DirectShow Sample -- PlayDMO
----------------------------

PlayDMO allows you to open any media file, view its video component (if present), and
apply any number of DMO audio effects to its audio component.  

Select a media file by typing its name in the edit box or by clicking the 
Browse For Media button.  Then select one or more audio effects DMOs to apply to 
the audio.  Click Apply to immediately rebuild the filter graph with the selected DMOs.
You may also click Play instead, and any selected DMOs will be added to the newly built
filter graph.

Once you have started playing the media file, you can click on a filter or DMO in the
Filters/DMO listbox and edit its properties by clicking the Filter Properties button.
The corresponding filter properties dialog will be displayed.  If you click on a DMO in 
the Filters/DMO list, you'll also see extra DMO-specific information provided by the IMediaObject interface.

Note: Since the filter graph is rebuilt when you click Play, don't set Filter Properties
until the graph is already playing.  Otherwise, you will lose your settings when you
start playing the media file.
