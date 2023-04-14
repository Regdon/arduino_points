# Arduino Point Control
This project aims to control up to 16 points on a model railway through the use of Arduino. This cuts down on the number of wires running all over the layout to just 4. 

The project is split into two sections, a controller and several nodes.

## Point Controller
This is the brains of the project. It will react to user input and will display the output state of each point. It will also use i2c commands to tell the nodes which physical point to switch.

Further details in the Point_Controller folder.

## Point Node
This Arduino recieves i2c commands from the controller, and decodes them to tell a specific servo controlled point to swap to either straight or turnout. A controller can handle up to four nodes, and each node can handle upto 8 points.

Further details in the Point_Node folder.

## i2c Testing
This project exists to provide a simple controllable i2c output for use in testing node circuits.

Further details in the i2c_testing folder.