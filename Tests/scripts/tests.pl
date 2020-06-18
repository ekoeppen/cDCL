#!/usr/bin/perl -w
# ==============================
# Fichier:			tests.pl
# Projet:			(Desktop Connection Library)
# Ecrit par:		Paul Guyot (pguyot@kallisys.net)
# 
# Créé le:			20/6/2003
# Tabulation:		4 espaces
# 
# Copyright:		© 2003 by Paul Guyot.
# 					Tous droits réservés pour tous pays.
# Licence:			Licence Réflexive Kallisys
# 					http://www.kallisys.org/reflexive/
# ===========
# $Id: tests.pl,v 1.4 2003/08/06 04:48:08 paul Exp $
# ===========

# Syntaxe:
# perl tests.pl <chemin vers DCLTests> [<test> | all]

use strict;	# I am a quiche eater.

# --------------------------------------------------------------------------- #
# Globales
# --------------------------------------------------------------------------- #

# Chemin vers le programme DCLTests.
my $DCLTests;

# Répertoire où se trouve tests.pl
my $ProgramPath;

# Test ou all.
my $Test;

# Chemin vers le master ou undef si le test est all.
my $MasterFile;

# --------------------------------------------------------------------------- #
# Usage()
# --------------------------------------------------------------------------- #
sub Usage()
{
	printf STDERR "Syntaxe: perl tests.pl <chemin vers DCLTests> [<test> | all]\n";
	exit 2;
}

# --------------------------------------------------------------------------- #
# GetProgramPath()
# --------------------------------------------------------------------------- #
sub GetProgramPath()
{
	$ProgramPath = $0;
	if ($ProgramPath =~ m/^(.*)(\/[^\/]+)$/)
	{
		$ProgramPath = "$1/";
	} else {
		$ProgramPath = "";
	}
}

# --------------------------------------------------------------------------- #
# CheckAndParseArguments()
# --------------------------------------------------------------------------- #
sub CheckAndParseArguments()
{
	if ($#ARGV != 1)
	{
		printf STDERR "Nombre incorrect d'arguments\n";
		Usage();
	}
	
	# Extraction du chemin vers DCLTests.
	$DCLTests = $ARGV[0];
	if (! -x "$DCLTests")
	{
		printf STDERR "DCLTests ($DCLTests) n'est pas exécutable\n";
		Usage();
	}
	
	$Test = $ARGV[1];
	# Recherche du master-test-
	if ($Test ne "all")
	{
		$MasterFile = $ProgramPath . "master-test-" . $Test;
		if (! -e $MasterFile)
		{
			printf STDERR "Aucun fichier maître pour $Test ne fut trouvé\n";
			Usage();
		}
	}
}

# --------------------------------------------------------------------------- #
# Test( $Test, $MasterFile )
# --------------------------------------------------------------------------- #
sub Test( $ $ )
{
    my $result = 0;
	my $Test = shift;
	my $MasterFile = shift;
	
	open MASTER_FILE, "<$MasterFile";
	my @Master = <MASTER_FILE>;
	close MASTER_FILE;
	
	open TEST_RESULT, "'$DCLTests' $Test 2>&1 |" or die "Impossible de lancer le test";
	my @TestResult = <TEST_RESULT>;
	close TEST_RESULT or $TestResult[$#TestResult + 1] = "Erreur lors de la fermeture de DCLTests.";
	
	# Comparaison.
	my $lineCount = $#TestResult;
	if (($lineCount == 0)
		&& ($TestResult[0] eq "Test non disponible pour cette plateforme\n"))
	{
		print "Test $Test : NON DISPONIBLE\n";
	} else {
		# Itération sur les deux.
		my $different = 0;
		if ($lineCount != $#Master)
		{
			$different = 1;
			
			if ($lineCount < $#Master)
			{
				$lineCount = $#Master;
			}
		}
		
		for (my $indexLine = 0; $indexLine <= $lineCount; $indexLine++)
		{
			my $TestLine;
			my $MasterLine;

			if ($indexLine <= $#TestResult)
			{
				$TestLine = $TestResult[$indexLine];
			} else {
				$TestLine = '';
			}
			if ($indexLine <= $#Master)
			{
				$MasterLine = $Master[$indexLine];
			} else {
				$MasterLine = '';
			}
			
			chomp $TestLine;
			chomp $MasterLine;
			if ($TestLine ne $MasterLine)
			{
				if (!($TestLine =~ m/^$MasterLine$/))
				{
					$different = 1;
					print "- $MasterLine\n";
					print "+ $TestLine\n";
				}
			}
		}
		
		if ($different)
		{
			print "Test $Test : ECHEC\n";
			$result = -1;
		} else {
			print "Test $Test : SUCCES\n";
		}
	}
	return $result;
}

# --------------------------------------------------------------------------- #
# main
# --------------------------------------------------------------------------- #
GetProgramPath();
CheckAndParseArguments();

my $exitCode = 0;

# Si le test est all, on itère sur tous les master-test- du dossier.
if ($Test eq "all")
{
	# Itération sur les tous les master-test dans $ProgramPath
	opendir MASTER_DIR, $ProgramPath or die "Impossible d'ouvrir $ProgramPath";
	my @MasterDirFiles = readdir MASTER_DIR;
	closedir MASTER_DIR or die "Erreur à la fermeture de $ProgramPath";
	
	foreach my $MasterDirFile (@MasterDirFiles)
	{
		if ($MasterDirFile =~ m/^master-test-(.*)$/)
		{
			if (Test( $1, $ProgramPath . $MasterDirFile ))
			{
			    $exitCode = 1;
			}
		}
	}
} else {
	if (Test( $Test, $MasterFile ))
	{
	    $exitCode = 1;
	}
}

exit $exitCode;

## =================================================================== ##
## The reason computer chips are so small is computers don't eat much. ##
## =================================================================== ##
