// режимы:
// 0 - input A gain 128
// 1 - input B gain 32
// 2 - input A gain 64
#define SCK_DELAY 10

class HX711 {
  public:
    HX711 (byte data, byte clock, byte mode = 2) :
      _data(data), _clock(clock), _mode(mode) {
      pinMode(data, INPUT);
      pinMode(clock, OUTPUT);
    }
    boolean isReady() {
      return (!digitalRead(_data));
    }
    long getData() {
      if (isReady()) {
        _weight = 0;
        for (byte i = 0; i < 24; i++) {
          digitalWrite(_clock, HIGH);
          _weight <<= 1;
          if (digitalRead(_data)) _weight |= 1;
          digitalWrite(_clock, LOW);
        }
        for (byte i = 0; i < _mode + 1; i++) {
          digitalWrite(_clock, 1);
          digitalWrite(_clock, 0);
        }
        _weight = median3(_weight);
      }
      return _weight + _cal;
    }
    void calibrate() {
      _cal = -_weight;
    }
    void setOffset(long cal) {
      _cal = cal;
    }
    long getOffset() {
      return _cal;
    }

  private:
    long buf[3];
    byte counter = 0;
    // быстрая медиана
    long median3(long value) {
      buf[counter] = value;
      if (++counter > 2) counter = 0;
      if ((buf[0] <= buf[1]) && (buf[0] <= buf[2])) return (buf[1] <= buf[2]) ? buf[1] : buf[2];
      else {
        if ((buf[1] <= buf[0]) && (buf[1] <= buf[2])) return (buf[0] <= buf[2]) ? buf[0] : buf[2];
        else return (buf[0] <= buf[1]) ? buf[0] : buf[1];
      }
    }

    long _weight = 0, _cal = 0;
    const byte _data, _clock, _mode;
};
