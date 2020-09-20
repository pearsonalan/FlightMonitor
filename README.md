# FlightMonitor

## Summary

FlightMonitor connects to Microsoft Flight Simulator 2020 using the SimConnect interface
to get the current position and attitude information of the simulated flight.  The 
position is then broadcast via UDP on the local network to allow the flight to be tracked
with ForeFlight on an iPhone or iPad device.

## ForeFlight GPS Integration

The FlightMonitor App sends UDP broadcasts to port 49002 for both position and
attitude data. Attitude data needed for ForeFlight to show AHRS information is 
broadcast 5 times per second. Position information is broadcast once per second. 
Integration is done using XGPS and XATTR text packets as documented at
https://support.foreflight.com/hc/en-us/articles/204115005-Flight-Simulator-GPS-Integration-UDP-Protocol-

## License

FlightMonitor is released under the GNU GPL v3.  See [LICENSE.txt](LICENSE.txt)
for more information.
