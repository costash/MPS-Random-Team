$BAMS = @("bam1.exe", "bam2.exe");
$TEST_IMAGES = @(
	"doom.jpg", 
	"degraded.png", 
	"newspaper.jpg", 
	"perf.jpg", 
	"truth.jpg"
	"newspaper2.jpg"
);

$OUT_IMAGES = @(
	"doom", 
	"degraded", 
	"newspaper", 
	"perf", 
	"truth"
	"newspaper2"
);

##------------------------------------------------------------------------------
## Select BAM for testing

echo "";
echo "Select bam:";
echo "    0) bam1.exe";
echo "    1) bam2.exe";
echo "    2) vbam.exe";

echo "";
$program_id = read-host "BAM number";

if (($program_id -lt 0) -or ($program_id -gt 2)) {
	echo "Incorrect BAM";
	echo "";
	EXIT;
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
$test = read-host "Test number";
echo "";

##------------------------------------------------------------------------------
## Functions

function getBAMCommand($IMAGE_ID){
	$command = ".\src\Release\" +  $BAMS[$program_id] + " .\images\" + $TEST_IMAGES[$IMAGE_ID] + " .\output\" + $OUT_IMAGES[$IMAGE_ID] + ".tiff .\output\" + $OUT_IMAGES[$IMAGE_ID] + "_conf.tiff";
    return $command;
}

function getVBAMCommand($IMAGE_ID){
	$command = ".\src\Release\vbam.exe 100 1000 .\bam .\images\" + $TEST_IMAGES[$IMAGE_ID] + " .\output\ " + $OUT_IMAGES[$IMAGE_ID];
    return $command;
}

function RunCommand($image_id) {
	if ($program_id -ne 2) {
		echo ("RUNNING BAM on " + $TEST_IMAGES[$image_id]);
		$command = getBAMCommand($image_id);
		iex $command;
	}
	elseif ($program_id -eq 2) {
		echo ("RUNNING VBAM on " + $TEST_IMAGES[$image_id]);
		$command = getVBAMCommand($image_id);
		iex $command;
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

