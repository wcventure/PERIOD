This is "foo":

In the file main.c you will find a call to the TRC_INIT macro.

Remake "foo" with the following TRC_INIT values to test different CTrace levels:

Stack Trace
-----------
TRC_INIT(NULL, TRC_ENABLED, TRC_ON, TRC1, UNIT_MAX, 0);
$./foo 5	// command line test

Print Traces Only
-----------------
TRC_INIT(NULL, TRC_ENABLED, TRC_ON, TRC0, UNIT_MAX, 0);
$./foo 5	// command line test

All Traces
----------
TRC_INIT(NULL, TRC_ENABLED, TRC_ON, TRC_ALL, UNIT_MAX, 0);
$./foo 5	// command line test

Error Traces Only
-----------------
TRC_INIT(NULL, TRC_ENABLED, TRC_ON, TRC_ERR, UNIT_MAX, 0);
$./foo -5	// command line test
