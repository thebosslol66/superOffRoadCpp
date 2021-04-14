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
    uniform_real_distribution<float> dist(0.0f, 1.0f);
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

struct Position
{
  float x;
  float y;
};
struct Hitbox2P
{
  Position corner1;
  Position corner2;
};
struct Hitbox4P
{
  Position corner1;
  Position corner2;
  Position corner3;
  Position corner4;
};
struct Speed
{
  float x;
  float y;
};
struct Flag
{
  Hitbox2P hitbox;
  int nb;
};
struct Wall
{
  Hitbox2P hitbox;
  int directionStop;
};
struct Car
{
  Position pos;
  Speed speed;
  int direction;
  int laps;
  Hitbox4P hitbox;
  int flag;
  int nbNitro;
};
struct Bonus
{
  Position pos;
  Hitbox4P hitbox;
};
struct Mud
{
  Position pos;
  Hitbox4P hitbox;
};
struct Ground
{
  std::vector<Wall> walls;
  std::vector<Position> spawnPosNitro;
  std::vector<Mud> muds;
  std::vector<Flag> flags;
};
//algo de silvio
//algo de collision
int vectorDotProduct(const Position& pt1, const Position& pt2){
  return (pt1.x * pt2.x + pt1.y * pt2.y);
}
//
std::vector<float> getProject(const Position& axis, const std::vector<Position>& verticle){
  std::vector<float> listProject(verticle.size());
  for (int i = 0; i < verticle.size(); ++i)
  {
    listProject[i] = vectorDotProduct(axis, verticle[i]);
  }
  return listProject;
}
//
std::vector<Position> getAxisList(const std::vector<Position>& verticle){
  std::vector<Position> listAxis(verticle.size());
  float magnitude;
  for (int i = 0; i < verticle.size(); ++i)
  {
    listAxis[i].x = -(verticle[i+1%4].y - verticle[i].y);
    listAxis[i].y = (verticle[i+1%4].x - verticle[i].y);
    magnitude = hypot(listAxis[i].x, listAxis[i].y);
    if (magnitude != 0)
    {
      listAxis[i].x /= magnitude;
      listAxis[i].y /= magnitude;
    }
  }
  return listAxis;
}
//
bool isCollisionAxis(const Position& axis, const std::vector<Position>& verticle1, const std::vector<Position>& verticle2){
  std::vector<float> listProj1(verticle1.size());
  std::vector<float> listProj2(verticle2.size());
  bool isCollision = false;
  listProj1 = getProject(axis, verticle1);
  listProj2 = getProject(axis, verticle2);
  std::sort(listProj1.begin(), listProj1.end());
  std::sort(listProj1.begin(), listProj1.end());
  if (listProj1[listProj1.size()-1]>=listProj2[0] && listProj1[0] <= listProj2[listProj2.size()-1])
  {
    isCollision = true;
  }
  return isCollision;
}
//
bool isCollision(const std::vector<Position>& verticle1, const std::vector<Position>& verticle2){
  std::vector<Position> listAxis(verticle1.size());
  listAxis = getAxisList(verticle1);
  bool collision = false;
  int count = 0;
  for (int i = 0; i < listAxis.size(); ++i)
  {
    if (isCollisionAxis(listAxis[i], verticle1, verticle2))
    {
      count++;
    }
  }
  if (count == listAxis.size())
  {
    collision = true;
  }
  return collision;
}
//fin des algo de collision
std::vector<Position> hitbox4ToList(const Hitbox4P& hitbox){
  std::vector<Position> verticle(4);
  verticle[0].x = hitbox.corner1.x;
  verticle[0].y = hitbox.corner1.y;
  verticle[1].x = hitbox.corner2.x;
  verticle[1].y = hitbox.corner2.y;
  verticle[2].x = hitbox.corner3.x;
  verticle[2].y = hitbox.corner3.y;
  verticle[3].x = hitbox.corner4.x;
  verticle[3].y = hitbox.corner1.y;
  return verticle;
}
//
std::vector<Position> hitbox2ToList(const Hitbox2P& hitbox){
  std::vector<Position> verticle(2);
  verticle[0].x = hitbox.corner1.x;
  verticle[0].y = hitbox.corner1.y;
  verticle[1].x = hitbox.corner2.x;
  verticle[1].y = hitbox.corner2.y;
  return verticle;
}
//
Hitbox4P getHitboxCar(const Car& car, const int& longe, const int& large){
  Hitbox4P hitbox;
  float angleRad = fmod((M_PI - car.direction * (M_PI/8)+2*M_PI),(2*M_PI));
  float diag = hypot(large, longe);
  float angleCar = (asin(large/diag));
  hitbox.corner1.x = car.pos.x - cos(angleRad - angleCar)*diag;
  hitbox.corner1.y = car.pos.y + sin(angleRad - angleCar)*diag;
  hitbox.corner2.x = car.pos.x - cos(angleRad + angleCar)*diag;
  hitbox.corner2.y = car.pos.y + sin(angleRad + angleCar)*diag;
  hitbox.corner3.x = car.pos.x + cos(angleRad - angleCar)*diag;
  hitbox.corner3.y = car.pos.y + sin(angleRad + angleCar)*diag;
  hitbox.corner4.x = car.pos.x + cos(angleRad - angleCar)*diag;
  hitbox.corner4.y = car.pos.y + sin(angleRad - angleCar)*diag;
  return hitbox;
}
//
int countTour(Car& car, const Flag& flag, const int& nbFlag){
  if (flag.nb == 0 && car.flag == nbFlag-1)
  {
    car.laps++;
    car.flag = 0;
  }else if (flag.nb - car.flag == 1)
  {
    car.flag++;
  }
  return car.laps;
}
//fin des algo de silvio


//Algo de moi meme
int redirectIfPunchWall(const Car& car, const Wall& wall){
	int mustRedir = (car.direction - wall.directionStop +16) %16;
	if (mustRedir == 0){
		return wall.directionStop;
	} else if(mustRedir < 4) {
		return ( wall.directionStop - 4) % 16;
	} else if (mustRedir > 12){
		return ( wall.directionStop + 4) % 16;
	}
	return -1;
}

float calculateNorme(const float& x, const float& y){
	return hypot(x, y);
}

Speed calculateSpeed(const Car& car, int acceleration, 
	const int& avgAcceleration, const bool& isAccelerate, 
	const bool& isBreack, const bool& isNitro, const float& dt){
	
	float angleRad = fmod((M_PI - car.direction * (M_PI /8)
			+ 2 * M_PI), (2 * M_PI));

	float normeSpeed = calculateNorme(car.speed.x, car.speed.y);
	float maxSpeed = avgAcceleration * 5;
	float accelerationX;
	float accelerationY;
	Speed speed;
	
	
	if (!isAccelerate){
		acceleration = 0;
	}
	
	if (isNitro){
		speed.x = cos ( angleRad ) * avgAcceleration * 6;
		speed.y = sin ( angleRad ) * avgAcceleration * 6;
		return speed;
	}
	
	if (isBreack){
		if ( normeSpeed < (1/5 * ( avgAcceleration * 5)) ) {
			normeSpeed = 0;
		}
		speed.x = cos(angleRad) * (normeSpeed - normeSpeed * 3 * dt);
		speed.y = sin(angleRad) * (normeSpeed - normeSpeed * 3 * dt);
		return speed;
	}
	
	if (acceleration != 0){
		if ((normeSpeed / maxSpeed) > 0.95){
			accelerationX = 9;
			accelerationY = 0;
		} else {
			accelerationX = cos(angleRad)*(acceleration * 2 - 
					(1/5 * normeSpeed * 2));
			accelerationY = sin(angleRad)*(acceleration * 2 - 
							(1/5 * normeSpeed * 2));
		}
		
		speed.x = car.speed.x + accelerationX * dt;
		speed.y = car.speed.y + accelerationY * dt;
	}
	else {
		
		if ((normeSpeed / maxSpeed) > 0.2){
			float deceleration = maxSpeed * (normeSpeed / maxSpeed) * 0.5;
			accelerationX = -cos( angleRad ) * ( deceleration );
			accelerationY = -sin( angleRad ) * ( deceleration );
			speed.x = car.speed.x + accelerationX * dt;
			speed.y = car.speed.y + accelerationY * dt;
		} else {
			speed.x = 0;
			speed.y = 0;
		}
	}
	return speed;
}


void moveCar(Car& car, const float& dt){
	car.pos.x = car.pos.x + car.speed.x * dt;
	car.pos.y = car.pos.y + car.speed.y * dt;
}


//il doit y avoir obligatoirement une place de libre dans nitroList 
Bonus generateNitro(const std::vector<Position>& spawnNitro,
		const std::vector<Bonus>& nitroList){
	Bonus nitro;
	bool present = false;
	do{
		int hazard = (int) (Math::random() * spawnNitro.size());
		nitro.pos.x = spawnNitro[hazard].x;
		nitro.pos.y = spawnNitro[hazard].y;
		for (int i = 0; i < spawnNitro.size(); i++){
			if (nitroList[i].pos.x == nitro.pos.x && 
					nitroList[i].pos.y == nitro.pos.y){
				present = true;
			}
		}
	} while(present);
	return nitro;
}

void recalculateSpeedDirection(Car& car){
	float angleRad = fmod((M_PI - car.direction * (M_PI / 8) + 2*M_PI),
			(2 * M_PI));
	float normeVitesse = calculateNorme(car.speed.x, car.speed.y);
	car.speed.x = cos(angleRad) * normeVitesse;
	car.speed.y = sin(angleRad) * normeVitesse;
}


void makeLevel(Ground& level, std::string src){
	Ground cache;
	ifstream levelData(src);

	if(levelData)
	{
		std::string line;
		std::string delimiter = ":";
		
		
		
		std::regex wallPattern("\\(([0-9]+),([0-9]+)\\)-([0-9]+)-\\(([0-9]+),([0-9]+)\\)");
		std::regex nitroPattern("\\(([0-9]+),([0-9]+)\\)");
		
		std::regex flagPattern("\\(([0-9]+),([0-9]+)\\)-([0-9]+)-\\(([0-9]+),([0-9]+)\\)");
		
		std::smatch m;

	    while(getline(levelData, line)){
	    	std::string token = line.substr(0, line.find(delimiter));
	    	if (token == "Wall"){
		    	line = line.substr(line.find(delimiter)+2);
	    		while (std::regex_search(line, m, wallPattern)){
	    			
	    			Wall wall;
	    			
	    			wall.hitbox.corner1.x = std::stoi(m[1]);
	    			wall.hitbox.corner1.y = std::stoi(m[2]);
	    			wall.directionStop = std::stoi(m[3]);
	    			wall.hitbox.corner2.x = std::stoi(m[4]);
	    			wall.hitbox.corner2.y = std::stoi(m[5]);
	    			
	    			level.walls.push_back(wall);
	    			
	    			line = line.substr(line.find(")-")+1);
	    		}
	    	}
	    	if (token == "Nitro"){
		    	line = line.substr(line.find(delimiter)+2);
	    		while (std::regex_search(line, m, nitroPattern)){
	    			Position nitro;
	    			nitro.x = std::stoi(m[1]);
	    			nitro.y = std::stoi(m[2]);
	    			level.spawnPosNitro.push_back(nitro);
	    			line = line.substr(line.find(')')+1);
	    		}
	    	}
	    	
	    	if (token == "Mud"){
	    		line = line.substr(line.find(delimiter)+2);
	    		while (std::regex_search(line, m, nitroPattern)){
	    			Mud mud;
	    			
	    		}
	    	}
	    	if (token == "Flag"){
	    		line = line.substr(line.find(delimiter)+2);
	    		while (std::regex_search(line, m, flagPattern)){
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
	    cout << cache.walls.size() << " " << cache.spawnPosNitro.size()<< " " << cache.muds.size() << " " << cache.flags.size() << endl;
	    
	}
	else
	{
	    throw "Can't read the file " + src;
	}
}

int main() {
	
	
	const int WINDOW_WIDTH = 1600;
	const int WINDOW_HEIGHT = 1200;
	const std::string WINDOW_TITLE = "Super off Road";
	const int MAX_FPS = 120;
	
	const float TIME_BEFORE_REACTIVATE = 0.1;
	
	
	std::string levelFile("level1");
	
	
	
	const int NITRO_SPAWN_TIME = 10000;
	const int ACCELERATION = 30;
	bool up, down, left, right, nitro;
	up = down = left = right = nitro = false;
	
	double lastActiveUp, lastActiveDown, lastActiveLeft, lastActiveRight, lastActiveNitro;
	lastActiveUp = lastActiveDown = lastActiveLeft = lastActiveRight = lastActiveNitro = 0;
	
	const int CAR_LONGUEUR = 40;
	const int CAR_HAUTEUR = 20;
			
			
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
  
  Ground level;
  //makeLevel(level, levelFile + ".txt");
  //cout << level;
  sf::Texture backgroundTexture;
  backgroundTexture.loadFromFile(levelFile + ".png");
  sf::Sprite background;
  background.setTexture(backgroundTexture);
  
  background.setScale(sf::Vector2f((WINDOW_WIDTH/backgroundTexture.getSize().x), (WINDOW_HEIGHT/backgroundTexture.getSize().y)));
  
  /*
  level.walls = [];
  level.spawnPosNitro = [];
  level.muds = [];
  level.flags = [];
  */
  Car playerCar;
  playerCar.pos.x = 500;//la position initial de la voiture en x
  playerCar.pos.y = 500;//la position initial de la voiture en y
  playerCar.speed.x = 0;
  playerCar.speed.y = 0;
  playerCar.direction = 0;
  playerCar.laps = 0;
  playerCar.flag = 0;
  playerCar.nbNitro = 3;
  float timer = 0;
  
  float malusBonusSpeed = 1.0;
  std::vector<Bonus> nitroList;
  
  
  


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
    	  if (event.key.code == sf::Keyboard::Left){
    		  left = true;
    	  }
    	  else if (event.key.code == sf::Keyboard::Right){
    		  right = true;
    	  }
    	  else if (event.key.code == sf::Keyboard::Up){
    		  up = true;
    		  lastActiveUp = TIME_BEFORE_REACTIVATE;
    	  }
    	  else if (event.key.code == sf::Keyboard::Down){
    		  down = true;
    		  lastActiveDown = TIME_BEFORE_REACTIVATE;
    	  }
    	  else if (event.key.code == sf::Keyboard::Backspace){
    		  nitro = true;
    		  lastActiveNitro = TIME_BEFORE_REACTIVATE;
    	  }
      }
      if (event.type == Event::KeyReleased) {
    	  if (event.key.code == sf::Keyboard::Left){
    		  left = false;
    	  }
    	  else if (event.key.code == sf::Keyboard::Right){
    		  right = false;
    	  }
    	  else if (event.key.code == sf::Keyboard::Up){
    		  up = false;
    	  }
    	  else if (event.key.code == sf::Keyboard::Down){
    		  down = false;
    	  }
    	  else if (event.key.code == sf::Keyboard::Backspace){
    		  nitro = false;
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
    
    
    
    //On applique la direction a la voiture 
     if (left && lastActiveLeft <= 0){
     	lastActiveLeft = TIME_BEFORE_REACTIVATE + dt;
     	playerCar.direction = ( playerCar.direction + 1) % 16;
     }
     if (right && lastActiveRight <= 0) {
 		lastActiveRight = TIME_BEFORE_REACTIVATE + dt;
     	playerCar.direction = ( playerCar.direction - 1) % 16;
     }
     if (left || right) {
     	recalculateSpeedDirection(playerCar);
     }
     
     if (lastActiveLeft > 0){
    	 lastActiveLeft -= dt;
     }
     if (lastActiveRight){
         lastActiveRight -= dt;
     }
     
    
    for (int i = 0; i < level.walls.size(); i++) {
    	if (isCollision(hitbox4ToList(playerCar.hitbox),
    			hitbox2ToList(level.walls[i].hitbox))){
    				redirectIfPunchWall( playerCar, level.walls[i]);
    				recalculateSpeedDirection(playerCar);
    				malusBonusSpeed = malusBonusSpeed - 0.40;
    	}
    }
    
    for (int i = 0; i < level.muds.size(); i++) {
        if (isCollision( hitbox4ToList (playerCar.hitbox),
       			hitbox4ToList(level.muds[i].hitbox))){
       				malusBonusSpeed = malusBonusSpeed - 0.20;
       	}
    }
    
    for (int i = 0; i < nitroList.size(); i++) {
    	if (isCollision( hitbox4ToList (playerCar.hitbox),
    			hitbox4ToList(nitroList[i].hitbox))){
    		playerCar.nbNitro = playerCar.nbNitro + 1;
			nitroList[i].pos.x = 0;
			nitroList[i].pos.y = 0;
    	}
    }
    
     
    //On calcule ensuite la nouvelle vitesse de la voiture
    Speed playerNewSpeed = calculateSpeed(playerCar, ACCELERATION * malusBonusSpeed, ACCELERATION, up, down, nitro, dt);
    
    
    //Timer Pour le spawn de nitro
    timer += dt;
    bool empty = false;
    int i = 0;
    if (timer == NITRO_SPAWN_TIME){
    	do{
    		if (nitroList[i].pos.x == 0 && nitroList[i].pos.y == 0){
    			empty = true;
    		} else {
    			empty = false;
    		}
    		i++;
    	} while (i < nitroList.size() || empty == true);
    }
    
	if (empty == true){
		nitroList[i] = generateNitro(level.spawnPosNitro, nitroList);
	}
	
	//Comte les tours
	for (int i = 0; i < level.flags.size(); i++) {
	    if (isCollision(hitbox4ToList(playerCar.hitbox),
	   			hitbox2ToList(level.flags[i].hitbox))){
	    	countTour(playerCar, level.flags[i], level.flags.size());
    	}
	}
	
    
	playerCar.speed = playerNewSpeed;
	moveCar(playerCar, dt);
	malusBonusSpeed = 1;

    
    
    /*
     * Affichage de l'état du jeu.
     * À chaque tour de boucle, on efface tout grâce à `clear` (qui prend
     * en paramètre la couleur de fond), puis on dessine tous les éléments,
     * puis on affiche la nouvelle image grâce à `display`.
     */
    
	window.clear(Color::White);
	
	window.draw(background);
	
	sf::RectangleShape carShape(sf::Vector2f(CAR_LONGUEUR, CAR_HAUTEUR));
	carShape.setPosition(playerCar.pos.x + CAR_LONGUEUR/2 , playerCar.pos.y + CAR_HAUTEUR/2);
	carShape.setOrigin(CAR_LONGUEUR/2, CAR_HAUTEUR/2);
	carShape.setRotation(180 - (playerCar.direction/16.0 * 360));
	carShape.setFillColor(sf::Color::Blue);
	
	window.draw(carShape);

    window.display();
    
    float framerate = 1 / (clock.getElapsedTime().asSeconds());
    sf::sleep(sf::seconds((1.0/MAX_FPS)-clock.getElapsedTime().asSeconds()));
    //std::cout << framerate << std::endl;

  }

  return 0;
}
