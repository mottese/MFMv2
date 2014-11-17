#!/usr/bin/perl

use strict;
use warnings;    
use List::Util qw(reduce);     

my %naturals = (
      C => 24, 
      D => 27, 
      E => 30, 
      F => 32, 
      G => 36, 
      A => 40, 
      B => 45,
    );

open my $read, "<$ARGV[0].xml" or die $!;
open my $write, ">$ARGV[0].txt" or die $!;

my @orig;
my @reduced; 
my $gcd;

while (my $line = <$read>) {
  if (index($line, "<measure ") != -1) {
    @orig = ();
  }
  elsif (index($line, "</measure") != -1){
    my $arrSize = @orig;
    if ($arrSize > 0) {
      @reduced = @orig;
      $gcd = gcd_list(@reduced);
      @reduced = map {$_ / $gcd} @orig;
      @reduced = uniq(@reduced);
      print $write join(":", @reduced) . "\n";
    }
  }
  elsif (index($line, "<step>") != -1) {
    push (@orig, $naturals{substr($line, index($line, ">") + 1, 1)});
  }
}


close $read;
close $write;


sub gcd_list {
  return reduce { euclid($a, $b) } @_;
}

sub euclid {
 my ($a, $b) = @_;
 return ($b) ? euclid($b, $a % $b) : $a;
}

sub uniq {
  my %seen;
  grep !$seen{$_}++, @_;
}
