#include <Arduboy2.h>
#include "PatMath.h"
#include "Drops.h"

const float GRAVITY = .01;
const byte DROPCOUNT = 30;

Arduboy2 arduboy;
Drop allDropObjects[DROPCOUNT];
Drops allDrops(allDropObjects, DROPCOUNT);
//RainEmitter emitter(allDrops, 0, arduboy.width());
SprinklerEmitter emitter(allDrops, Point(arduboy.width() / 2, arduboy.height() / 2));
//ShowerEmitter emitter(allDrops, Point(arduboy.width() / 2, 0));
//MultiShowerEmitter emitter(allDrops, 0, 30, 90, 5);
// TODO figure out abstract array initialization
LineDeflector deflectors[] = {
  LineDeflector(Line(80, 15, 80, 30)),
  // LineDeflector(Line(55, 60, 70, 60)),
  // LineDeflector(Line(20, 20, 40, 40)),
  LineDeflector(Line(50, 45, 40, 50)),
  // LineDeflector(Line(50, 45, 60, 50))
};
Deflectors allDeflectors(deflectors, 2);
// LineDeflector deflector(Line(80, 10, 80, 40));

void setup() {
  // initiate arduboy instance
  arduboy.begin();

  // arduboy.setFrameRate(5);
  arduboy.initRandomSeed();
}

void loop() {
  // pause render until it's time for the next frame
  if (!(arduboy.nextFrameDEV()))
    return;

  // first we clear our screen to black
  arduboy.clear();

  // Preallocate a line for this run.  TODO maybe even put it in globals
  Line lineTemp;

  // Update drops for gravity / off screen:
  for (int i = 0; i < DROPCOUNT; ++i) {
    Drop drop = allDropObjects[i];

    // Update drop if active:
    if (drop.inUse) {
      drop.location.x += drop.vector.x;
      drop.location.y += drop.vector.y;
      drop.vector.y += GRAVITY;

      // check if off screen:
      if (drop.location.y > arduboy.height()
       || drop.location.x < 0
       || drop.location.x > arduboy.width()) {
        drop.inUse = false;
        drop.collided = false;
      }
    }

    for (int j = 0; j < allDeflectors.count; ++j) {
      deflectors[j].maybeDeflect(drop, lineTemp);
    }

    allDropObjects[i] = drop;
  }

  // Emit new drops:
  emitter.maybeEmit();

  // Render:
  for (int j = 0; j < allDeflectors.count; ++j) {
    deflectors[j].draw(arduboy);
  }

  for (int i = 0; i < DROPCOUNT; ++i) {
    Drop drop = allDropObjects[i];

    // Show drop if active:
    if (drop.inUse) {
      if (arduboy.pressed(A_BUTTON)) {
        arduboy.setCursor(drop.location.x, drop.location.y);
        arduboy.write('A' + i);
      } else if (arduboy.pressed(B_BUTTON)) {
        arduboy.drawCircle(drop.location.x, drop.location.y, 1);
      } else if (drop.collided) {
        arduboy.drawCircle(drop.location.x, drop.location.y, 1);
      } else {
        // arduboy.drawLine(drop.location.x, drop.location.y, 
        //   drop.location.x + drop.vector.x, drop.location.y + drop.vector.y);
        arduboy.drawPixel(drop.location.x, drop.location.y);
      }
    }
  }

  arduboy.display();
}
