# RF Signal Heatmap
Create a heatmap of the RF signal power values (dBm) for 4 different frequenices. The heatmap is coloured red -> yellow -> green -> blue.  
Click on wherever the reading was/is being taking on the map and assign the dBm value to that point.  Hit the spacebar and 
the point cloud is resolved into a mesh, which is then coloured based on the intensity of the signal at each point.  
If an Arduino with the correct protocol is connected, the power value at a position can be automatically tabulated by pressing Tab.  

![](RFHeatmap_Video.gif)  

## Interesting Features
- Uses Delaunay triangulation to compute the mesh from a point cloud
- By pressing [1] [2] [3] [4] the user can cycle through layers for 4 different frequencies and define new heatmaps.
- By doing the above, if the correct Arduino protocol is connected, the Arduino's receiver frequency is automatically changed to match the C# app.

## Arduino Protocol
To make this work with an Arduino, follow this protocol:
```
// In loop()
if (Serial.available() > 0) {
  String command = Serial.readStringUntil('\n');
  command.trim();
  switch(command[0]){
    case '>': // Command to send dBm value
      Serial.println(dbmValue);
      break;
    case 'L': // Command to change frequency
      changeFrequencies(command.substring(1,command.length()).toInt());
      break;
    default:
      Serial.println("Unknown command");
      break;
  }
}
```
```
// Assuming you already have an array of frequencies, and a reference to your RF transceiver called rf69
void changeFrequencies(int newFrequencyIndex){
  currentFrequency = frequencies[newFrequencyIndex];
  rf69.setFrequency(currentFrequency);
}
```

## To Do
- Loading/saving point clouds
- Implement up/down arrows to move between background images (so you can move up and down floors)
- Automatically resolve Arduino COM port
- Make measurement points moveable
- Fix negative number graphical bug
- Add custom heatmap colour range (current range is -20 to -80)
