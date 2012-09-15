# print out machine distribution, into a web page

require 'timelocal.pl';

main();

sub main
{
    local ($beginning_time,$i);

    $time = time;
    $beginning_time = &calc_beginning_time();

    unshift(@min_time,$time - $time % (60 * 60 * 24));
    unshift(@max_time,$min_time[0] + (60 * 60 * 24) - 1);
    for ($i=1;$min_time[$i-1] > $beginning_time;$i++)
    {
	push(@min_time,$min_time[$i-1] - 60*60*24);
	push(@max_time,$max_time[$i-1] - 60*60*24);
    }

    while (<>)
    {
	($junk, $function, $junk) =
	    /(.*) \| (\S*) (.*)/;
	
	if ($function eq "LogUserData")
	{
	    ($month,$day,$year,$time, $function, $account, $host, $os, $cpu, $ram) =
		/^(\w*)\s*(\w*)\s*(\w*)\s*(\S*)\s* \| (\S*) got (.*) from (.*), (.*), (.*), (.*) MB/;
	    if (index($account,"Kirmse") == -1 && index($account,"Sellers") == -1 && 
		index($account,"Schubert") == -1 && index($account,"cds\@cyberramp") == -1)
	    {
		#printf("%s-%s-%s-%s-%s\n",$account,$host,$os,$cpu,$ram);
		
		$time = &strings_to_time($month,$day,$year,$time),"\n";
		$index = &get_index(@min_time,@max_time,$time);
		
		# if user logged in in last week, increment loginsX array of that user
		if ($index != -1)
		{
		    eval "\$logins" . $index . "{\$account}++;";
		}
		
		$logins{$account} ++;
		if ($os =~ /95/)
		{
		    $os{$account} = $os . " (4.0)";
		}
		else
		{
		    $os{$account} = $os . " (pre-1/5)";
		}
		$cpu{$account} = $cpu;
		$ram{$account} = $ram;
	    }
	}
	if ($function eq "LogUserData/2")
	{
	    ($month,$day,$year,$time, $function, $account, $host, $os, $os_ver_major, 
	     $os_ver_minor, $cpu, $ram) =
		 /^(\w*)\s*(\w*)\s*(\w*)\s*(\S*)\s* \| (\S*) got (.*) from (.*), (.*), (.*), (.*), (.*), (.*) MB/;
	    if (index($account,"Kirmse") == -1 && index($account,"Sellers") == -1 && 
		index($account,"Schubert") == -1 && index($account,"cds\@cyberramp") == -1)
	    {
		$time = &strings_to_time($month,$day,$year,$time),"\n";
		$index = &get_index(@min_time,@max_time,$time);
		
		# if user logged in in last week, increment loginsX array of that user
		if ($index != -1)
		{
		    eval "\$logins" . $index . "{\$account}++;";
		}
		$logins{$account} ++;
		$os{$account} = $os . " (" . $os_ver_major . "." . $os_ver_minor . ")";
		$cpu{$account} = $cpu;
		$ram{$account} = $ram;
	    }
	}
	
	if ($function eq "LogUserData/3")
	{
	    ($month,$day,$year,$time, $function, $account, $host, $os, $os_ver_major, 
	     $os_ver_minor, $cpu, $ram,$screen_x,$screen_y) =
		 /^(\w*)\s*(\w*)\s*(\w*)\s*(\S*)\s* \| (\S*) got (.*) from (.*), (.*), (.*), (.*), (.*), (.*) MB, ([0-9]*)x([0-9]*)/;
	    
	    if (index($account,"Kirmse") == -1 && index($account,"Sellers") == -1 && 
		index($account,"Schubert") == -1 && index($account,"cds\@cyberramp") == -1)
	    {
		$time = &strings_to_time($month,$day,$year,$time),"\n";
		$index = &get_index(@min_time,@max_time,$time);
		
		# if user logged in in last week, increment loginsX array of that user
		if ($index != -1)
		{
		    eval "\$logins" . $index . "{\$account}++;";
		}
		$logins{$account} ++;
		$os{$account} = $os . " (" . $os_ver_major . "." . $os_ver_minor . ")";
		$cpu{$account} = $cpu;
		$ram{$account} = $ram;
		$screen{$account} = $screen_x . " x " . $screen_y;
	    }
	}
    }

    # figure out unique number of logins per day
    for ($i=0;$i<=$#min_time;$i++)
    {
	eval "foreach \$key (sort keys(\%logins$i)) { \$uniquerangelogins$i++; }";
	eval "foreach \$key (sort keys(\%logins$i)) { \$rangelogins$i += \$logins$i {\$key}; }";
    }

    
    foreach $key (sort keys(%logins))
    {
	$total++;
	$loginscount{$logins{$key}} ++;
	$oscount{$os{$key}} ++;
	$cpucount{$cpu{$key}} ++;
	$ramcount{$ram{$key}} ++;
	$screencount{$screen{$key}} ++;

	($junk, $domain) = ($key =~ /(.*)\.(.*)$/);

	# check if there is a .xxx, and it's not a number
	if (($domain ne "") and ($domain == 0))
	{
	    # lowercase it and add in!
	    $domain =~ tr/A-Z/a-z/;
	    $domaincount{$domain}++;
	}
    }

    &make_top_page();

    &make_login_page();
    &make_os_page();
    &make_cpu_page();
    &make_memory_page();
    &make_domain_page();
    &make_range_logins_page();
    &make_screen_page();
}

sub get_index
{
    local ($i);

    for ($i=0;$i<=$#min_time;$i++)
    {
	#print "$min_time[$i], $time, $max_time[$i]\n";
	if (($time >= $min_time[$i]) && ($time <= $max_time[$i]))
	{
	    return $i;
	}
    }
    
    return -1;
}

sub make_top_page
{
    open(PAGE,">default.htm") || die "Can't open default.htm: $!";

    local($oldfh) = select(PAGE);

    html_begin("Other Realms User Statistics");
    
    header1("Other Realms User Statistics");

    print_auto_gen();

    #print "This page has been accessed ";
    #print "<img src=/cgi-bin/counter.exe?-sblutrns+stats.html> times.";
    #&new_line();
  
    print "There have been $total different users logged onto the beta server.\n";
    new_line();

    header2_begin();

    reference("logins.html","User logins");
    new_line();
    reference("os.html","User operating systems ");
    new_line();
    reference("cpu.html","User processors");
    new_line();
    reference("memory.html","User memory");
    new_line();
    reference("domain.html","User domains");
    new_line();
    reference("range.html","User logins since 4/6");
    new_line();
    reference("screen.html","User screen sizes");
    new_line();

    header2_end();

    html_end();
    close(PAGE);

    select($oldfh);
}

sub make_login_page
{
    open(PAGE,">logins.html") || die "Can't open logins.html: $!";

    local($oldfh) = select(PAGE);

    html_begin("Login Server Statistics");
    
    header1("Login Server Statistics");

    print_auto_gen();
    print_login_counts();

    reference("default.htm","Stats");

    html_end();
    close(PAGE);

    select($oldfh);

}

sub make_os_page
{
    open(PAGE,">os.html") || die "Can't open os.html: $!";

    local($oldfh) = select(PAGE);

    html_begin("Operating System User Statistics");
    
    header1("Operating System User Statistics");

    print_auto_gen();
    print_os_counts();

    reference("default.htm","Stats");

    html_end();
    close(PAGE);

    select($oldfh);
}

sub make_cpu_page
{
    open(PAGE,">cpu.html") || die "Can't open cpu.html: $!";

    local($oldfh) = select(PAGE);

    html_begin("Processor User Statistics");
    
    header1("Processor User Statistics");
    
    print_auto_gen();
    print_cpu_counts();

    reference("default.htm","Stats");

    html_end();
    close(PAGE);

    select($oldfh);
}

sub make_memory_page
{
    open(PAGE,">memory.html") || die "Can't open memory.html: $!";

    local($oldfh) = select(PAGE);

    &html_begin("Memory User Statistics");
    
    &header1("Memory User Statistics");

    &print_auto_gen();
    &print_memory_counts();

    &reference("default.htm","Stats");

    &html_end();
    close(PAGE);

    select($oldfh);
}

sub make_domain_page
{
    open(PAGE,">domain.html") || die "Can't open domain.html: $!";

    local($oldfh) = select(PAGE);

    &html_begin("Domain User Statistics");
    
    &header1("Domain User Statistics");

    &print_auto_gen();

    print "These are the domains of the email addresses of all the users who have ";
    print "actually logged in.\n";
    &new_line();

    &print_domain_counts();

    &reference("default.htm","Stats");

    &html_end();
    close(PAGE);

    select($oldfh);
}

sub make_range_logins_page
{
    open(PAGE,">range.html") || die "Can't open range.html: $!";

    local($oldfh) = select(PAGE);

    &html_begin("Logins over time");
    
    &header1("Logins over time");

    &print_auto_gen();

    print "Times are specified in local time for $ENV{TZ}.\n";
    &new_line();
    &print_range_counts();

    &reference("default.htm","Stats");

    &html_end();
    close(PAGE);

    select($oldfh);
}

sub make_screen_page
{
    open(PAGE,">screen.html") || die "Can't open screen.html: $!";

    local($oldfh) = select(PAGE);

    &html_begin("User Screen Sizes");
    
    &header1("User Screen Sizes");

    &print_auto_gen();

    &print_screen_counts();

    &reference("default.htm","Stats");

    &html_end();
    close(PAGE);

    select($oldfh);
}

sub print_auto_gen
{
    print "Automatically generated at ";
    &print_local_time(time);
    print " $ENV{TZ}.\n";
    
    &new_line();
}

sub print_login_counts
{
    &table_begin();
    
    &table_data("Login count");
    &table_data("# of people logged this many times");
    &table_data("Cumulative percent");

    &table_next_entry();
    
    $cumulative = 0;
    
    foreach $key (reverse sort numerically keys(%loginscount))
    {
	$cumulative += $loginscount{$key};

	&table_data($key);
	&table_data($loginscount{$key});

	&table_data_begin();
	printf("%d",100*$cumulative/$total);
	&table_data_end();

	&table_next_entry();
    }
    
    &table_end();
}

sub print_os_counts
{
    &table_begin();
    
    &table_data("Operating System Counts");
    &table_data(" ");

    &table_next_entry();
    
    foreach $key (sort keys(%oscount))
    {
	&table_data($key);
	&table_data($oscount{$key});
	&table_next_entry();
    }
    
    &table_end();
}

sub print_cpu_counts
{
    &table_begin();
    
    &table_data("CPU Counts");
    &table_data(" ");
    &table_next_entry();
    
    foreach $key (sort keys(%cpucount))
    {
	&table_data($key);
	&table_data($cpucount{$key});
	&table_next_entry();
    }
    
    &table_end();
}

sub print_memory_counts
{
    print "These may be off by a meg or two\n";
    
    &table_begin();
    
    &table_data("RAM counts");
    &table_data(" ");
    &table_next_entry();
   
    
    foreach $key (sort numerically keys(%ramcount))
    {
	&table_data_begin();
	print "$key MB";
	&table_data_end();
		    
	&table_data($ramcount{$key});

	&table_next_entry();
    }
    
    &table_end();
}

sub print_domain_counts
{
    &table_begin();
    
    &table_data("Domain");
    &table_data("# of users");
    &table_next_entry();
   
    
    foreach $key (sort keys(%domaincount))
    {
	&table_data($key);
	&table_data($domaincount{$key});

	&table_next_entry();
    }

    &table_end();
}

sub print_range_counts
{
    &table_begin();
    
    &table_data("Begin");
    &table_data("End");
    &table_data("Unique user logins");
    &table_data("Total logins");
    &table_next_entry();
   
    for ($i=0;$i<=$#min_time;$i++)
    {
	&table_data_begin();
	&print_local_time($min_time[$i]);
	&table_data_end();
	&table_data_begin();
	&print_local_time($max_time[$i]);
	&table_data_end();
	&table_data_begin();
	eval "printf(\"%d\",\$uniquerangelogins$i);";
	&table_data_end();
	&table_data_begin();
	eval "printf(\"%d\",\$rangelogins$i);";
	&table_data_end();

	&table_next_entry();
    }

    &table_end();
}

sub print_screen_counts
{
    &table_begin();
    
    &table_data("Screen size");
    &table_data("# of users");
    &table_next_entry();
   
    
    foreach $key (sort keys(%screencount))
    {
	&table_data($key);
	&table_data($screencount{$key});

	&table_next_entry();
    }

    &table_end();
}


# time util functions

sub print_local_time
{
    local($time) = @_;
    local($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst);
    local($month_str);

    ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($time);
    
    $month_str = (Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec)[$mon];
    printf("%s %d 19%d %2d:%02d:%02d",$month_str,$mday,$year,$hour,$min,$sec);
}

sub strings_to_time
{
    local($smonth,$day,$year,$stime) = @_;
    local ($hour,$minute,$second);
    local (%month_map);

    ($hour,$minute,$second) = ($stime =~ /(..):(..):(..)/);

    $month_map{"Jan"} = 0;
    $month_map{"Feb"} = 1;
    $month_map{"Mar"} = 2;
    $month_map{"Apr"} = 3;
    $month_map{"May"} = 4;
    $month_map{"Jun"} = 5;
    $month_map{"Jul"} = 6;
    $month_map{"Aug"} = 7;
    $month_map{"Sep"} = 8;
    $month_map{"Oct"} = 9;
    $month_map{"Nov"} = 10;
    $month_map{"Dec"} = 11;

    $month = $month_map{$smonth};

    #print "$second,$minute,$hour,$day,$month,$year\n";

    # add 8 hours to the pseudo-gm time, to get real gm time
    # since the time we're sending it in from PST
    60*60*8 + timegm($second,$minute,$hour,$day,$month,$year % 100);

}

sub calc_beginning_time
{
    # return a time on the start day

    local($month,$day,$year);
    local ($hour,$minute,$second);
    
    $month = 3; # december
    $day = 7;   # 7th
    $year = 96;  # 1996
    
    $hour = 12;
    $minute = 0;
    $second = 0;
    
    #print "$second,$minute,$hour,$day,$month,$year\n";
    
    # add 8 hours to the pseudo-gm time, to get real gm time
    # since the time we're sending it in from PST
    60*60*8 + timegm($second,$minute,$hour,$day,$month,$year % 100);
}

# html functions

sub table_begin
{
    print "<TABLE BORDER=7 CELLPADDING=2>\n";
    table_next_entry();
}

sub table_end
{
    print "</TABLE>\n";
}

sub table_data
{
    local($data) = @_;

    table_data_begin();
    print $data;
    table_data_end();
}

sub table_data_begin
{
    print "<TD>";
}

sub table_data_end
{
    print "</TD>\n";
}

sub table_next_entry
{
    print "<TR>\n";
}

sub reference
{
    local($link,$desc) = @_;

    print "<A HREF=$link>$desc</A>\n";
}

sub new_line
{
    print "<P>\n";
}

sub header1
{
    local($data) = @_;

    print "<H1>$data</H1>\n";
}

sub header2_begin
{
    print "<H2>";
}

sub header2_end
{
    print "</H2>\n";
}

sub html_begin
{
    local($title) = @_;

    print "<HTML>\n";
    
    print "<HEAD>\n";
    print "<!-- Automatically generated at $^T by $0-->\n";
    print "<TITLE>",$title,"</TITLE>\n";
    print "</HEAD>\n";
    
    print "<BODY BACKGROUND=\"/rock.jpg\">\n";
}

sub html_end
{
    print "</BODY>\n";
    print "</HTML>\n";
}

# sort function

sub numerically { $a <=> $b; }
