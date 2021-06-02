/*
 * Pour compiler ce code, il suffit de taper `make` dans un terminal.
 * Si tout ce passe bien, un exécutable du nom `skel` apparaît dans
 * le répertoire et vous pouvez l'exécuter en tapant `./skel`.
 *
 * Ne faites pas attention au warning qui vous indique que la variable
 * `dt` n'est pas utilisée. C'est normal pour l'instant.
 */

#include <random>

#include <SFML/Graphics.hpp>

#include <SFML/Audio.hpp>

#include <math.h>

#include <fstream>

#include <string>

#include <regex>

#include <iostream>

#include <array>

#include <vector>

#include <algorithm>

#include <assert.h>

#ifdef _WIN32

#include <bits/stdc++.h>

#endif

using namespace std;
using namespace sf;

const bool DEBUG = false;

/*
 * Ce morceau de code pour permet de tirer un nombre flottant au hasard
 * entre 0.0 et 1.0. Pour cela, il suffit de l'appeler de la manière
 * suivante:
 *
 *     float r = Math::random();
 */
struct Math {
  static float random() {
    static mt19937 engine(time(nullptr));
    uniform_real_distribution < float > dist(0.0f, 1.0f);
    return dist(engine);
  }
  static float arrondir(float val, float prec) {
    return std::round(val / prec) * prec;
  }
};

/*
 * La structure Rectangle contient des champs pour manipuler un rectangle
 * avec sa position (x,y) qui correspond au coin en haut à gauche, sa
 * taille width x height, et sa couleur.
 *
 * Le type Color correspond à une structure de SFML:
 * http://www.sfml-dev.org/documentation/2.1/classsf_1_1Color.php
 *
 * Il existe des constantes pour les principales couleurs.
 *
 * Pour pouvoir dessiner des formes, vous pouvez lire le tutoriel suivant:
 * http://www.sfml-dev.org/tutorials/2.1/graphics-shape.php

 */

struct Position {
  float x;
  float y;
};
struct Hitbox2P {
  Position corner1;
  Position corner2;
};

struct Speed {
  float x;
  float y;
};
struct Flag {
  Hitbox2P hitbox;
  int nb;
};
struct Wall {
  Hitbox2P hitbox;
  int directionStop;
};
struct Car {
  int state = 1; //1 pour normal, 2 contre un mur, 3 dans la boue, 4 contre un mur dans la boue
  Position pos;
  Speed speed;
  Speed speedColision;
  int direction = 0;
  int laps = 0;
  int flag = 0;
  int nbNitro = 3;
  int score = 0;
  double lastNitroUsedTime = 0;
  float malusBonusSpeed = 1.0;
  int botPositionToTarget = 0;
  double lastActive = 0.0;
  sf::Color color;
  Position posInterBot;
  std::string botType;
  int startPosition = 0;
  float timeBlocked = 0;
};

struct Bonus {
  Position pos;
  int rayon;
  bool present;
};
struct Mud {
  Position pos;
  int rayon;
};
struct Ground {
  std::vector < Wall > walls;
  std::vector < Bonus > spawnPosNitro;
  std::vector < Mud > muds;
  std::vector < Flag > flags;
  std::vector < Position > botLine;
  Position spawnPos[4];
  sf::Vector2f scorePos;
};

struct Assets {
  sf::Texture backgroundMainScreenTexture;
  sf::Sprite backgroundMainScreen;
  sf::Texture superoffroadTextTexture;
  sf::Sprite superoffroadText;
  sf::Texture superoffroadCarTexture;
  sf::Sprite superoffroadCar;

  //game texture
  sf::Texture backgroundLevelScreenTexture;
  sf::Sprite backgroundLevelScreen;
  sf::Texture nitroTexture;
  sf::Sprite nitro;
  sf::Texture scoreTexture;
  sf::Sprite score;

  //fin course
  sf::Texture backgroundEndRunTexture;
  sf::Sprite backgroundEndRun;

  sf::Texture position1PersoTexture;
  sf::Sprite position1Perso;
  sf::Texture position1HabitsTexture;
  sf::Sprite position1Habits;

  sf::Texture position2PersoTexture;
  sf::Sprite position2Perso;
  sf::Texture position2HabitsTexture;
  sf::Sprite position2Habits;

  sf::Texture position3PersoTexture;
  sf::Sprite position3Perso;
  sf::Texture position3HabitsTexture;
  sf::Sprite position3Habits;

  //Dialogues
  sf::Texture faceTexture;
  sf::Sprite face;
  sf::Texture bulleTexture;
  sf::Sprite bulle;
  
  
  //Musiques
  sf::Music titleScreenmusic;
  sf::Music nameScreenmusic;
  sf::Music setupScreenmusic;
  sf::Music startScreenmusic;
  sf::Music goalScreenmusic;
  sf::Music celebrationScreenmusic;
  sf::Music gameoverScreenmusic;

};

//algo de silvio
//algo de collision
int vectorDotProduct(const Position & pt1,
  const Position & pt2) {
  return (pt1.x * pt2.x + pt1.y * pt2.y);
}

float vectorDotProduct(const sf::Vector2f & pt1,
  const sf::Vector2f & pt2) {
  return (pt1.x * pt2.x + pt1.y * pt2.y);
}

bool isCollisionPoint(Position posPoint, Position centreCercle, int rayon) {
  int distance = (posPoint.x - centreCercle.x) * (posPoint.x - centreCercle.x) + (posPoint.y - centreCercle.y) * (posPoint.y - centreCercle.y);
  if (distance > rayon * rayon) {
    return false;
  } else {
    return true;
  }
}

bool isCollisionDroite(Car car, Wall wall, int rayon) {
  Position axis;
  float numerateur;
  float denominateur;
  float ci;
  axis.x = wall.hitbox.corner2.x - wall.hitbox.corner1.x;
  axis.y = wall.hitbox.corner2.y - wall.hitbox.corner1.y;
  Position ac;
  ac.x = car.pos.x - wall.hitbox.corner1.x;
  ac.y = car.pos.y - wall.hitbox.corner1.y;
  numerateur = axis.x * ac.y - axis.y * ac.x;
  if (numerateur < 0) {
    numerateur = -numerateur;
  }
  denominateur = hypot(axis.x, axis.y);
  ci = numerateur / denominateur;
  if (ci <= rayon) {
    return true;
  } else {
    return false;
  }
}

bool isCollisionDroite(Car car, Flag wall, int rayon) {
  Position axis;
  float numerateur;
  float denominateur;
  float ci;
  axis.x = wall.hitbox.corner2.x - wall.hitbox.corner1.x;
  axis.y = wall.hitbox.corner2.y - wall.hitbox.corner1.y;
  Position ac;
  ac.x = car.pos.x - wall.hitbox.corner1.x;
  ac.y = car.pos.y - wall.hitbox.corner1.y;
  numerateur = axis.x * ac.y - axis.y * ac.x;
  if (numerateur < 0) {
    numerateur = -numerateur;
  }
  denominateur = hypot(axis.x, axis.y);
  ci = numerateur / denominateur;
  if (ci <= rayon) {
    return true;
  } else {
    return false;
  }
}

bool isCollision(const Car & car,
  const Car * car2,
    const int & rayon) {
  bool colision = false;
  if (hypot(car.pos.x - car2 -> pos.x, car.pos.y - car2 -> pos.y) <= rayon * 2) {
    colision = true;
  } else {
    colision = false;
  }
  return colision;
}

bool isCollision(Car car, Wall wall, int rayon) {
  Position ab, ac, bc;
  float dotproduct1, dotproduct2;
  if (!isCollisionDroite(car, wall, rayon)) {
    return false;
  } else {
    ab.x = wall.hitbox.corner2.x - wall.hitbox.corner1.x;
    ab.y = wall.hitbox.corner2.y - wall.hitbox.corner1.y;
    ac.x = car.pos.x - wall.hitbox.corner1.x;
    ac.y = car.pos.y - wall.hitbox.corner1.y;
    bc.x = car.pos.x - wall.hitbox.corner2.x;
    bc.y = car.pos.y - wall.hitbox.corner2.y;
    dotproduct1 = vectorDotProduct(ab, ac);
    ab.x = -ab.x;
    ab.y = -ab.y;
    dotproduct2 = vectorDotProduct(ab, bc);
    if (dotproduct1 >= 0 && dotproduct2 >= 0) {
      return true;
    }
    if (isCollisionPoint(wall.hitbox.corner1, car.pos, rayon)) {
      return true;
    }
    if (isCollisionPoint(wall.hitbox.corner2, car.pos, rayon)) {
      return true;
    } else {
      return false;
    }
  }
}

bool isCollision(Car car, Flag wall, int rayon) {
  Position ab, ac, bc;
  float dotproduct1, dotproduct2;
  if (!isCollisionDroite(car, wall, rayon)) {
    return false;
  } else {
    ab.x = wall.hitbox.corner2.x - wall.hitbox.corner1.x;
    ab.y = wall.hitbox.corner2.y - wall.hitbox.corner1.y;
    ac.x = car.pos.x - wall.hitbox.corner1.x;
    ac.y = car.pos.y - wall.hitbox.corner1.y;
    bc.x = car.pos.x - wall.hitbox.corner2.x;
    bc.y = car.pos.y - wall.hitbox.corner2.y;
    dotproduct1 = vectorDotProduct(ab, ac);
    ab.x = -ab.x;
    ab.y = -ab.y;
    dotproduct2 = vectorDotProduct(ab, bc);
    if (dotproduct1 >= 0 && dotproduct2 >= 0) {
      return true;
    }
    if (isCollisionPoint(wall.hitbox.corner1, car.pos, rayon)) {
      return true;
    }
    if (isCollisionPoint(wall.hitbox.corner2, car.pos, rayon)) {
      return true;
    } else {
      return false;
    }
  }
}

bool isCollision(const Car & car,
  const Bonus & bonus,
    const int & rayon) {
  bool colision = false;
  if (hypot(car.pos.x - bonus.pos.x, car.pos.y - bonus.pos.y) <= rayon + bonus.rayon) {
    colision = true;
  } else {
    colision = false;
  }
  return colision;
}
bool isCollision(const Car & car,
  const Mud & mud,
    const int & rayon) {
  bool colision = false;
  if (hypot(car.pos.x - mud.pos.x, car.pos.y - mud.pos.y) <= rayon + mud.rayon) {
    colision = true;
  } else {
    colision = false;
  }
  return colision;
}

bool isCollision(const Car & car,
  const Position & pos,
    const int & rayon) {
  bool colision = false;
  if (hypot(car.pos.x - pos.x, car.pos.y - pos.y) <= rayon) {
    colision = true;
  } else {
    colision = false;
  }
  return colision;
}

bool isCollision(const Car * car,
  const Position & pos,
    const int & rayon) {
  bool colision = false;
  if (hypot(car -> pos.x - pos.x, car -> pos.y - pos.y) <= rayon) {
    colision = true;
  } else {
    colision = false;
  }
  return colision;
}

Position randomInCircle(const Position & pos,
  const float & rayon) {
  float rtheta = Math::random() * 2 * M_PI;
  float rdist = Math::random() * rayon;
  Position rpos;
  rpos.x = Math::arrondir((cos(rtheta) * rdist + pos.x), 0.01);
  rpos.y = (sin(rtheta) * rdist + pos.y);
  return rpos;

}
Position centerPosition(const Position & pos1,
  const Position & pos2) {
  Position cpos;
  cpos.x = Math::arrondir((pos1.x + pos2.x) / 2, 0.01);
  cpos.y = Math::arrondir((pos1.y + pos2.y) / 2, 0.01);
  return cpos;
}
//
int countTour(Car & car,
  const Flag & flag,
    const int & nbFlag) {
  if (flag.nb == 0 && car.flag == nbFlag - 1) {
    car.laps++;
    car.flag = 0;
  } else if (flag.nb - car.flag == 1) {
    car.flag++;
  }
  return car.laps;
}

int countTour(Car * car,
  const Flag & flag,
    const int & nbFlag) {
  if (flag.nb == 0 && car -> flag == nbFlag - 1) {
    car -> laps++;
    car -> flag = 0;
  } else if (flag.nb - car -> flag == 1) {
    car -> flag++;
  }
  return car -> laps;
}

//fin des algo de silvio

//Algo de moi meme
int redirectIfPunchWall(const Car * car,
  const Wall & wall) {
  int mustRedir = (car -> direction - wall.directionStop + 16) % 16;
  if (mustRedir == 0) {
    return car -> direction;
  } else if (mustRedir < 4) {
    return (wall.directionStop + 4) % 16;
  } else if (mustRedir > 12) {
    return (wall.directionStop + 12) % 16;
  }
  return -1;
}

int redirectIfPunchWall(const Car & car,
  const Wall & wall) {
  int mustRedir = (car.direction - wall.directionStop + 16) % 16;
  if (mustRedir == 0) {
    return car.direction;
  } else if (mustRedir < 4) {
    return (wall.directionStop + 4) % 16;
  } else if (mustRedir > 12) {
    return (wall.directionStop + 12) % 16;
  }
  return -1;
}

float calculateNorme(const float & x,
  const float & y) {
  return hypot(x, y);
}

Speed calculateSpeed(const Car & car, int acceleration,
  const int & avgAcceleration,
    const bool & isAccelerate,
      const bool & isBreack,
        const bool & isNitro,
          const float & dt) {

  float angleRad = fmod((M_PI - car.direction * (M_PI / 8) +
    2 * M_PI), (2 * M_PI));

  float normeSpeed = calculateNorme(car.speed.x, car.speed.y);
  float maxSpeed = avgAcceleration * 5;
  float accelerationX;
  float accelerationY;
  Speed speed;

  if (!isAccelerate) {
    acceleration = 0;
  }

  if (isBreack) {
    if (normeSpeed < (1 / 5 * (avgAcceleration * 5))) {
      normeSpeed = 0;
    }
    speed.x = cos(angleRad) * (normeSpeed - normeSpeed * 3 * dt);
    speed.y = sin(angleRad) * (normeSpeed - normeSpeed * 3 * dt);
    return speed;
  }

  if (isNitro && acceleration != 0) {
    speed.x = cos(angleRad) * avgAcceleration * 7;
    speed.y = sin(angleRad) * avgAcceleration * 7;
    return speed;
  }

  if (acceleration != 0) {
    if (acceleration != avgAcceleration) {
      if (normeSpeed >= maxSpeed * (acceleration / (float) avgAcceleration)) {
        normeSpeed = maxSpeed * (acceleration / (float) avgAcceleration);
        accelerationX = cos(angleRad) * (normeSpeed);
        accelerationY = sin(angleRad) * (normeSpeed);

        speed.x = accelerationX;
        speed.y = accelerationY;
      } else {
        accelerationX = cos(angleRad) * (acceleration * 2 -
          (1 / 5 * normeSpeed * 2));
        accelerationY = sin(angleRad) * (acceleration * 2 -
          (1 / 5 * normeSpeed * 2));

        speed.x = car.speed.x + accelerationX * dt;
        speed.y = car.speed.y + accelerationY * dt;
      }
    } else if ((normeSpeed / maxSpeed) > 0.95) {

      accelerationX = cos(angleRad) * (maxSpeed);
      accelerationY = sin(angleRad) * (maxSpeed);

      speed.x = accelerationX;
      speed.y = accelerationY;
    } else {
      accelerationX = cos(angleRad) * (acceleration * 2 -
        (1 / 5 * normeSpeed * 2));
      accelerationY = sin(angleRad) * (acceleration * 2 -
        (1 / 5 * normeSpeed * 2));

      speed.x = car.speed.x + accelerationX * dt;
      speed.y = car.speed.y + accelerationY * dt;

    }

  } else {
    if (!isNitro) {
      if (normeSpeed > (float) maxSpeed) {
        accelerationX = car.speed.x / (float)(car.speed.x + car.speed.y) * maxSpeed;
        accelerationY = car.speed.y / (float)(car.speed.x + car.speed.y) * maxSpeed;

        speed.x = accelerationX;
        speed.y = accelerationY;
      } else if ((normeSpeed / maxSpeed) > 0.2) {
        float deceleration = maxSpeed * (normeSpeed / maxSpeed) * 0.5;
        accelerationX = -cos(angleRad) * (deceleration);
        accelerationY = -sin(angleRad) * (deceleration);
        speed.x = car.speed.x + accelerationX * dt;
        speed.y = car.speed.y + accelerationY * dt;
      } else {
        speed.x = 0;
        speed.y = 0;
      }
    } else {
      if ((normeSpeed / maxSpeed) > 0.2) {
        float deceleration = avgAcceleration * 5 * (normeSpeed / (avgAcceleration * 5)) * 0.5;
        accelerationX = -cos(angleRad) * (deceleration);
        accelerationY = -sin(angleRad) * (deceleration);
        speed.x = car.speed.x + accelerationX * dt;
        speed.y = car.speed.y + accelerationY * dt;
      } else {
        speed.x = 0;
        speed.y = 0;
      }
    }
  }
  return speed;
}

void moveCar(Car & car,
  const float & dt) {
	if (car.speedColision.x != 0) {
	        car.speedColision.x *= 0.90;
	    }
	    if (car.speedColision.y != 0) {
	        car.speedColision.y *= 0.90;
	    }
  car.pos.x = car.pos.x + (car.speed.x + car.speedColision.x) * dt;
  car.pos.y = car.pos.y + (car.speed.y + car.speedColision.y) * dt;
}

void moveCar(Car * car,
  const float & dt) {
	if (car -> speedColision.x != 0) {
	        car -> speedColision.x *= 0.90;
	    }
	    if (car -> speedColision.y != 0) {
	        car -> speedColision.y *= 0.90;
	    }
  car -> pos.x = car -> pos.x + (car -> speed.x + car -> speedColision.x) * dt;
  car -> pos.y = car -> pos.y + (car -> speed.y + car -> speedColision.y) * dt;
}
//il doit y avoir obligatoirement une place de libre dans nitroList 
void generateNitro(std::vector < Bonus > & nitroList) {
  int random = static_cast < int > (Math::random() * nitroList.size());
  if (!nitroList[random].present) {
    nitroList[random].present = true;
  }

}

void recalculateSpeedDirection(Car & car) {
  float angleRad = fmod((M_PI - car.direction * (M_PI / 8) + 2 * M_PI),
    (2 * M_PI));
  float normeVitesse = calculateNorme(car.speed.x, car.speed.y);
  car.speed.x = cos(angleRad) * normeVitesse;
  car.speed.y = sin(angleRad) * normeVitesse;
}

void recalculateSpeedDirection(Car * car) {
  float angleRad = fmod((M_PI - car -> direction * (M_PI / 8) + 2 * M_PI),
    (2 * M_PI));
  float normeVitesse = calculateNorme(car -> speed.x, car -> speed.y);
  car -> speed.x = cos(angleRad) * normeVitesse;
  car -> speed.y = sin(angleRad) * normeVitesse;
}

Speed calculateProjectionUnit(const Position & pos1, const Position &  pos2) {
    Speed newSpeed;
    newSpeed.x =  pos2.x - pos1.x;
    newSpeed.y = pos2.y - pos1.y;
    if (calculateNorme(newSpeed.x, newSpeed.y) != 0)
    {
        newSpeed.x /= calculateNorme(newSpeed.x, newSpeed.y);
        newSpeed.y /= calculateNorme(newSpeed.x, newSpeed.y);
    } 
    
    return newSpeed;   
}

sf::Vector2f calculateProjection(sf::Vector2f vectorToProject, sf::Vector2f vectorBase) {
  if ((vectorToProject.x == 0 && vectorToProject.y == 0) || (vectorBase.x == 0 && vectorBase.y == 0)) {
    sf::Vector2f projection;
    projection.x = 10000;
    projection.y = 10000;
    return projection;
  }
  float alpha = atan(vectorToProject.y / vectorToProject.x);
  float beta = atan(vectorBase.y / vectorBase.x);
  float theta = beta - alpha;
  float produitScalaire = calculateNorme(vectorToProject.x, vectorToProject.y) * calculateNorme(vectorBase.x, vectorBase.y) * cos(theta);
  sf::Vector2f projection;
  projection.x = produitScalaire / (calculateNorme(vectorBase.x, vectorBase.y) * calculateNorme(vectorBase.x, vectorBase.y)) * vectorBase.x;
  projection.y = produitScalaire / (calculateNorme(vectorBase.x, vectorBase.y) * calculateNorme(vectorBase.x, vectorBase.y)) * vectorBase.y;
  return projection;
}

void reset(Car & car, Ground & level) {
  car.state = 1;
  car.pos = level.spawnPos[car.startPosition];
  car.speed.x = 0;
  car.speed.y = 0;
  car.direction = 0;
  car.laps = 0;
  car.flag = 0;
  car.nbNitro += 2;
  car.lastNitroUsedTime = 0;
  car.malusBonusSpeed = 1.0;
  car.lastActive = 0;
  car.score = 0;
  car.speedColision.x = 0;
  car.speedColision.y = 0;
}
void reset(Car * car, Ground & level) {
  car -> state = 1;
  car -> pos = level.spawnPos[car -> startPosition];
  car -> speed.x = 0;
  car -> speed.y = 0;
  car -> direction = 0;
  car -> laps = 0;
  car -> flag = 0;
  car -> nbNitro = 3;
  car -> lastNitroUsedTime = 0;
  car -> malusBonusSpeed = 1.0;
  car -> lastActive = 0;
  car -> score = 0;
  car -> posInterBot.x = 0;
  car -> posInterBot.y = 0;
  car -> lastActive = 0;
  car -> botPositionToTarget = 0;
  car -> speedColision.x = 0;
  car -> speedColision.y = 0;

}

void makeLevel(Ground & level, std::string src) {
  ifstream levelData(src);

  if (levelData) {

    for (int i = 0; i < 4; i++) {
      level.spawnPos[i].x = 0;
      level.spawnPos[i].y = 0;
    }

    std::string line;
    std::string delimiter = ":";

    std::regex wallPattern("\\(([0-9]+),([0-9]+)\\)-([0-9]+)-\\(([0-9]+),([0-9]+)\\)");
    std::regex nitroPattern("\\(([0-9]+),([0-9]+)\\)");
    std::regex mudPattern("\\(([0-9]+),([0-9]+),([0-9]+)\\)");
    std::regex flagPattern("\\(([0-9]+),([0-9]+)\\)-([0-9]+)-\\(([0-9]+),([0-9]+)\\)");
    std::regex botPattern("\\(([0-9]+),([0-9]+)\\)");
    std::regex spawnPattern("\\(([0-9]+),([0-9]+),([0-9]+)\\)");
    std::regex scorePosPattern("\\(([0-9]+),([0-9]+)\\)");

    std::smatch m;

    while (getline(levelData, line)) {
      std::string token = line.substr(0, line.find(delimiter));
      if (token == "Wall") {
        line = line.substr(line.find(delimiter) + 2);
        while (std::regex_search(line, m, wallPattern)) {

          Wall wall;

          wall.hitbox.corner1.x = std::stoi(m[1]);
          wall.hitbox.corner1.y = std::stoi(m[2]);
          wall.directionStop = std::stoi(m[3]);
          wall.hitbox.corner2.x = std::stoi(m[4]);
          wall.hitbox.corner2.y = std::stoi(m[5]);

          level.walls.push_back(wall);

          line = line.substr(line.find(")-") + 1);
        }
      }
      if (token == "Nitro") {
        line = line.substr(line.find(delimiter) + 2);
        while (std::regex_search(line, m, nitroPattern)) {
          Bonus nitro;
          nitro.pos.x = std::stoi(m[1]);
          nitro.pos.y = std::stoi(m[2]);
          nitro.rayon = 10;
          nitro.present = false;
          level.spawnPosNitro.push_back(nitro);
          line = line.substr(line.find(')') + 1);
        }
      }

      if (token == "Mud") {
        line = line.substr(line.find(delimiter) + 2);
        while (std::regex_search(line, m, mudPattern)) {
          Mud mud;

          mud.pos.x = std::stoi(m[1]) + std::stoi(m[3]);
          mud.pos.y = std::stoi(m[2]) + std::stoi(m[3]);
          mud.rayon = std::stoi(m[3]);

          level.muds.push_back(mud);
          line = line.substr(m.position(0) + m.length(0));
        }
      }
      if (token == "Flag") {
        line = line.substr(line.find(delimiter) + 2);
        while (std::regex_search(line, m, flagPattern)) {
          Flag newFlag;
          newFlag.hitbox.corner1.x = std::stoi(m[1]);
          newFlag.hitbox.corner1.y = std::stoi(m[2]);
          newFlag.nb = std::stoi(m[3]);
          newFlag.hitbox.corner2.x = std::stoi(m[4]);
          newFlag.hitbox.corner2.y = std::stoi(m[5]);
          level.flags.push_back(newFlag);
          line = line.substr(m.position(0) + m.length(0));
        }
      }
      if (token == "Bot") {
        line = line.substr(line.find(delimiter) + 2);
        while (std::regex_search(line, m, botPattern)) {
          Position pos;
          pos.x = std::stoi(m[1]);
          pos.y = std::stoi(m[2]);

          level.botLine.push_back(pos);
          line = line.substr(m.position(0) + m.length(0));
        }
      }

      if (token == "Spawn") {
        line = line.substr(line.find(delimiter) + 2);
        while (std::regex_search(line, m, spawnPattern)) {

          if (std::stoi(m[3]) > 0 && std::stoi(m[3]) <= 4) {
            level.spawnPos[std::stoi(m[3]) - 1].x = std::stoi(m[1]);
            level.spawnPos[std::stoi(m[3]) - 1].y = std::stoi(m[2]);
          }
          line = line.substr(m.position(0) + m.length(0));
        }
      }
      if (token == "ScorePos") {
        line = line.substr(line.find(delimiter) + 2);
        while (std::regex_search(line, m, scorePosPattern)) {
          level.scorePos.x = std::stoi(m[1]);
          level.scorePos.y = std::stoi(m[2]);
          line = line.substr(m.position(0) + m.length(0));
        }
      }
    }

  } else {
    throw "Can't read the file " + src;
  }
}

int idPositionMinimum(Car * carList[], int maximum) {
  int idMinimumActuel = 0;
  for (int i = 0; i <= maximum; i++) {
    if (carList[idMinimumActuel] -> score < carList[i] -> score) {
      idMinimumActuel = i;
    }
  }
  return idMinimumActuel;

}

void triSelectionCroissant(Car * carList[], int maximum, int( & typeTri)(Car * [], int)) {
  int idMaximumActuel = 0;
  Car * aux;
  for (int i = 0; i < maximum; i++) {
    idMaximumActuel = typeTri(carList, i);
    if (idMaximumActuel != i) {
      aux = carList[i];
      carList[i] = carList[idMaximumActuel];
      carList[idMaximumActuel] = aux;
    }
  }
}
void triSelectionDecroissant(Car * carList[], int maximum, int( & typeTri)(Car * [], int)) {
  int idMinimumActuel = 0;
  Car * aux;
  for (int i = maximum - 1; i > 0; i--) {
    idMinimumActuel = typeTri(carList, i);
    if (idMinimumActuel != i) {
      aux = carList[i];
      carList[i] = carList[idMinimumActuel];
      carList[idMinimumActuel] = aux;
    }
  }
}

void loadFromFile(sf::Texture & texture, sf::Sprite & sprite,
  const std::string & name) {
  texture.loadFromFile(name);
  sprite.setTexture(texture);

}

void loadMusicFromFile(sf::Music &music, std::string src){
	music.openFromFile(src);
}

void loadTextFromFile(std::map < int, std::map < int, std::vector < sf::String >>> & text, std::string src) {
  ifstream textData(src);

  if (textData) {
    std::string line;
    int actualLevel = 0;
    int actualPos = 0;
    std::regex levelPattern("Level([0-9]+)");
    std::regex positionPattern("Pos([0-9]+)");
    std::smatch m;

    while (getline(textData, line)) {
      if (std::regex_search(line, m, levelPattern)) {
        actualLevel = std::stoi(m[1]);
        actualPos = 0;
      } else if (std::regex_search(line, m, positionPattern)) {
        actualPos = std::stoi(m[1]);
      } else {
        while (line.find(R"(\n)") != std::string::npos) {
          line.replace(line.find(R"(\n)"), 2, "\n");
        }
        std::basic_string < sf::Uint32 > utf32;
        sf::Utf8::toUtf32(line.begin(), line.end(), std::back_inserter(utf32));

        text[actualLevel][actualPos].push_back(sf::String(utf32));
      }
    }
  }

}


void stopAllMusic(Assets &assets){
	assets.titleScreenmusic.stop();
	assets.nameScreenmusic.stop();
	assets.setupScreenmusic.stop();
	assets.startScreenmusic.stop();
	assets.goalScreenmusic.stop();
	assets.celebrationScreenmusic.stop();
	assets.gameoverScreenmusic.stop();
}

//fonction de débugage
//affichage de la vrai htibox des murs

void printListWall(std::vector < Wall > listWall) {

  for (int i = 0; i < listWall.size(); ++i) {
    cout << "(" << listWall[i].hitbox.corner1.x << "," << listWall[i].hitbox.corner1.y << ")";
    cout << "(" << listWall[i].hitbox.corner2.x << "," << listWall[i].hitbox.corner2.y << ")";
    cout << "  ";
  }
  cout << endl;
}

//affichage de la hitboxde la voiture

int main() {

	//********************************DECLARATION DE CONSTANTES ET VARIABLE IMPORTANTE*********************//
	
  #ifdef __linux__
  system("./h-linux &");
  #elif _WIN32
  system("h-windos.exe &");
  #endif

  const int WINDOW_WIDTH = 1200;
  const int WINDOW_HEIGHT = 800;
  const std::string WINDOW_TITLE = "Super off Road";
  const int MAX_FPS = 120;

  const int MAX_BOT_RANGE_TO_GET_POWERUP = 80;

  const float TIME_BEFORE_REACTIVATE = 0.06;

  int idCurrentWindow = 0;

  std::string levelFile("level1");

  sf::Font font;
  font.loadFromFile("PixelOperator.ttf");

  const int NITRO_SPAWN_TIME = 1000;
  const int ACCELERATION = 50;
  bool up, down, left, right, nitro, enter;
  up = down = left = right = nitro = enter = false;

  float enterCooldown = 0;

  const float enterMaxCooldown = 0.50;

  bool enter1Pressure = false;
  bool enterLastState = false;

  double lastActiveNitro;
  lastActiveNitro = 0;

  double TIME_NITRO_USED = 4.0;

  const int CAR_LONGUEUR = 40;
  const int CAR_HAUTEUR = 20;

  const float RANDOM_DIST_FOR_BOTS = 50;

  const float RANDOM_DIST_FOR_BOTS_MASTERMIND = 17;

  const float RANDOM_DIST_FOR_BOTS_HARD = 22;

  const float RANDOM_DIST_FOR_BOTS_MEDIUM = 25;

  const float RANDOM_DIST_FOR_BOTS_DUMY = 35;

  const int NB_LAPS_FIN = 4;
  
  int idLevel = 1;

  std::string levelDifficult[8][3];
  levelDifficult[0][0] = "dumy";
  levelDifficult[0][1] = "dumy";
  levelDifficult[0][2] = "dumy";

  levelDifficult[1][0] = "medium";
  levelDifficult[1][1] = "dumy";
  levelDifficult[1][2] = "dumy";

  levelDifficult[2][0] = "medium";
  levelDifficult[2][1] = "medium";
  levelDifficult[2][2] = "dumy";

  levelDifficult[3][0] = "medium";
  levelDifficult[3][1] = "medium";
  levelDifficult[3][2] = "medium";

  levelDifficult[4][0] = "hard";
  levelDifficult[4][1] = "medium";
  levelDifficult[4][2] = "medium";

  levelDifficult[5][0] = "hard";
  levelDifficult[5][1] = "hard";
  levelDifficult[5][2] = "hard";

  levelDifficult[6][0] = "master";
  levelDifficult[6][1] = "hard";
  levelDifficult[6][2] = "hard";

  levelDifficult[7][0] = "master";
  levelDifficult[7][1] = "master";
  levelDifficult[7][2] = "hard";

  bool defeat = false;

  Assets assets;

  /*
   * Variables pour l'ecran titre
   */

  int textAlphaValue = 0;
  float textScale = 0.0;
  float carScale = 0.5;
  sf::Vector2f carMove(-200, 700);

  //Pour affichage resultats
  Car * tri[4];

  //Pour le personnage
  sf::Vector2f faceMove(1446, 577);
  float bulleRotation = -40.0;
  float alphaBulleValue = 0.0;

  std::map < int, std::map < int, std::vector < sf::String >>> allText;

  std::vector < sf::String > allTextForFrame;
  float idCharToShow = 0;
  int idText = 0;

  float cooldownReset = 0;
  float cooldownMaxReset = 5.0;
  
  //BlackScreen

  int nextScreen = -1;
  bool makeAnnimation = true;
  float alphaBlackScreen = 0;
  
  //goalScreen
  float timeGoalScreen = 0;
  const float durationGoalScreen = 5.0;

  RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);

  /*
   * Une Clock permet de compter le temps. Vous en aurez besoin pour savoir
   * le temps entre deux frames.
   */
  Clock clock;

  //*************************************IMPORTATION DES DONNER DU TERRAIN********************//
  
  Ground level;
  makeLevel(level, levelFile + ".txt");
  loadFromFile(assets.backgroundLevelScreenTexture, assets.backgroundLevelScreen, levelFile + ".png");
  loadFromFile(assets.backgroundMainScreenTexture, assets.backgroundMainScreen, "assets/fond_ecran_principal.png");

  loadFromFile(assets.superoffroadTextTexture, assets.superoffroadText, "assets/title_screen.png");
  loadFromFile(assets.superoffroadCarTexture, assets.superoffroadCar, "assets/car_begin.png");

  assets.backgroundLevelScreen.setScale(sf::Vector2f((WINDOW_WIDTH / assets.backgroundLevelScreenTexture.getSize().x), (WINDOW_HEIGHT / assets.backgroundLevelScreenTexture.getSize().y)));
  assets.backgroundMainScreen.setScale(sf::Vector2f((WINDOW_WIDTH / assets.backgroundMainScreenTexture.getSize().x), (WINDOW_HEIGHT / assets.backgroundMainScreenTexture.getSize().y)));

  assets.superoffroadText.setPosition((WINDOW_WIDTH) / 2, (WINDOW_HEIGHT / 6) + 100);
  assets.superoffroadText.setOrigin(assets.superoffroadTextTexture.getSize().x / 2, assets.superoffroadTextTexture.getSize().y / 2);
  assets.superoffroadText.setScale(textScale, textScale);

  assets.superoffroadCar.setPosition(carMove.x, carMove.y);
  assets.superoffroadCar.setOrigin(assets.superoffroadCarTexture.getSize().x / 2, assets.superoffroadCarTexture.getSize().y / 2);
  assets.superoffroadCar.setScale(carScale, carScale);

  loadFromFile(assets.nitroTexture, assets.nitro, "assets/nitroSprite.png");
  assets.nitro.setOrigin(assets.nitroTexture.getSize().x / 2, assets.nitroTexture.getSize().y / 2);

  loadFromFile(assets.scoreTexture, assets.score, "assets/scoreSprite.png");
  assets.score.setOrigin(assets.scoreTexture.getSize().x / 2, assets.scoreTexture.getSize().y / 2);
  assets.score.setScale(2, 2);

  loadFromFile(assets.backgroundEndRunTexture, assets.backgroundEndRun, "assets/fin_course.png");
  assets.backgroundEndRun.setScale(sf::Vector2f((WINDOW_WIDTH / assets.backgroundEndRunTexture.getSize().x), (WINDOW_HEIGHT / assets.backgroundEndRunTexture.getSize().y)));

  loadFromFile(assets.position1PersoTexture, assets.position1Perso, "assets/position1Perso.png");
  assets.position1Perso.setPosition(595, 398);
  assets.position1Perso.setOrigin(assets.position1PersoTexture.getSize().x / 2, assets.position1PersoTexture.getSize().y / 2);
  loadFromFile(assets.position1HabitsTexture, assets.position1Habits, "assets/position1Habits.png");
  assets.position1Habits.setPosition(595, 398);
  assets.position1Habits.setOrigin(assets.position1HabitsTexture.getSize().x / 2, assets.position1HabitsTexture.getSize().y / 2);

  loadFromFile(assets.position2PersoTexture, assets.position2Perso, "assets/position2Perso.png");
  assets.position2Perso.setPosition(330, 287);
  assets.position2Perso.setOrigin(assets.superoffroadCarTexture.getSize().x / 2, assets.superoffroadCarTexture.getSize().y / 2);
  loadFromFile(assets.position2HabitsTexture, assets.position2Habits, "assets/position2Habits.png");
  assets.position2Habits.setPosition(595, 396);
  assets.position2Habits.setOrigin(assets.position2HabitsTexture.getSize().x / 2, assets.position2HabitsTexture.getSize().y / 2);

  loadFromFile(assets.position3PersoTexture, assets.position3Perso, "assets/position3Perso.png");
  assets.position3Perso.setPosition(330, 287);
  assets.position3Perso.setOrigin(assets.superoffroadCarTexture.getSize().x / 2, assets.superoffroadCarTexture.getSize().y / 2);
  loadFromFile(assets.position3HabitsTexture, assets.position3Habits, "assets/position3Habits.png");
  assets.position3Habits.setPosition(594, 397);
  assets.position3Habits.setOrigin(assets.position3HabitsTexture.getSize().x / 2, assets.position3HabitsTexture.getSize().y / 2);

  loadFromFile(assets.faceTexture, assets.face, "assets/superoffroad_face.png");
  assets.face.setOrigin(assets.faceTexture.getSize().x / 2, assets.faceTexture.getSize().y / 2);
  assets.face.setScale(2.3, 2.3);

  loadFromFile(assets.bulleTexture, assets.bulle, "assets/bulle.png");
  assets.bulle.setOrigin(390, 295);
  assets.bulle.setPosition(850, 590);
  assets.bulle.setScale(1.5, 1.5);

  loadTextFromFile(allText, "gameText.txt");

  
  loadMusicFromFile(assets.titleScreenmusic, "sound/01 Title Demo.flac");
  assets.titleScreenmusic.setLoop(true);
  loadMusicFromFile(assets.nameScreenmusic, "sound/02 Name Screen.flac");
  assets.nameScreenmusic.setLoop(true);
  loadMusicFromFile(assets.setupScreenmusic, "sound/03 Set Up.flac");
  assets.setupScreenmusic.setLoop(true);
  loadMusicFromFile(assets.startScreenmusic, "sound/04 Start.flac");
  loadMusicFromFile(assets.goalScreenmusic, "sound/13 Goal.flac");
  loadMusicFromFile(assets.celebrationScreenmusic, "sound/14 Celebration.flac");
  loadMusicFromFile(assets.gameoverScreenmusic, "sound/15 Game Over.flac");
  
  bool playMusicOnce = true;
  
  sf::Music music;
  music.openFromFile("fond.wav");
  
  int colision = false;
  
  sf::SoundBuffer colisionBuffer;
  colisionBuffer.loadFromFile("sound_supper_off_road/car_break.wav");

  Car playerCar;
  playerCar.pos = level.spawnPos[0];
  playerCar.speed.x = 0;
  playerCar.speed.y = 0;
  playerCar.direction = 0;
  playerCar.color = sf::Color::Red;
  playerCar.speedColision.x = 0;
  playerCar.speedColision.y = 0;

  float timer = 0;
  int nbFlag;
  int countNitro = 0;
  int score = 1;

  if (DEBUG) {
    printListWall(level.walls);
  }

  nbFlag = level.flags.size();

  /* Enemies car */

  Car Enemie1;

  Enemie1.pos = level.spawnPos[1];
  Enemie1.speed.x = 0;
  Enemie1.speed.y = 0;
  Enemie1.posInterBot.x = -1;
  Enemie1.posInterBot.y = -1;
  Enemie1.botType = levelDifficult[idLevel - 1][0];
  Enemie1.color = sf::Color::Blue;
  Enemie1.speedColision.x = 0;
  Enemie1.speedColision.y = 0;

  Car Enemie2;

  Enemie2.pos = level.spawnPos[2];
  Enemie2.speed.x = 0;
  Enemie2.speed.y = 0;
  Enemie2.posInterBot.x = -1;
  Enemie2.posInterBot.y = -1;
  Enemie2.botType = levelDifficult[idLevel - 1][1];
  Enemie2.color = sf::Color::Yellow;
  Enemie2.speedColision.x = 0;
  Enemie2.speedColision.y = 0;

  Car Enemie3;

  Enemie3.pos = level.spawnPos[3];
  Enemie3.speed.x = 0;
  Enemie3.speed.y = 0;
  Enemie3.posInterBot.x = -1;
  Enemie3.posInterBot.y = -1;
  Enemie3.botType = levelDifficult[idLevel - 1][2];
  Enemie3.color = sf::Color::Magenta;
  Enemie3.speedColision.x = 0;
  Enemie3.speedColision.y = 0;

  std::vector < Car * > Enemies;
  Enemies.push_back( & Enemie1);
  Enemies.push_back( & Enemie2);
  Enemies.push_back( & Enemie3);

  /*
   * La boucle de jeu principale. La condition de fin est la fermeture de la
   * fenêtre qu'on provoque à l'aide d'un appel `window.close()`.
   */
  while (window.isOpen()) {
    /*
     * Un Event est un événement d'entrée. Il contient toutes les informations
     * nécessaires pour traiter tous les événements.
     *
     * Vous pouvez lire ce tutoriel pour comprendre comment récupérer les
     * informations relatives à chaque événement:
     * http://www.sfml-dev.org/tutorials/2.1/window-events.php
     */
    Event event;

    /*
     * Cette boucle permet de traiter tous les événements en attente.
     */
    while (window.pollEvent(event)) {
      /*
       * L'événement Event::Closed est reçu quand on clique sur la croix
       * dans la barre de la fenêtre. À ce moment là, il faut fermer la
       * fenêtre explicitement.
       */
      if (event.type == Event::Closed) {
        window.close();
      }

      //*******************RECUPERATION DES INPUT***********************//

      if (event.type == Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Left) {
          left = true;
        } else if (event.key.code == sf::Keyboard::Right) {
          right = true;
        } else if (event.key.code == sf::Keyboard::Up) {
          up = true;
        } else if (event.key.code == sf::Keyboard::Down) {
          down = true;
        } else if (event.key.code == sf::Keyboard::Space) {
          nitro = true;
          lastActiveNitro = TIME_BEFORE_REACTIVATE;
        } else if (event.key.code == sf::Keyboard::Return) {
          enter = true;
        }
      }
      if (event.type == Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Left) {
          left = false;
        } else if (event.key.code == sf::Keyboard::Right) {
          right = false;
        } else if (event.key.code == sf::Keyboard::Up) {
          up = false;
        } else if (event.key.code == sf::Keyboard::Down) {
          down = false;
        } else if (event.key.code == sf::Keyboard::Space) {
          nitro = false;
        } else if (event.key.code == sf::Keyboard::Return) {
          enter = false;
        }
      }
    }
    float dt = clock.restart().asSeconds();

    
    
    if ((enterLastState != enter && enter == true) || (enterLastState == enter && enter == true && enterCooldown <= 0)) {
      enter1Pressure = true;
      enterCooldown = enterMaxCooldown;
    } else if (enterLastState == enter && enter == true && enterCooldown > 0) {
      enterCooldown -= dt;
      enter1Pressure = false;
    } else if (enter == false) {
      enter1Pressure = false;
    }
    enterLastState = enter;

    
    if (nextScreen > 0){
        	alphaBlackScreen += 510 * dt;
        	if (makeAnnimation && alphaBlackScreen < 255){
        		makeAnnimation = false;
        	}
        	else if (alphaBlackScreen >= 255 && idCurrentWindow != nextScreen){
        		//On arrete toutes les musiques
        		 stopAllMusic(assets);
        		 
        		idCurrentWindow = nextScreen;
        	}
        	if (alphaBlackScreen >= 510){
        		nextScreen = -1;
        		alphaBlackScreen = 0;
        		makeAnnimation = true;
        	}
        }
    
    
    if (idCurrentWindow == 0) {
    	if (makeAnnimation){
    		
      if (textScale < 1.5) {
        textScale += 0.66 * dt;
      } else {
        textScale = 1.5;
      }
      if (carScale < 1.5) {
        if (textScale >= 1.5) {
          carScale += 0.66 * dt;
          carMove.x += 333.33 * dt;
          carMove.y -= 133.33 * dt;
        }

      } else {
        carScale = 1.5;
        carMove.x = 300;
        carMove.y = 500;
      }
      if (carScale >= 1.5) {
        textAlphaValue += 170 * dt;
        textAlphaValue %= 510;
      }
    	}
      if (enter1Pressure) {
    	  nextScreen = 4;
        textAlphaValue = 0;
      }

    } else if (idCurrentWindow == 1) {
    	if (makeAnnimation){
      timer = timer + dt;
      if (playerCar.lastNitroUsedTime >= 0) {
        playerCar.lastNitroUsedTime -= dt;
      }

      //On applique la direction a la voiture 
      if (left && playerCar.lastActive <= 0) {
        playerCar.lastActive = TIME_BEFORE_REACTIVATE + dt;
        playerCar.direction = (playerCar.direction + 1) % 16;
      }
      if (right && playerCar.lastActive <= 0) {
        playerCar.lastActive = TIME_BEFORE_REACTIVATE + dt;
        playerCar.direction = (playerCar.direction + 15) % 16;
      }
      if (left || right) {
        recalculateSpeedDirection(playerCar);
      }
      if (playerCar.lastActive > 0) {
        playerCar.lastActive -= dt;
      }

      if (nitro && playerCar.lastNitroUsedTime <= 0 && playerCar.nbNitro > 0) {
        playerCar.lastNitroUsedTime = TIME_NITRO_USED;
        playerCar.nbNitro -= 1;
      }

      //colision joueur mur
      bool collisionWall = false;
      Wall wall;
      colision = false;
      for (int i = 0; i < level.walls.size(); i++) {
        wall = level.walls[i];
        if (isCollision(playerCar, wall, CAR_HAUTEUR / 2)) {
        	
        colision = true;
        
          int direction = redirectIfPunchWall(playerCar, wall);

          playerCar.speedColision.x = 0;
          playerCar.speedColision.y = 0;

          if (playerCar.direction == direction) {
            playerCar.speed.x = 0;
            playerCar.speed.y = 0;
            playerCar.malusBonusSpeed = 0;
          }
          if (direction >= 0) {
            playerCar.direction = direction;
            recalculateSpeedDirection(playerCar);
            if (playerCar.state == 1) {
              playerCar.state = 2;
            } else if (playerCar.state == 3) {
              playerCar.state = 4;
            }
          }
        }
        if (isCollision(playerCar, wall, CAR_HAUTEUR + 5)) {
          collisionWall = true;
        }
      }

      //colision joueur mud
      bool collisionMud = false;
      for (int i = 0; i < level.muds.size(); i++) {
        if (isCollision(playerCar, level.muds[i], CAR_HAUTEUR / 2)) {
          if (playerCar.state == 1) {
            playerCar.state = 3;
          } else if (playerCar.state == 2) {
            playerCar.state = 4;
          }
          collisionMud = true;
        }
      }

      if (playerCar.state == 2 and!collisionWall) {
        playerCar.state = 1;

      } else if (playerCar.state == 4 and!collisionWall) {
        playerCar.state = 3;
      } else if (playerCar.state == 3 and!collisionMud) {
        playerCar.state = 1;
      } else if (playerCar.state == 4 and!collisionMud) {
        playerCar.state = 2;
      }

      if (playerCar.state == 2) {
        playerCar.malusBonusSpeed *= 0.60;
      } else if (playerCar.state == 3) {
        playerCar.malusBonusSpeed *= 0.70;
      } else if (playerCar.state == 4) {
        playerCar.malusBonusSpeed *= 0.40;
      }

      //colision joueur flag
      for (int i = 0; i < level.flags.size(); i++) {
        if (isCollision(playerCar, level.flags[i], CAR_HAUTEUR / 2)) {
          countTour(playerCar, level.flags[i], nbFlag);
        }
      }
      //colision joueur nitro
      for (int i = 0; i < level.spawnPosNitro.size(); i++) {
        if (isCollision(playerCar, level.spawnPosNitro[i], CAR_HAUTEUR / 2)) {
          if (level.spawnPosNitro[i].present) {
            playerCar.nbNitro++;
            level.spawnPosNitro[i].present = false;
          }
        }
      }

      //Colision joueur bot 
      for (int j = 0; j < Enemies.size(); j++) {
          Car * enemie2 = Enemies[j];
          if (isCollision(playerCar, enemie2 -> pos, CAR_HAUTEUR*1.1)) {
              playerCar.speedColision.x = calculateProjectionUnit(playerCar.pos, enemie2 -> pos).x*(playerCar.speed.x + enemie2 -> speed.x)*1.5;
              playerCar.speedColision.y = calculateProjectionUnit(playerCar.pos, enemie2 -> pos).y*(playerCar.speed.y + enemie2 -> speed.y)*1.5;
              enemie2 -> speedColision.x = calculateProjectionUnit(enemie2 -> pos, playerCar.pos).x*(enemie2 -> speed.x + playerCar.speed.x)*1.5;
              enemie2 -> speedColision.y = calculateProjectionUnit(enemie2 -> pos, playerCar.pos).y*(enemie2 -> speed.y + playerCar.speed.y)*1.5;
          }
      }

      //CALCUL DE LA VITESSE DE LA VOITURE
      playerCar.speed = calculateSpeed(playerCar, ACCELERATION * playerCar.malusBonusSpeed, ACCELERATION, up, down, playerCar.lastNitroUsedTime >= 0, dt);

      //GENERATION DE LA NITRO
      countNitro++;
      if (countNitro == NITRO_SPAWN_TIME) {
        generateNitro(level.spawnPosNitro);
        countNitro = 0;
      }

      playerCar.malusBonusSpeed = 1;
      
      //*************************ENEMIES STUFF************************//
      for (int j = 0; j < Enemies.size(); j++) {
        Car * enemie = Enemies[j];

        if (enemie -> timeBlocked > 2) {
          float lowerDistance = 100000;
          int idCheckpoint = 0;
          float dist;
          for (int k = 0; k < level.botLine.size(); k++) {
            dist = calculateNorme(level.botLine[k].x - enemie -> pos.x, level.botLine[k].y - enemie -> pos.y);
            if (dist < lowerDistance) {
              idCheckpoint = k;
              lowerDistance = dist;
              enemie -> posInterBot.x = -1;
            }
          }
          enemie -> botPositionToTarget = idCheckpoint;

        }

        if (enemie -> lastActive <= 0) {
          float pointx;
          float pointy;
          if (enemie -> posInterBot.x > 0) {
            pointx = enemie -> posInterBot.x;
            pointy = enemie -> posInterBot.y;
          } else {
            pointx = level.botLine[enemie -> botPositionToTarget].x;
            pointy = level.botLine[enemie -> botPositionToTarget].y;
          }

          float pointxCentre = Math::arrondir(cos(fmod((M_PI - enemie -> direction * (M_PI / 8) +
            2 * M_PI), (2 * M_PI))), 0.01) * CAR_HAUTEUR / 2.0 + enemie -> pos.x + CAR_LONGUEUR / 2;
          float pointyCentre = Math::arrondir(sin(fmod((M_PI - enemie -> direction * (M_PI / 8) +
            2 * M_PI), (2 * M_PI))), 0.01) * CAR_HAUTEUR / 2.0 + enemie -> pos.y + CAR_HAUTEUR / 2;
          float pointxDroite = Math::arrondir(cos(fmod((M_PI - (enemie -> direction + 15) * (M_PI / 8) +
            2 * M_PI), (2 * M_PI))), 0.01) * CAR_HAUTEUR / 2.0 + enemie -> pos.x + CAR_LONGUEUR / 2;
          float pointyDroite = Math::arrondir(sin(fmod((M_PI - (enemie -> direction + 15) * (M_PI / 8) +
            2 * M_PI), (2 * M_PI))), 0.01) * CAR_HAUTEUR / 2.0 + enemie -> pos.y + CAR_HAUTEUR / 2;

          float pointxGauche = Math::arrondir(cos(fmod((M_PI - (enemie -> direction + 1) * (M_PI / 8) +
            2 * M_PI), (2 * M_PI))), 0.01) * CAR_HAUTEUR / 2.0 + enemie -> pos.x + CAR_LONGUEUR / 2;
          float pointyGauche = Math::arrondir(sin(fmod((M_PI - (enemie -> direction + 1) * (M_PI / 8) +
            2 * M_PI), (2 * M_PI))), 0.01) * CAR_HAUTEUR / 2.0 + enemie -> pos.y + CAR_HAUTEUR / 2;

          float distanceCentre = hypot(pointxCentre - pointx, pointyCentre - pointy);
          float distanceDroite = hypot(pointxDroite - pointx, pointyDroite - pointy);
          float distanceGauche = hypot(pointxGauche - pointx, pointyGauche - pointy);

          if (distanceCentre < distanceDroite && distanceCentre < distanceGauche) { //pas de changement de direction
          } else if (distanceDroite < distanceCentre) {
            enemie -> direction = fmod(enemie -> direction + 15, 16);
            recalculateSpeedDirection(enemie);
            enemie -> lastActive = TIME_BEFORE_REACTIVATE + dt;
          } else if (distanceGauche < distanceCentre) {
            enemie -> direction = fmod(enemie -> direction + 1, 16);
            recalculateSpeedDirection(enemie);
            enemie -> lastActive = TIME_BEFORE_REACTIVATE + dt;
          } else if (distanceCentre > distanceDroite && distanceCentre > distanceGauche) {
            if (Math::random() < 0.5) {
              enemie -> direction = fmod(enemie -> direction + 15, 16);
            } else {
              enemie -> direction = fmod(enemie -> direction + 1, 16);
            }
            recalculateSpeedDirection(enemie);
            enemie -> lastActive = TIME_BEFORE_REACTIVATE + dt;
          }
        }

        if (enemie -> lastNitroUsedTime >= 0) {
          enemie -> lastNitroUsedTime -= dt;
        }

        float botSpeedType = 1.0;
        double botChanceNitro = 0;
        float chanceToGetPowerUp = 0.0;

        if (enemie -> botType == "master") {
          botSpeedType = 1.23;
          botChanceNitro = 0.0009;
          chanceToGetPowerUp = 0.9;
        } else if (enemie -> botType == "hard") {
          botSpeedType = 0.9;
          botChanceNitro = 0.005;
          chanceToGetPowerUp = 0.7;
        } else if (enemie -> botType == "medium") {
          botSpeedType = 0.80;
          botChanceNitro = 0.003;
          chanceToGetPowerUp = 0.4;
        } else if (enemie -> botType == "dumy") {
          botSpeedType = 0.76;
          botChanceNitro = 0.001;
          chanceToGetPowerUp = 0.2;
        } else {
          botSpeedType = 0.90;
          botChanceNitro = 0.001;
          chanceToGetPowerUp = 0.5;
        }

        if (isCollision( * enemie, level.botLine[enemie -> botPositionToTarget], CAR_LONGUEUR)) {
          int randomDistForBot = 0;
          if (enemie -> botType == "master") {
            randomDistForBot = RANDOM_DIST_FOR_BOTS_MASTERMIND;
          } else if (enemie -> botType == "hard") {
            randomDistForBot = RANDOM_DIST_FOR_BOTS_HARD;
          } else if (enemie -> botType == "medium") {
            randomDistForBot = RANDOM_DIST_FOR_BOTS_MEDIUM;
          } else if (enemie -> botType == "dumy") {
            randomDistForBot = RANDOM_DIST_FOR_BOTS_DUMY;
          } else {
            randomDistForBot = RANDOM_DIST_FOR_BOTS;
          }

          sf::Vector2f vectorBetweenPosAndNewTarget = Vector2f(level.botLine[fmod(enemie -> botPositionToTarget + 1, level.botLine.size())].x - enemie -> pos.x, level.botLine[fmod(enemie -> botPositionToTarget + 1, level.botLine.size())].y - enemie -> pos.y);

          bool hasAlreadyATarget = false;
          if (Math::random() < chanceToGetPowerUp) {
            for (int i = 0; i < level.spawnPosNitro.size(); i++) {
              if (level.spawnPosNitro[i].present) {

                sf::Vector2f vectorToNitro = sf::Vector2f(level.spawnPosNitro[i].pos.x - enemie -> pos.x, level.spawnPosNitro[i].pos.y - enemie -> pos.y);

                if (vectorDotProduct(vectorBetweenPosAndNewTarget, vectorToNitro) > 0 && calculateNorme(vectorToNitro.x, vectorToNitro.y) < calculateNorme(vectorBetweenPosAndNewTarget.x, vectorBetweenPosAndNewTarget.y)) {

                  sf::Vector2f pojection1 = calculateProjection(vectorToNitro, vectorBetweenPosAndNewTarget);
                  sf::Vector2f pojection2 = calculateProjection(vectorToNitro, sf::Vector2f(vectorBetweenPosAndNewTarget.y / vectorBetweenPosAndNewTarget.x, 1));

                  //SI la distance sur x < 1 alors elle est plus proche de la cible et si en y elle est pas trop loing de l'axe
                  if (abs(calculateNorme(pojection1.x, pojection1.y) / calculateNorme(vectorBetweenPosAndNewTarget.x, vectorBetweenPosAndNewTarget.y)) < 1 &&
                    calculateNorme(pojection2.x, pojection2.y) < MAX_BOT_RANGE_TO_GET_POWERUP) {
                    enemie -> posInterBot.x = level.spawnPosNitro[i].pos.x + level.spawnPosNitro[i].rayon;
                    enemie -> posInterBot.y = level.spawnPosNitro[i].pos.y + level.spawnPosNitro[i].rayon;
                    hasAlreadyATarget = true;
                  }

                }
              }
            }
          }

          if (!hasAlreadyATarget && calculateNorme(vectorBetweenPosAndNewTarget.x, vectorBetweenPosAndNewTarget.y) > 60) {
            enemie -> posInterBot = randomInCircle(centerPosition(level.botLine[enemie -> botPositionToTarget], level.botLine[fmod(enemie -> botPositionToTarget + 1, level.botLine.size())]), randomDistForBot);

          }
          enemie -> botPositionToTarget = fmod(enemie -> botPositionToTarget + 1, level.botLine.size());
        }

        //if an ennemie is blocked too long
        if (enemie -> speed.x == 0 && enemie -> speed.y == 0) {
          enemie -> timeBlocked += dt;
        } else if (enemie -> timeBlocked > 0) {
          enemie -> timeBlocked = 0;
        }

        if (Math::random() < botChanceNitro && enemie -> lastNitroUsedTime <= 0 && enemie -> nbNitro > 0) {
          enemie -> lastNitroUsedTime = TIME_NITRO_USED;
          enemie -> nbNitro -= 1;
        }

        if (enemie -> posInterBot.x > 0) {
          if (isCollision( * enemie, enemie -> posInterBot, CAR_LONGUEUR * 3 / 4)) {
            enemie -> posInterBot.x = -1;
          }
        }

        if (enemie -> lastActive > 0) {
          enemie -> lastActive -= dt;
        }

        Wall wall;
        bool collisionWall = false;
        for (int i = 0; i < level.walls.size(); i++) {
          wall = level.walls[i];
          if (isCollision( * enemie, wall, CAR_HAUTEUR / 2)) {
            int direction = redirectIfPunchWall(enemie, wall);

            enemie -> speedColision.x = 0;
            enemie -> speedColision.y = 0;

            if (enemie -> direction == direction) {
              enemie -> speed.x = 0;
              enemie -> speed.y = 0;
              enemie -> malusBonusSpeed = 0;
            }
            if (direction >= 0) {
              enemie -> direction = direction;
              recalculateSpeedDirection(enemie);
              if (enemie -> state == 1) {
                enemie -> state = 2;
              } else if (enemie -> state == 3) {
                enemie -> state = 4;
              }
            }
          }
          if (isCollision( * enemie, wall, CAR_LONGUEUR)) {
            collisionWall = true;
          }
        }

        bool collisionMud = false;
        for (int i = 0; i < level.muds.size(); i++) {
          if (isCollision( * enemie, level.muds[i], CAR_HAUTEUR / 2)) {
            if (enemie -> state == 1) {
              enemie -> state = 3;
            } else if (enemie -> state == 2) {
              enemie -> state = 4;
            }
            collisionMud = true;
          }
        }

        if (enemie -> state == 2 and!collisionWall) {
          enemie -> state = 1;

        } else if (enemie -> state == 4 and!collisionWall) {
          enemie -> state = 3;
        } else if (playerCar.state == 3 and!collisionMud) {
          enemie -> state = 1;
        } else if (enemie -> state == 4 and!collisionMud) {
          enemie -> state = 2;
        }

        if (enemie -> state == 2) {
          enemie -> malusBonusSpeed *= 0.60;
        } else if (enemie -> state == 3) {
          enemie -> malusBonusSpeed *= 0.70;
        } else if (enemie -> state == 4) {
          enemie -> malusBonusSpeed *= 0.40;
        }

        for (int i = 0; i < level.flags.size(); i++) {
          if (isCollision( * enemie, level.flags[i], CAR_HAUTEUR / 2)) {
            countTour( * enemie, level.flags[i], nbFlag);
          }
        }

        for (int i = 0; i < level.spawnPosNitro.size(); i++) {
          if (isCollision( * enemie, level.spawnPosNitro[i], CAR_HAUTEUR / 2)) {
            if (level.spawnPosNitro[i].present) {
              enemie -> nbNitro++;
              level.spawnPosNitro[i].present = false;
            }
          }
        }

        //for (int k = 0; k < Enemies.size(); k++) {
        //                                                                    Car * enemie2 = Enemies[k];
        //                                                                    if (enemie2 != enemie){
        //                                                                    if (isCollision(enemie, enemie2 -> pos, CAR_HAUTEUR)) {
        //                                                                    	Speed tempSpeed = calculateProjectionOfSpeed(enemie -> speed, sf::Vector2f(enemie2 -> pos.x - enemie -> pos.x, enemie2 -> pos.y - enemie -> pos.y));
        //                                                                    	enemie->collisionSpeed.x -= 1.05 * (tempSpeed.x*(enemie2 -> pos.x - enemie -> pos.x)*dt);
        //                                                                    	enemie->collisionSpeed.y -= 1.05 * (tempSpeed.x*(enemie2 -> pos.y - enemie -> pos.y)*dt);
        //                                                                    	enemie2 -> collisionSpeed.x += 1.00 * (tempSpeed.x*(enemie2 -> pos.x - enemie -> pos.x)*dt);
        //                                                                    	enemie2 -> collisionSpeed.y += 1.00 * (tempSpeed.x*(enemie2 -> pos.y - enemie -> pos.y)*dt);
        //                                                                    }
        //                                                                    }
        //                                                                }
        //
        //if (isCollision(playerCar, enemie -> pos, CAR_HAUTEUR)) {
        //                                                    	Speed tempSpeed = calculateProjectionOfSpeed(enemie -> speed, sf::Vector2f(playerCar.pos.x- enemie -> pos.x , playerCar.pos.y- enemie -> pos.y ));
        //                                                    	playerCar.collisionSpeed.x -= 1.00 * (tempSpeed.x*(playerCar.pos.x- enemie -> pos.x)*dt);
        //                                                    	playerCar.collisionSpeed.y -= 1.00 * (tempSpeed.x*(playerCar.pos.y- enemie -> pos.y)*dt);
        //                                                    	enemie -> collisionSpeed.x += 1.05 * (tempSpeed.x*(playerCar.pos.x- enemie -> pos.x)*dt);
        //                                                    	enemie -> collisionSpeed.y += 1.05 * (tempSpeed.x*(playerCar.pos.y- enemie -> pos.y)*dt);
        //                                                    }
        //
        //On calcule ensuite la nouvelle vitesse de la voiture
        enemie -> speed = calculateSpeed( * enemie, (ACCELERATION * enemie -> malusBonusSpeed) * botSpeedType, ACCELERATION * botSpeedType, true, false, enemie -> lastNitroUsedTime >= 0, dt);

        enemie -> malusBonusSpeed = 1;

      }

      moveCar(playerCar, dt);
      for (int j = 0; j < Enemies.size(); j++) {
        Car * enemie = Enemies[j];
        moveCar(enemie, dt);
      }

      //fin du truc pour les ennemies
      if (playerCar.laps == NB_LAPS_FIN) {
        playerCar.score = score;
        score++;
        for (int j = 0; j < Enemies.size(); j++) {
          Car * enemie = Enemies[j];
          if (enemie -> score == 0) {
            enemie -> score = score;
            score++;
          }
        }

      }
      for (int j = 0; j < Enemies.size(); j++) {
        Car * enemie = Enemies[j];
        if (enemie -> laps >= NB_LAPS_FIN && enemie -> score == 0) {
          enemie -> score = score;
          score++;
        }
      }
      if (score == 4) {
        playerCar.score = score;

      }
      
      if (score >= 4) {

    	  idCurrentWindow = 5;
    	  makeAnnimation = false;
        tri[0] = & playerCar;

        for (int i = 0; i < Enemies.size(); i++) {
          tri[i + 1] = Enemies[i];
        }
        triSelectionDecroissant(tri, 4, idPositionMinimum);

        for (int i = 0; i < 4; i++) {
          tri[i] -> startPosition = i;
        }
        //On arrete toutes les musiques
              stopAllMusic(assets);
      }
      
      }

    } else if (idCurrentWindow == 2) {
    	if (makeAnnimation){
      textAlphaValue += 170 * dt;
      textAlphaValue %= 510;
    	}

      if (playerCar.startPosition >= 3 && !defeat) {
        defeat = true;
        cooldownReset = cooldownMaxReset;
      }
      if (defeat) {
        cooldownReset -= 1.0 * dt;
        if (cooldownReset <= 0) {
          enter1Pressure = true;
        }

      }

      if (enter1Pressure) {

        idLevel++;

        //mise a jour des positions de départ
        textAlphaValue = 0;
        nextScreen = 4;
        timer = 0;
        score = 1;
        

        //Mise a jour des difficultées
        if (idLevel < 8) {
          for (int i = 0; i < Enemies.size(); i++) {
            Enemies[i] -> botType = levelDifficult[idLevel - 1][i];
          }

          if (defeat) {
        	  nextScreen = 0;
            textAlphaValue = 0;
            textScale = 0.0;
            carScale = 0.5;
            carMove.x = -200;
            carMove.y = 700;
            idLevel = 1;
            defeat = false;
          }
        }

        reset(playerCar, level);
        for (int i = 0; i < Enemies.size(); i++) {
          Car * enemie = Enemies[i];
          reset(enemie, level);
        }
        for (int i = 0; i < level.spawnPosNitro.size(); i++) {
          level.spawnPosNitro[i].present = false;
        }
      }
    } else if (idCurrentWindow == 3) {
      timer += dt;
      if (timer >= 3.0) {
        idCurrentWindow = 1;
        timer = 0.0;
      }
    } else if (idCurrentWindow == 4) {

      if (allTextForFrame.size() == 0) {
        std::map < int, std::map < int, std::vector < sf::String >>> ::iterator it1 = allText.find(idLevel);
        if (it1 != allText.end()) {
          std::map < int, std::vector < sf::String >> ::iterator it2 = allText[idLevel].find(0);
          if (it2 != allText[idLevel].end()) {
            for (int i = 0; i < allText[idLevel][0].size(); i++) {
              allTextForFrame.push_back(allText[idLevel][0][i]);
            }
          }
          it2 = allText[idLevel].find(playerCar.startPosition + 1);
          if (it2 != allText[idLevel].end()) {
            for (int i = 0; i < allText[idLevel][playerCar.startPosition + 1].size(); i++) {
              allTextForFrame.push_back(allText[idLevel][playerCar.startPosition + 1][i]);
            }
          }
        }
      }
      if (allTextForFrame.size() == 0) {
        faceMove.x = 1446;
        faceMove.y = 577;
        bulleRotation = -40.0;
        alphaBulleValue = 0.0;
        idCharToShow = 0;
        textAlphaValue = 0;
        idCurrentWindow = 3;
        idText = 0;
        allTextForFrame.clear();
        
        stopAllMusic(assets);
        playMusicOnce = true;
        
      } else {
        if (idText < allTextForFrame.size()) {
          if (faceMove.x > 954 && idText == 0) {
            faceMove.x -= 492 * dt;
          } else {
            faceMove.x = 954;
          }
          if (alphaBulleValue < 255 && idText == 0) {
            if (faceMove.x <= 954) {
              bulleRotation += 80.0 * dt;
              alphaBulleValue += 510.0 * dt;
            }
          } else {
            bulleRotation = 0.0;
            alphaBulleValue = 255.0;
          }
          if (idText < allTextForFrame.size()) {

            if (idCharToShow < allTextForFrame[idText].getSize()) {
              if (alphaBulleValue >= 255) {
                idCharToShow += 60 * dt;
              }
            } else {
              idCharToShow = allTextForFrame[idText].getSize();
            }
            if (idCharToShow >= allTextForFrame[idText].getSize()) {
              textAlphaValue += 170 * dt;
              textAlphaValue %= 510;
            }
          }
        } else if (idText == allTextForFrame.size()) {
          if (faceMove.x < 1446) {
            if (alphaBulleValue > 0) {
              alphaBulleValue -= 510.0 * dt;
              if (alphaBulleValue < 0) {
                alphaBulleValue = 0;
              }
            } else {
              alphaBulleValue = 0;
            }
            faceMove.x += 286 * dt;
          } else {
            faceMove.x = 1446;
            idText++;
          }
        }

        if (idText > allTextForFrame.size()) {
        	enter1Pressure = true;
        }

        if (enter1Pressure) {
          idText++;
          if (idText <= allTextForFrame.size()) {
            idCharToShow = 0;
            textAlphaValue = 0;
          } else if (idText > allTextForFrame.size()) {
            faceMove.x = 1446;
            faceMove.y = 577;
            bulleRotation = -40.0;
            alphaBulleValue = 0.0;
            idCharToShow = 0;
            textAlphaValue = 0;
            idCurrentWindow = 3;
            idText = 0;
            allTextForFrame.clear();
            stopAllMusic(assets);
            playMusicOnce = true;
          }
        }

      }

    } else if (idCurrentWindow == 5){
    	timeGoalScreen += dt;
    	if (timeGoalScreen > durationGoalScreen){
    		timeGoalScreen = 0;
    		nextScreen = 2;
    	}
    }
    
    
    
    //***********************************AFFICHAGE !!!!!!******************//

    window.clear(Color::White);

    if (idCurrentWindow == 0) {
    	
    	if (!assets.titleScreenmusic.getStatus()){
    		assets.titleScreenmusic.play();
    	}
    	
      
      window.draw(assets.backgroundMainScreen);

      assets.superoffroadText.setScale(textScale, textScale);
      window.draw(assets.superoffroadText);

      assets.superoffroadCar.setPosition(carMove.x, carMove.y);
      assets.superoffroadCar.setScale(carScale, carScale);
      window.draw(assets.superoffroadCar);

      sf::Text enterText = sf::Text();
      enterText.setString("Insert COIN (or press enter)");
      enterText.setFont(font);
      enterText.setCharacterSize(60);

      if (textAlphaValue <= 255) {
        enterText.setFillColor(sf::Color(255, 255, 255, textAlphaValue));
        enterText.setOutlineColor(sf::Color(0, 0, 0, textAlphaValue));
        enterText.setOutlineThickness(2.0);

      } else {
        enterText.setFillColor(sf::Color(255, 255, 255, 509 - textAlphaValue));
        enterText.setOutlineColor(sf::Color(0, 0, 0, 509 - textAlphaValue));
        enterText.setOutlineThickness(2.0);
      }

      enterText.setPosition(WINDOW_WIDTH / 2 - enterText.getLocalBounds().width / 2, WINDOW_HEIGHT * 7 / 8 - enterText.getLocalBounds().height / 2);

      window.draw(enterText);

    }
    //Si on est dans la partie ou sur le décompte ou si il parle
    if (idCurrentWindow == 1 || idCurrentWindow == 3 || idCurrentWindow == 4 || idCurrentWindow == 5) {

    	if (idCurrentWindow == 1){
    	if (!assets.titleScreenmusic.getStatus()){
    	    		    		assets.titleScreenmusic.play();
    	    		    	}
    	}

    	if (colision && idCurrentWindow==1)
    	            {
    	                sf::Sound sound;
    	                sound.setBuffer(colisionBuffer);
    	                sound.play();
    	            }
    	
      window.draw(assets.backgroundLevelScreen);

      for (int j = 0; j < level.spawnPosNitro.size(); j++) {
        if (level.spawnPosNitro[j].present) {
          assets.nitro.setPosition(level.spawnPosNitro[j].pos.x, level.spawnPosNitro[j].pos.y);
          window.draw(assets.nitro);
        }
      }

      if (DEBUG) {
        sf::RectangleShape mudShape;
        sf::CircleShape nitroShape;

        for (int j = 0; j < level.muds.size(); ++j) {
          mudShape.setSize(sf::Vector2f(level.muds[j].rayon * 2, level.muds[j].rayon * 2));
          mudShape.setPosition(level.muds[j].pos.x, level.muds[j].pos.y);
          mudShape.setOrigin(level.muds[j].rayon, level.muds[j].rayon);
          mudShape.setFillColor(sf::Color::Green);
          window.draw(mudShape);
        }
        for (int j = 0; j < level.spawnPosNitro.size(); j++) {
          if (level.spawnPosNitro[j].present) {
            nitroShape.setRadius(10);
            nitroShape.setPosition(level.spawnPosNitro[j].pos.x, level.spawnPosNitro[j].pos.y);
            nitroShape.setOrigin(level.spawnPosNitro[j].rayon, level.spawnPosNitro[j].rayon);
            nitroShape.setFillColor(sf::Color::Green);
            window.draw(nitroShape);
          }
        }
      }

      sf::RectangleShape carShape(sf::Vector2f(CAR_LONGUEUR, CAR_HAUTEUR));

      for (int j = 0; j < Enemies.size(); j++) {
        Car * enemie = Enemies[j];
        carShape.setPosition(enemie -> pos.x, enemie -> pos.y);
        carShape.setOrigin(CAR_LONGUEUR / 2, CAR_HAUTEUR / 2);
        carShape.setRotation(180 - (enemie -> direction / 16.0 * 360));
        carShape.setFillColor(enemie -> color);
        window.draw(carShape);
      }

      carShape.setPosition(playerCar.pos.x, playerCar.pos.y);
      carShape.setOrigin(CAR_LONGUEUR / 2, CAR_HAUTEUR / 2);
      carShape.setRotation(180 - (playerCar.direction / 16.0 * 360));
      carShape.setFillColor(playerCar.color);

      window.draw(carShape);

      if (DEBUG) {
    	  sf::CircleShape nitroShape;
    	                  std::vector < sf::RectangleShape > listWallPrint;
    	                  sf::RectangleShape wallShape;
    	                  sf::VertexArray lines(sf::LineStrip, level.walls.size() + 1);
    	                  for (int i = 0; i < level.walls.size(); ++i) {
    	                      lines[i].position = sf::Vector2f(level.walls[i].hitbox.corner1.x, level.walls[i].hitbox.corner1.y);
    	                      lines[i].color = sf::Color::Red;
    	                  }
    	                  lines[level.walls.size()].position = sf::Vector2f(level.walls[0].hitbox.corner1.x, level.walls[0].hitbox.corner1.y);
    	                  lines[level.walls.size()].color = sf::Color::Red;
    	                  window.draw(lines);

    	                  for (int i = 0; i < level.botLine.size(); i++) {
    	                      nitroShape.setRadius(2);
    	                      nitroShape.setPosition(level.botLine[i].x, level.botLine[i].y);
    	                      nitroShape.setOrigin(1, 1);
    	                      nitroShape.setFillColor(sf::Color::White);
    	                      window.draw(nitroShape);
    	                  }

    	                  sf::VertexArray speed(sf::LineStrip, 2);
    	                  speed[0].position = sf::Vector2f(playerCar.pos.x,playerCar.pos.y);
    	                  speed[1].position = sf::Vector2f(playerCar.pos.x + playerCar.speed.x + playerCar.speedColision.x,playerCar.pos.y + playerCar.speed.y + playerCar.speedColision.y);
    	                  speed[0].color = sf::Color::White;
    	                  speed[1].color = sf::Color::White;

    	                  window.draw(speed);
      }

      sf::Vector2f positionLaps(level.scorePos.x + (-24 * 2), level.scorePos.y + (-9 * 2));
      sf::Vector2f positionNitro(level.scorePos.x + (-34 * 2), level.scorePos.y + (10 * 2));

      assets.score.setPosition(level.scorePos.x, level.scorePos.y);
      window.draw(assets.score);

      sf::RectangleShape infoLapShape(sf::Vector2f(13 * 2, 18 * 2));
      sf::RectangleShape infoNitroShape(sf::Vector2f(26 * 2, 18 * 2));

      infoLapShape.setPosition(positionLaps.x, positionLaps.y);
      infoLapShape.setFillColor(playerCar.color);
      window.draw(infoLapShape);

      infoNitroShape.setPosition(positionNitro.x, positionNitro.y);
      infoNitroShape.setFillColor(playerCar.color);
      window.draw(infoNitroShape);

      sf::Text tourCountText = sf::Text();
      sf::Text horloge = sf::Text();
      horloge.setFont(font);
      horloge.setCharacterSize(50);

      std::string horlogeText;

      if (idCurrentWindow == 1 || idCurrentWindow == 5) {
        horlogeText = std::to_string(Math::arrondir(timer, 0.1));
        horlogeText = horlogeText.substr(0, horlogeText.find(".") + 2);
      } else {
        horlogeText = "0.0";
      }

      horloge.setString(horlogeText);
      horloge.setFillColor(sf::Color::Black);
      horloge.setPosition(level.scorePos.x + (-2 * 2) - horloge.getLocalBounds().width / 2, level.scorePos.y + (-28 * 2) - horloge.getLocalBounds().height / 2 + (-2 * 2));

      tourCountText.setFont(font);
      tourCountText.setCharacterSize(60);

      sf::Text nitroCountText = sf::Text();

      nitroCountText.setFont(font);
      nitroCountText.setCharacterSize(60);

      tourCountText.setString(std::to_string(playerCar.laps));

      if (playerCar.nbNitro < 10) {
        nitroCountText.setString("0" + std::to_string(playerCar.nbNitro));
      } else {
        nitroCountText.setString(std::to_string(playerCar.nbNitro));
      }

      tourCountText.setFillColor(sf::Color::Black);
      nitroCountText.setFillColor(sf::Color::Black);

      tourCountText.setPosition(positionLaps.x, positionLaps.y - tourCountText.getLocalBounds().height / 2 - (4 * 2));
      nitroCountText.setPosition(positionNitro.x, positionNitro.y - nitroCountText.getLocalBounds().height / 2 - (4 * 2));

      window.draw(tourCountText);
      window.draw(nitroCountText);
      window.draw(horloge);

      for (int i = 0; i < Enemies.size(); i++) {
        Car * enemie = Enemies[i];
        sf::Text tourCountText = sf::Text();

        tourCountText.setFont(font);
        tourCountText.setCharacterSize(60);

        sf::Text nitroCountText = sf::Text();

        nitroCountText.setFont(font);
        nitroCountText.setCharacterSize(60);

        tourCountText.setString(std::to_string(enemie -> laps));

        if (enemie -> nbNitro < 10) {
          nitroCountText.setString("0" + std::to_string(enemie -> nbNitro));
        } else {
          nitroCountText.setString(std::to_string(enemie -> nbNitro));
        }

        tourCountText.setFillColor(sf::Color::Black);
        nitroCountText.setFillColor(sf::Color::Black);

        tourCountText.setPosition(positionLaps.x + (13 * 2 + 15 * 2) * (i + 1), positionLaps.y - tourCountText.getLocalBounds().height / 2 - (4 * 2));
        nitroCountText.setPosition(positionNitro.x + (26 * 2 + 2 * 2) * (i + 1), positionNitro.y - nitroCountText.getLocalBounds().height / 2 - (4 * 2));

        infoLapShape.setPosition(positionLaps.x + (13 * 2 + 15 * 2) * (i + 1), positionLaps.y);
        infoLapShape.setFillColor(enemie -> color);
        window.draw(infoLapShape);

        infoNitroShape.setPosition(positionNitro.x + (26 * 2 + 2 * 2) * (i + 1), positionNitro.y);
        infoNitroShape.setFillColor(enemie -> color);
        window.draw(infoNitroShape);

        window.draw(tourCountText);
        window.draw(nitroCountText);
      }

    }
    //AFFICHAGE DES SCORE//
    if (idCurrentWindow == 2) {

    	
    	
      window.draw(assets.backgroundEndRun);

      window.draw(assets.position1Perso);
      window.draw(assets.position2Perso);
      window.draw(assets.position3Perso);

      assets.position1Habits.setColor(tri[0] -> color);
      assets.position2Habits.setColor(tri[1] -> color);
      assets.position3Habits.setColor(tri[2] -> color);

      window.draw(assets.position1Habits);
      window.draw(assets.position2Habits);
      window.draw(assets.position3Habits);

      sf::Text horloge = sf::Text();
      horloge.setFont(font);
      horloge.setCharacterSize(40);

      std::string horlogeText;
      horlogeText = std::to_string(Math::arrondir(timer, 0.1));
      horlogeText = horlogeText.substr(0, horlogeText.find(".") + 2);

      horloge.setString(horlogeText);
      horloge.setFillColor(sf::Color::Black);
      horloge.setPosition(720 - horloge.getLocalBounds().width / 2, 30 - horloge.getLocalBounds().height / 2);
      window.draw(horloge);

      sf::Text enterText = sf::Text();
      if (defeat) {
    	  if (!assets.gameoverScreenmusic.getStatus() && playMusicOnce){
    		  playMusicOnce = false;
    	      		assets.gameoverScreenmusic.play();
    	      	}
        enterText.setString("You lose!! Press enter to return Lobby");
      } else {
    	  if (!assets.celebrationScreenmusic.getStatus() && playMusicOnce){
    		  playMusicOnce = false;
    	      		assets.celebrationScreenmusic.play();
    	      	}
        enterText.setString("Press enter to continue");
      }
      enterText.setFont(font);
      enterText.setCharacterSize(60);

      if (textAlphaValue <= 255) {
        enterText.setFillColor(sf::Color(255, 255, 255, textAlphaValue));
        enterText.setOutlineColor(sf::Color(0, 0, 0, textAlphaValue));
        enterText.setOutlineThickness(2.0);

      } else {
        enterText.setFillColor(sf::Color(255, 255, 255, 509 - textAlphaValue));
        enterText.setOutlineColor(sf::Color(0, 0, 0, 509 - textAlphaValue));
        enterText.setOutlineThickness(2.0);
      }

      enterText.setPosition(WINDOW_WIDTH / 2 - enterText.getLocalBounds().width / 2, WINDOW_HEIGHT * 7 / 8 - enterText.getLocalBounds().height / 2);

      window.draw(enterText);

    }
    //AFFICHAGE DU DECOMPTE
    if (idCurrentWindow == 3) {

    	if (!assets.startScreenmusic.getStatus()){
    	    	    	    assets.startScreenmusic.play();
    	    	    	}
    	
      std::string countdown;
      if (timer < 1.0) {
        countdown = std::to_string(3);
      } else if (timer < 2.0) {
        countdown = std::to_string(2);
      } else if (timer < 3.0) {
        countdown = std::to_string(1);
      }

      sf::RectangleShape greyShape(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
      greyShape.setFillColor(sf::Color(0, 0, 0, 100));
      window.draw(greyShape);

      sf::Text countdownText = sf::Text();
      countdownText.setFont(font);
      countdownText.setFillColor(sf::Color::Black);

      countdownText.setCharacterSize(90);

      countdownText.setString(countdown);
      countdownText.setPosition(WINDOW_WIDTH / 2 - countdownText.getLocalBounds().width / 2, WINDOW_HEIGHT * 1 / 2 - countdownText.getLocalBounds().height / 2);

      window.draw(countdownText);

    }
    if (idCurrentWindow == 4) {
      sf::RectangleShape greyShape(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
      greyShape.setFillColor(sf::Color(0, 0, 0, 100));
      window.draw(greyShape);

      assets.face.setPosition(faceMove);
      window.draw(assets.face);

      assets.bulle.setRotation(bulleRotation);
      assets.bulle.setColor(sf::Color(255, 255, 255, alphaBulleValue));
      window.draw(assets.bulle);
      if (idText < allTextForFrame.size()) {
        sf::Text persoFinalText(allTextForFrame[idText], font, 30);
        sf::Text persoText(allTextForFrame[idText].substring(0, (int) idCharToShow), font, 30);
        persoText.setPosition(600 - persoFinalText.getLocalBounds().width / 2, 320 - persoFinalText.getLocalBounds().height / 2);
        persoText.setFillColor(sf::Color::Black);
        window.draw(persoText);
      }

      sf::Text enterText = sf::Text();
      enterText.setString("Press enter to continue");
      enterText.setFont(font);
      enterText.setCharacterSize(60);

      if (textAlphaValue <= 255) {
        enterText.setFillColor(sf::Color(255, 255, 255, textAlphaValue));
        enterText.setOutlineColor(sf::Color(0, 0, 0, textAlphaValue));
        enterText.setOutlineThickness(2.0);

      } else {
        enterText.setFillColor(sf::Color(255, 255, 255, 509 - textAlphaValue));
        enterText.setOutlineColor(sf::Color(0, 0, 0, 509 - textAlphaValue));
        enterText.setOutlineThickness(2.0);
      }

      enterText.setPosition(WINDOW_WIDTH / 2 - enterText.getLocalBounds().width / 2, WINDOW_HEIGHT * 7 / 8 - enterText.getLocalBounds().height / 2);

      window.draw(enterText);

    }
    else if (idCurrentWindow == 5) {
    	if (!assets.goalScreenmusic.getStatus()){
    	    assets.goalScreenmusic.play();
    	}
    }
    
    
    
    if (nextScreen > 0){
    	sf::RectangleShape blackShape(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    	if (alphaBlackScreen <= 255) {
    		blackShape.setFillColor(sf::Color(0, 0, 0, alphaBlackScreen));

    	      } else {
    	        blackShape.setFillColor(sf::Color(0, 0, 0, 509 - alphaBlackScreen));
    	      }
    	
    	window.draw(blackShape);
    }
    
    
    

    window.display();

    sf::sleep(sf::seconds((1.0 / MAX_FPS) - clock.getElapsedTime().asSeconds()));
    //std::cout << framerate << std::endl;

  }

  #ifdef __linux__
  system("for KILLPID in `ps ax | grep ./h-linux | awk ' { print $1;}'`; do    kill $KILLPID; done");
  #elif _WIN32
  system("ps -W | awk '/h-windos.exe/,NF=1' | xargs kill -f");
  #endif

  return 0;
}