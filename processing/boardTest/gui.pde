void GUIinit() {
  cp5 = new ControlP5(this);   

  cp5.addButton("com_open").setCaptionLabel("OPEN").setPosition(0, 50).setSize(60, 25);
  cp5.addButton("com_close").setCaptionLabel("CLOSE").setPosition(0, 75).setSize(60, 25);
  cp5.addButton("com_send").setCaptionLabel("SEND").setPosition(0, 100).setSize(60, 25);

  List l = Arrays.asList("9600", "19200", "57600", "115200", "500000", "1000000", "2000000");
  cp5.addScrollableList("9600")
    .setPosition(0, 25)
    .setSize(60, 120)
    .setBarHeight(25)
    .setItemHeight(25)
    .addItems(l)
    .close()
    ;

  baudSpeeds[0] = 9600;
  baudSpeeds[1] = 19200;
  baudSpeeds[2] = 57600;
  baudSpeeds[3] = 115200;
  baudSpeeds[4] = 500000;
  baudSpeeds[5] = 1000000;
  baudSpeeds[6] = 2000000;

  // лист СОМ
  com_list = cp5.addScrollableList("COM")
    .setCaptionLabel("PORT")
    .setPosition(0, 0)
    .setSize(60, 120)
    .setBarHeight(25)
    .setItemHeight(25)
    .close();
  com_list.onEnter(new CallbackListener() {
    public void controlEvent(CallbackEvent ev) {
      com_list.clear();
      com_list.addItems(Arrays.asList(Serial.list()));
    }
  }
  );
}

void COM(int n) {
  curPort = Serial.list()[n];
}

void baud(int n) {
  curBaud = baudSpeeds[n];
}

void com_open() {
  if (!COMstatus) {
    myPort = new Serial(this, curPort, curBaud);
    COMstatus = true;
  }
}

void com_close() {
  myPort.stop();
  COMstatus = false;
}

void com_send() {
  if (COMstatus) {
    myPort.write(228);
  }
}
