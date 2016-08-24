# ParticleSamplePrograms
Particle Photon Sample Programs and Test Library Applications

<h3>xlightsDemoLight</h3>
RF receiver and parser of MySensor messages, adheres to xlights protocol.
Changes color of internal 3 rings. 

<h3>xlightsDemoTransmitter</h3>
RF sender of MySensor messages, adhers to xlights protocol. 
Sends on/off and change color commands based on MySensor Serial format input

<h3>TestSensors</h3>
Testing program for MIC, DHT (temperature), ALS (light), and PIR (motion) sensors

<h3>TestMQTT</h3>
MQTT testing app on the Photon, connected to the public HiveMQ broker.
Does not offer TLS features. 

<h3>Test8BytePayloadParse</h3>
Package and parse mechanism for MySensors C_CUSTOM's V_VAR1 variable payload. 
Adheres to xlights protocol, used for controlling an individual ring