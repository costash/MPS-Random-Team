$BAMS = @("bam1.exe", "bam2.exe", "bam3.exe", "bam4.exe", "bam5.exe");
$TEST_IMAGES = @(
	"doom.jpg", 
	"degraded.png", 
	"newspaper.jpg", 
	"perf.jpg", 
	"truth.jpg",
	"newspaper2.jpg",
	"test3.jpg",
	"test4.jpg",
	"test5.jpg",
	"test6.jpg",
	"test7.jpg"
);

$OUT_IMAGES = @(
	"doom", 
	"degraded", 
	"newspaper", 
	"perf", 
	"truth",
	"newspaper2",
	"test3",
	"test4",
	"test5",
	"test6",
	"test7"
);

##------------------------------------------------------------------------------
## Select running mode

echo "";
echo "Select mode:";
echo "    0) RUN VBAM";
echo "    1) RUN BAM";
echo "    2) TEST (TESSERACT)";
echo "    3) Copy-Debug";
echo "    4) Copy-Release";
echo "    5) CLEAN";
echo "";
[int]$mode = read-host "Running mode";

##------------------------------------------------------------------------------
## Copy

if ($mode -eq 3) {
	Copy-Item .\src\Debug\* -include bam*.exe, *.dll `
		-destination .\bam\
	Remove-Item -Recurse -Force .\bam\vbam\		
	New-Item  .\bam\vbam\ -itemtype directory
	Copy-Item .\src\Debug\* -include vbam.exe, FreeImage.dll `
		-destination .\bam\vbam\
	EXIT;
}

if ($mode -eq 4) {
	Copy-Item .\src\Release\* -include bam*.exe, *.dll `
		-destination .\bam\
	Remove-Item -Recurse -Force .\bam\vbam\		
	New-Item  .\bam\vbam\ -itemtype directory
	Copy-Item .\src\Release\* -include vbam.exe, FreeImage.dll `
		-destination .\bam\vbam\
	EXIT;
}

##------------------------------------------------------------------------------
## CLEAN

if ($mode -eq 5) {
	Remove-Item .\bam\* -include *.tif, *.tiff
	Remove-Item .\output\*
	Remove-Item .\* -include *.tif, *.tiff
	EXIT;
}

##------------------------------------------------------------------------------
## Get BAM ID if running mode is BAM

if ($mode -eq 1) {
	[int]$bam_id = read-host "Insert BAM ID: ";
}

##------------------------------------------------------------------------------
## Select test number

echo "";
echo "Select test:";
echo "    0) All";
for ($i=1; $i -le $OUT_IMAGES.length; $i++) {
	$option = "    " + $i + ") " + $OUT_IMAGES[$i - 1];
	echo "$option";
}

echo "";
[int]$test = read-host "Test number";
echo "";

##------------------------------------------------------------------------------
## Functions

function getBAMCommand($IMAGE_ID){
	$command = ".\bam\" +  $BAMS[$bam_id - 1] + " .\images\" + $TEST_IMAGES[$IMAGE_ID] + " .\output\" + $OUT_IMAGES[$IMAGE_ID] + ".tiff .\output\" + $OUT_IMAGES[$IMAGE_ID] + "_conf.tiff";
    return $command;
}

function getVBAMCommand($IMAGE_ID){
	$command = ".\bam\vbam\vbam.exe 100 1000 .\bam .\images\" + $TEST_IMAGES[$IMAGE_ID] + " .\output\ " + $OUT_IMAGES[$IMAGE_ID];
    return $command;
}

function RunCommand($image_id) {
	if ($mode -eq 0) {
		echo ("RUNNING VBAM on " + $TEST_IMAGES[$image_id]);
		$command = getVBAMCommand($image_id);
		iex $command;
	}

	if ($mode -eq 1) {
		echo ("RUNNING BAM on " + $TEST_IMAGES[$image_id]);
		$command = getBAMCommand($image_id);
		iex $command;
		return;
	}

	if ($mode -eq 2) {
		return;
	}
}

function RunTest($test_id) {
	$command = ".\src\Debug\test.exe " + $OUT_IMAGES[$test_id];
	iex $command;
}

##------------------------------------------------------------------------------
## Run BAM tests
if ($test -eq 0) {
	for ($i=0; $i -lt $TEST_IMAGES.length; $i++) {
		RunCommand($i);
	}
	for ($i=0; $i -lt $OUT_IMAGES.length; $i++) {
		RunTest($i);
	}
}
elseif ($test -le $TEST_IMAGES.length) {
	RunCommand($test - 1);
	RunTest($test - 1);
}
else {
	echo ("Incorrect test number: " + $test);
	echo "";
}

