#! /usr/bin/perl

$buf="";
while(<>){
    if(($buf =~ /=/)|($buf =~ /}/)|($_ =~ /}/)){
        $buf =~ s#,$##;
    }
    print $buf;
    $buf=$_;
}
$buf =~ s#,$##;
print $buf;
