This is the "foobar" ctrace client:

Run this to connect to the foobar server. The foobar server can be started
from the directory ../foobar_server.

To run:
 
$./ctrace

Instructions
------------
Each line you enter should replicate a CTrace configuration macro without
brackets, ie spaces only separating the arguments from the macro. For example:

TRC_SET_UNIT_LEVEL UNIT_FOO TRC_ERR

When you have finished use:

QUIT

Do not use CTRL-C or the server may crash with a broken pipe. The server does
not handle this error condition.
