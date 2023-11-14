#include <math.h>
#ifndef DROPS_H
#define DROPS_H

#include "PatMath.h"

struct Drop {
  PointF location;
  Vector vector;
  // byte size;
  // float bounciness = 1.45;
  bool collided;
  bool inUse;
  void toLine(Line& line) {
    line.updateFrom(location.x, location.y, 
        location.x + vector.x, location.y + vector.y);
  }
};

struct Drops {
  Drop* drops;
  byte count;

  Drops(Drop drops[], byte count) : drops(drops), count(count) {}
};

class Emitter {
  public:
    virtual void maybeEmit();
};

class LimitEmitter : public Emitter {
  public:
    LimitEmitter(int limit, Drops drops) : limit(limit), drops(drops) {}

    void maybeEmit() override {
      int dropsLeft = limit;
      for (int i = 0; i < drops.count; ++i) {
        Drop drop = drops.drops[i];
        if (!drop.inUse) {
          if (dropsLeft-- == 0) return;
          drop.inUse = true;
          doEmit(drop);
          drops.drops[i] = drop;
        }
      }
    }

    virtual void doEmit(Drop& drop);

  private:
    int limit;
    Drops drops;
};


// grabs up to 5 unused drops and start at the top of the screen:
class RainEmitter : public LimitEmitter {
  public:
    RainEmitter(Drops drops, int left, int right) : LimitEmitter(5, drops), left(left), right(right) {}
    void doEmit(Drop& drop) override {
      drop.location.x = randomInRange(left, right);
      drop.location.y = random(4);
      drop.vector.x = .25;
      drop.vector.y = randomf(0, 2);
    }
    private:
      int left, right;
};

// Emits drops upward / sidweays from a movable point
class SprinklerEmitter : public LimitEmitter {
  public:
    SprinklerEmitter(Drops drops, Point location) : LimitEmitter(3, drops), location(location) {}

    void doEmit(Drop& drop) override {
      drop.location.x = location.x;
      drop.location.y = location.y;
      drop.vector.y = randomf(-1, .1);
      drop.vector.x = randomf(-1, 1);
    }
    private:
      Point location;
};

class ShowerEmitter : public LimitEmitter {
  public:
    ShowerEmitter(Drops drops, Point location) : LimitEmitter(3, drops), location(location) {}

    void doEmit(Drop& drop) override {
      drop.location.x = location.x;
      drop.location.y = location.y;
      drop.vector.y = randomf(.1, 1);
      drop.vector.x = randomf(-.25, .25);
    }
    private:
      Point location;
};

class MultiShowerEmitter : public LimitEmitter {
  public:
    MultiShowerEmitter(Drops drops, byte height, byte left, byte right, byte count) : LimitEmitter(3, drops), height(height), left(left), right(right), count(count) {
      pixelsBetweenHeads = (float)(right - left) / (float) count;
    }

    void doEmit(Drop& drop) override {
      // pick a showerhead to come out of:
      int head = randomInRange(0, count);
      float offset = left + head * pixelsBetweenHeads;
      drop.location.x = offset;
      drop.location.y = height;
      drop.vector.y = randomf(.1, 1);
      drop.vector.x = randomf(-.1, .1);
    }
    private:
      byte height;
      byte left, right;
      byte count;
      float pixelsBetweenHeads;
};

class Deflector {
  public:
    virtual void draw(Arduboy2 arduboy) = 0;
    virtual void maybeDeflect(Drop& drop, Line& lineTemp) = 0;
};

class LineDeflector : public Deflector {
  public:
    float bounceMagnitude = .1;
    LineDeflector(Line line) : line(NormalLine(line)) {
      // // any reason to keep these separate byte field instead of using PointF?
      // calculateLineNormal(myNormal, startX, startY, endX, endY);
      // calculateSlopeAndYIntercept();
    }

    void draw(Arduboy2 arduboy) override {
      arduboy.drawLine(line.startX, line.startY, line.endX, line.endY);
    }

    void maybeDeflect(Drop& drop, Line& lineTemp) override {
      drop.toLine(lineTemp);
      if (line.collidesWith(lineTemp)) {
        drop.collided = true;
        calculateBounce(drop.vector, line.normal);
      }
    }

    private:
      NormalLine line;
};

// TODO go back to just deflector array
struct Deflectors {
  LineDeflector* deflectors;
  byte count;

  Deflectors(LineDeflector deflectors[], byte count) : deflectors(deflectors), count(count) {}
};

#endif
