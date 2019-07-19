# Tesla-Model-3-Charger
Reverse Engineering of the Tesla Model 3 charger and development of an open source control board 

25/06/19 : Thanks to a kind donation I will soon have a Model 3 battery charger for reverse engineering. Setting up a repo to contain the knowledge.

08/07/19 : Received the charger (corectly known as the Power Conversion System) as it also contains a DC DC converter. Also received a HV Controller and wiring looms from Model 3. Uploaded CAN logs from pcs and HV controller from the CP CAN bus. 500k.

To Power up the PCS : Connect 12V battery or power supply to the DC DC converter high power pins. Ground is pin nearest the edge. Then connect Wire W02 to the 12v battery ground. PCS will then wake up.


10/07/19 : Added more can logs

15/07/19 : so it seems I was wrong. The PCS does indeed transmit something by itself over IPC CAN Tx. Log uploaded using a Salea Logic analyser. Free software available here : https://www.saleae.com/downloads/

19/07/19 : Sucessfully interfaced with the IPC CAN using an SN65HVD255 CAN transciever. Uploaded new log files with IPC CAN captures. DCDC Converter section now waking up. Message id 0x22A is responsible for commanding the DCDC to startup. 

V1 design for a PCS controller done and released. Do not use this. It probably won't work....


