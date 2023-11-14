#include <math.h>
#ifndef PATMATH_H
#define PATMATH_H

using byte = unsigned char;

struct Vector {
  float x, y;
};

using PointF = Vector;

int randomInRange(int min, int max) {
    return min + rand() % (max - min + 1);
}

float randomf(float min, float max) {
  float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
  return min + scale * ( max - min );      /* [min, max] */
}

// TODO rename/caps, etc
const float tolerance = 0.01f;

bool approximatelyEqual(float left, float right) {
  return abs(left - right) < tolerance;
}

bool approximatelyGreaterThanOrEqual(float a, float b) {
  return a - b > -tolerance * max(abs(a), abs(b));
}

bool approximatelyLessThanOrEqual(float a, float b) {
  return a - b < tolerance * max(abs(a), abs(b));
}

bool inRangeInclusive(float value, float bound1, float bound2) {
    float lower, upper;
    if (bound1 < bound2) {
      lower = bound1;
      upper = bound2;
    } else {
      lower = bound2;
      upper = bound1;
    }
    return approximatelyGreaterThanOrEqual(value, lower)
        && approximatelyLessThanOrEqual(value, upper);
}

class Line {
  private:
    float slope;
    bool hasYIntercept;
    float yIntercept;

  public:
    float startX;
    float startY;
    float endX;
    float endY;
    Vector normal;

    Line() {}
    
    Line(float startX, float startY, float endX, float endY) {
      updateFrom(startX, startY, endX, endY);
    }

    virtual void updateFrom(float startX, float startY, float endX, float endY) {
      this->startX = startX;
      this->startY = startY;
      this->endX = endX;
      this->endY = endY;
      if (approximatelyEqual(startX, endX)) {
        slope = INFINITY;
        hasYIntercept = false;
        yIntercept = NAN;
      } else {
        slope = (endY - startY) / (endX - startX);
        hasYIntercept = true;
        yIntercept = startY - slope * startX;
      }
    }

    bool collidesWith(Line otherLine) {
      if (approximatelyEqual(slope, otherLine.slope)) {
        return false;
      } else if (hasYIntercept && otherLine.hasYIntercept) {
        float x = (otherLine.yIntercept - yIntercept) / (slope - otherLine.slope);
        float y = slope * x + yIntercept;

        // TODO rewrite with inRangeInclusive
        if (x >= min(startX, endX) && x <= max(startX, endX)
          && x >= min(otherLine.startX, otherLine.endX)
          && x <= max(otherLine.startX, otherLine.endX)) {
            return true;
        }
      } else if (hasYIntercept) {
        // TODO rewrite with inRangeInclusive
        float y = otherLine.slope * startX + otherLine.yIntercept;
        if (y >= min(startY, endY) && y <= max(startY, endY) 
         && startX >= min(otherLine.startX, otherLine.endX)
         && startX <= max(otherLine.startX, otherLine.endX)) {
          return true;
        }
      } else if (otherLine.hasYIntercept) {
        // (this block actually written by Pat because AI kept screwing it up.)
        // I am vertical and have no slope or yIntercept; thus, we must check that:
        // my X is in range of other's X, and its Y at my X is in my Y range.
        if (inRangeInclusive(startX, otherLine.startX, otherLine.endX)) {
          // slope-incercept line equation: y = mx + b, and we already have m and b.
          float otherYatMyX = otherLine.slope * startX + otherLine.yIntercept;
          if (inRangeInclusive(otherYatMyX, startY, endY)) {
            return true;
          }
        }
      }
      return false;
    }
};

class NormalLine : public Line {
  private: 
    void updateNormal() {
      // Calculate the direction vector of the line.
      float lineDirectionX = endX - startX;
      float lineDirectionY = endY - startY;

      // Calculate the line's normal vector.
      float lineNormalX = -lineDirectionY;
      float lineNormalY = lineDirectionX;

      // Normalize the line normal vector.
      float lineNormalMagnitude = sqrt(lineNormalX * lineNormalX + lineNormalY * lineNormalY);
      lineNormalX /= lineNormalMagnitude;
      lineNormalY /= lineNormalMagnitude;

      normal.x = lineNormalX;
      normal.y = lineNormalY;
    }

  public:
    NormalLine() {}

    NormalLine(Line line) : NormalLine(line.startX, line.startY, line.endX, line.endY) {}
    
    NormalLine(float startX, float startY, float endX, float endY) : Line() {
      updateFrom(startX, startY, endX, endY);
    }

    void updateFrom(float startX, float startY, float endX, float endY) override {
      Line::updateFrom(startX, startY, endX, endY);
      updateNormal();
    }
};

// TODO move these into NormalLine
float dotProduct(Vector vector1, Vector vector2) {
  // Calculate the dot product of the two vectors.
  return vector1.x * vector2.x + vector1.y * vector2.y;
}

void calculateBounce(Vector& particleVelocity, Vector surfaceNormal) {
  // Calculate the projection of the particle's velocity vector onto the surface's normal vector.
  float projection = dotProduct(particleVelocity, surfaceNormal);

  // Reflect the projection over the surface's normal vector.
  float reflectedProjection = projection; // TODO figure out why this was negated before

  // Bard says: The coefficient of restitution is a measure of how much energy is lost 
  // when two objects collide. A coefficient of restitution of 1 means that the collision 
  // is perfectly elastic, and all of the kinetic energy is conserved. A coefficient of
  // restitution of 0 means that the collision is perfectly inelastic, and all of the 
  // kinetic energy is converted into other forms of energy, such as heat and sound. 
  // A value 2.0f is a common value for the coefficient of restitution for many ypes of 
  // materials. It represents a collision where approximately 75% of the kinetic energy 
  // is conserved.
  float restitutionAndReflection = 1.4f * reflectedProjection;

  // Update the x and y fields of the particle's velocity vector separately.
  particleVelocity.x -= restitutionAndReflection * surfaceNormal.x;
  particleVelocity.y -= restitutionAndReflection * surfaceNormal.y;
}


// Bard-created fixed-point math stuff:
int toFixedPoint(int integerPart, int fractionalPart) {
  return (integerPart << 8) | fractionalPart;
}

int getIntegerPart(int fixedPointNumber) {
  return (fixedPointNumber >> 8) & 0xFF;
}

int getFractionalPart(int fixedPointNumber) {
  return fixedPointNumber & 0xFF;
}

int addFixedPoint(int fixedPointNumber1, int fixedPointNumber2) {
  // Add the integer parts
  int integerPart = getIntegerPart(fixedPointNumber1) + getIntegerPart(fixedPointNumber2);

  // Add the fractional parts
  int fractionalPart = getFractionalPart(fixedPointNumber1) + getFractionalPart(fixedPointNumber2);

  // Handle overflow
  if (fractionalPart > 255) {
    integerPart++;
    fractionalPart -= 256;
  }

  // Combine the integer and fractional parts into a fixed-point number
  return toFixedPoint(integerPart, fractionalPart);
}


#endif