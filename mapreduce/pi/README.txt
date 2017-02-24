This directory contains tools for computing digits of pi in parallel.
It performs a "digit extraction", which you choose a digit N and it will
compute 16 hexadecimal digits starting at that position.

Just for reference, pi in hexadecimal starts with:
  3.243f6a8885a308d313198a...

The digits starting at 0 are:  243f6a8885a308d3
The digits starting at 3 are:     f6a8885a308d3131


To use this tool:
 1. Create input file with create_pi_input.py
 2. Run MapReduce on the file using mapper_pi and reducer_pi.
 3. The output file will contain 16 hex digits at the requested offset.


For example, to compute the millionth digit of pi:

    % pi/create_pi_input.py 1000000 > in
    % ./mr1 in out mapper_pi reducer_pi 4
    % cat out
    pi: 0x6c65e52cb45925b7


To confirm the results, see the "Some Pi digits in base 16" section of
this web page:
   http://bellard.org/pi/pi2700e9/pidigits.html
Notice that the last few digits of our computation are wrong:
   pi:      0x6c65e52cb45925b7
   correct:   6C65E52CB4593500...
                          ^^^^
This is inevitable; there is always a little roundoff error.


To check the accuracy of your results, you can run the computation again,
but start one or more digits later:

    % pi/create_pi_input.py 1000008 > in
    % ./mr1 in out mapper_pi reducer_pi 4
    % cat out
    pi: 0xb459350050e4a960

  From 1000000: 0x6c65e52cb45925b7
  From 1000008:         0xb459350050e4a960
                              ^^^^
You can see the results were correct except the 4 least significant digits.


The BBP.py script does this computation in Python with a single
thread, and it can compute an arbitrary number of digits.  You can use
this to check your results for small runs.  For example, to compute 50
digits (which is 200 bits) starting at offset 5:

    % ./BBP.py 1000000 200
    Scale = 16^1000000
    sum(1) = 0xb9626d73d74cf295557abc911bfbb1ff67bca186886632cdbf
    sum(4) = 0x8d5876b7f41e192c5d8650b5d3d88193e64f32eafb63601e57
    sum(5) = 0x5cca159ed696ac18c6820ace485331352eaa64de78441ff2ce
    sum(6) = 0x62d9fbf0583ef84fe08da5f57083979555cb4a95f45c128ea1
    
      6c65e52cb459350050e4bb178f4c67a0fcf7bf27206290ebb9
    
    time: 6.470 sec
