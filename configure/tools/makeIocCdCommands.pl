eval 'exec perl -S $0 ${1+"$@"}'  # -*- Mode: perl -*-
    if $running_under_some_shell; # makeIocCdCommands.pl

use Cwd;

$cwd  = cwd();
#hack for sun4
$cwd =~ s|/tmp_mnt||;
#hack for win32
$cwd =~ s|\\|\/|g;
$arch = $ARGV[0];
$iocsApplTop = $ARGV[1];
#remove top parts of current working directory
#and add ioc's appl top alias (if supplied)
if ( $iocsApplTop ne "" ) {
    $cwd =~ s|.*(/iocBoot.*)|$1|;
    $cwd = $iocsApplTop . $cwd;
}

unlink("cdCommands");
open(OUT,">cdCommands") or die "$! opening cdCommands";
print OUT "startup = \"$cwd\"\n";

#appbin is kept for compatibility with 3.13.1 
$appbin = $cwd;
$appbin =~ s/iocBoot.*//;
$appbin = $appbin . "/bin/${arch}";
print OUT "appbin = \"$appbin\"\n";

$top = $cwd;
$top =~ s/\/iocBoot.*//;
$applications{TOP} = $top;
print OUT "top = \"$top\"\n";
$topbin = "${top}/bin/${arch}";
#skip check that top/bin/${arch} exists; src may not have been builT
print OUT "topbin = \"$topbin\"\n";
$release = "$top/configure/RELEASE";
ProcessFile($release);
close OUT;

sub ProcessFile
{
  local *IN;
  my ($file) = @_;
  my $line;
  my $prefix;
  my $base;
  my ($macro,$post);

  if (-r "$file") {
    open(IN, "$file") or return;
    while ($line = <IN>) {
        next if ( $line =~ /\s*#/ );
	chomp($line);
		$line =~ s/[ 	]//g; # remove blanks and tabs
        next if ( $line =~ /^$/ ); # skip empty lines

        $_ = $line;

        #the following looks for
        # include $(macro)post
        ($macro,$post) = /include\s*\$\((.*)\)(.*)/;
        if ($macro ne "") { # true if macro is present
            $base = $applications{$macro};
            if ($base eq "") {
                #print "error: $macro was not previously defined\n";
                next;
            } else {
                $post = $base . $post;
                ProcessFile(${post});
                next;
            }
        }

        # the following looks for
        # include post
        ($post) = /include\s*(.*)/;
        if ($post ne "") {
            ProcessFile(${post});
            next;
        }

        #the following looks for
        # prefix = $(macro)post
        ($prefix,$macro,$post) = /(.*)\s*=\s*\$\((.*)\)(.*)/;
        if ($macro eq "") { # true if no macro is present
            # the following looks for
            # prefix = post
            ($prefix,$post) = /(.*)\s*=\s*(.*)/;
        } else {
            $base = $applications{$macro};
            if ($base eq "") {
                #print "error: $macro was not previously defined\n";
            } else {
                $post = $base . $post;
            }
        }
        $applications{$prefix} = $post;
        $app = lc($prefix);
        if ( -d "$post") { #check that directory exists
            print OUT "$app = \"$post\"\n";
        }
        if ( -d "$post/bin/$arch") { #check that directory exists
            print OUT "${app}bin = \"$post/bin/$arch\"\n";
        }
    }
    close IN;
    return ;
  }
}

