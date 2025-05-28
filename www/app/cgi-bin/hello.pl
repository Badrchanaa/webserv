use strict;
use warnings;

# Read POST data
my $post_data = '';
if ($ENV{'REQUEST_METHOD'} eq 'POST') {
    read(STDIN, $post_data, $ENV{'CONTENT_LENGTH'});
}

# Send HTTP headers
print "Content-Type: text/html\r\n\r\n";

# Output HTML
print "<html><body>\n";
print "<h1>Perl CGI Test</h1>\n";
print "<p><b>Request Method:</b> $ENV{'REQUEST_METHOD'}</p>\n";
print "<p><b>Script Name:</b> $ENV{'SCRIPT_NAME'}</p>\n";
print "<p><b>Query String:</b> $ENV{'QUERY_STRING'}</p>\n";
print "<h2>POST Data</h2>\n";
print "<pre>$post_data</pre>\n";

print "<h2>Environment Variables</h2><pre>\n";
foreach my $key (sort keys %ENV) {
    print "$key = $ENV{$key}\n";
}
print "</pre>\n";

print "</body></html>\n";
