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

#include <math.h>

#include <fstream>

#include <string>

#include <regex>

#include <iostream>

#include <array>

#include <vector>

#include <algorithm>

#include <assert.h>

using namespace std;
using namespace sf;

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
    int state; //1 pour normal, 2 contre un mur, 3 dans la boue, 4 contre un mur dans la boue
    Position pos;
    Speed speed;
    int direction;
    int laps;
    int flag;
    int nbNitro;
    int score;
    double lastNitroUsedTime;
    float malusBonusSpeed;
    int botPositionToTarget;
    double lastActive;
    sf::Color color;
    Position posInterBot;
    std::string botType;
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
    std::vector < Wall > walls2;
    std::vector < Bonus > spawnPosNitro;
    std::vector < Mud > muds;
    std::vector < Flag > flags;
};
//algo de silvio
//algo de collision
int vectorDotProduct(const Position & pt1,
    const Position & pt2) {
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

        if ((normeSpeed / maxSpeed) > 0.2) {
            float deceleration = maxSpeed * (normeSpeed / maxSpeed) * 0.5;
            accelerationX = -cos(angleRad) * (deceleration);
            accelerationY = -sin(angleRad) * (deceleration);
            speed.x = car.speed.x + accelerationX * dt;
            speed.y = car.speed.y + accelerationY * dt;
        } else {
            speed.x = 0;
            speed.y = 0;
        }
    }
    return speed;
}

void moveCar(Car & car,
    const float & dt) {
    car.pos.x = car.pos.x + car.speed.x * dt;
    car.pos.y = car.pos.y + car.speed.y * dt;
}

void moveCar(Car * car,
    const float & dt) {
    car -> pos.x = car -> pos.x + car -> speed.x * dt;
    car -> pos.y = car -> pos.y + car -> speed.y * dt;
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
void reset(Car & car) {
    car.state = 1;
    car.pos.x = 750; //la position initial de la voiture en x
    car.pos.y = 675; //la position initial de la voiture en y
    car.speed.x = 0;
    car.speed.y = 0;
    car.direction = 0;
    car.laps = 0;
    car.flag = 0;
    car.nbNitro = 3;
    car.lastNitroUsedTime = 0;
    car.malusBonusSpeed = 1.0;
    car.lastActive = 0;
    car.score = 0;
}
void reset(Car * car) {
    car -> state = 1;
    car -> pos.x = 750; //la position initial de la voiture en x
    car -> pos.y = 675; //la position initial de la voiture en y
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

}

// void makeLevel(Ground & level, std::string src) {
//   Ground cache;
//   ifstream levelData(src);

//   if (levelData) {
//     std::string line;
//     std::string delimiter = ":";

//     std::regex wallPattern("\\(([0-9]+),([0-9]+)\\)-([0-9]+)-\\(([0-9]+),([0-9]+)\\)");
//     std::regex nitroPattern("\\(([0-9]+),([0-9]+)\\)");

//     std::regex flagPattern("\\(([0-9]+),([0-9]+)\\)-([0-9]+)-\\(([0-9]+),([0-9]+)\\)");

//     std::smatch m;

//     while (getline(levelData, line)) {
//       std::string token = line.substr(0, line.find(delimiter));
//       if (token == "Wall") {
//         line = line.substr(line.find(delimiter) + 2);
//         while (std::regex_search(line, m, wallPattern)) {

//           Wall wall;

//           wall.hitbox.corner1.x = std::stoi(m[1]);
//           wall.hitbox.corner1.y = std::stoi(m[2]);
//           wall.directionStop = std::stoi(m[3]);
//           wall.hitbox.corner2.x = std::stoi(m[4]);
//           wall.hitbox.corner2.y = std::stoi(m[5]);

//           level.walls.push_back(wall);

//           line = line.substr(line.find(")-") + 1);
//         }
//       }
//       if (token == "Nitro") {
//         line = line.substr(line.find(delimiter) + 2);
//         while (std::regex_search(line, m, nitroPattern)) {
//           Position nitro;
//           nitro.x = std::stoi(m[1]);
//           nitro.y = std::stoi(m[2]);
//           level.spawnPosNitro.push_back(nitro);
//           line = line.substr(line.find(')') + 1);
//         }
//       }

//       if (token == "Mud") {
//         line = line.substr(line.find(delimiter) + 2);
//         while (std::regex_search(line, m, nitroPattern)) {
//           Mud mud;

//         }
//       }
//       if (token == "Flag") {
//         line = line.substr(line.find(delimiter) + 2);
//         while (std::regex_search(line, m, flagPattern)) {
//           Flag newFlag;
//           newFlag.hitbox.corner1.x = std::stoi(m[1]);
//           newFlag.hitbox.corner1.y = std::stoi(m[2]);
//           newFlag.nb = std::stoi(m[3]);
//           newFlag.hitbox.corner2.x = std::stoi(m[4]);
//           newFlag.hitbox.corner2.y = std::stoi(m[5]);
//           level.flags.push_back(newFlag);
//           line = line.substr(m.position(0) + m.length(0));
//         }

//       }
//     }

//   } else {
//     throw "Can't read the file " + src;
//   }
// }

//fonction de débugage
//affichage de la vrai htibox des murs

int getWallLength(Wall wall) {
    return (hypot((wall.hitbox.corner2.x - wall.hitbox.corner1.x), (wall.hitbox.corner2.y - wall.hitbox.corner1.y)));
}

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

    const int WINDOW_WIDTH = 1320;
    const int WINDOW_HEIGHT = 880;
    const std::string WINDOW_TITLE = "Super off Road";
    const int MAX_FPS = 120;

    const float TIME_BEFORE_REACTIVATE = 0.05;

    int idCurrentWindow = 0;

    std::string levelFile("level1");

    sf::Font font;
    font.loadFromFile("PixelOperator.ttf");

    const int NITRO_SPAWN_TIME = 1000;
    const int ACCELERATION = 60;
    bool up, down, left, right, nitro, enter;
    up = down = left = right = nitro = enter = false;

    double lastActiveNitro;
    lastActiveNitro = 0;

    double TIME_NITRO_USED = 4.0;

    const int CAR_LONGUEUR = 40;
    const int CAR_HAUTEUR = 20;

    const float RANDOM_DIST_FOR_BOTS = 50;

    const float RANDOM_DIST_FOR_BOTS_MASTERMIND = 10;

    const float RANDOM_DIST_FOR_BOTS_MEDIUM = 10;

    const float RANDOM_DIST_FOR_BOTS_DUMY = 10;
    const int NB_LAPS_FIN = 5;

    /*
     * Variables pour l'ecran titre
     */

    int textAlphaValue = 0;

    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);

    /*
     * Une Clock permet de compter le temps. Vous en aurez besoin pour savoir
     * le temps entre deux frames.
     */
    Clock clock;

    Ground level;
    //makeLevel(level, levelFile + ".txt");
    sf::Texture backgroundTexture;
    backgroundTexture.loadFromFile(levelFile + ".png");
    sf::Sprite background;
    background.setTexture(backgroundTexture);

    background.setScale(sf::Vector2f((WINDOW_WIDTH / backgroundTexture.getSize().x), (WINDOW_HEIGHT / backgroundTexture.getSize().y)));
    Car playerCar;
    playerCar.state = 1;
    playerCar.pos.x = 770; //la position initial de la voiture en x
    playerCar.pos.y = 655; //la position initial de la voiture en y
    playerCar.speed.x = 0;
    playerCar.speed.y = 0;
    playerCar.direction = 0;
    playerCar.laps = 0;
    playerCar.flag = 0;
    playerCar.nbNitro = 3;
    playerCar.lastNitroUsedTime = TIME_NITRO_USED;
    playerCar.malusBonusSpeed = 1.0;
    playerCar.lastActive = 0;
    playerCar.score = 0;
    playerCar.color = sf::Color::Red;
    float timer = 0;
    int nbFlag;
    int countNitro = 0;
    int score = 1;

    printListWall(level.walls);

    /*Walls 
     */

    Wall wall;
    Mud mud;
    Bonus bonus;

    wall.hitbox.corner1.x = 450;
    wall.hitbox.corner1.y = 750;
    wall.directionStop = 2;
    wall.hitbox.corner2.x = 200;
    wall.hitbox.corner2.y = 500;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 200;
    wall.hitbox.corner1.y = 500;
    wall.directionStop = 0;
    wall.hitbox.corner2.x = 200;
    wall.hitbox.corner2.y = 200;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 200;
    wall.hitbox.corner1.y = 200;
    wall.directionStop = 14;
    wall.hitbox.corner2.x = 350;
    wall.hitbox.corner2.y = 50;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 350;
    wall.hitbox.corner1.y = 50;
    wall.directionStop = 12;
    wall.hitbox.corner2.x = 600;
    wall.hitbox.corner2.y = 50;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 600;
    wall.hitbox.corner1.y = 50;
    wall.directionStop = 10;
    wall.hitbox.corner2.x = 800;
    wall.hitbox.corner2.y = 250;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 800;
    wall.hitbox.corner1.y = 250;
    wall.directionStop = 8;
    wall.hitbox.corner2.x = 800;
    wall.hitbox.corner2.y = 350;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 800;
    wall.hitbox.corner1.y = 350;
    wall.directionStop = 10;
    wall.hitbox.corner2.x = 900;
    wall.hitbox.corner2.y = 450;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 900;
    wall.hitbox.corner1.y = 450;
    wall.directionStop = 12;
    wall.hitbox.corner2.x = 1100;
    wall.hitbox.corner2.y = 450;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 1100;
    wall.hitbox.corner1.y = 450;
    wall.directionStop = 10;
    wall.hitbox.corner2.x = 1200;
    wall.hitbox.corner2.y = 550;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 1200;
    wall.hitbox.corner1.y = 550;
    wall.directionStop = 8;
    wall.hitbox.corner2.x = 1200;
    wall.hitbox.corner2.y = 650;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 1200;
    wall.hitbox.corner1.y = 650;
    wall.directionStop = 6;
    wall.hitbox.corner2.x = 1100;
    wall.hitbox.corner2.y = 750;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 1100;
    wall.hitbox.corner1.y = 750;
    wall.directionStop = 4;
    wall.hitbox.corner2.x = 400;
    wall.hitbox.corner2.y = 750;

    level.walls.push_back(wall);

    wall.hitbox.corner1.x = 450;
    wall.hitbox.corner1.y = 600;
    wall.directionStop = 10;
    wall.hitbox.corner2.x = 350;
    wall.hitbox.corner2.y = 500;

    level.walls2.push_back(wall);

    wall.hitbox.corner1.x = 350;
    wall.hitbox.corner1.y = 500;
    wall.directionStop = 8;
    wall.hitbox.corner2.x = 350;
    wall.hitbox.corner2.y = 300;

    level.walls2.push_back(wall);

    wall.hitbox.corner1.x = 350;
    wall.hitbox.corner1.y = 300;
    wall.directionStop = 6;
    wall.hitbox.corner2.x = 500;
    wall.hitbox.corner2.y = 150;

    level.walls2.push_back(wall);

    wall.hitbox.corner1.x = 500;
    wall.hitbox.corner1.y = 150;
    wall.directionStop = 2;
    wall.hitbox.corner2.x = 600;
    wall.hitbox.corner2.y = 250;

    level.walls2.push_back(wall);

    wall.hitbox.corner1.x = 600;
    wall.hitbox.corner1.y = 250;
    wall.directionStop = 4;
    wall.hitbox.corner2.x = 650;
    wall.hitbox.corner2.y = 250;

    level.walls2.push_back(wall);

    wall.hitbox.corner1.x = 650;
    wall.hitbox.corner1.y = 250;
    wall.directionStop = 0;
    wall.hitbox.corner2.x = 650;
    wall.hitbox.corner2.y = 400;

    level.walls2.push_back(wall);

    wall.hitbox.corner1.x = 650;
    wall.hitbox.corner1.y = 400;
    wall.directionStop = 2;
    wall.hitbox.corner2.x = 800;
    wall.hitbox.corner2.y = 550;

    level.walls2.push_back(wall);

    wall.hitbox.corner1.x = 800;
    wall.hitbox.corner1.y = 550;
    wall.directionStop = 4;
    wall.hitbox.corner2.x = 1000;
    wall.hitbox.corner2.y = 550;

    level.walls2.push_back(wall);

    wall.hitbox.corner1.x = 1000;
    wall.hitbox.corner1.y = 550;
    wall.directionStop = 0;
    wall.hitbox.corner2.x = 1000;
    wall.hitbox.corner2.y = 600;

    level.walls2.push_back(wall);

    wall.hitbox.corner1.x = 1000;
    wall.hitbox.corner1.y = 600;
    wall.directionStop = 12;
    wall.hitbox.corner2.x = 450;
    wall.hitbox.corner2.y = 600;

    level.walls2.push_back(wall);

    //mud

    mud.pos.x = 225;
    mud.pos.y = 475;
    mud.rayon = 25;

    level.muds.push_back(mud);

    mud.pos.x = 225;
    mud.pos.y = 425;
    mud.rayon = 25;

    level.muds.push_back(mud);

    mud.pos.x = 225;
    mud.pos.y = 375;
    mud.rayon = 25;

    level.muds.push_back(mud);

    mud.pos.x = 525;
    mud.pos.y = 125;
    mud.rayon = 25;

    level.muds.push_back(mud);

    mud.pos.x = 725;
    mud.pos.y = 325;
    mud.rayon = 25;

    level.muds.push_back(mud);

    mud.pos.x = 925;
    mud.pos.y = 525;
    mud.rayon = 25;

    level.muds.push_back(mud);

    mud.pos.x = 1025;
    mud.pos.y = 675;
    mud.rayon = 25;

    level.muds.push_back(mud);

    mud.pos.x = 975;
    mud.pos.y = 675;
    mud.rayon = 25;

    level.muds.push_back(mud);

    //flag
    Flag flag;

    flag.hitbox.corner1.x = 750;
    flag.hitbox.corner1.y = 600;
    flag.nb = 0;
    flag.hitbox.corner2.x = 750;
    flag.hitbox.corner2.y = 750;

    level.flags.push_back(flag);

    flag.hitbox.corner1.x = 450;
    flag.hitbox.corner1.y = 600;
    flag.nb = 1;
    flag.hitbox.corner2.x = 450;
    flag.hitbox.corner2.y = 750;

    level.flags.push_back(flag);

    flag.hitbox.corner1.x = 350;
    flag.hitbox.corner1.y = 500;
    flag.nb = 2;
    flag.hitbox.corner2.x = 200;
    flag.hitbox.corner2.y = 500;

    level.flags.push_back(flag);

    flag.hitbox.corner1.x = 500;
    flag.hitbox.corner1.y = 50;
    flag.nb = 3;
    flag.hitbox.corner2.x = 500;
    flag.hitbox.corner2.y = 200;

    level.flags.push_back(flag);

    flag.hitbox.corner1.x = 650;
    flag.hitbox.corner1.y = 250;
    flag.nb = 4;
    flag.hitbox.corner2.x = 800;
    flag.hitbox.corner2.y = 250;

    level.flags.push_back(flag);

    flag.hitbox.corner1.x = 1000;
    flag.hitbox.corner1.y = 450;
    flag.nb = 5;
    flag.hitbox.corner2.x = 1000;
    flag.hitbox.corner2.y = 550;

    level.flags.push_back(flag);
    nbFlag = level.flags.size();

    //nitro
    bonus.pos.x = 325;
    bonus.pos.y = 175;
    bonus.rayon = 10;
    bonus.present = false;

    level.spawnPosNitro.push_back(bonus);

    bonus.pos.x = 625;
    bonus.pos.y = 175;
    bonus.rayon = 10;
    bonus.present = false;

    level.spawnPosNitro.push_back(bonus);

    bonus.pos.x = 1125;
    bonus.pos.y = 575;
    bonus.rayon = 10;
    bonus.present = false;

    level.spawnPosNitro.push_back(bonus);

    bonus.pos.x = 625;
    bonus.pos.y = 675;
    bonus.rayon = 10;
    bonus.present = false;

    level.spawnPosNitro.push_back(bonus);

    /* Enemies car */

    Car Enemie1;

    Enemie1.state = 1;
    Enemie1.pos.x = 770; //la position initial de la voiture en x
    Enemie1.pos.y = 680; //la position initial de la voiture en y
    Enemie1.speed.x = 0;
    Enemie1.speed.y = 0;
    Enemie1.direction = 0;
    Enemie1.laps = 0;
    Enemie1.flag = 0;
    Enemie1.nbNitro = 3;
    Enemie1.lastNitroUsedTime = 0;
    Enemie1.malusBonusSpeed = 1.0;
    Enemie1.botPositionToTarget = 0;
    Enemie1.posInterBot.x = 0;
    Enemie1.posInterBot.y = 0;
    Enemie1.lastActive = 0;
    Enemie1.botType = "master";
    Enemie1.score = 0;
    Enemie1.color = sf::Color::Blue;

    Car Enemie2;

    Enemie2.state = 1;
    Enemie2.pos.x = 770; //la position initial de la voiture en x
    Enemie2.pos.y = 705; //la position initial de la voiture en y
    Enemie2.speed.x = 0;
    Enemie2.speed.y = 0;
    Enemie2.direction = 0;
    Enemie2.laps = 0;
    Enemie2.flag = 0;
    Enemie2.nbNitro = 3;
    Enemie2.lastNitroUsedTime = 0;
    Enemie2.malusBonusSpeed = 1.0;
    Enemie2.botPositionToTarget = 0;
    Enemie2.posInterBot.x = 0;
    Enemie2.posInterBot.y = 0;
    Enemie2.lastActive = 0;
    Enemie2.botType = "medium";
    Enemie2.score = 0;
    Enemie2.color = sf::Color::Yellow;

    Car Enemie3;

    Enemie3.state = 1;
    Enemie3.pos.x = 770; //la position initial de la voiture en x
    Enemie3.pos.y = 730; //la position initial de la voiture en y
    Enemie3.speed.x = 0;
    Enemie3.speed.y = 0;
    Enemie3.direction = 0;
    Enemie3.laps = 0;
    Enemie3.flag = 0;
    Enemie3.nbNitro = 3;
    Enemie3.lastNitroUsedTime = 0;
    Enemie3.malusBonusSpeed = 1.0;
    Enemie3.botPositionToTarget = 0;
    Enemie3.posInterBot.x = 0;
    Enemie3.posInterBot.y = 0;
    Enemie3.lastActive = 0;
    Enemie3.botType = "dumy";
    Enemie3.score = 0;
    Enemie3.color = sf::Color::Magenta;

    std::vector < Car * > Enemies;
    Enemies.push_back( & Enemie1);
    Enemies.push_back( & Enemie2);
    Enemies.push_back( & Enemie3);

    std::vector < Position > botLine;

    Position pos1;
    pos1.x = 450;
    pos1.y = 660;
    Position pos2;
    pos2.x = 290;
    pos2.y = 500;
    Position pos3;
    pos3.x = 280;
    pos3.y = 250;
    Position pos4;
    pos4.x = 500;
    pos4.y = 100;
    Position pos5;
    pos5.x = 700;
    pos5.y = 250;
    Position pos6;
    pos6.x = 735;
    pos6.y = 375;
    Position pos7;
    pos7.x = 845;
    pos7.y = 495;
    Position pos8;
    pos8.x = 1060;
    pos8.y = 510;
    Position pos9;
    pos9.x = 1060;
    pos9.y = 650;

    botLine.push_back(pos1);
    botLine.push_back(pos2);
    botLine.push_back(pos3);
    botLine.push_back(pos4);
    botLine.push_back(pos5);
    botLine.push_back(pos6);
    botLine.push_back(pos7);
    botLine.push_back(pos8);
    botLine.push_back(pos9);

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

            /*
             * Les événements dont vous aurez besoin sont Event::KeyPressed,
             * Event::KeyReleased et Event::MouseButtonPressed.
             */

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

        bool colisionMud = false;
        bool colisionFlag = false;
        float dt = clock.restart().asSeconds();
        if (idCurrentWindow == 0) {
            textAlphaValue += 170 * dt;
            textAlphaValue %= 510;

            if (enter) {
                idCurrentWindow = 3;
            }

        } else if (idCurrentWindow == 1) {
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
            for (int i = 0; i < (level.walls.size() + level.walls2.size()); i++) {
                if (i < level.walls.size()) {
                    wall = level.walls[i];
                } else {
                    wall = level.walls2[i - level.walls.size()];
                }
                if (isCollision(playerCar, wall, CAR_HAUTEUR / 2)) {
                    int direction = redirectIfPunchWall(playerCar, wall);
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

            //fin des colision joueur mur

            //fin des colision joueur mud
            //colision joueur flag
            colisionFlag = false;
            for (int i = 0; i < level.flags.size(); i++) {
                if (isCollision(playerCar, level.flags[i], CAR_HAUTEUR / 2)) {
                    colisionFlag = true;
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
            //cout<<playerCar.nbNitro<<endl;

            //On calcule ensuite la nouvelle vitesse de la voiture

            Speed playerNewSpeed = calculateSpeed(playerCar, ACCELERATION * playerCar.malusBonusSpeed, ACCELERATION, up, down, playerCar.lastNitroUsedTime >= 0, dt);

            //Timer Pour le spawn de nitro
            countNitro++;
            if (countNitro == NITRO_SPAWN_TIME) {
                generateNitro(level.spawnPosNitro);
                countNitro = 0;
            }

            playerCar.speed = playerNewSpeed;
            moveCar(playerCar, dt);
            playerCar.malusBonusSpeed = 1;
            //truc pour les ennemies
            for (int j = 0; j < Enemies.size(); j++) {
                Car * enemie = Enemies[j];

                if (enemie -> lastActive <= 0) {
                    float pointx;
                    float pointy;
                    if (enemie -> posInterBot.x > 0) {
                        pointx = enemie -> posInterBot.x;
                        pointy = enemie -> posInterBot.y;
                    } else {
                        pointx = botLine[enemie -> botPositionToTarget].x;
                        pointy = botLine[enemie -> botPositionToTarget].y;
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

                

				
                if (isCollision( * enemie, botLine[enemie -> botPositionToTarget], CAR_LONGUEUR)) {
                    int randomDistForBot = 0;
                    if (enemie -> botType == "master") {
                        randomDistForBot = RANDOM_DIST_FOR_BOTS_MASTERMIND;
                    } else if (enemie -> botType == "medium") {
                        randomDistForBot = RANDOM_DIST_FOR_BOTS_MEDIUM;
                    } else if (enemie -> botType == "dumy") {
                        randomDistForBot = RANDOM_DIST_FOR_BOTS_DUMY;
                    } else {
                        randomDistForBot = RANDOM_DIST_FOR_BOTS;
                    }
                    enemie -> posInterBot = randomInCircle(centerPosition(botLine[enemie -> botPositionToTarget], botLine[fmod(enemie -> botPositionToTarget + 1, botLine.size())]), randomDistForBot);

                    enemie -> botPositionToTarget = fmod(enemie -> botPositionToTarget + 1, botLine.size());
                }
                
				float botSpeedType = 1.0;
				double botChanceNitro = 0;
				
                if (enemie -> botType == "master") {
                                        botSpeedType = 1.3;
                                        botChanceNitro = 0.0009;
                                    } else if (enemie -> botType == "medium") {
                                        botSpeedType = 0.95;
                                        botChanceNitro = 0.005;
                                    } else if (enemie -> botType == "dumy") {
                                        botSpeedType = 0.80;
                                        botChanceNitro = 0.001;
                                    } else {
                                        botSpeedType = 0.95;
                                        botChanceNitro = 0.001;
                                    }
                
                if (Math::random() < botChanceNitro && enemie -> lastNitroUsedTime <= 0 && enemie -> nbNitro > 0) {
                                    enemie -> lastNitroUsedTime = TIME_NITRO_USED;
                                    enemie -> nbNitro -= 1;
                                }
                
                

                if (enemie -> posInterBot.x > 0) {
                    if (isCollision( * enemie, enemie -> posInterBot, CAR_LONGUEUR)) {
                        enemie -> posInterBot.x = -1;
                    }
                }

                if (enemie -> lastActive > 0) {
                    enemie -> lastActive -= dt;
                }

                Wall wall;
                bool collisionWall = false;
                for (int i = 0; i < (level.walls.size() + level.walls2.size()); i++) {
                    if (i < level.walls.size()) {
                        wall = level.walls[i];
                    } else {
                        wall = level.walls2[i - level.walls.size()];
                    }
                    if (isCollision( * enemie, wall, CAR_HAUTEUR / 2)) {
                        int direction = redirectIfPunchWall(enemie, wall);
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
                //On calcule ensuite la nouvelle vitesse de la voiture
                Speed enemieNewSpeed = calculateSpeed( * enemie, (ACCELERATION * enemie -> malusBonusSpeed) * botSpeedType, ACCELERATION * botSpeedType, true, false, enemie -> lastNitroUsedTime >= 0, dt);

                enemie -> speed = enemieNewSpeed;
                moveCar(enemie, dt);
                enemie -> malusBonusSpeed = 1;
            }
            //fin du truc pour les ennemies
            if (playerCar.laps == NB_LAPS_FIN) {
                playerCar.score = score;
                score++;
                idCurrentWindow = 2;
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
                idCurrentWindow = 2;
                playerCar.score = score;
            }
            //cout<<score<<endl;
        } else if (idCurrentWindow == 2) {

            if (enter) {
                idCurrentWindow = 3;
                timer = 0;
                score = 0;
                reset(playerCar);
                for (int j = 0; j < Enemies.size(); j++) {
                    Car * enemie = Enemies[j];
                    reset(enemie);
                }
            }
        } else if (idCurrentWindow == 3) {
            timer += dt;
            if (timer >= 3.0) {
                idCurrentWindow = 1;
            }
        }
        /*
         * Affichage de l'état du jeu
         */

        window.clear(Color::White);

        if (idCurrentWindow == 0) {

            sf::Text enterText = sf::Text();
            enterText.setString("Insert COIN (or press enter)");
            enterText.setFont(font);
            enterText.setCharacterSize(60);

            if (textAlphaValue <= 255) {
                enterText.setFillColor(sf::Color(0, 0, 0, textAlphaValue));

            } else {
                enterText.setFillColor(sf::Color(0, 0, 0, 509 - textAlphaValue));
            }

            enterText.setPosition(WINDOW_WIDTH / 2 - enterText.getLocalBounds().width / 2, WINDOW_HEIGHT * 7 / 8 - enterText.getLocalBounds().height / 2);

            window.draw(enterText);
        } else if (idCurrentWindow == 1) {

            window.draw(background);

            sf::RectangleShape carShape(sf::Vector2f(CAR_LONGUEUR, CAR_HAUTEUR));
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

            std::vector < sf::RectangleShape > listWallPrint;
            sf::RectangleShape wallShape;
            sf::VertexArray lines(sf::LineStrip, level.walls.size() + 1);
            sf::VertexArray lines2(sf::LineStrip, level.walls2.size() + 1);
            for (int i = 0; i < level.walls.size(); ++i) {
                lines[i].position = sf::Vector2f(level.walls[i].hitbox.corner1.x, level.walls[i].hitbox.corner1.y);
                lines[i].color = sf::Color::Red;
            }
            lines[level.walls.size()].position = sf::Vector2f(level.walls[0].hitbox.corner1.x, level.walls[0].hitbox.corner1.y);
            lines[level.walls.size()].color = sf::Color::Red;
            window.draw(lines);
            for (int i = 0; i < level.walls2.size(); ++i) {
                lines2[i].position = sf::Vector2f(level.walls2[i].hitbox.corner1.x, level.walls2[i].hitbox.corner1.y);
                lines2[i].color = sf::Color::Red;
            }
            lines2[level.walls2.size()].position = sf::Vector2f(level.walls2[0].hitbox.corner1.x, level.walls2[0].hitbox.corner1.y);
            lines2[level.walls2.size()].color = sf::Color::Red;
            window.draw(lines2);

            int posXaffichage = 900;
            int posYaffichage = 100;

            sf::RectangleShape infoShape(sf::Vector2f(70, 100));
            infoShape.setPosition(posXaffichage - 5, posYaffichage + 10);
            infoShape.setFillColor(playerCar.color);
            window.draw(infoShape);

            sf::Text tourCountText = sf::Text();
            sf::Text horloge = sf::Text();
            horloge.setFont(font);
            horloge.setCharacterSize(40);
            horloge.setString(std::to_string(timer));
            horloge.setFillColor(sf::Color::Black);
            horloge.setPosition(posXaffichage + 140 - horloge.getLocalBounds().width / 2, posYaffichage - 60);

            tourCountText.setFont(font);
            tourCountText.setCharacterSize(60);

            sf::Text nitroCountText = sf::Text();

            nitroCountText.setFont(font);
            nitroCountText.setCharacterSize(30);

            if (playerCar.laps < 10) {
                tourCountText.setString("0" + std::to_string(playerCar.laps));
            } else {
                tourCountText.setString(std::to_string(playerCar.laps));
            }
            nitroCountText.setString(std::to_string(playerCar.nbNitro));

            tourCountText.setFillColor(sf::Color::Black);
            nitroCountText.setFillColor(sf::Color::Black);

            tourCountText.setPosition(posXaffichage, posYaffichage);
            nitroCountText.setPosition(posXaffichage + 20, posYaffichage + 60);

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
                nitroCountText.setCharacterSize(30);

                if (enemie -> laps < 10) {
                    tourCountText.setString("0" + std::to_string(enemie -> laps));
                } else {
                    tourCountText.setString(std::to_string(enemie -> laps));
                }
                nitroCountText.setString(std::to_string(enemie -> nbNitro));

                tourCountText.setFillColor(sf::Color::Black);
                nitroCountText.setFillColor(sf::Color::Black);

                tourCountText.setPosition(posXaffichage + (10 + 60) * (i + 1), posYaffichage);
                nitroCountText.setPosition(posXaffichage + (10 + 60) * (i + 1) + 20, posYaffichage + 60);

                infoShape.setPosition(posXaffichage + (10 + 60) * (i + 1) - 5, posYaffichage + 10);
                infoShape.setFillColor(enemie -> color);
                window.draw(infoShape);

                window.draw(tourCountText);
                window.draw(nitroCountText);
            }

            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(750, 600)),
                sf::Vertex(sf::Vector2f(750, 750))
            };
            line[0].color = sf::Color::Red;
            line[1].color = sf::Color::Red;
            window.draw(line, 2, sf::Lines);

        } else if (idCurrentWindow == 2) {

            int posXaffichage = 400;
            int posYaffichage = 100;

            sf::RectangleShape infoShape(sf::Vector2f(80, 80));
            for (int j = 0; j < Enemies.size(); j++) {
                Car * enemie = Enemies[j];
                if (enemie -> score == 0) {
                    enemie -> score = score;
                    score++;
                }

                infoShape.setPosition(posXaffichage + (20 + 80) * (enemie -> score - 1), posYaffichage + 70);
                infoShape.setFillColor(enemie -> color);
                window.draw(infoShape);
            }

            sf::Text firstText = sf::Text();
            sf::Text secondText = sf::Text();
            sf::Text thirdText = sf::Text();
            sf::Text fourText = sf::Text();

            firstText.setFont(font);
            secondText.setFont(font);
            thirdText.setFont(font);
            fourText.setFont(font);

            firstText.setString("1st");
            secondText.setString("2nd");
            thirdText.setString("3rd");
            fourText.setString("4d");

            firstText.setPosition(posXaffichage + (40 + 60) * (0), posYaffichage);
            secondText.setPosition(posXaffichage + (40 + 60) * (1), posYaffichage);
            thirdText.setPosition(posXaffichage + (40 + 60) * (2), posYaffichage);
            fourText.setPosition(posXaffichage + (40 + 60) * (3), posYaffichage);

            firstText.setFillColor(sf::Color::Black);
            secondText.setFillColor(sf::Color::Black);
            thirdText.setFillColor(sf::Color::Black);
            fourText.setFillColor(sf::Color::Black);

            window.draw(firstText);
            window.draw(secondText);
            window.draw(thirdText);
            window.draw(fourText);

            infoShape.setPosition(posXaffichage + (20 + 80) * (playerCar.score - 1), posYaffichage + 70);
            infoShape.setFillColor(playerCar.color);
            window.draw(infoShape);

            sf::Text resultText = sf::Text();
            resultText.setFont(font);
            resultText.setFillColor(sf::Color::Black);

            if (playerCar.score == 1) {
                resultText.setString("Vous etes premier");
                resultText.setPosition(WINDOW_WIDTH / 2 - resultText.getLocalBounds().width / 2, WINDOW_HEIGHT * 1 / 2 - resultText.getLocalBounds().height / 2);
            } else {
                resultText.setString("Vous avez perdu!!!");
                resultText.setPosition(WINDOW_WIDTH / 2 - resultText.getLocalBounds().width / 2, WINDOW_HEIGHT * 1 / 2 - resultText.getLocalBounds().height / 2);
            }
            window.draw(resultText);

        } else if (idCurrentWindow == 3) {

            std::string countdown;
            if (timer < 1.0) {
                countdown = std::to_string(3);
            } else if (timer < 2.0) {
                countdown = std::to_string(2);
            } else if (timer < 3.0) {
                countdown = std::to_string(1);
            }

            window.draw(background);

            sf::RectangleShape carShape(sf::Vector2f(CAR_LONGUEUR, CAR_HAUTEUR));
            sf::RectangleShape mudShape;
            sf::CircleShape nitroShape;

            for (int j = 0; j < Enemies.size(); j++) {
                Car * enemie = Enemies[j];
                carShape.setPosition(enemie -> pos.x, enemie -> pos.y);
                carShape.setOrigin(CAR_LONGUEUR / 2, CAR_HAUTEUR / 2);
                carShape.setRotation(180 - (enemie -> direction / 16.0 * 360));
                carShape.setFillColor(enemie -> color);
                window.draw(carShape);
            }
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

            carShape.setPosition(playerCar.pos.x, playerCar.pos.y);
            carShape.setOrigin(CAR_LONGUEUR / 2, CAR_HAUTEUR / 2);
            carShape.setRotation(180 - (playerCar.direction / 16.0 * 360));
            carShape.setFillColor(playerCar.color);

            window.draw(carShape);

            std::vector < sf::RectangleShape > listWallPrint;
            sf::RectangleShape wallShape;
            sf::VertexArray lines(sf::LineStrip, level.walls.size() + 1);
            sf::VertexArray lines2(sf::LineStrip, level.walls2.size() + 1);
            for (int i = 0; i < level.walls.size(); ++i) {
                lines[i].position = sf::Vector2f(level.walls[i].hitbox.corner1.x, level.walls[i].hitbox.corner1.y);
                lines[i].color = sf::Color::Red;
            }
            lines[level.walls.size()].position = sf::Vector2f(level.walls[0].hitbox.corner1.x, level.walls[0].hitbox.corner1.y);
            lines[level.walls.size()].color = sf::Color::Red;
            window.draw(lines);
            for (int i = 0; i < level.walls2.size(); ++i) {
                lines2[i].position = sf::Vector2f(level.walls2[i].hitbox.corner1.x, level.walls2[i].hitbox.corner1.y);
                lines2[i].color = sf::Color::Red;
            }
            lines2[level.walls2.size()].position = sf::Vector2f(level.walls2[0].hitbox.corner1.x, level.walls2[0].hitbox.corner1.y);
            lines2[level.walls2.size()].color = sf::Color::Red;
            window.draw(lines2);

            int posXaffichage = 900;
            int posYaffichage = 100;

            sf::RectangleShape infoShape(sf::Vector2f(70, 100));
            infoShape.setPosition(posXaffichage - 5, posYaffichage + 10);
            infoShape.setFillColor(playerCar.color);
            window.draw(infoShape);

            sf::Text tourCountText = sf::Text();

            tourCountText.setFont(font);
            tourCountText.setCharacterSize(60);

            sf::Text nitroCountText = sf::Text();

            nitroCountText.setFont(font);
            nitroCountText.setCharacterSize(30);

            if (playerCar.laps < 10) {
                tourCountText.setString("0" + std::to_string(playerCar.laps));
            } else {
                tourCountText.setString(std::to_string(playerCar.laps));
            }
            nitroCountText.setString(std::to_string(playerCar.nbNitro));

            tourCountText.setFillColor(sf::Color::Black);
            nitroCountText.setFillColor(sf::Color::Black);

            tourCountText.setPosition(posXaffichage, posYaffichage);
            nitroCountText.setPosition(posXaffichage + 20, posYaffichage + 60);

            window.draw(tourCountText);
            window.draw(nitroCountText);

            for (int i = 0; i < Enemies.size(); i++) {
                Car * enemie = Enemies[i];
                sf::Text tourCountText = sf::Text();

                tourCountText.setFont(font);
                tourCountText.setCharacterSize(60);

                sf::Text nitroCountText = sf::Text();

                nitroCountText.setFont(font);
                nitroCountText.setCharacterSize(30);

                if (enemie -> laps < 10) {
                    tourCountText.setString("0" + std::to_string(enemie -> laps));
                } else {
                    tourCountText.setString(std::to_string(enemie -> laps));
                }
                nitroCountText.setString(std::to_string(enemie -> nbNitro));

                tourCountText.setFillColor(sf::Color::Black);
                nitroCountText.setFillColor(sf::Color::Black);

                tourCountText.setPosition(posXaffichage + (10 + 60) * (i + 1), posYaffichage);
                nitroCountText.setPosition(posXaffichage + (10 + 60) * (i + 1) + 20, posYaffichage + 60);

                infoShape.setPosition(posXaffichage + (10 + 60) * (i + 1) - 5, posYaffichage + 10);
                infoShape.setFillColor(enemie -> color);
                window.draw(infoShape);

                window.draw(tourCountText);
                window.draw(nitroCountText);
            }

            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(750, 600)),
                sf::Vertex(sf::Vector2f(750, 750))
            };
            line[0].color = sf::Color::Red;
            line[1].color = sf::Color::Red;
            window.draw(line, 2, sf::Lines);

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

        window.display();

        sf::sleep(sf::seconds((1.0 / MAX_FPS) - clock.getElapsedTime().asSeconds()));
        //std::cout << framerate << std::endl;

    }
    return 0;
}