/*	File:		nespkgtemplate output.f	Created on:	10/19/2004	Copyright 2004 by Paul Guyot.	All rights reserved worldwide.*/theOutput := {	app: kAppSymbol,	// <-- will be replaced by NESPackager	theForm: GetLayout("nespkgtemplate.lyt"),	autoClose: 'noAutoClose,	installScript: func(partFrame)	begin	end,	removeScript: func(partFrame)	begin	end,};if (kPackageExtrasIcons.icon) then	theOutput.icon := kPackageExtrasIcons.icon;if (kPackageExtrasIcons.iconPro) then	theOutput.iconPro := kPackageExtrasIcons.iconPro;