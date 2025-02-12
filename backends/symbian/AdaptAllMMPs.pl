
use Cwd;

$buildDir = getcwd();
chdir("../../");

# list of project files to process
@mmp_files = (
	"mmp/scummvm_scumm.mmp", 
	"mmp/scummvm_queen.mmp", 
	"mmp/scummvm_simon.mmp", 
	"mmp/scummvm_sky.mmp", 
	"mmp/scummvm_gob.mmp", 
	"mmp/scummvm_saga.mmp", 
	"mmp/scummvm_kyra.mmp", 
	"mmp/scummvm_sword1.mmp", 
	"mmp/scummvm_sword2.mmp", 
	"mmp/scummvm_lure.mmp", 
	"mmp/scummvm_cine.mmp", 
	"mmp/scummvm_agi.mmp", 
	"S60/ScummVM_S60.mmp",  
	"S60v3/ScummVM_S60v3.mmp", 
	"S80/ScummVM_S80.mmp", 
	"S90/ScummVM_S90.mmp",
	"UIQ2/ScummVM_UIQ2.mmp", 
	"UIQ3/ScummVM_UIQ3.mmp" 
);

# do this first to set all *.mmp & *.inf files to *.*.in states
ResetProjectFiles();

print "
=======================================================================================
Updating slave MACRO settings in MMP files from master 'scummvm_base.mmp'
=======================================================================================

";

# do this first so we have @DisableDefines for correct inclusion of SOURCE files later
UpdateSlaveMacros();

print "
=======================================================================================
Preparing to update all the Symbian MMP project files with objects from module.mk files
=======================================================================================

";
	
my @section_empty = (""); # section standard: no #ifdef's in module.mk files
my @sections_scumm = ("", "DISABLE_SCUMM_7_8", "DISABLE_HE"); # special sections for engine SCUMM

# files excluded from build, case insensitive, will be matched in filename string only
my @excludes_snd = ( 
	"mt32",
	"fluidsynth",
	"i386",
	"part.cpp",
	"partial.cpp",
	"partialmanager.cpp",
	"synth.cpp",
	"tables.cpp",
	"freeverb.cpp"
); 
my @excludes_gui = ( 
); 

#arseModule(mmpStr,		dirStr,		ifdefArray,		[exclusionsArray])
ParseModule("_base",	"base",		\@section_empty); # now in ./TRG/ScummVM_TRG.mmp, these never change anyways...
ParseModule("_base",	"common",	\@section_empty);
ParseModule("_base",	"gui",		\@section_empty,		\@excludes_gui);
ParseModule("_base",	"graphics",	\@section_empty);
ParseModule("_base",	"sound",	\@section_empty,		\@excludes_snd);

chdir("engines/");
ParseModule("_scumm",	"scumm",	\@sections_scumm);
ParseModule("_queen",	"queen",	\@section_empty);
ParseModule("_simon",	"simon",	\@section_empty);
ParseModule("_sky",		"sky",		\@section_empty);
ParseModule("_gob",		"gob",		\@section_empty);
ParseModule("_saga",	"saga",		\@section_empty);

ParseModule("_kyra",	"kyra",		\@section_empty);
ParseModule("_sword1",	"sword1",	\@section_empty);
ParseModule("_sword2",	"sword2",	\@section_empty);
ParseModule("_lure",	"lure",		\@section_empty);
ParseModule("_cine",	"cine",		\@section_empty);
ParseModule("_agi",	"agi",		\@section_empty);
print "
=======================================================================================
Done. Enjoy :P
=======================================================================================
";

##################################################################################################################
##################################################################################################################

# parses multiple sections per mmp/module 
sub ParseModule
{
	my ($mmp,$module,$sections,$exclusions) = @_;
	my @sections = @{$sections};
	my @exclusions = @{$exclusions};

	foreach $section (@sections)
	{
		CheckForModuleMK($module, $section, @exclusions);
		UpdateProjectFile($mmp, $module, $section);
	}
}

##################################################################################################################

# parses all module.mk files in a dir and its subdirs
sub CheckForModuleMK
{
	my ($item,$section,@exclusions) = @_;

	# if dir: check subdirs too
	if (-d $item)
	{
		#print "$item\n";
		
		opendir DIR, $item;
		#my @Files = readdir DIR;
		my @Files = grep s/^([^\.].*)$/$1/, readdir DIR;
		closedir DIR;

		foreach $entry (@Files)
		{
			CheckForModuleMK("$item/$entry", $section, @exclusions);
		}
	}

	# if this is a module.mk file
	if (-f $item and $item =~ /.*\/module.mk$/)
	{
		my $sec = "";
		my $ObjectsSelected = 0;
		my $ObjectsTotal = 0;
		
		print "$item for section '$section' ... ";

		open FILE, $item;
		my @lines = <FILE>;
		close FILE;

		my $count = @lines;
		print "$count lines";
		
		A: foreach $line (@lines)
		{
			# found a section? reset 
			if ($line =~ /^ifndef (.*)/)
			{
				$sec = $1;
			}
			# found an object? Not uncommented!
			if (!($line =~ /^#/) && $line =~ s/\.o/.cpp/)
			{
				# handle this section?
				if ($sec eq $section)
				{
					$ObjectsTotal++;

					$line =~ s/^\s*//g; # remove possible leading whitespace
					$line =~ s/ \\//; # remove possible trailing ' \'
					$line =~ s/\//\\/g; # replace / with \
					chop($line); # remove \n
					
					# do we need to skip this file? According to our own @exclusions array
					foreach $exclusion (@exclusions)
					{
						if ($line =~ /$exclusion/i)
						{
							print "\n      ! $line (excluded, \@exclusions[$exclusion])";
							next A;
						}
					}
					
					# do we need to skip this file? According to MACROs in .MMPs
					foreach $DisableDefine (@DisableDefines)
					{
						if ($DisableDefine eq $section && $section ne '')
						{
							print "\n         !$line (excluded, MACRO $DisableDefine)";
							next A;
						}
					}
										
					$ObjectsSelected++;
					#print "\n         $line";
					$output .= "SOURCE $line\n";
				}
			}
		}
		print " -- $ObjectsSelected/$ObjectsTotal objects selected\n";
	}
}

##################################################################################################################

# update an MMP project file with the new objects
sub UpdateProjectFile
{
	my ($mmp,$module,$section) = @_;
	my $n = "AUTO_OBJECTS_".uc($module)."_$section";
	my $a = "\/\/START_$n\/\/";
	my $b = "\/\/STOP_$n\/\/";
	my $updated = " Updated @ ".localtime();
	my $name;
	my @mmp_files_plus_one = @mmp_files;
	unshift @mmp_files_plus_one, "mmp/scummvm_base.mmp";	

	foreach $name (@mmp_files_plus_one)
	{
		my $file = "$buildDir/$name";

		open FILE, "$file";
		my @lines = <FILE>;
		close FILE;
		
		my $onestr = join("",@lines);
		
		if ($onestr =~ /$n/)
		{

			print "      - $name @ $n updating ... ";
	
			$onestr =~ s/$a.*$b/$a$updated\n$output$b/s;
			open FILE, ">$file";
			print FILE $onestr;
			close FILE;
			
			print "done.\n";
		}
	}
		
	$output = "";
}

##################################################################################################################

sub UpdateSlaveMacros
{
	my $updated = " Updated @ ".localtime();

	my $name = "mmp/scummvm_base.mmp";	
	my $file = "$buildDir/$name";
	print "Reading master MACROS from backends/symbian/$name ... ";

	open FILE, "$file";
	my @lines = <FILE>;
	close FILE;
	my $onestr = join("",@lines);

	my $n = "AUTO_MACROS_MASTER";
	my $a = "\/\/START_$n\/\/";
	my $b = "\/\/STOP_$n\/\/";
	$onestr =~ /$a(.*)$b/s;
	my $macros = $1;
	
	my $libs_first = "\n// automagically enabled static libs from macros above\n";
	my $libs_second = "STATICLIBRARY	scummvm_base.lib // must be above USE_* .libs\n";
	my $macro_counter = 0;
	my $macros2 = "\n"; # output for in *.mmp MACROS section
	my $projects = "\n..\\mmp\\scummvm_base.mmp\n"; # output for in BLD.INF projects section

	foreach $line (split("\n", $macros))
	{
		# do we need to add a static .lib?
		if ($line =~ /^.*MACRO\s*([0-9A-Z_]*)\s*\/\/\s*LIB\:(.*)$/)
		{
			my $macro = $1; my $lib = $2;
			
			# this macro enabled? then also add the .lib
			if ($line =~ /^\s*MACRO\s*$macro/m)
			{
				# these are the USE_ libs
				$libs_second .= "STATICLIBRARY	$lib\n" if ($macro =~ /^USE_/);
			}
			else
			{
				# these are the non DISABLED_ libs
				$libs_first .= "STATICLIBRARY	$lib\n" if ($macro =~ /^DISABLE_/);
				
				# add projects for BLD.INF's
				my $projectname = substr("$lib",0,-4);
				$projects .= "..\\mmp\\$projectname.mmp\n" if ($macro =~ /^DISABLE_/);				
			}
			$macro_counter++;
		}
		# not commented out? then add the macro to output string
		if ($line =~ /^\s*MACRO\s*([0-9A-Z_]*)\s*/)
		{
			my $macro = $1;
			$macros2 .= "$line\n";
			push @DisableDefines, $macro; # used in CheckForModuleMK()!!
		}
	}		

	print "$macro_counter macro lines.\n";

	$n = "AUTO_MACROS_SLAVE";
	$a = "\/\/START_$n\/\/";
	$b = "\/\/STOP_$n\/\/";

	$m = "AUTO_PROJECTS";
	$p = "\/\/START_$m\/\/";
	$q = "\/\/STOP_$m\/\/";
	
	foreach $name (@mmp_files)
	{
		$file = "$buildDir/$name";
		$fileBLDINF = $buildDir .'/'. substr($name, 0, rindex($name, "/")) . "/BLD.INF";
		print "Updating macros   in $file ... ";
		#print "Updating macros in backends/symbian/$name ... ";
	
		open FILE, "$file";	@lines = <FILE>; close FILE;
		$onestr = join("",@lines);
	
		my $extralibs = ""; # output
		# slash in name means it's a phone specific build file: add LIBs
		$extralibs .= "$libs_first$libs_second" if (-e $fileBLDINF);
		
		$onestr =~ s/$a.*$b/$a$updated$macros2$extralibs$b/s;
		
		open FILE, ">$file"; print FILE $onestr; close FILE;

		my $count = @lines;
		print "wrote $count lines.\n";

		if (-e $fileBLDINF)
		{
			# slash in name means it's a phone specific build file:
			# this also means we need to update a BLD.INF file here!
			print "Updating projects in $fileBLDINF ... \n";

			open FILE, "$fileBLDINF";	@lines = <FILE>; close FILE;
			$onestr = join("",@lines);
		
			$onestr =~ s/$p.*$q/$p$updated$projects$q/s;
			
			open FILE, ">$fileBLDINF"; print FILE $onestr; close FILE;
		}
	}
}		

##################################################################################################################

sub ResetProjectFiles()
{
	my $onestr, @lines;
	my @mmp_files_plus_one = @mmp_files;
#	unshift @mmp_files_plus_one, "mmp/scummvm_base.mmp";	
	
	print "Resetting project files: ";
	
	# we don't need to do mmp/scummvm_base.mmp", it was done in BuildPackageUpload.pl before the call to this script
	foreach $name (@mmp_files_plus_one)
	{
		my $file  = "$buildDir/$name";

		print "$name ";
		open FILE, "$file.in";	@lines = <FILE>; close FILE;
		$onestr = join("",@lines);
		open FILE, ">$file"; print FILE $onestr; close FILE;

		# also do BLD.INF if it is there...
		my $fileBLDINF = $buildDir .'/'. substr($name, 0, rindex($name, "/")) . "/BLD.INF";
		if (-e "$fileBLDINF.in")
		{
			print substr($name, 0, rindex($name, "/")) . "/BLD.INF ";
			open FILE, "$fileBLDINF.in";	@lines = <FILE>; close FILE;
			$onestr = join("",@lines);
			open FILE, ">$fileBLDINF"; print FILE $onestr; close FILE;
		}
	}	

	print "... done.\n";
}

##################################################################################################################
