#!/usr/bin/perl

use strict;
use warnings;
use List::Util qw(reduce);

open my $read, "<$ARGV[0].txt" or die $!;
open my $write, ">$ARGV[0].dat" or die $!;

#2-7 notes
my @totals = (0, 0, 0, 0, 0, 0);
my @num    = (0, 0, 0, 0, 0, 0);
my @spl;
my $size;

while (my $line = <$read>) {
  if (index($line, ":") > -1) {
    @spl = split(':', $line);
    $size = @spl;
    my $temp = reduce { $a + $b} map { 1 / $_ } @spl;
    $totals[$size-2] += $temp;
    $num[$size-2]++;
  }
}

for (my $i = 0; $i < 6; $i++) {
  my $j = $i + 2;
  my $avg = $totals[$i] / ($num[$i] ? $num[$i] : 1);
  print $write "$j avg = $avg\n";
}

close $read;
close $write;
