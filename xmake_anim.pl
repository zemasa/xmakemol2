#! /usr/bin/perl -w

#  Copyright (C) 1999, 2000, 2002, 2003, 2005 Matthew P. Hodges
#  This file is part of XMakemol.

#  XMakemol is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2, or (at your option)
#  any later version.

#  XMakemol is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License
#  along with XMakemol; see the file COPYING. If not, write to the
#  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#  MA 02111-1307 USA

# Script to make an animated gif from a series of .xpm files produced
# by XMakemol. Uses the `convert' and `gifsicle' programs.

# 	$rcs = ' $Id: xmake_anim.pl,v 1.23 2007/08/25 11:25:41 MPHodges Exp $ ' ;

use Getopt::Std;

# Configure: specify where the binaries are

$convert = "/usr/bin/convert";
if (! -e $convert)
  {
    die ("Cannot find convert program (part of ImageMagick). Please install it.\n");
  }

$gifsicle = "/usr/bin/gifsicle";
if (! -e $gifsicle)
  {
    print ("Cannot find gifsicle program, install it for better performance.\n");
    $gifsicle = "0";
  }

# Set the default options

$opt_c = 0;                     # Clean up *.gif *.xpm files
$opt_d = 0;                     # The delay in 1/100th seconds

if ($gifsicle)
  {
    $opt_l = "forever";         # The number of loops
  }
else
  {
    $opt_l = 0;
  }

getopts ("cd:l:o:");

$0 =~ s/.*\///;                 # Basename

if ($#ARGV != 0)
  {
    print ("Usage: $0 [options] prefix\n");
    print ("       -c                (clean up files)\n");
    print ("       -d <delay>        (in 1/100th seconds)\n");
    print ("       -l <no_loops>     (0 for infinite)\n");
    print ("       -o <output>\n");
    die ();
  }

$root = $ARGV[0];

# Protect against some regexp constructs

$root =~ s/\./\\./g;
$root =~ s/\+/\\+/g;

if (defined ($opt_o))
  {
    $output = $opt_o;
  }
else
  {
    $output = $root . ".gif";
  }

opendir (DIR, ".");
@xpm_files =
  sort by_frame_number (grep (/$root\.[0-9]+\.xpm$/, readdir (DIR)));

if ($#xpm_files == -1)
  {
    die ("No files matching $root*.xpm\n");
  }

# Convert each xpm file into a gif file

if ($gifsicle)
  {
    print ("Converting files from xpm to gif ");

    foreach $xpm_file (@xpm_files)
      {
        $gif_file = $xpm_file;
        $gif_file =~ s/xpm$/gif/;

        # Don't run if $gif_file exists and is newer than $xpm_file

        if ((-e $gif_file) &&
            ((-M $gif_file) < (-M $xpm_file)))
          {
            print ("$gif_file newer than $xpm_file -- skipping\n");
          }
        else
          {
            print ("\n   $xpm_file -> $gif_file\n");
            `$convert $xpm_file $gif_file`;
          }

        push (@gif_files, $gif_file);
      }

    # Run gifsicle

    $command = "$gifsicle --delay $opt_d --loopcount=$opt_l @gif_files > $output";
    print ("\nRunning gifsicle --- $command\n\n");
    `$command`;
  }
else
  {
    # Use convert, if gifsicle is not available
    $command = "$convert -delay $opt_d -loop $opt_l @xpm_files $output";
    print ("\nRunning convert --- $command\n\n");
    `$command`;
  }

print ("\n$output written\n");

# Clean up files

if ($opt_c)
  {
    print ("\nCleaning up *.xpm and *.gif files\n");

    if (defined (@gif_files))
      {
        unlink (@gif_files);
      }

    unlink (@xpm_files);
  }

sub by_frame_number
  {
    $frame1 = $a;
    $frame2 = $b;

    # Remove all but the number frame the file name

    $frame1 =~ s/.*\.(\d+)\.xpm$/$1/;
    $frame2 =~ s/.*\.(\d+)\.xpm$/$1/;

    $frame1 <=> $frame2;
  }
