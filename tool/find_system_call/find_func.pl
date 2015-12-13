#! /usr/bin/perl -w

my %system_tab;
my %wrapper_tab;

sub tag_init_core {
    my ($fname, $tab) = @_;
    open TAB_FD, '<', $fname;
    while(<TAB_FD>) {
        chomp;
        if ($_ =~ /(\w+)/) {
            $$tab{"$1"} = "$1";
        }
    }
    close TAB_FD;
}

sub tab_init {
    foreach $item("./sys_tab", "/usr/bin/sys_tab") {
        if (! -e $item) {
            next;
        }
        tag_init_core($item, \%system_tab, );
    }
    foreach $item("./lib_tab", "/usr/bin/lib_tab") {
        if (!-e $item) {
            next;
        }
        tag_init_core($item, \%wrapper_tab);
    }
}

sub tab_find_core {
    my($check_fname, $system, $wrapper) = @_;

    open CHECK_FD, '<', $check_fname;
    while(<CHECK_FD>) {
        chomp;
        while ($_ =~ m{(\w+)}g) {
            if (exists $$wrapper{"$1"}) {
                print "$1 is library functions\n";
            } 
            if (exists $$system{"$1"}) {
                print "$1 is the system call\n";
            }
        }
    }
    close CHECK_FD;
}

sub tab_find {
    my($argv, $system, $wrapper) = @_;

    foreach $item (@$argv) {
        #print "item($item)\n";
        tab_find_core($item, $system, $wrapper);
    }
}

sub tab_dump {
    my($system, $wrapper) = @_;
    while (my($key, $val) = each %$wrapper) {
        print "wrapper key($key), val($val)\n";
    }

    while (my($key, $val) = each %$system) {
        print "system key($key), val($val)\n";
    }
}

tab_init(\%system_tab, \%wrapper_tab);

tab_find(\@ARGV, \%system_tab, \%wrapper_tab);

#tab_dump(\%system_tab, \%wrapper_tab);

