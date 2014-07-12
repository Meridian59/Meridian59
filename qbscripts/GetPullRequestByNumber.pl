#GetPullRequestByNumber.pl
#Creates a branch for a specific pull request, overwrites existing branchess if prompted

#use strict;
use warnings;
use LWP::Simple;
use Getopt::Long;
use Pithub;

my $overwrite = 1;

GetOptions ("pullrequest=s" => \$pullrequest,
            "overwrite=s" => \$overwrite);

my $p = Pithub::PullRequests->new(prepare_request => sub 
													 {
													   my ($request) = @_;
													   $request->header( Authorization => 'token 27cd8d4bdcfabc6a330b2275761a38c92b194848' );
													 }
								 );

my $pullinfo = $p->get(user => 'Daenks',
					   #token => '27cd8d4bdcfabc6a330b2275761a38c92b194848',
					   repo => 'Meridian59_103',
					   pull_request_id => $pullrequest,);
	
unless ( $pullinfo->success ) 
{
  printf  "GetPullRequestByNumber:Unable to make GitHub API Call: %s\n", $pullinfo->response->status_line;
  exit 1;
}
	
my $cloneurl = $pullinfo->content->{head}->{repo}->{clone_url};
my $branch = $pullinfo->content->{head}->{ref};
my $author = $pullinfo->content->{head}->{user}->{login};
my $localbranch = "$author.$branch.$pullrequest";

printf "GetPullRequestByNumber: Getting: (Pull: %d Branch: %s Author: %s) as $localbranch\n", $pullrequest, $branch, $author;
$output =`git checkout -b $localbranch master 2>&1`;
if ($output !~ /Switched to a new branch/) #we didn't switch
{
	if ($overwrite)
	{
		print "GetPullRequestByNumber: Deleting existing $localbranch and re-aquiring\n";
		$output = `git checkout master`;
		$output = `git branch -D $localbranch`;
		$output = `git checkout -b $localbranch master 2>&1`;
	}
	else
	{
		die "GetPullRequestByNumber:Couldn't create branch: $output"; 
	}
}

$output = `git pull $cloneurl $branch`;

print "GetPullRequestByNumber:Completed download of $localbranch\n";