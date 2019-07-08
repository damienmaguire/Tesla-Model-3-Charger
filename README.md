# Tesla-Model-3-Charger
Reverse Engineering of the Tesla Model 3 charger and development of an open source control board 

25/06/19 : Thanks to a kind donation I will soon have a Model 3 battery charger for reverse engineering. Setting up a repo to contain the knowledge.

08/07/19 : Received the charger (corectly known as the Power Conversion System) as it also contains a DC DC converter. Also received a HV Controller and wiring looms from Model 3. Uploaded CAN logs from pcs and HV controller from the CP CAN bus. 500k.

To Power up the PCS : Connect 12V battery or power supply to the DC DC converter high power pins. Ground is pin nearest the edge. Then connect Wire W02 to the 12v battery ground. PCS will then wake up.

