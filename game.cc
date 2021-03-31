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

#include <iostream>
#include <array>

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
struct Rectangle {
  float x;
  float y;
  float width;
  float height;
  
};



int main() {
	
	
  /*
   * Une RenderWindow est une fenêtre qui permet de récupérer des événements
   * d'entrée (comme le clavier et la souris) et d'afficher des entités.
   *
   * La documentation se trouve ici:
   * http://www.sfml-dev.org/documentation/2.1/classsf_1_1RenderWindow.php
   */
  RenderWindow window(VideoMode(1600, 1200), "Titre");

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
    	 
      }

      if (event.type == Event::KeyReleased) {
    	  
    	  
      }

      if (event.type == sf::Event::MouseButtonPressed) {
    	  
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
