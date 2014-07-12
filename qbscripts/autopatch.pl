use strict;
use warnings;
use LWP::Simple;
use Getopt::Long;
use Git::Repository;
use Pithub;
use Data::Dumper;
use Env;

my $outdir="C:/patchfiles";
my @changelists;
my $build;
my $buildid;
my $lastbranch = "master";
my $stacked = 0;
my $pullsobject = Pithub::PullRequests->new;
my $output;
my $update;
my $finalize = 0;
my $token = $ENV{'TOKEN'};

my @failed ;

GetOptions ("change=s" => \@changelists,
            "build=s" => \$build,
			"stacked=s" => \$stacked,
			"update=s" => \$update,
			"finalize=s" => \$finalize,
			"buildid=s" => \$buildid);
			
print "AUTOBUILD: Options: Updating pre-existing branches\n" if ($update);
print "AUTOBUILD: Options: Stacked Build ENABLED\n" if ($stacked);
print "AUTOBUILD: Options: Finalize Build ENABLED\n" if ($finalize);

if ($finalize)
{
	print "AUTOBUILD: creating $build-complete branch\n";
	$output =`git checkout -b $build-complete 2>&1`;
	if ($output !~ /Switched to a new branch/) #we didn't switch
	{
		die "AUTOBUILD: Couldn't create branch $build-complete:\n $output"; 
	}
}
			
foreach my $changelist (@changelists)
{
	my $skip = 0;
	
	my $p = Pithub::PullRequests->new(prepare_request => sub 
	                                                     {
														   my ($request) = @_;
														   $request->header( Authorization => "token $token" );
														 }
									 );

	my $pullinfo = $p->get(user => 'Daenks',
	                       repo => 'Meridian59_103',
						   pull_request_id => $changelist,);
	
	
	
	unless ( $pullinfo->success ) 
	{
      printf  "Unable to make GitHub API Call: '%s' while looking up pull $changelist\n", $pullinfo->response->status_line;
      exit 1;
	}
	
	print "********************************************************************************\n";
	printf "AUTOBUILD: Pull: %d Branch: %s Author: %s\n".
	       "AUTOBUILD: SHA: %s\n"
	       ,$pullinfo->content->{number}, $pullinfo->content->{head}->{ref}, $pullinfo->content->{head}->{user}->{login},
		   $pullinfo->content->{head}->{sha};
	print "********************************************************************************\n";
	       
	
	my $pullreq = $changelist;
	my $cloneurl = $pullinfo->content->{head}->{repo}->{clone_url};
	my $branch = $pullinfo->content->{head}->{ref};
	my $author = $pullinfo->content->{head}->{user}->{login};
	my $sha = $pullinfo->content->{head}->{sha};
	
	my $localbranch = "build.$build-req.$pullreq-br.$branch-auth.$author";
	
	if ($stacked)
	{
	  print "AUTOBUILD: Creating stacked branch $localbranch based from $lastbranch\n";
	  $output =`git checkout -b $localbranch $lastbranch 2>&1`;
	  print $output . "\n";
	}
	else
	{
	  print "AUTOBUILD: Creating branch $localbranch based from master\n";
	  $output =`git checkout -b $localbranch master 2>&1`;
	  print $output . "\n";
	}
	
	
	if ($output !~ /Switched to a new branch/) #we didn't switch
	{
		if ($output !~ /already exists./) #we dont already have this branch, oops.
		{
			die "AUTOBUILD: Couldn't create branch $localbranch:\n $output"; 
		}
		print "AUTOBUILD: Using pre-existing branch....\n";
		$output =`git checkout $localbranch master 2>&1`; #switch to existing branch
		print $output . "\n";
		$skip = 1;
	}
	
	if ($skip)
	{
		print "AUTOBUILD: Using existing code in $branch\n";
	}
	if (!$skip and $update)
	{
		print "AUTOBUILD: Downloading changes\n";
		$output = `git pull $cloneurl $branch`;
		print $output . "\n";
		if ($output =~ /Automatic merge failed/)
		{
			print "AUTOBUILD: Merge failed. Resetting $localbranch...\n";
			$output = `git reset --hard origin`;
			print $output . "\n";
			print   "AUTOBUILD: Checking out master branch...\n";
			$output = `git checkout master`;
			print $output . "\n";
			print "AUTOBUILD: Deleting failed branch....\n";
			$output = `git branch -D $localbranch`;
			print $output . "\n";
			
			print $output;
			print "**********************************FAILURE***************************************\n";
			print "Automatic Merge of $branch ($pullreq) failed.\n";
			print "At this stage, what this usually means, is that the code in the above pull was not coded against a fresh version of Meridian59_103\\master.\n";
			print "TO RESOLVE: (https://help.github.com/articles/syncing-a-fork)\n";
			print "1. Download updates to your repository....\n";
			print "  a. run: \"git remote add upstream https://github.com/Daenks/Meridian59_103.git\" to add the main repository as a link to your own.\n";
			print "  b. run: \"git fetch upstream\" to pull the changes down.\n";
			print "2. Merge updates into your master branch.....\n";
			print "  a. run \"git checkout master\" to switch to your master branch\n";
			print "  b. run \"git merge upstream/master\" to apply changes to your master branch\n";
			print "3. Re-Test your code locally and push any changes to the pull request.....\n";
			print "4. Re-Merge with this utility.\n";  
			print "********************************************************************************\n";
			push (@failed,$changelist);
			print "AUTOBUILD: Continuing merge without $pullreq...\n";
			
			
			
			my $statuses = Pithub::Repos::Statuses->new(prepare_request => sub 
	                                                     {
														   my ($request) = @_;
														   $request->header( Authorization => "token $token" );
														 }
									 );
			my $result   = $statuses->create(
				user => 'Daenks',
				repo => 'Meridian59_103',
				sha => $sha,
				data => {
					state => 'error',
					description => 'Merge failed!',
					target_url => "http://build.openmeridian.org:8810/build/$buildid"
				},
			);
			#print "AUTOBUILD: Status update result:\n" . Dumper(\$result);

		}
		else
		{
			#we update the stack of previous merges only if successful, this prevents the rest of the merges from trying against a failed merge.
			$lastbranch = $localbranch if ($stacked);
			my $statuses = Pithub::Repos::Statuses->new(prepare_request => sub 
	                                                     {
														   my ($request) = @_;
														   $request->header( Authorization => "token $token" );
														 }
									 );
			my $result   = $statuses->create(
				user => 'Daenks',
				repo => 'Meridian59_103',
				sha => $sha,
				data => {
					state => 'success',
					description => 'Merge succeded!',
					target_url => "http://build.openmeridian.org:8810/build/$buildid"
				},
			);
			#print "AUTOBUILD: Status update result:\n" . Dumper(\$result);
			print "AUTOBUILD: Success Merging $changelist\n";
		}
		
		if ($finalize)
		{
			print "AUTOBUILD: checking out $build-complete\n";
			$output = `git checkout $build-complete`;
			print $output . "\n";
			print "AUTOBUILD: merging $localbranch into $build-complete\n";
			$output = `git merge $localbranch`;
			print $output . "\n";
		}
		
		print "AUTOBUILD: Finished Processing $changelist\n";
	}
}
if ($#failed > 0)
{
	print "********************************************************************************\n";
	print "AUTOBUILD: WARNING: Failed Pulls: "  . join(",",@failed) . "\n";
	print "********************************************************************************\n";
}