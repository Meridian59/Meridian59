use File::Copy;

my $prodkod = "C:/Users/Administrator/Desktop/server/kodbase.txt";
my $buildkod = "C:/Users/Administrator/Desktop/server104/download/kodbase.txt";


print "COPYPRODKOD: Copying production kodbase.txt for download by buildserver...\n";
copy($prodkod,$buildkod);