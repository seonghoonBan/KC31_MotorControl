import datetime

versionHeaderFile = open('Heliostat_Axis_Control/Version.h', 'r');

lines = [];
for line in versionHeaderFile:
	lines.append(line);

versionHeaderFile.close();

today = datetime.date.today()
datestring = str(today.year) + '.' + str(today.month).rjust(2, '0') + "." + str(today.day).rjust(2, '0');
dateStringLine = "#define BUILD_VERSION_DATE \"" + datestring + "\"\n";

#version per day
#defaults
versionPerDay = 1
buildVersion = 1;

#read existing
if(len(lines) >=5):
	#version per day
	if(lines[2] == dateStringLine) :
		parts = lines[3].split("\"");
		if(len(parts) >= 3):
			versionPerDay = int(parts[1]);
			versionPerDay = versionPerDay + 1;

	#build revision
	parts = lines[4].split("\"");

	if(len(parts) >= 3):
		buildVersion = int(parts[1]);
		buildVersion += 1;


lines = [
	'#pragma once \n',
	'\n',
	dateStringLine,
	'#define BUILD_VERSION_PER_DAY "' + str(versionPerDay) + '"\n',
	'#define BUILD_VERSION_REVISION "' + str(buildVersion) + '"\n',
	'#define BUILD_VERSION_STRING "v" BUILD_VERSION_DATE "-" BUILD_VERSION_PER_DAY " (r" BUILD_VERSION_REVISION ")"\n'
];


versionHeaderFile = open('Heliostat_Axis_Control/Version.h', 'w');
versionHeaderFile.writelines(lines);
versionHeaderFile.close()
