#!/usr/bin/perl

use strict;
use warnings;

open my $read, "<$ARGV[0].mfs" or die $!;
open my $write, ">$ARGV[0].xml" or die $!;

my $attributes = "      <attributes>\n";
$attributes = $attributes . "        <divisions>1</divisions>\n";
$attributes = $attributes . "        <key>\n          <fifths>0</fifths>\n        </key>\n";
$attributes = $attributes . "        <time>\n          <beats>4</beats>\n          <beat-type>4</beat-type>\n        </time>\n";;
$attributes = $attributes . "        <clef>\n          <sign>G</sign>\n          <line>2</line>\n        </clef>\n";
$attributes = $attributes . "      </attributes>\n";


print $write "<?xml version=\"1.0\" encoding=\"UTF-8\" standalong=\"no\"?>\n";
print $write "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 3.0 Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dtd\">\n";
print $write "<score-partwise version=\"3.0\">\n";
print $write "  <part-list>\n";
print $write "    <score-part id=\"P1\">\n";
print $write "      <part-name>Music</part-name>\n";
print $write "    </score-part>\n";
print $write "  </part-list>\n";
print $write "  <part id=\"P1\">\n"; #only doing 1 part for now

my @notes = ();
my $num_of_notes = 0;
my $id;
my $to_elements = 0;

#read in the given .mfs file
while (my $line = <$read>) {
  chomp $line;
  
  #find Note's ID
  if (index($line, "RegisterElement(Note") != -1) {
    $id = substr($line, rindex($line, ',') + 1, -1); 
    next;
  }
  
  if (not $to_elements and index($line, "GA(") != -1) {
    $to_elements = 1;
  }
  
  #find all of the Note elements
  if ($to_elements and index($line, "GA(" . $id) != -1) {
    $notes[$num_of_notes] = $line;
    $num_of_notes++;
    next;     
  }
}

#sort the array of note elements by their x position
@notes = sort {
  (split /,/, $a)[1] <=> (split /,/, $b)[1]
} @notes;


my $max_x = (split /,/, $notes[@notes - 1])[1];
my ($i, $n) = (0, 0); #$i is where in the @notes array you are
                      #$n is where in a chord you are
for (my $x = 0; $x < $max_x; $x++) {
  print $write "    <measure number=\"" . ($x + 1) . "\">\n";
  if ($x > 0) {
    print $write "      <attributes/>\n";
  } else {
    print $write $attributes;
  }
  
  $n = 0;
  for (; $i < @notes; $i++) {
    if ((split /,/, $notes[$i])[1] != $x) {
      last; #if the x position of our note doesn't equal the measure that we're in, move on to the next measure
    }
    print $write "      <note>\n";
    if ($n > 0) {
      print $write "        <chord/>\n"; #if we are more than 1 note into the measure, we will be making a chord
    }
    print $write "        <pitch>\n";
    
    my @note_info = get_note($notes[$i]);
    
    print $write "          <step>" . $note_info[0] . "</step>\n";
    print $write "          <alter>" . $note_info[1] . "</alter>\n";
    print $write "          <octave>4</octave>\n"; #only using 1 octave for now
    print $write "        </pitch>\n";
    print $write "        <duration>4</duration>\n";
    print $write "        <type>whole</type>\n"; #only using whole notes for now
    print $write "      </note>\n";
    $n++;    
  }
  print $write "    </measure>\n";  
}

print $write "  </part>\n";
print $write "</score-partwise>\n";



close $read;
close $write;


sub get_note {
  my $note = shift;
  my $bits = substr((split /,/, $note)[3], 0, -1); #hex
  my @scale = ('C', 'D', 'E', 'F', 'G', 'A', 'B', 'C');
  my @accidentals = ('0', '1', '-1', '2', '-2', '0', '0', '0');
  
  $bits = unpack ('B*', pack ('H*', $bits)); #binary
  
  my $sd = substr ($bits, 25, 3); #scale degree bits
  my $a = substr ($bits, 28, 3); #accidental bits
  
  return ($scale[oct("0b".$sd)], $accidentals[oct("ob".$a)]); #binary string to decimal number to note name
}
