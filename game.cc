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
        uniform_real_distribution <float> dist(0.0f, 1.0f);
        return dist(engine);
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
    int state;
    Position pos;
    Speed speed;
    int direction;
    int laps;
    int flag;
    int nbNitro;
    float malusBonusSpeed;
    bool collision;
};
struct Bonus {
    Position pos;
    int rayon;
};
struct Mud {
    Position pos;
    int rayon;
};
struct Ground {
    std::vector < Wall > walls;
    std::vector < Position > spawnPosNitro;
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
bool isCollision(Car car, Bonus bonus, int rayon) {
    bool colision = false;
    if (hypot(car.pos.x - bonus.pos.x, car.pos.y - bonus.pos.y) <= rayon + bonus.rayon) {
        colision = true;
    }
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
//fin des algo de silvio

//Algo de moi meme
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

    if (isNitro) {
        speed.x = cos(angleRad) * avgAcceleration * 6;
        speed.y = sin(angleRad) * avgAcceleration * 6;
        return speed;
    }

    if (isBreack) {
        if (normeSpeed < (1 / 5 * (avgAcceleration * 5))) {
            normeSpeed = 0;
        }
        speed.x = cos(angleRad) * (normeSpeed - normeSpeed * 3 * dt);
        speed.y = sin(angleRad) * (normeSpeed - normeSpeed * 3 * dt);
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

//il doit y avoir obligatoirement une place de libre dans nitroList 
Bonus generateNitro(const std::vector < Position > & spawnNitro,
    const std::vector < Bonus > & nitroList) {
    Bonus nitro;
    bool present = false;
    do {
        int hazard = (int)(Math::random() * spawnNitro.size());
        nitro.pos.x = spawnNitro[hazard].x;
        nitro.pos.y = spawnNitro[hazard].y;
        for (int i = 0; i < spawnNitro.size(); i++) {
            if (nitroList[i].pos.x == nitro.pos.x &&
                nitroList[i].pos.y == nitro.pos.y) {
                present = true;
            }
        }
    } while (present);
    return nitro;
}

void recalculateSpeedDirection(Car & car) {
    float angleRad = fmod((M_PI - car.direction * (M_PI / 8) + 2 * M_PI),
        (2 * M_PI));
    float normeVitesse = calculateNorme(car.speed.x, car.speed.y);
    car.speed.x = cos(angleRad) * normeVitesse;
    car.speed.y = sin(angleRad) * normeVitesse;
}

void makeLevel(Ground & level, std::string src) {
    Ground cache;
    ifstream levelData(src);

    if (levelData) {
        std::string line;
        std::string delimiter = ":";

        std::regex wallPattern("\\(([0-9]+),([0-9]+)\\)-([0-9]+)-\\(([0-9]+),([0-9]+)\\)");
        std::regex nitroPattern("\\(([0-9]+),([0-9]+)\\)");

        std::regex flagPattern("\\(([0-9]+),([0-9]+)\\)-([0-9]+)-\\(([0-9]+),([0-9]+)\\)");

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
                    Position nitro;
                    nitro.x = std::stoi(m[1]);
                    nitro.y = std::stoi(m[2]);
                    level.spawnPosNitro.push_back(nitro);
                    line = line.substr(line.find(')') + 1);
                }
            }

            if (token == "Mud") {
                line = line.substr(line.find(delimiter) + 2);
                while (std::regex_search(line, m, nitroPattern)) {
                    Mud mud;

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
        }

    } else {
        throw "Can't read the file " + src;
    }
}

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

        const int WINDOW_WIDTH = 1600;
        const int WINDOW_HEIGHT = 1200;
        const std::string WINDOW_TITLE = "Super off Road";
        const int MAX_FPS = 120;

        const float TIME_BEFORE_REACTIVATE = 0.1;

        int idCurrentWindow = 0;

        std::string levelFile("level1");

        sf::Font font;
        font.loadFromFile("PixelOperator.ttf");

        const int NITRO_SPAWN_TIME = 10000;
        const int ACCELERATION = 50;
        bool up, down, left, right, nitro, enter;
        up = down = left = right = nitro = enter = false;

        double lastActiveUp, lastActiveDown, lastActiveLeft, lastActiveRight, lastActiveNitro;
        lastActiveUp = lastActiveDown = lastActiveLeft = lastActiveRight = lastActiveNitro = 0;

        const int CAR_LONGUEUR = 40;
        const int CAR_HAUTEUR = 20;

        /*
         * Variables pour l'ecran titre
         */

        int textAlphaValue = 0;

        /*
         * Une RenderWindow est une fenêtre qui permet de récupérer des événements
         * d'entrée (comme le clavier et la souris) et d'afficher des entités.
         *
         * La documentation se trouve ici:
         * http://www.sfml-dev.org/documentation/2.1/classsf_1_1RenderWindow.php
         */
        RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);

        /*
         * Une Clock permet de compter le temps. Vous en aurez besoin pour savoir
         * le temps entre deux frames.
         */
        Clock clock;
        int cnt = 0;

        Ground level;
        //makeLevel(level, levelFile + ".txt");
        sf::Texture backgroundTexture;
        backgroundTexture.loadFromFile(levelFile + ".png");
        sf::Sprite background;
        background.setTexture(backgroundTexture);

        background.setScale(sf::Vector2f((WINDOW_WIDTH / backgroundTexture.getSize().x), (WINDOW_HEIGHT / backgroundTexture.getSize().y)));
        /*
        level.walls = [];
        level.spawnPosNitro = [];
        level.muds = [];
        level.flags = [];
        */
        Car playerCar;
        playerCar.state = 1;
        playerCar.pos.x = 500; //la position initial de la voiture en x
        playerCar.pos.y = 500; //la position initial de la voiture en y
        playerCar.speed.x = 0;
        playerCar.speed.y = 0;
        playerCar.direction = 0;
        playerCar.laps = 0;
        playerCar.flag = 0;
        playerCar.nbNitro = 3;
        playerCar.malusBonusSpeed = 1.0;
        playerCar.collision = false;
        float timer = 0;

        std::vector < Bonus > nitroList;

        printListWall(level.walls);

        /*Walls 
         */

        Wall wall;

        wall.hitbox.corner1.x = 200;
        wall.hitbox.corner1.y = 200;
        wall.directionStop = 0;
        wall.hitbox.corner2.x = 200;
        wall.hitbox.corner2.y = 800;

        level.walls.push_back(wall);

        wall.hitbox.corner1.x = 200;
        wall.hitbox.corner1.y = 800;
        wall.directionStop = 4;
        wall.hitbox.corner2.x = 800;
        wall.hitbox.corner2.y = 800;

        level.walls.push_back(wall);

        wall.hitbox.corner1.x = 800;
        wall.hitbox.corner1.y = 800;
        wall.directionStop = 8;
        wall.hitbox.corner2.x = 800;
        wall.hitbox.corner2.y = 200;

        level.walls.push_back(wall);

        wall.hitbox.corner1.x = 800;
        wall.hitbox.corner1.y = 200;
        wall.directionStop = 12;
        wall.hitbox.corner2.x = 200;
        wall.hitbox.corner2.y = 200;

        level.walls.push_back(wall);

        /* Enemies car */

        Car Enemie1;

        Enemie1.state = 1;
        Enemie1.pos.x = 300; //la position initial de la voiture en x
        Enemie1.pos.y = 500; //la position initial de la voiture en y
        Enemie1.speed.x = 0;
        Enemie1.speed.y = 0;
        Enemie1.direction = 0;
        Enemie1.laps = 0;
        Enemie1.flag = 0;
        Enemie1.nbNitro = 3;
        Enemie1.malusBonusSpeed = 1.0;
        Enemie1.collision = false;

        Car Enemie2;

        Enemie2.state = 1;
        Enemie2.pos.x = 400; //la position initial de la voiture en x
        Enemie2.pos.y = 500; //la position initial de la voiture en y
        Enemie2.speed.x = 0;
        Enemie2.speed.y = 0;
        Enemie2.direction = 0;
        Enemie2.laps = 0;
        Enemie2.flag = 0;
        Enemie2.nbNitro = 3;
        Enemie2.malusBonusSpeed = 1.0;
        Enemie2.collision = false;

        Car Enemie3;

        Enemie3.state = 1;
        Enemie3.pos.x = 600; //la position initial de la voiture en x
        Enemie3.pos.y = 500; //la position initial de la voiture en y
        Enemie3.speed.x = 0;
        Enemie3.speed.y = 0;
        Enemie3.direction = 0;
        Enemie3.laps = 0;
        Enemie3.flag = 0;
        Enemie3.nbNitro = 3;
        Enemie3.malusBonusSpeed = 1.0;
        Enemie3.collision = false;

        std::vector < Car > Enemies;
        Enemies.push_back(Enemie1);
        Enemies.push_back(Enemie2);
        Enemies.push_back(Enemie3);

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
                        lastActiveUp = TIME_BEFORE_REACTIVATE;
                    } else if (event.key.code == sf::Keyboard::Down) {
                        down = true;
                        lastActiveDown = TIME_BEFORE_REACTIVATE;
                    } else if (event.key.code == sf::Keyboard::Backspace) {
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
                    } else if (event.key.code == sf::Keyboard::Backspace) {
                        nitro = false;
                    } else if (event.key.code == sf::Keyboard::Return) {
                        enter = false;
                    }
                }
            }

            /*
             * Mise à jour de l'état du jeu.
             * On calcule le temps (en secondes) depuis la dernière frame qu'on place
             * dans la variable `dt`. Ensuite, il faut compléter suivant ce qui est
             * demandé.
             */

            float dt = clock.restart().asSeconds();
            if (idCurrentWindow == 0) {
                textAlphaValue += 170 * dt;
                textAlphaValue %= 510;
            } else if (idCurrentWindow == 1) {
                //On applique la direction a la voiture 
                if (left && lastActiveLeft <= 0) {
                    lastActiveLeft = TIME_BEFORE_REACTIVATE + dt;
                    playerCar.direction = (playerCar.direction + 1) % 16;
                }
                if (right && lastActiveRight <= 0) {
                    lastActiveRight = TIME_BEFORE_REACTIVATE + dt;
                    playerCar.direction = (playerCar.direction + 15) % 16;
                }
                if (left || right) {
                    recalculateSpeedDirection(playerCar);
                }
                if (lastActiveLeft > 0) {
                    lastActiveLeft -= dt;
                }
                if (lastActiveRight) {
                    lastActiveRight -= dt;
                }

                playerCar.collision = false;
                for (int i = 0; i < level.walls.size(); i++) {
                    if (isCollision(playerCar, level.walls[i], CAR_HAUTEUR / 2)) {
                        int direction = redirectIfPunchWall(playerCar, level.walls[i]);
                        if (playerCar.direction == direction) {
                            playerCar.speed.x = 0;
                            playerCar.speed.y = 0;
                            playerCar.malusBonusSpeed = 0;
                        }
                        if (direction >= 0) {
                            playerCar.direction = direction;
                            recalculateSpeedDirection(playerCar);
                            if (playerCar.state != 2) {
                                playerCar.state = 2;
                            }
                        }
                    }
                    if (isCollision(playerCar, level.walls[i], CAR_LONGUEUR)) {
                        playerCar.collision = true;
                    }
                }

                if (playerCar.state == 2 and!playerCar.collision) {
                    playerCar.state = 1;

                }
                if (playerCar.state == 2) {
                    playerCar.malusBonusSpeed *= 0.60;
                }

                // for (int i = 0; i < level.muds.size(); i++) {
                //     if (isCollision( hitbox4ToList (playerCar.hitbox),
                //          hitbox4ToList(level.muds[i].hitbox))){
                //            malusBonusSpeed *= 0.80;
                //      }
                // }
                //
                // for (int i = 0; i < nitroList.size(); i++) {
                //  if (isCollision( hitbox4ToList (playerCar.hitbox),
                //      hitbox4ToList(nitroList[i].hitbox))){
                //    playerCar.nbNitro = playerCar.nbNitro + 1;
                //    // nitroList[i].pos.x = 0;
                //    // nitroList[i].pos.y = 0;
                //  }
                // }

                //On calcule ensuite la nouvelle vitesse de la voiture
                Speed playerNewSpeed = calculateSpeed(playerCar, ACCELERATION * playerCar.malusBonusSpeed, ACCELERATION, up, down, nitro, dt);

                //Timer Pour le spawn de nitro
                timer += dt;
                bool empty = false;
                int i = 0;
                if (timer == NITRO_SPAWN_TIME) {
                    do {
                        if (nitroList[i].pos.x == 0 && nitroList[i].pos.y == 0) {
                            empty = true;
                        } else {
                            empty = false;
                        }
                        i++;
                    } while (i < nitroList.size() || empty == true);
                }

                if (empty == true) {
                    nitroList[i] = generateNitro(level.spawnPosNitro, nitroList);
                }

                //Comte les tours
                // for (int i = 0; i < level.flags.size(); i++) {
                //     if (isCollision(hitbox4ToList(playerCar.hitbox),
                //          hitbox2ToList(level.flags[i].hitbox))){
                //      countTour(playerCar, level.flags[i], level.flags.size());
                //      }
                // }

                playerCar.speed = playerNewSpeed;
                moveCar(playerCar, dt);
                playerCar.malusBonusSpeed = 1;

                for (int j = 0; j < Enemies.size(); j++) {
                    Car enemie = Enemies[j];
                    enemie.collision = false;
                    for (int i = 0; i < level.walls.size(); i++) {
                        if (isCollision(enemie, level.walls[i], CAR_HAUTEUR / 2)) {
                            int direction = redirectIfPunchWall(enemie, level.walls[i]);
                            if (enemie.direction == direction) {
                                enemie.speed.x = 0;
                                enemie.speed.y = 0;
                                enemie.malusBonusSpeed = 0;
                            }
                            if (direction >= 0) {
                                enemie.direction = direction;
                                recalculateSpeedDirection(enemie);
                                if (enemie.state != 2) {
                                    enemie.state = 2;
                                }
                            }
                        }
                        if (isCollision(enemie, level.walls[i], CAR_LONGUEUR)) {
                            enemie.collision = true;
                        }
                    }

                    if (enemie.state == 2 and!enemie.collision) {
                        enemie.state = 1;

                    }
                    if (enemie.state == 2) {
                        enemie.malusBonusSpeed *= 0.60;
                    }

                    // for (int i = 0; i < level.muds.size(); i++) {
                    //     if (isCollision( hitbox4ToList (playerCar.hitbox),
                    //          hitbox4ToList(level.muds[i].hitbox))){
                    //            malusBonusSpeed *= 0.80;
                    //      }
                    // }
                    //
                    // for (int i = 0; i < nitroList.size(); i++) {
                    //  if (isCollision( hitbox4ToList (playerCar.hitbox),
                    //      hitbox4ToList(nitroList[i].hitbox))){
                    //    playerCar.nbNitro = playerCar.nbNitro + 1;
                    //    // nitroList[i].pos.x = 0;
                    //    // nitroList[i].pos.y = 0;
                    //  }
                    // }

                    //On calcule ensuite la nouvelle vitesse de la voiture
                    Speed enemieNewSpeed = calculateSpeed(enemie, ACCELERATION * enemie.malusBonusSpeed, ACCELERATION, true, false, false, dt);

                    //Comte les tours
                    // for (int i = 0; i < level.flags.size(); i++) {
                    //     if (isCollision(hitbox4ToList(playerCar.hitbox),
                    //          hitbox2ToList(level.flags[i].hitbox))){
                    //      countTour(playerCar, level.flags[i], level.flags.size());
                    //      }
                    // }

                    enemie.speed = enemieNewSpeed;
                    moveCar(enemie, dt);
                    enemie.malusBonusSpeed = 1;
                }

            }     /*
                 * Affichage de l'état du jeu.
                 * À chaque tour de boucle, on efface tout grâce à `clear` (qui prend
                 * en paramètre la couleur de fond), puis on dessine tous les éléments,
                 * puis on affiche la nouvelle image grâce à `display`.
                 */

                window.clear(Color::White);

                if (idCurrentWindow == 0) {

                    if (enter) {
                        idCurrentWindow = 1;
                    }

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

                    for (int j = 0; j < Enemies.size(); j++) {
                        Car enemie = Enemies[j];
                        carShape.setPosition(enemie.pos.x + CAR_LONGUEUR / 2, enemie.pos.y + CAR_HAUTEUR / 2);
                        carShape.setOrigin(CAR_LONGUEUR / 2, CAR_HAUTEUR / 2);
                        carShape.setRotation(180 - (enemie.direction / 16.0 * 360));
                        carShape.setFillColor(sf::Color::Yellow);
                        window.draw(carShape);
                    }

                    carShape.setPosition(playerCar.pos.x + CAR_LONGUEUR / 2, playerCar.pos.y + CAR_HAUTEUR / 2);
                    carShape.setOrigin(CAR_LONGUEUR / 2, CAR_HAUTEUR / 2);
                    carShape.setRotation(180 - (playerCar.direction / 16.0 * 360));
                    carShape.setFillColor(sf::Color::Blue);

                    window.draw(carShape);

                    std::vector < sf::RectangleShape > listWallPrint;
                    sf::RectangleShape wallShape;
                    sf::VertexArray lines(sf::LineStrip, level.walls.size()+1);
                    for (int i = 0; i < level.walls.size(); ++i) {
                        lines[i].position = sf::Vector2f(level.walls[i].hitbox.corner1.x+20, level.walls[i].hitbox.corner1.y+10);
                        lines[i].color = sf::Color::Red;
                    }
                    lines[level.walls.size()].position = sf::Vector2f(level.walls[0].hitbox.corner1.x+20, level.walls[0].hitbox.corner1.y+10);
                    lines[level.walls.size()].color = sf::Color::Red;
                    window.draw(lines);

                }

                window.display();

                 sf::sleep(sf::seconds((1.0 / MAX_FPS) - clock.getElapsedTime().asSeconds()));
                //std::cout << framerate << std::endl;

            }
            return 0;
        }