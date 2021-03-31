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

#include <iostream>
#include <array>
#include <vector>

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
  int x;
  int y;
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
std::vector<float> getProject(Position axis, std::vector<Position> verticle){
  std::vector<float> listProject;
  for (int i = 0; i < verticle.size(); ++i)
  {
    listProject[i] = vectorDotProduct(axis, verticle[i]);
  }
  return listProject;
}
//fin des algo de silvio


//Algo de moi meme
int redirectIfPunchWall(Car car, Wall wall){
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

float calculateNorme(float x, float y){
	return hypot(x * x + y * y);
}

Speed calculateSpeed(Car car, int acceleration, 
		int avgAcceleration, bool isAccelerate, 
		bool isBreack, bool isNitro, float dt){
	float angleRad = fmod((M_PI - car.direction * (M_PI /8)
			+ 2 * M_PI), (2 * M_PI));
	float normeSpeed = calculateNorme(car.speed.x, car.speed.y);
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
		speed.x = cos(angleRad) * normeSpeed * (1/3) * dt;
		speed.y = sin(angleRad) * normeSpeed * (1/3) * dt;
		return speed;
	}
	
	if (acceleration != 0){
		accelerationX = cos(angleRad)*(acceleration * 2 - 
				(1/5 * normeSpeed * 2))* dt;
		accelerationY = cos(angleRad)*(acceleration * 2 - 
						(1/5 * normeSpeed * 2))* dt;
		speed.x = car.speed.x + accelerationX * dt;
		speed.y = car.speed.y + accelerationY * dt;
	}
	else {
		float maxSpeed = avgAcceleration * 5;
		if ((normeSpeed / maxSpeed) > 0.05){
			float deceleration = maxSpeed * (normeSpeed / maxSpeed) * 0.4;
			accelerationX = -cos( angleRad ) * ( deceleration ) * dt;
			accelerationY = -sin( angleRad ) * ( deceleration ) * dt;
			speed.x = car . speed . x + accelerationX * dt;
			speed.y = car.speed.y + accelerationY * dt;
		} else {
			speed.x = 0;
			speed.y = 0;
		}
	}
	return speed;
}


void moveCar(Car car, float dt){
	car.position.x = car.position.x + car.speed.x * dt;
	car.position.y = car.position.y + car.speed.y * dt;
}

int main() {
	
	
	const int WINDOW_WIDTH = 1600;
	const int WINDOW_HEIGHT = 1200;
	const std::string WINDOW_TITLE = "Super off Road";
	bool up, down, left, right, nitro;
	up = down = left = right = nitro = false;
			
			
			
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
    	  }
    	  else if (event.key.code == sf::Keyboard::Down){
    		  down = true;
    	  }
    	  else if (event.key.code == sf::Keyboard::Backspace){
    		  nitro = true;
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
    
    

    /*
     * Affichage de l'état du jeu.
     * À chaque tour de boucle, on efface tout grâce à `clear` (qui prend
     * en paramètre la couleur de fond), puis on dessine tous les éléments,
     * puis on affiche la nouvelle image grâce à `display`.
     */
    

    window.display();
    
    float framerate = 1 / (clock.getElapsedTime().asSeconds());
    std::cout << framerate << std::endl;

  }

  return 0;
}
