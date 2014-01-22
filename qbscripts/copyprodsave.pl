use File::Copy;

my $proddir = "C:/Users/Administrator/Desktop/server";
my $testdir = "C:/Users/Administrator/Desktop/server104";

print "Reading production lastsave.txt\n";
open my $lastsavetxt,"$proddir/savegame/lastsave.txt";
my @lines = <$lastsavetxt>;
close $lastsavetxt;
my @lastsavesplit = split(' ',@lines[12]);
my $lastsave = @lastsavesplit[1];

print "Copying save game $lastsave to test \n";
copy("$proddir/savegame/accounts.$lastsave","$testdir/savegame/accounts.$lastsave");
copy("$proddir/savegame/dynarscs.$lastsave","$testdir/savegame/dynarscs.$lastsave");
copy("$proddir/savegame/gameuser.$lastsave","$testdir/savegame/gameuser.$lastsave");
copy("$proddir/savegame/striings.$lastsave","$testdir/savegame/striings.$lastsave");
copy("$proddir/savegame/lastsave.txt","$testdir/savegame/lastsave.txt");