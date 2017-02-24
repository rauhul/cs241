import sys;
import os;
from random import randint

if os.path.isfile(sys.argv[1]):
	os.remove(sys.argv[1]);

wfile = open(sys.argv[1], "a");
types = ["body_temp", "heart_beat", "blood_sugar"];

while 1:
	opt = raw_input("Add Wearable or Quit? (q to quit, otherwise anything):");
	if opt == "q":
		break;

	for start in range(10, 350, 10):
		#start = raw_input("Start time in millis:");
		interval = 10;
		#interval = raw_input("Interval time between data points in millis:");
		#num = raw_input("Number of datapoints:");
		num = 400;
		wfile.write("BEGIN\n");
		wfile.write("START:" + str(start) + "\n");
		wfile.write("INTERVAL:" + str(interval) + "\n");


		for i in range(0, int(num)):
			dtype = types[randint(0, 2)];
			dval = randint(70, 130);
			wfile.write(dtype + ":" + str(dval)  + "\n");

		wfile.write("END\n");

wfile.close();