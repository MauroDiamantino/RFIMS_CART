# RFIMS_CART
This software is intended to run in the "RF Interference Measurement System (RFIMS)" which is going to be installed beside the China-Argentina Radio Telescope (CART) to analize the RFI which could reach the telescope. The aim of this system is to measure the RF signals which arrive at the place where the telescope will be installed, taking into account different azimuth positions and two antenna polarizations, horizontal and vertical. Then, it must process the sweeps, identify the interferences and register them in a database. 

Also, this software must generate the signals to drive the antenna positioning system and capture and process the signals which will be sent by this system in response.

Moreover, this software must do a front end calibration every so often. This process consists in disconnecting the antenna and connecting a noise source to the system input, capturing sweeps with this device turned on and off and then processing these sweeps to get the actual parameters of the front end: gain and noise source versus frequency.
