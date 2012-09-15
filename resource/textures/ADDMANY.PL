# Prompts for many texture updates and creates a bat file
# to process them.


print "\nBatch up some texture adds...\n";
print "Your current directory is: "; print `cd`;

PROMPT_MAKE:

print "Enter the build file name: ";
chop($mak = <STDIN>);
die if ($mak eq "" or $mak eq "\\");
unless (-e $mak)
{
        print "<$mak> does not exist.  Try again.\n";
        goto PROMPT_MAKE;
}

my $item_cnt = 0;
my @names;
my @descs;
while (1)
{
        my $item_num;
        $item_num = $item_cnt + 1;
        print "Enter a BGF file name ($item_num) (ENTER=done, \\=backup): ";        
        chop($bgf = <STDIN>);
        if ($bgf eq "")
        {
               print "\n\n";
               printf "%-7s %s\n", "BGFfile", "DESCRIPTION";
               for ($i=0;$i<=$item_cnt;$i++)
                {
                        printf "%-7s %s\n",@names[$i], @descs[$i];
                }
                print "If this is OK, enter YES: ";
                chop($ans = <STDIN>);
                $ans =~ tr/[a-z/[A-Z]/;
                next if ($ans ne "YES");
                last;
        }
        elsif ($bgf eq "\\")
        {
                goto PROMPT_MAKE if ($item_cnt == 0);
                $item_cnt -= 1;
                printf "%-7s %s\n",@names[$item_cnt], @descs[$item_cnt];
                next;
        }
        else
        {
                $bgf =~ tr/[A-Z]/[a-z]/;
                unless ($bgf =~ /^[0-9][0-9][0-9][0-9][0-9](w|f)[0-9]$/)
                {
                        print "?Invalid BGF format <$bgf>\n\n";
                        next;
                }
                $dup = 0;
                for ($i=0;$i<$item_cnt;$i++)
                {
                        if ($bgf eq @names[$i])
                        {
                                my $tmpi;
                                $tmpi = $i + 1;
                                print "?DUPLICATE: <$bgf> was entered as item $tmpi\n\n";
                                $dup = 1;
                                last;
                        }
                }
                next if $dup;
                print "Enter description ($item_num) (\\=backup): ";        
                chop($desc = <STDIN>);
                next if ($desc eq "\\");        # just go 'round again...
        }
        @names[$item_cnt] = $bgf;
        @descs[$item_cnt] = $desc;
        $item_cnt++;
        print "\n";
 }

PROMPT_BAT:
print "Enter a BAT file name: ";        
chop($ans = <STDIN>);
if (-e $ans)
{
        print "The file <$ans> already exists\n";
        goto PROMPT_BAT;
}

unless (open (OUTFILE,">$ans"))
{
        print "Can't open the file $ans for output: $!\n";
        goto PROMPT_BAT;
}
for ($i=0;$i<$item_cnt;$i++)
{
        printf OUTFILE "perl at.pl $mak @names[$i] \"@descs[$i]\"\n";
}
for ($i=0;$i<$item_cnt;$i++)
{
        $grdname = @names[$i];
        $grdname =~ s/([0-9]+).*/$1/;
        $grdname = "grd".$grdname.".bgf";
        printf OUTFILE "call build $grdname\n";
}
close(OUTFILE);

exit (1);

