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
    int state; //1 pour normal, 2 contre un mur, 3 dans la boue, 4 contre un mur dans la boue
    Position pos;
    Speed speed;
    Speed lastSpeed;
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
    int points = 0;
    int startPosition;
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
    if (hypot(car->pos.x - pos.x, car->pos.y - pos.y) <= rayon) {
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
    	if (normeSpeed >(float) maxSpeed) {
    	                accelerationX = car.speed.x/(float)(car.speed.x + car.speed.y) * maxSpeed;
    	                accelerationY = car.speed.y/(float)(car.speed.x + car.speed.y) * maxSpeed;

    	                speed.x = accelerationX;
    	                speed.y = accelerationY;
    	            }
    	else if ((normeSpeed / maxSpeed) > 0.2) {
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
    car.pos.x = car.pos.x + (car.speed.x) * dt;
    car.pos.y = car.pos.y + (car.speed.y) * dt;
}

void moveCar(Car * car,
    const float & dt) {
    car -> pos.x = car -> pos.x + (car -> speed.x) * dt;
    car -> pos.y = car -> pos.y + (car -> speed.y)* dt;
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


Speed calculateProjectionOfSpeed(Speed speedToProject, sf::Vector2f vectorBase){
	if ((speedToProject.x == 0 && speedToProject.y == 0) || (vectorBase.x == 0 && vectorBase.y == 0)){
		Speed newSpeed;
			newSpeed.x = 0;
			newSpeed.y = 0;
			return newSpeed;
	}
	float scalar = speedToProject.x * vectorBase.y + speedToProject.y * vectorBase.x ;
	Speed newSpeed;
	newSpeed.x = scalar / (calculateNorme(vectorBase.x, vectorBase.y)*calculateNorme(vectorBase.x, vectorBase.y)) * vectorBase.x;
	newSpeed.y = scalar / (calculateNorme(vectorBase.x, vectorBase.y)*calculateNorme(vectorBase.x, vectorBase.y)) * vectorBase.y;
	return newSpeed;
}

sf::Vector2f calculateProjection(sf::Vector2f vectorToProject, sf::Vector2f vectorBase){
	if ((vectorToProject.x == 0 && vectorToProject.y == 0) || (vectorBase.x == 0 && vectorBase.y == 0)){
		sf::Vector2f projection;
			projection.x = 10000;
			projection.y = 10000;
			return projection;
	}
	float scalar = vectorToProject.x * vectorBase.y + vectorToProject.y * vectorBase.x;
	sf::Vector2f projection;
	projection.x = scalar / (calculateNorme(vectorBase.x, vectorBase.y)*calculateNorme(vectorBase.x, vectorBase.y)) * vectorBase.x;
	projection.y = scalar / (calculateNorme(vectorBase.x, vectorBase.y)*calculateNorme(vectorBase.x, vectorBase.y)) * vectorBase.y;
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
    car.nbNitro = 3;
    car.lastNitroUsedTime = 0;
    car.malusBonusSpeed = 1.0;
    car.lastActive = 0;
    car.score = 0;
}
void reset(Car * car, Ground & level) {
    car -> state = 1;

    car -> pos = level.spawnPos[car->startPosition];
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

 void makeLevel(Ground & level, std::string src) {
   Ground cache;
   ifstream levelData(src);

   if (levelData) {
	   
	   for (int i=0; i <4; i++){
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

           mud.pos.x = std::stoi(m[1])+std::stoi(m[3]);
           mud.pos.y = std::stoi(m[2])+std::stoi(m[3]);
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
                    	   
                    	 if (std::stoi(m[3]) > 0 && std::stoi(m[3]) <= 4){
                    		 level.spawnPos[std::stoi(m[3])-1].x = std::stoi(m[1]);
                    		 level.spawnPos[std::stoi(m[3])-1].y = std::stoi(m[2]);
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
 
 int idPositionMinimum(Car* carList[], int maximum){
	 int idMinimumActuel = 0;
	 for (int i=0; i<maximum ; i++){
	 		if (carList[idMinimumActuel]->points > carList[i]->points){
	 			idMinimumActuel = i;
	 		}
	 	}
	 return idMinimumActuel;
	 
 }
 
void triSelectionDecroissant(Car* carList[], int maximum){
	int idMinimumActuel = 0;
	Car * aux;
	for (int i=maximum-1; i>0 ; i--){
		idMinimumActuel = idPositionMinimum(carList, i);
		if (idMinimumActuel != i){
			aux = carList[i];
			carList[i] = carList[idMinimumActuel];
			carList[idMinimumActuel] = aux;
		}
	}
}

void loadFromFile(sf::Texture & texture, sf::Sprite & sprite, const std::string & name){
	texture.loadFromFile(name);
	sprite.setTexture(texture);
	
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

	#ifdef __linux__ 
	system("./h-linux &");
	#elif _WIN32
	system("h-windos.exe &");
	#endif
	
	
    const int WINDOW_WIDTH = 1200;
    const int WINDOW_HEIGHT = 800;
    const std::string WINDOW_TITLE = "Super off Road";
    const int MAX_FPS = 120;
    
    const int MAX_BOT_RANGE_TO_GET_POWERUP = 20;

    const float TIME_BEFORE_REACTIVATE = 0.06;

    int idCurrentWindow = 0;

    std::string levelFile("level1");

    sf::Font font;
    font.loadFromFile("PixelOperator.ttf");

    const int NITRO_SPAWN_TIME = 1000;
    const int ACCELERATION = 50;
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
    
    Assets asssets;

    /*
     * Variables pour l'ecran titre
     */

    int textAlphaValue = 0;
    float textScale = 0.0;
    float carScale = 0.5;
    sf::Vector2f carMove(-200,700);

    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);

    /*
     * Une Clock permet de compter le temps. Vous en aurez besoin pour savoir
     * le temps entre deux frames.
     */
    Clock clock;

    Ground level;
    makeLevel(level, levelFile + ".txt");
    loadFromFile(asssets.backgroundLevelScreenTexture, asssets.backgroundLevelScreen, levelFile + ".png");
    loadFromFile(asssets.backgroundMainScreenTexture, asssets.backgroundMainScreen, "assets/fond_ecran_principal.png");
    
    loadFromFile(asssets.superoffroadTextTexture, asssets.superoffroadText, "assets/title_screen.png");
    loadFromFile(asssets.superoffroadCarTexture, asssets.superoffroadCar,"assets/car_begin.png");

    asssets.backgroundLevelScreen.setScale(sf::Vector2f((WINDOW_WIDTH / asssets.backgroundLevelScreenTexture.getSize().x), (WINDOW_HEIGHT / asssets.backgroundLevelScreenTexture.getSize().y)));
    asssets.backgroundMainScreen.setScale(sf::Vector2f((WINDOW_WIDTH / asssets.backgroundMainScreenTexture.getSize().x), (WINDOW_HEIGHT / asssets.backgroundMainScreenTexture.getSize().y)));
    

    asssets.superoffroadText.setPosition((WINDOW_WIDTH)/2, (WINDOW_HEIGHT/6) + 100);
    asssets.superoffroadText.setOrigin(asssets.superoffroadTextTexture.getSize().x/2, asssets.superoffroadTextTexture.getSize().y/2);
    asssets.superoffroadText.setScale(textScale,textScale);
    
    
    asssets.superoffroadCar.setPosition(carMove.x, carMove.y);
    asssets.superoffroadCar.setOrigin(asssets.superoffroadCarTexture.getSize().x/2, asssets.superoffroadCarTexture.getSize().y/2);
    asssets.superoffroadCar.setScale(carScale,carScale);
    
    loadFromFile(asssets.nitroTexture, asssets.nitro, "assets/nitroSprite.png");
    asssets.nitro.setOrigin(asssets.nitroTexture.getSize().x/2, asssets.nitroTexture.getSize().y/2);
    
    loadFromFile(asssets.scoreTexture, asssets.score, "assets/scoreSprite.png");
    asssets.score.setOrigin(asssets.scoreTexture.getSize().x/2, asssets.scoreTexture.getSize().y/2);
    asssets.score.setScale(2,2);
    
    Car playerCar;
    playerCar.state = 1;
    playerCar.pos= level.spawnPos[0];
    playerCar.speed.x = 0;
    playerCar.speed.y = 0;
	playerCar.lastSpeed.x = 0;
	playerCar.lastSpeed.y = 0;
    playerCar.direction = 0;
    playerCar.laps = 0;
    playerCar.flag = 0;
    playerCar.nbNitro = 3;
    playerCar.lastNitroUsedTime = 0;
    playerCar.malusBonusSpeed = 1.0;
    playerCar.lastActive = 0;
    playerCar.score = 0;
    playerCar.color = sf::Color::Red;
    float timer = 0;
    int nbFlag;
    int countNitro = 0;
    int score = 1;

    if (DEBUG){
        printListWall(level.walls);
    }

    
    nbFlag = level.flags.size();

    /* Enemies car */

    Car Enemie1;

    Enemie1.state = 1;
    Enemie1.pos = level.spawnPos[1];
    Enemie1.speed.x = 0;
    Enemie1.speed.y = 0;
    Enemie1.lastSpeed.x = 0;
    Enemie1.lastSpeed.y = 0;
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
    Enemie2.pos = level.spawnPos[2];
    Enemie2.speed.x = 0;
    Enemie2.speed.y = 0;
    Enemie2.lastSpeed.x = 0;
	Enemie2.lastSpeed.y = 0;
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
    Enemie3.pos = level.spawnPos[3];
    Enemie3.speed.x = 0;
    Enemie3.speed.y = 0;
    Enemie3.lastSpeed.x = 0;
    Enemie3.lastSpeed.y = 0;
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

        float dt = clock.restart().asSeconds();
        if (idCurrentWindow == 0) {
        	if (textScale < 1.5){
        		textScale += 0.66*dt;
        	}
        	else {
        		textScale = 1.5;
        	}
        	if (carScale < 1.5){
        		if (textScale>=1.5){
            		carScale +=0.66*dt;
            		carMove.x += 333.33*dt;
            		carMove.y -= 133.33*dt;
        		}
	           
        	}
        	else {
        		carScale = 1.5;
        		carMove.x = 300;
        		carMove.y = 500;
        	}
        	if (carScale>=1.5){
        		 textAlphaValue += 170 * dt;
        		textAlphaValue %= 510;
        	}
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
            for (int i = 0; i < level.walls.size(); i++) {
                    wall = level.walls[i];
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
                                 // for (int j = 0; j < Enemies.size(); j++) {
                                 //                 Car * enemie2 = Enemies[j];
                                 //                 if (isCollision(playerCar, enemie2 -> pos, CAR_HAUTEUR)) {
                                 //                 	Speed tempSpeed = calculateProjectionOfSpeed(playerCar.lastSpeed, sf::Vector2f(enemie2 -> pos.x - playerCar.pos.x, enemie2 -> pos.y - playerCar.pos.y));
                                 //                 	playerCar.speed.x -= 3.05 * (tempSpeed.x*(enemie2 -> pos.x - playerCar.pos.x)*dt);
                                 //                 	playerCar.speed.y -= 3.05 * (tempSpeed.x*(enemie2 -> pos.y - playerCar.pos.y)*dt);
                                 //                 	enemie2 -> speed.x += 3.00 * (tempSpeed.x*(enemie2 -> pos.x - playerCar.pos.x)*dt);
                                 //                 	enemie2 -> speed.y += 3.00 * (tempSpeed.x*(enemie2 -> pos.y - playerCar.pos.y)*dt);
                                 //                 }
                                 //             }
                                 // 
            //On calcule ensuite la nouvelle vitesse de la voiture

            //Speed playerNewSpeed = calculateSpeed(playerCar, ACCELERATION * playerCar.malusBonusSpeed, ACCELERATION, up, down, playerCar.lastNitroUsedTime >= 0, dt);
            playerCar.speed = calculateSpeed(playerCar, ACCELERATION * playerCar.malusBonusSpeed, ACCELERATION, up, down, playerCar.lastNitroUsedTime >= 0, dt);

            
            //Timer Pour le spawn de nitro
            countNitro++;
            if (countNitro == NITRO_SPAWN_TIME) {
                generateNitro(level.spawnPosNitro);
                countNitro = 0;
            }

            
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
                                                        botSpeedType = 1.15;
                                                        botChanceNitro = 0.0009;
                                                        chanceToGetPowerUp = 0.9;
                                                    } else if (enemie -> botType == "medium") {
                                                        botSpeedType = 0.95;
                                                        botChanceNitro = 0.005;
                                                        chanceToGetPowerUp = 0.6;
                                                    } else if (enemie -> botType == "dumy") {
                                                        botSpeedType = 0.80;
                                                        botChanceNitro = 0.001;
                                                        chanceToGetPowerUp = 0.2;
                                                    } else {
                                                        botSpeedType = 0.95;
                                                        botChanceNitro = 0.001;
                                                        chanceToGetPowerUp = 0.5;
                                                    }

				
                if (isCollision( * enemie, level.botLine[enemie -> botPositionToTarget], CAR_LONGUEUR)) {
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
                    
                    sf::Vector2f vectorBetweenPosAndNewTarget = Vector2f(level.botLine[fmod(enemie -> botPositionToTarget + 1, level.botLine.size())].x - enemie -> pos.x, level.botLine[fmod(enemie -> botPositionToTarget + 1, level.botLine.size())].y - enemie -> pos.y);

		
		bool hasAlreadyATarget = false;
		if (Math::random() < chanceToGetPowerUp){
		for (int i = 0; i < level.spawnPosNitro.size(); i++) {
                           if (level.spawnPosNitro[i].present) {
                        	   
                        	   
                        	   sf::Vector2f vectorToNitro = sf::Vector2f(level.spawnPosNitro[i].pos.x - enemie -> pos.x, level.spawnPosNitro[i].pos.y - enemie -> pos.y);
                        	                           		   
                        	   
                        	   if (vectorDotProduct(vectorBetweenPosAndNewTarget, vectorToNitro) > 0){
                        		   
                        		   sf::Vector2f pojection = calculateProjection(vectorToNitro, vectorBetweenPosAndNewTarget);
                        		   //SI la distance sur x < 1 alors elle est plus proche de la cible et si en y elle est pas trop loing de l'axe
                        		   if (abs(pojection.x) < 1 && abs(pojection.y) < MAX_BOT_RANGE_TO_GET_POWERUP){
                        			   enemie -> posInterBot.x = level.spawnPosNitro[i].pos.x + level.spawnPosNitro[i].rayon;
                        			   enemie -> posInterBot.y = level.spawnPosNitro[i].pos.y + level.spawnPosNitro[i].rayon;
                        			   hasAlreadyATarget = true;
                        		   }
								   
                        	   }
                           } 
                    }
		}
                    
		if (!hasAlreadyATarget && calculateNorme(vectorBetweenPosAndNewTarget.x , vectorBetweenPosAndNewTarget.y) > 60){
			enemie -> posInterBot = randomInCircle(centerPosition(level.botLine[enemie -> botPositionToTarget], level.botLine[fmod(enemie -> botPositionToTarget + 1, level.botLine.size())]), randomDistForBot);
			
		}
		enemie -> botPositionToTarget = fmod(enemie -> botPositionToTarget + 1, level.botLine.size());
                }
                
				
                
                if (Math::random() < botChanceNitro && enemie -> lastNitroUsedTime <= 0 && enemie -> nbNitro > 0) {
                                    enemie -> lastNitroUsedTime = TIME_NITRO_USED;
                                    enemie -> nbNitro -= 1;
                                }
                
                

                if (enemie -> posInterBot.x > 0) {
                    if (isCollision( * enemie, enemie -> posInterBot, CAR_LONGUEUR*3/4)) {
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
                    	enemie->lastSpeed.x = 0;
                    	enemie->lastSpeed.y = 0;
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
                
                //for (int k = 0; k < Enemies.size(); k++) {
                //                                                                    Car * enemie2 = Enemies[k];
                //                                                                    if (enemie2 != enemie){
                //                                                                    if (isCollision(enemie, enemie2 -> pos, CAR_HAUTEUR)) {
                //                                                                    	Speed tempSpeed = calculateProjectionOfSpeed(enemie -> lastSpeed, sf::Vector2f(enemie2 -> pos.x - enemie -> pos.x, enemie2 -> pos.y - enemie -> pos.y));
                //                                                                    	enemie->speed.x -= 10.05 * (tempSpeed.x*(enemie2 -> pos.x - enemie -> pos.x)*dt);
                //                                                                    	enemie->speed.y -= 10.05 * (tempSpeed.x*(enemie2 -> pos.y - enemie -> pos.y)*dt);
                //                                                                    	enemie2 -> speed.x += 10.00 * (tempSpeed.x*(enemie2 -> pos.x - enemie -> pos.x)*dt);
                //                                                                    	enemie2 -> speed.y += 10.00 * (tempSpeed.x*(enemie2 -> pos.y - enemie -> pos.y)*dt);
                //                                                                    }
                //                                                                    }
                //                                                                }
                //
                //if (isCollision(playerCar, enemie -> pos, CAR_HAUTEUR)) {
                //                                                    	Speed tempSpeed = calculateProjectionOfSpeed(enemie -> lastSpeed, sf::Vector2f(playerCar.pos.x- enemie -> pos.x , playerCar.pos.y- enemie -> pos.y ));
                //                                                    	playerCar.speed.x -= 3.00 * (tempSpeed.x*(playerCar.pos.x- enemie -> pos.x)*dt);
                //                                                    	playerCar.speed.y -= 3.00 * (tempSpeed.x*(playerCar.pos.y- enemie -> pos.y)*dt);
                //                                                    	enemie -> speed.x += 3.05 * (tempSpeed.x*(playerCar.pos.x- enemie -> pos.x)*dt);
                //                                                    	enemie -> speed.y += 3.05 * (tempSpeed.x*(playerCar.pos.y- enemie -> pos.y)*dt);
                //                                                    }
                //
                //On calcule ensuite la nouvelle vitesse de la voiture
                enemie -> speed = calculateSpeed( * enemie, (ACCELERATION * enemie -> malusBonusSpeed) * botSpeedType, ACCELERATION * botSpeedType, true, false, enemie -> lastNitroUsedTime >= 0, dt);
                
                enemie -> malusBonusSpeed = 1;
                
                
                
            }
            
            playerCar.lastSpeed = playerCar.speed;
            
            moveCar(playerCar, dt);
            for (int j = 0; j < Enemies.size(); j++) {
                            Car * enemie = Enemies[j];
                            enemie ->lastSpeed = enemie->speed;
                            moveCar(enemie, dt);
                        }
            
            
            //fin du truc pour les ennemies
            if (playerCar.laps == NB_LAPS_FIN) {
                playerCar.score = score;
                playerCar.points += 5 - score;
                score++;
                idCurrentWindow = 2;
                for (int j = 0; j < Enemies.size(); j++) {
                    Car * enemie = Enemies[j];
                    if (enemie -> score == 0) {
                        enemie -> score = score;
                        enemie -> points += 5 - score;
                        score++;
                    }
                }

            }
            for (int j = 0; j < Enemies.size(); j++) {
                Car * enemie = Enemies[j];
                if (enemie -> laps >= NB_LAPS_FIN && enemie -> score == 0) {
                    enemie -> score = score;
                    enemie -> points += 5 - score;
                    score++;
                }
            }
            if (score == 4) {
                idCurrentWindow = 2;
                playerCar.score = score;
                playerCar.points += 5 - score;
            }
            //cout<<score<<endl;
        } else if (idCurrentWindow == 2) {

            if (enter) {
                idCurrentWindow = 3;
                timer = 0;
                score = 0;
                
                Car * tri[4];
                tri[0] = &playerCar;
                
                for (int i =0; i < Enemies.size(); i++) {
                	tri[i+1] = Enemies[i];
                }
                triSelectionDecroissant(tri,4);
                
                for (int i =0; i < 4; i++) {
                    tri[i]->startPosition = i;
                }
                
                reset(playerCar, level);
                for (int j = 0; j < Enemies.size(); j++) {
                    Car * enemie = Enemies[j];
                    reset(enemie, level);
                }
            }
        } else if (idCurrentWindow == 3) {
            timer += dt;
            if (timer >= 3.0) {
                idCurrentWindow = 1;
                timer = 0.0;
            }
        }
        /*
         * Affichage de l'état du jeu
         */

        window.clear(Color::White);

        if (idCurrentWindow == 0) {
        	window.draw(asssets.backgroundMainScreen);
        	
        	asssets.superoffroadText.setScale(textScale,textScale);
        	window.draw(asssets.superoffroadText);
        	
        	asssets.superoffroadCar.setPosition(carMove.x, carMove.y);
        	asssets.superoffroadCar.setScale(carScale,carScale);
        	window.draw(asssets.superoffroadCar);
        	
        	
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
        //Si on est dans la partie ou sur le décopte
		 if (idCurrentWindow == 1 || idCurrentWindow == 3) {

            window.draw(asssets.backgroundLevelScreen);
            
            for (int j = 0; j < level.spawnPosNitro.size(); j++) {
            	                if (level.spawnPosNitro[j].present) {
            	                    asssets.nitro.setPosition(level.spawnPosNitro[j].pos.x, level.spawnPosNitro[j].pos.y);
            	                    window.draw(asssets.nitro);
            	                }
            	            }

		 
		 
			if (DEBUG){
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
            
            if (DEBUG){
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
	            
	            for (int i=0; i<level.botLine.size();i++){
	            	nitroShape.setRadius(2);
	            	                    nitroShape.setPosition(level.botLine[i].x, level.botLine[i].y);
	            	                    nitroShape.setOrigin(1, 1);
	            	                    nitroShape.setFillColor(sf::Color::White);
	            	                    window.draw(nitroShape);
	            }
            }

            sf::Vector2f positionLaps(level.scorePos.x + (-24 *2), level.scorePos.y + (-9 * 2));
            sf::Vector2f positionNitro(level.scorePos.x + (-34 * 2), level.scorePos.y + (10 *2));
            
            asssets.score.setPosition(level.scorePos.x,level.scorePos.y);
            window.draw(asssets.score);

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
			
            if (idCurrentWindow == 1){
            	horlogeText = std::to_string(Math::arrondir(timer, 0.1));
		    	horlogeText = horlogeText.substr(0, horlogeText.find(".")+2);
            }
            else {
            	horlogeText = "0.0";
            }
            
            
            horloge.setString(horlogeText);
            horloge.setFillColor(sf::Color::Black);
            horloge.setPosition(level.scorePos.x +(-2 * 2) - horloge.getLocalBounds().width / 2, level.scorePos.y + (-28 * 2) - horloge.getLocalBounds().height / 2 + (-2*2));
            
            tourCountText.setFont(font);
            tourCountText.setCharacterSize(60);
            
            sf::Text nitroCountText = sf::Text();
            
            nitroCountText.setFont(font);
            nitroCountText.setCharacterSize(60);
            

            tourCountText.setString(std::to_string(playerCar.laps));

            if (playerCar.nbNitro < 10){
                            	nitroCountText.setString("0" + std::to_string(playerCar.nbNitro));
                            }
                            else {
                            	nitroCountText.setString(std::to_string(playerCar.nbNitro));
                            }
            
            tourCountText.setFillColor(sf::Color::Black);
            nitroCountText.setFillColor(sf::Color::Black);
            
            tourCountText.setPosition(positionLaps.x, positionLaps.y - tourCountText.getLocalBounds().height / 2 - (4*2));
            nitroCountText.setPosition(positionNitro.x, positionNitro.y - nitroCountText.getLocalBounds().height / 2 - (4*2));
            
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
                
                if (enemie -> nbNitro < 10){
                	nitroCountText.setString("0" + std::to_string(enemie -> nbNitro));
                }
                else {
                	nitroCountText.setString(std::to_string(enemie -> nbNitro));
                }
            
                tourCountText.setFillColor(sf::Color::Black);
                nitroCountText.setFillColor(sf::Color::Black);
            
                tourCountText.setPosition(positionLaps.x + (13 * 2 + 15 * 2) * (i + 1), positionLaps.y - tourCountText.getLocalBounds().height / 2 - (4*2));
                nitroCountText.setPosition(positionNitro.x + (26 * 2 + 2 * 2) * (i + 1), positionNitro.y - nitroCountText.getLocalBounds().height / 2 - (4*2));
            
                
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
	if (idCurrentWindow == 2) {

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

        } 
		if (idCurrentWindow == 3) {

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