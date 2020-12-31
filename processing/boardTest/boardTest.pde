int maxWei = 150;

// Для работы нужна либа P5
// Набросок/Импортировать библиотеку/Добавить библиотеку
// Искать ControlP5

// P5
import controlP5.*;
ControlP5 cp5;
ScrollableList com_list;

// JAVA
import java.util.*;
import processing.serial.*;
Serial myPort;
String curPort = "COM0";
boolean COMstatus = false;
int baudSpeeds[] = new int [7];
int curBaud = 9600;

PImage img;
PGraphics pg;
int FR;
int BR;
int FL;
int BL;
int valX;
int valY;

void setup() {  
  size(380, 640, P3D);
  smooth(6);
  pg = createGraphics(380, 640, P3D);
  pg.imageMode(CENTER); 
  GUIinit();
  img = loadImage("board.png");
  img.resize(0, 500);
}


void draw() {
  background(130);  
  byte[] buf = new byte[12];
  if (COMstatus && myPort.available() > 0) {    
    if (myPort.readBytes(buf) == 12) {
      FR = mergeBytes(buf[0], buf[1]);
      BR = mergeBytes(buf[2], buf[3]);
      FL = mergeBytes(buf[4], buf[5]);
      BL = mergeBytes(buf[6], buf[7]);
      valX = mergeBytes(buf[8], buf[9]);
      valY = mergeBytes(buf[10], buf[11]);
    }
  }
  
  int valXraw = ((FR - FL) + (BR - BL)) / 2;
  int valYraw = ((FL - BL) + (FR - BR)) / 2; 

  pg.beginDraw();
  pg.background(130);
  pg.translate(width/2, height/2);
  pg.rotateY(valXraw/2000.0);
  pg.rotateX(valYraw/2000.0);  
  pg.image(img, 0, 0);
  pg.translate(-width/2, -height/2); 
  pg.endDraw();

  image(img, width/2-img.width/2, height/2-img.height/2);

  fill(20);
  noStroke();
  rect(width/2+80, 150, 20, -FR/15);
  rect(width/2-80-20, 150, 20, -FL/15);
  rect(width/2+80, 550, 20, -BR/15);
  rect(width/2-80-20, 550, 20, -BL/15);
  text(FR, width/2+110, 150);
  text(FL, width/2-110-40, 150);
  text(BR, width/2+110, 550);
  text(BL, width/2-110-40, 550);

  noFill();
  stroke(20);
  strokeWeight(4);
  circle(width/2, height/2, 120);
  fill(20);
  noStroke();
  circle(width/2+valX*60.0/0x7FFF, height/2+valY*60.0/0x7FFF, 70);
  
}

int mergeBytes(byte low, byte high) {
  return ( (low & 0xff) + ((high << 8)) );
}
int mergeBytesU(byte low, byte high) {
  return ( (low & 0xff) + ((high << 8)& 0xff00) );
}
