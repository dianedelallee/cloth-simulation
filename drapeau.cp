#ifdef _WIN32
#include <windows.h> 
#endif
#include <GL/gl.h>
#include <GL/glut.h> 

#include <openGL/gl.h>	   // Fichier Header pour OpenGL32 Library
#include <openGL/glu.h>    // Fichier Header pour GLu32 Library
#include <glut/glut.h>	  // Fichier Header pour GLut Library 
#include <math.h>
#include <vector>
#include <iostream>



#define DAMPING 0.01 
#define TIME_STEPSIZE2 0.5*0.5 
#define CONSTRAINT_ITERATIONS 15 
// ========== DEFINITION DE LA CLASSE VEC3 ==========
class Vec3 {	
public:
	float f[3]; // tableau de taille 3

	Vec3(float x, float y, float z){
		f[0] =x;
		f[1] =y;
		f[2] =z;
	}

	Vec3() {}

	// calcul de la taille
	float length(){
		return sqrt(f[0]*f[0]+f[1]*f[1]+f[2]*f[2]);
	}

	// normalisation
	Vec3 normalized(){
		float l = length();
		return Vec3(f[0]/l,f[1]/l,f[2]/l);
	}

	// surcharge des operateurs
	void operator+= (const Vec3 &v){
		f[0]+=v.f[0];
		f[1]+=v.f[1];
		f[2]+=v.f[2];
	}

	Vec3 operator/ (const float &a){
		return Vec3(f[0]/a,f[1]/a,f[2]/a);
	}

	Vec3 operator- (const Vec3 &v){
		return Vec3(f[0]-v.f[0],f[1]-v.f[1],f[2]-v.f[2]);
	}

	Vec3 operator+ (const Vec3 &v)	{
		return Vec3(f[0]+v.f[0],f[1]+v.f[1],f[2]+v.f[2]);
	}

	Vec3 operator* (const float &a){
		return Vec3(f[0]*a,f[1]*a,f[2]*a);
	}

	Vec3 operator-(){
		return Vec3(-f[0],-f[1],-f[2]);
	}

	
	Vec3 cross(const Vec3 &v){
		return Vec3(f[1]*v.f[2] - f[2]*v.f[1], f[2]*v.f[0] - f[0]*v.f[2], f[0]*v.f[1] - f[1]*v.f[0]);
	}

	
	float dot(const Vec3 &v){
		return f[0]*v.f[0] + f[1]*v.f[1] + f[2]*v.f[2];
	}
};

// ========== PARTICULES ==========
class Particule {
private:
	bool movable; // pour savoir si la particule bouge ou non
	float mass; // poids de la particule
	Vec3 pos; // position de la particule 
	Vec3 old_pos; // position de la particule au temps t-1
	Vec3 acceleration; // acceleration de la particule
	Vec3 accumulated_normal; //

public:
	Particule(Vec3 pos) : pos(pos), old_pos(pos),acceleration(Vec3(0,0,0)), mass(1), movable(true), accumulated_normal(Vec3(0,0,0)){}
	Particule(){}

	void addForce(Vec3 f){
		acceleration += f/mass;
	}

	/* 
	 donne l'equation force = masse*acceleration : la prochaine position est trouvee par l'integrataion de verlet*/
	void timeStep() {
		if(movable)
		{
			Vec3 temp = pos;
			pos = pos + (pos-old_pos)*(1.0-DAMPING) + acceleration*TIME_STEPSIZE2;
			old_pos = temp;
			acceleration = Vec3(0,0,0); 
		}
	}

	Vec3& getPos() {
		return pos;
	}

	void resetAcceleration() {
		acceleration = Vec3(0,0,0);
	}

	void offsetPos(const Vec3 v) { 
		if(movable) pos += v;
	}

	void makeUnmovable() {
		movable = false;
	}
	
	void makeMovable() {
		movable = true;
	}

	void addToNormal(Vec3 normal){
		accumulated_normal += normal.normalized();
	}

	Vec3& getNormal() { 
		return accumulated_normal;
	}

	void resetNormal() {
		accumulated_normal = Vec3(0,0,0);
	}

};

// ========== CLASSE LIEN ===========
class Lien {
private:
	float rest_distance; // distance entre deux particules

public:
	Particule *p1, *p2; // deux particules reliees par une meme contrainte

	Lien(Particule *p1, Particule *p2) :  p1(p1),p2(p2){
		Vec3 vec = p1->getPos()-p2->getPos();
		rest_distance = vec.length();
	}

	/* lien entre deux particules	*/
	void lienPossible() {
		Vec3 p1_to_p2 = p2->getPos()-p1->getPos(); // vecteur de p1 a p2
		float current_distance = p1_to_p2.length(); //  distance entre p1  p2
		Vec3 correctionVector = p1_to_p2*(1 - rest_distance/current_distance); // vecteur de compensation : deplace p1 d'une distance rest_distance de p2
		Vec3 correctionVectorHalf = correctionVector*0.5; // on prend la moitie de la longueur precedente pour bouger P1 et P2
		p1->offsetPos(correctionVectorHalf); // correctionVectorHalf pointe de p1 a P2 pour que la longueur puisse bouger P2 de moitie pour satisfaire la creation des liens.
		p2->offsetPos(-correctionVectorHalf); // on deplace p2 de -direction si on va de P2 a p1 au lieu de P1 a P2 
	}
};

// ========== DEFINITION DE LA CLASSE TISSU==========
class Tissu {
private:
	std::vector<Particule> particules; // all particles that are part of this cloth
	std::vector<Lien> liens; // alle constraints between particles as part of this cloth
	
	
	int nb_particules_large; // nombre de particules dans la largeur
	int nb_particules_hauteur; // nombre de particules dans la hauteur
	// nb total de particules =  nb_particules_large*nb_particules_hauteur
	
	Particule* getParticule(int x, int y) {
		return &particules[y*nb_particules_large + x];
	}
	
	void creerLien(Particule *p1, Particule *p2) {
		liens.push_back(Lien(p1,p2));
	}


	Vec3 calcTriangleNormal(Particule *p1,Particule *p2,Particule *p3) {
		Vec3 pos1 = p1->getPos();
		Vec3 pos2 = p2->getPos();
		Vec3 pos3 = p3->getPos();

		Vec3 v1 = pos2-pos1;
		Vec3 v2 = pos3-pos1;

		return v1.cross(v2);
	}

	void addWindForcesForTriangle(Particule *p1,Particule *p2,Particule *p3, const Vec3 direction) {
		Vec3 normal = calcTriangleNormal(p1,p2,p3);
		Vec3 d = normal.normalized();
		Vec3 force = normal*(d.dot(direction));
		p1->addForce(force);
		p2->addForce(force);
		p3->addForce(force);
	}

	/* Dessiner un triangle entre p1,p2,p3 avec une couleur*/
	void drawTriangle(Particule *p1, Particule *p2, Particule *p3, const Vec3 color){
		glColor3fv( (float*) &color );

		glNormal3fv((float *) &(p1->getNormal().normalized() ));
		glVertex3fv((float *) &(p1->getPos() ));

		glNormal3fv((float *) &(p2->getNormal().normalized() ));
		glVertex3fv((float *) &(p2->getPos() ));

		glNormal3fv((float *) &(p3->getNormal().normalized() ));
		glVertex3fv((float *) &(p3->getPos() ));
	}

public:
	
	


	/* Constructeur pour le tissu (particules + liens)*/
	Tissu(float large, float hauteur, int nb_particules_large, int nb_particules_hauteur) : nb_particules_large(nb_particules_large), nb_particules_hauteur(nb_particules_hauteur){
		particules.resize(nb_particules_large*nb_particules_hauteur); //I am essentially using this vector as an array with room for num_particles_width*num_particles_height particles

	// creation des particules dans une grille de (0,0,0) jusqu'a (largeur, hauteur, 0)
		for(int x=0; x<nb_particules_large; x++){
			for(int y=0; y<nb_particules_hauteur; y++){
				Vec3 pos = Vec3(large * (x/(float)nb_particules_large),
								-hauteur * (y/(float)nb_particules_hauteur),
								0);
				particules[y*nb_particules_large+x]= Particule(pos); // insert particle in column x at y'th row
			}
		}

		// Connexion des particules voisines avec un lien (distance 1 et racine de 2 dans la grille)
		for(int x=0; x<nb_particules_large; x++){
			for(int y=0; y<nb_particules_hauteur; y++){
				if (x<nb_particules_large-1) creerLien(getParticule(x,y),getParticule(x+1,y));
				if (y<nb_particules_large-1) creerLien(getParticule(x,y),getParticule(x,y+1));
				if (x<nb_particules_large-1 && y<nb_particules_hauteur-1) creerLien(getParticule(x,y),getParticule(x+1,y+1));
				if (x<nb_particules_large-1 && y<nb_particules_hauteur-1) creerLien(getParticule(x+1,y),getParticule(x,y+1));
			}
		}


		// connexion des particules presque voisines avec un lien(distance 2 et racine de 4 dans la grille
		for(int x=0; x<nb_particules_large; x++){
			for(int y=0; y<nb_particules_hauteur; y++){
				if (x<nb_particules_large-2) creerLien(getParticule(x,y),getParticule(x+2,y));
				if (y<nb_particules_hauteur-2) creerLien(getParticule(x,y),getParticule(x,y+2));
				if (x<nb_particules_large-2 && y<nb_particules_hauteur-2) creerLien(getParticule(x,y),getParticule(x+2,y+2));
				if (x<nb_particules_large-2 && y<nb_particules_hauteur-2) creerLien(getParticule(x+2,y),getParticule(x,y+2));			}
		}


	// le haut gauche sur 3 unites sera immobiles
		for(int i=0;i<50; i++){
			getParticule(0 ,0+i)->offsetPos(Vec3(0.0,0.5,0.0)); 
			getParticule(0 ,0+i)->makeUnmovable(); 

		}
	}

	/* 	Le tissu est donc un ensemble de trinagles. pour 4 particules, on a ainsi :
	 
	 (x,y)   *--* (x+1,y)
	 | /|
	 |/ |
	 (x,y+1) *--* (x+1,y+1)
	 
	 */
	
	
	void drawShaded(){
		// reinitialiser les normales, qui changent constamment.
		std::vector<Particule>::iterator particule;
		for(particule = particules.begin(); particule != particules.end(); particule++){
			(*particule).resetNormal();
		}
		
		//ajout des normales
		for(int x = 0; x<nb_particules_large-1; x++){
			for(int y=0; y<nb_particules_hauteur-1; y++){
				Vec3 normal = calcTriangleNormal(getParticule(x+1,y),getParticule(x,y),getParticule(x,y+1));
				getParticule(x+1,y)->addToNormal(normal);
				getParticule(x,y)->addToNormal(normal);
				getParticule(x,y+1)->addToNormal(normal);
				
				normal = calcTriangleNormal(getParticule(x+1,y+1),getParticule(x+1,y),getParticule(x,y+1));
				getParticule(x+1,y+1)->addToNormal(normal);
				getParticule(x+1,y)->addToNormal(normal);
				getParticule(x,y+1)->addToNormal(normal);
			}
		}
		
		// dessin des triangles avec la couleur
		glBegin(GL_TRIANGLES);
		for(int x = 0; x<nb_particules_large-1; x++){
			for(int y=0; y<nb_particules_hauteur-1; y++){
				Vec3 color(0,0,0);
				
				color = Vec3(0.6f,0.2f,0.2f);
				
				drawTriangle(getParticule(x+1,y),getParticule(x,y),getParticule(x,y+1),color);
				drawTriangle(getParticule(x+1,y+1),getParticule(x+1,y),getParticule(x,y+1),color);
			}
		}
		glEnd();
	}

	/*  on regarde comment vont reagir les liens et les particules au temps t+1*/
	void timeStep(){
		std::vector<Lien>::iterator lien;
		for(int i=0; i<CONSTRAINT_ITERATIONS; i++) {// iterations sur tous les liens
			for(lien = liens.begin(); lien != liens.end(); lien++ ){
				(*lien).lienPossible(); 
			}
		}

		std::vector<Particule>::iterator particule;
		for(particule = particules.begin(); particule != particules.end(); particule++){
			(*particule).timeStep(); // calcul de la position  de chaque particule au temps t+1
		}
	}

/* ajout de la gravite a toutes les particules*/
	void addForce(const Vec3 direction){
		std::vector<Particule>::iterator particule;
		for(particule = particules.begin(); particule != particules.end(); particule++){
			(*particule).addForce(direction); // add the forces to each particle
		}

	}

	/* ajout des forces de vent a toutes les particules*/
	void windForce(const Vec3 direction){
		for(int x = 0; x<nb_particules_large-1; x++){
			for(int y=0; y<nb_particules_hauteur-1; y++){
				addWindForcesForTriangle(getParticule(x+1,y),getParticule(x,y),getParticule(x,y+1),direction);
				addWindForcesForTriangle(getParticule(x+1,y+1),getParticule(x+1,y),getParticule(x,y+1),direction);
			}
		}
	}

	/* dectection et resolution de la collision tissu/ball
	 On compare la position de la sphere et de chaque particule et on les corrige
	 */
	
	void ballCollision(const Vec3 center,const float radius ){
		std::vector<Particule>::iterator particule;
		for(particule = particules.begin(); particule != particules.end(); particule++){
			Vec3 v = (*particule).getPos()-center;
			float l = v.length();
			if ( v.length() < radius){ // if the particle is inside the ball
				(*particule).offsetPos(v.normalized()*(radius-l)); // project the particle to the surface of the ball
			}
		}
	}

	void doFrame(){

	}
};
// ========== DESSINER UN MAT ==========
void drawMat(){
	float surf = 0.5;
	int n = 20;
	int i,s=(n<0?-1:+1);
	n*=s;
	double pas=2*M_PI/n;
	double a=0.,cosa,sina;
	glBegin(GL_QUAD_STRIP);
	for (i=0;i<=n;i++){	
		double u, v;
		for(u = 0. ; u <= 2*M_PI ; u += surf){
			for(v = -1. ; v <= 1. ; v += surf){
				glBegin(GL_QUADS);
				glNormal3f(cos(u), sin(u),0.0);
				glVertex3d(cos(u), sin(u), v);
				glNormal3f(cos(u+surf), sin(u+surf),0.0);
				glVertex3d(cos(u+surf), sin(u+surf), v);
				if(v+surf > 1.){
					glNormal3f(cos(u+surf), sin(u+surf),0.0);
					glVertex3d(cos(u+surf), sin(u+surf), 1.);
					glNormal3f(cos(u), sin(u),0.0);
					glVertex3d(cos(u), sin(u), 1.);
				}
				else{
					glNormal3f(cos(u+surf), sin(u+surf),0.0);
					glVertex3d(cos(u+surf), sin(u+surf), v+surf);
					glNormal3f(cos(u),sin(u),0.0);
					glVertex3d(cos(u),sin(u), v+surf);
				}
				glEnd();
			}
		}
		
	}
	glEnd();
}




Tissu drap(15,10,55,50); // creation d'un tissu
Vec3 ball_pos(5,-5,0); // centre de la balle
float ball_radius = 2; // rayon de la balle
float ball_time = 0; // pour calculer la position en z de la balle
int ball = 0; // pour savoir si on dessine la balle ou non




// ========== INIT ==========

void init(void) {
	glShadeModel(GL_SMOOTH);
	glClearColor(0.2f, 0.2f, 0.4f, 0.5f);				
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_COLOR_MATERIAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat lightPos[4] = {-1.0,1.0,0.5,0.0};
	glLightfv(GL_LIGHT0,GL_POSITION,(GLfloat *) &lightPos);

	glEnable(GL_LIGHT1);

	GLfloat lightAmbient1[4] = {0.0,0.0,0.0,0.0};
	GLfloat lightPos1[4] = {1.0,0.0,-0.2,0.0};
	GLfloat lightDiffuse1[4] = {0.5,0.5,0.3,0.0};

	glLightfv(GL_LIGHT1,GL_POSITION,(GLfloat *) &lightPos1);
	glLightfv(GL_LIGHT1,GL_AMBIENT,(GLfloat *) &lightAmbient1);
	glLightfv(GL_LIGHT1,GL_DIFFUSE,(GLfloat *) &lightDiffuse1);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
}


float x =0, y =0, z=0, r = 25;
float *px = &x, *py =&y, *pz =&z, *pr =&r;


// ========== DRAW ==========
void draw(void) {

	ball_time++;
	ball_pos.f[2] = cos(ball_time/50.0)*7;


	drap.addForce(Vec3(0,-0.2,0)*TIME_STEPSIZE2); // ajout de la gravite
	drap.windForce(Vec3(1.5,0,0.2)*TIME_STEPSIZE2); // calcul de la force du vent
	drap.timeStep(); // calcul de la position de la particule a la prochaine image
	if(ball == 1){
	drap.ballCollision(ball_pos,ball_radius); // collision balle-tissu
}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glDisable(GL_LIGHTING); // changement de couleur pour le fond de la scene 
	glBegin(GL_POLYGON);
	glColor3f(1.0f,1.0f,1.0f);
	glVertex3f(-200.0f,-100.0f,-100.0f);
	glVertex3f(200.0f,-100.0f,-100.0f);
	glColor3f(0.4f,0.4f,0.8f);	
	glVertex3f(200.0f,100.0f,-100.0f);
	glVertex3f(-200.0f,100.0f,-100.0f);
	glEnd();
	glEnable(GL_LIGHTING);
	
	
	


	glTranslatef(-6.5+x,6+y,-20.0f+z); // translation pour voir de loin le tissu
	glRotatef(r,0,1,0); // rotation pour voir le tissu de cote
		glPushMatrix(); 
	drap.drawShaded(); // dessin du tissu
	glColor3f(0.5f,0.25f,0.0f);
	glTranslatef(-0.51,-8.0,0.0);
	glRotatef(90, 1.0,0.0,0.0);
	glScalef(0.5,0.5,9.0);
	drawMat();
		glPopMatrix();
	
	
	glPushMatrix(); 
	glTranslatef(ball_pos.f[0],ball_pos.f[1],ball_pos.f[2]); 
	glColor3f(0.0f,0.0f,1.0f);
	if(ball == 1){
		glutSolidSphere(ball_radius-0.1,50,50); 
	}
	glPopMatrix();

	
	
	glutSwapBuffers();
	glutPostRedisplay();
}


// ==========  ==========
void reshape(int w, int h)  {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();  
	if (h==0)  
		gluPerspective(80,(float)w,1.0,5000.0);
	else
		gluPerspective (80,( float )w /( float )h,1.0,5000.0 );
	glMatrixMode(GL_MODELVIEW);  
	glLoadIdentity(); 
}

// ========== KEYBOARD ==========
void keyboard( unsigned char key, int x, int y ) {
	switch ( key ) {
		case 'q':    // pour quitter
			exit ( 0 );
			break;  
		case 'f':
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Texture
			glutPostRedisplay();
			break;
		case 'l':
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Lignes
			glutPostRedisplay();
			break;
		case 'p':
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // Points
			glutPostRedisplay();
			break;
		case 'b': // pour activer ou non le dessin de la balle
			if (ball == 0){
				ball = 1;
			}
			else {
				ball =0;
			}
			glutPostRedisplay();
			break;
			// effet de camera
		case 'x' :
			*px= *px+0.2;
			glutPostRedisplay();
			break;
		case 'X' :
			*px= *px-0.2;
			glutPostRedisplay();
			break;
		case 'y' :
			*py= *py+0.2;
			glutPostRedisplay();
			break;
		case 'Y' :
			*py= *py-0.2;
			glutPostRedisplay();
			break;
		case 'z' :
			*pz= *pz+0.2;
			glutPostRedisplay();
			break;
		case 'Z' :
			*pz= *pz-0.2;
			glutPostRedisplay();
			break;
		case 'r' :
			*pr= *pr+5;
			glutPostRedisplay();
			break;
		case 'R' :
			*pr= *pr-5;
			glutPostRedisplay();
			break;
		default: 
			break;
	}
}

// ========== SPECIAL KEYS ==========
//pour mettre en plein ecran ou non
void arrow_keys( int a_keys, int x, int y ) {
	switch(a_keys) {
	case GLUT_KEY_UP:
		glutFullScreen();
		break;
	case GLUT_KEY_DOWN: 
		glutReshapeWindow (1000, 700 );
		break;
	default:
		break;
	}
}


// ========== MAIN ==========
int main ( int argc, char** argv ) {
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ); 
	glutInitWindowSize(1000, 700 ); 

	glutCreateWindow( "Drapeau" );
	init();
	glutDisplayFunc(draw);  
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(arrow_keys);

	glutMainLoop();
}
