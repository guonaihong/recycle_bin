#!/usr/bin/perl -w
use utf8;
use open ":encoding(utf8)",":std";

my %hash_url;
while (<>) {
	while($_ =~ m{(第.*?[章卷][^<]*).*?<a\s+href="(http://[^ ]*)"}g) {
		#$1 is the chapter title
		#$2 is the url
		$hash_url{$2} = $1;
	}
}
my $key;
my $value;
while (($key, $value) = each %hash_url) {
	print "$key $value\n";
}
