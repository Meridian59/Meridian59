This is the README file for PHPlot
Last updated for PHPlot-6.1.0 on 2013-05-11
The project web site is http://sourceforge.net/projects/phplot/
The project home page is http://phplot.sourceforge.net/
-----------------------------------------------------------------------------

OVERVIEW:

PHPlot is a PHP class for creating scientific and business charts.

The release documentation contains only summary information. For more
complete information, download the PHPlot Reference Manual from the
Sourceforge project web site. You can also view the manual online at
http://phplot.sourceforge.net

For information about changes in this release, including any possible
incompatibilities, see the NEWS.txt file.


CONTENTS:

   COPYING  . . . . . . . . . . . . LGPL 2.1 License file
   ChangeLog  . . . . . . . . . . . Lists changes to the sources
   NEWS.txt . . . . . . . . . . . . Highlights changes in releases
   NEWS_*.txt . . . . . . . . . . . Older NEWS files
   README.txt   . . . . . . . . . . This file
   contrib  . . . . . . . . . . . . "Contributed" directory, add-ons
   phplot.php   . . . . . . . . . . The main PHPlot source file
   rgb.inc.php  . . . . . . . . . . Optional extended color table


REQUIREMENTS:

You need a recent version of PHP5, and you are advised to use the latest
stable release.  This version of PHPlot was tested with PHP-5.4.15 and
PHP-5.3.25 on Linux, and with PHP-5.4.15 on Windows XP.

You need the GD extension to PHP either built in to PHP or loaded as a
module. Refer to the PHP documentation for more information - see the
Image Functions chapter in the PHP Manual. We test PHPlot mostly with the
PHP-supported, bundled GD library.

If you want to display PHPlot charts on a web site, you need a PHP-enabled
web server. You can also use the PHP CLI interface without a web server.

PHPlot supports TrueType fonts, but does not include any TrueType font files.
If you want to use TrueType fonts on your plots, you need to have TrueType
support in GD, and some TrueType font files. (Your operating system most
likely includes TrueType fonts.) By default, PHPlot uses a simple font which
is built in to the GD library.


INSTALLATION:

Unpack the distribution. (If you are reading this file, you have probably
already done that.)

Installation of PHPlot simply involves copying two script files somewhere
your PHP application scripts will be able to find them. The scripts are:
     phplot.php   - The main script file
     rgb.inc.php  - Optional large color table
Make sure the permissions on these files allow the web server to read them.

The ideal place is a directory outside your web server document area,
and on your PHP include path. You can add to the include path in the PHP
configuration file; consult the PHP manual for details.


UPGRADING:

To upgrade PHPlot, follow the same instructions as for installing. There
may be changes between releases which can alter the appearance of your plots.
Please check the top section in NEWS.txt for details.


KNOWN ISSUES:

Here are some of the problems we know about in PHPlot. See the bug tracker
on the PHPlot project web site for more information.

#146 (was: 3142124) Clip plot elements to plot area
  Plot elements are not currently clipped to the plot area, and may extend
  beyond. PHP does not currently support the GD clipping control.

#93 (was: 1605558) Wide/Custom dashed lines don't work well
  There is a question whether this is a PHP/GD issue, or behaving as
  expected.


PHP Issues:

PHP has many build-time and configuration options, and these can affect
the operation of PHPlot (as well as any other application or library). Here
are some known issues:

  + If PHP is built with Thread Safety on (ZTS), the GD extension is
unable to find TrueType fonts unless the full path is given. (This is
probably a PHP bug.) The text in the PHPlot Reference Manual section
"TrueType Font Selection" concerning finding fonts without a path does not
work on these systems. This has been found to affect the version of PHP
distributed with Slackware, but not Ubuntu. It affects the thread-safe PHP
Windows release, but not the non-thread-safe (NTS) Windows release.

  + Slackware Linux includes a version of PHP built with --enable-gd-jis-conv
(JIS-mapped Japanese font support). This prevents the usual UTF-8 encoding
of characters from working in TrueType Font (TTF) text strings.

  + The Ubuntu Linux PHP GD package (php5-gd) was built to use the external
shared GD library, not the one bundled with PHP. This can result in small
differences in images, and some unsupported features (such as advanced
truecolor image operations). Also, although this Ubuntu GD library was
built with fontconfig support, PHP does not use it, so you still need to
specify TrueType fonts with their actual file names. These also affect
Ubuntu-derived distributions such as Linux Mint.

  + Some PHP installations may have a memory limit set too low to support
large images, especially truecolor images.

  + PHP-5.3.2 has a bug in rendering TrueType fonts (TTF).  Avoid using this
version if you use TTF text in PHPlot.

  + The default TrueType font can be changed by external factors. If you
enable TrueType fonts, but don't select a font name, PHPlot can find a
default TrueType font on many systems, including Windows. However, the
selected font can change if you install or remove software. For example, on
Windows, PHPlot may select the Arial font. But then, if you install
LibreOffice, the default font will change to Liberation Sans. This will
change plot appearance.  This happens because LibreOffice installs the
Liberation font family in the Windows fonts directory, and the Liberation
Sans font precedes Arial in PHPlot's list of candidate default fonts.



If you think you found a problem with PHPlot, or want to ask questions or
provide feedback, please use the Help and Discussion forum at
     http://sourceforge.net/projects/phplot/
If you are sure you have found a bug, you can report it on the Bug tracker
at the same web site. There is also a Features Request tracker.


TESTING:

You can test your installation by creating the following two files somewhere
in your web document area. First, the HTML file:

------------ simpleplot.html ----------------------------
<html>
<head>
<title>Hello, PHPlot!</title>
</head>
<body>
<h1>PHPlot Test</h1>
<img src="simpleplot.php">
</body>
</html>
---------------------------------------------------------

Second, in the same directory, the image file producing PHP script file.
Depending on where you installed phplot.php, you may need to specify a path
in the 'require' line below.

------------ simpleplot.php -----------------------------
<?php
require 'phplot.php';
$plot = new PHPlot();
$data = array(array('', 0, 0), array('', 1, 9));
$plot->SetDataValues($data);
$plot->SetDataType('data-data');
$plot->DrawGraph();
---------------------------------------------------------

Access the URL to 'simpleplot.html' in your web browser. If you see a
simple graph, you have successfully installed PHPlot. If you see no
graph, check your web server error log for more information.


COPYRIGHT and LICENSE:

PHPlot is Copyright (C) 1998-2013 Afan Ottenheimer

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation;
version 2.1 of the License.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
