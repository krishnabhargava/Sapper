#include <string.h>
#include <math.h>
#include <stdio.h>
#include <fstream>
#include <GL/glut.h>
using namespace std;

ofstream DEBUG_FILE;

#define GAME_WIDTH 1376
#define GAME_HEIGHT 768
#define SCREEN_WIDTH 1376
#define SCREEN_HEIGHT 800
#define HUD_HEIGHT ((SCREEN_HEIGHT) - (GAME_HEIGHT))
#define TILE_SIZE 32

#define CLOCK_SPEED 40
#define SPLASH_LN_TIME 0//150
#define SPLASH_SAPPER_TIME 0//162
#define SPLASH_INSTRUCTION_TIME 0//350
int IntroTime = 0;
int IntroSapperTime = 0;

int Time = 0;
int LevelTime = 0;

int InGame = 0;
int NewGame = 0;
int CurrentLevel = 0;
int InCredits = 0;
int LevelComplete = 0;
int GameOver = 0;
int Choice = 0;
int LevelChoice = 0;
int SelectLevel = 0;
int LevelCount = 0;
int SoldierCount = 0;
int GoSplat = 0;

int Textures[6];
GLuint LoadTexture( const char * filename, int width, int height )
{
    GLuint texture;
    unsigned char * data;
    FILE * file;

    //The following code will read in our RAW file
    file = fopen( filename, "rb" );
    if ( file == NULL ) return 0;
    data = (unsigned char *)malloc( width * height * 3 );
    fread( data, width * height * 3, 1, file );
    fclose( file );

    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
GL_MODULATE ); //set texture environment parameters

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
GL_LINEAR );

    //Here we are setting the parameter to repeat the texture 
//instead of clamping the texture
    //to the edge of our shape. 
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 
GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 
GL_REPEAT );

    //Generate the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, 
GL_RGB, GL_UNSIGNED_BYTE, data);
    free( data ); //free the texture
    return texture; //return whether it was successfull
}
void FreeTexture( GLuint texture )
{
  glDeleteTextures( 1, &texture ); 
}

int collide_detect(int bottom1, int top1, int left1, int right1, int bottom2, int top2, int left2, int right2)
{
		if (bottom1 > top2)
		{
			return(0);
		}
		
        if (top1 < bottom2)		
		{
			return(0);
		}
        if (right1 < left2) 		
		{

			return(0);
		}
        if (left1 > right2)		
		{
			return(0);
		}		
        return(1);
}

void drawTexture()
{

	glBegin (GL_QUADS); //begin drawing our quads
    glTexCoord2d(0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0); //with our vertices we have to assign a texcoord
    
    glTexCoord2d(1.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0); //so that our texture has some points to draw to
    
    glTexCoord2d(1.0, 1.0);
    glVertex3f(1.0, 1.0, 0.0);
    
    glTexCoord2d(0.0, 1.0);
    glVertex3f(0.0, 1.0, 0.0);
    glEnd();
}

void displayImage(char imageFile[], int width, int height, int x, int y)
{
	ifstream myfile;
	myfile.open(imageFile, ios::in);
	if (!myfile) {
		printf("Failed to open image file %s.", imageFile);
	  exit(1);
	}
	int r, g, b, a;

	glPushMatrix();
	glTranslatef(x, y, 0);

	for(int i = 0; i < width; i++)
	{
		for(int j = 0, k = height - 1; k >= 0; k--)
		{
			myfile>>r;
			myfile>>g;
			myfile>>b;
			myfile>>a;
			glColor4f(r/255.0, g/255.0, b/255.0, a/255.0);
			glBegin(GL_POLYGON);
			glVertex2f(i, k);
			glVertex2f(i + 1, k);
			glVertex2f(i + 1, k + 1);
			glVertex2f(i, k + 1);
			glEnd();
		}
	}
	glPopMatrix();
}

class Player
{
	// Screen co-ordinates
	int co_x, co_y;
	int angle;
	int direction;
	int state;
	char direc;
	int splatTime;

public:
	// Draw player sprite onto the screen
	void draw();

	// Update screen co-ordinates
	void update(int, int, char);
	void updateAngle(int);
	void refresh();
	
	// Constructor
	Player(int, int, int);
	
	// 
	int getX()
	{
		return co_x;
	}
	
	int getY()
	{
		return co_y;
	}
	
}player(0, 0, 0);
Player::Player(int x = 0, int y = 0, int degrees = 0)
{
	co_x = x;
	co_y = y;
	angle = degrees;
	direction = 1;
	state = 1;
	direc = 'a';
	splatTime = 0;
}

void Player::refresh()
{
	direction = 1;
	state = 1;
	direc = 'a';
	splatTime = 0;
}

void Player::update(int x, int y, char dir)
{
	if(direc != dir)
	{
		if(dir == 'd')
		{
			direction = 2;
			direc = dir;
		}
		
		if(dir == 'a')
		{
			direction = 1;
			direc = dir;
		}

		if(dir == 's')
		{
			direction = 4;
			direc = dir;
		}

		if(dir == 'w')
		{
			direction = 3;
			direc = dir;
		}

		if(dir == ' ')
		{
			direction = 5;
			direc = dir;
		}

	}

	if(state == 1)
		state = 2;
	else if(state == 2)
		state = 1;
	co_x = x;
	co_y = y;
}
void Player::updateAngle(int degrees)
{
	angle = degrees;
}
void Player::draw()
{
	glPushMatrix();
	if(!GoSplat)
	{

		if(direction == 1 && state == 1)
		{
			displayImage("images\\soldierleft1.raw", TILE_SIZE, TILE_SIZE, co_x, co_y);
		}

		else if(direction == 1 && state == 2)
		{
			displayImage("images\\soldierleft2.raw", TILE_SIZE, TILE_SIZE, co_x, co_y);

		}

		if(direction == 2 && state == 1)
		{
			displayImage("images\\soldierright1.raw", TILE_SIZE, TILE_SIZE, co_x, co_y);

		}

		else if(direction == 2 && state == 2)
		{
			displayImage("images\\soldierright2.raw", TILE_SIZE, TILE_SIZE, co_x, co_y);

		}

		if(direction == 3 && state == 1)
		{
			displayImage("images\\soldierup1.raw", TILE_SIZE, TILE_SIZE, co_x, co_y);

		}

		else if(direction == 3 && state == 2)
		{
			displayImage("images\\soldierup2.raw", TILE_SIZE, TILE_SIZE, co_x, co_y);

		}
		if(direction == 4 && state == 1)
		{
			displayImage("images\\soldierdown1.raw", TILE_SIZE, TILE_SIZE, co_x, co_y);

		}

		else if(direction == 4 && state == 2)
		{
			displayImage("images\\soldierdown2.raw", TILE_SIZE, TILE_SIZE, co_x, co_y);

		}

		else if(direction == 5)
		{
			displayImage("images\\soldiersit.raw", TILE_SIZE, TILE_SIZE, co_x, co_y);
		}
	
		
	}
	
	else
	{

		char imageFile[50];
		if(++splatTime >= 6)
		{
			displayImage("images\\splat5.raw", TILE_SIZE, TILE_SIZE, co_x, co_y);
			if(Time - GoSplat > 25)
				GameOver = 1;
		}
		else
		{
			sprintf(imageFile, "images\\splat%d.raw", splatTime);
			displayImage(imageFile, TILE_SIZE, TILE_SIZE, co_x, co_y);
		}
	}
	glPopMatrix();
}

struct SoldierValues
{
	int steps;
	int x[100], y[100];
}soldierValues;

class Soldier
{
	int reverse;
	int steps;
	int currentStep;
	char direction;
	int x[100];
	int y[100];
	int state;
	int foundPlayer;

	public:
		void init(int [], int [], int);
		void draw();
		void detect();
		
}soldier[100];
void Soldier::init(int x1[], int y1[], int count)
{

	currentStep = 1;
	reverse = 0;
	foundPlayer = 0;
	int direction;
	
	if(x[currentStep] - x[currentStep + 1] < 0)
	{
		direction = 2;
	}

	else if(x[currentStep] - x[currentStep + 1] > 0)
	{
		direction = 1;
	}

	else if(y[currentStep] - y[currentStep + 1] < 0)
	{
		direction = 3;
	}

	else if(y[currentStep] - y[currentStep + 1] > 0)
	{
		direction = 4;
	}

	steps = count;
	state = 1;
	for(int i = 0; i < count; i++)
	{
		x[i] = x1[i];
		y[i] = y1[i];
		// printf("Added %d, %d\n", x[i], y[i]);
	}
}
void Soldier::draw()
{
	if(collide_detect(y[currentStep] +1, y[currentStep] + TILE_SIZE - 1, x[currentStep] + 1, x[currentStep] + TILE_SIZE - 1, 
		player.getY() - TILE_SIZE * 4, player.getY() + TILE_SIZE * 5, player.getX() - TILE_SIZE * 4, player.getX() + TILE_SIZE * 4))
	{
		glColor4f(255.0/255.0, 56.0/255.0, 53.0/255.0, 60.0/255.0);
		glBegin(GL_POLYGON);
		glVertex2f(x[currentStep] - TILE_SIZE, y[currentStep] - TILE_SIZE);
		glVertex2f(x[currentStep] + 2 * TILE_SIZE, y[currentStep] - TILE_SIZE);
		glVertex2f(x[currentStep] + 2 * TILE_SIZE, y[currentStep] + 2 * TILE_SIZE);
		glVertex2f(x[currentStep] - TILE_SIZE, y[currentStep] + 2 * TILE_SIZE);
		glEnd();
		glColor4f(0, 0, 0, 1);
	
		glPushMatrix();
		if(direction == 1 && state == 1)
		{
			displayImage("images\\soldierleft1.raw", TILE_SIZE, TILE_SIZE, x[currentStep], y[currentStep]);

		}

		else if(direction == 1 && state == 2)
		{
			displayImage("images\\soldierleft2.raw", TILE_SIZE, TILE_SIZE, x[currentStep], y[currentStep]);

		}

		if(direction == 2 && state == 1)
		{
			displayImage("images\\soldierright1.raw", TILE_SIZE, TILE_SIZE, x[currentStep], y[currentStep]);

		}

		else if(direction == 2 && state == 2)
		{
			displayImage("images\\soldierright2.raw", TILE_SIZE, TILE_SIZE, x[currentStep], y[currentStep]);

		}

		if(direction == 3 && state == 1)
		{
			displayImage("images\\soldierup1.raw", TILE_SIZE, TILE_SIZE, x[currentStep], y[currentStep]);

		}

		else if(direction == 3 && state == 2)
		{
			displayImage("images\\soldierup2.raw", TILE_SIZE, TILE_SIZE, x[currentStep], y[currentStep]);

		}
		if(direction == 4 && state == 1)
		{
			displayImage("images\\soldierdown1.raw", TILE_SIZE, TILE_SIZE, x[currentStep], y[currentStep]);

		}

		else if(direction == 4 && state == 2)
		{
			displayImage("images\\soldierdown2.raw", TILE_SIZE, TILE_SIZE, x[currentStep], y[currentStep]);

		}

		glPopMatrix();
	}
	
	if(Time % 12 == 0 && !foundPlayer)
	{
		if(currentStep == steps - 1)
			reverse = 1;

		else if(currentStep == 0)
			reverse = 0;

		if(reverse == 1)
			currentStep--;
		else 
			currentStep++;

		if(reverse == 0)
		{
			if(x[currentStep] - x[currentStep + 1] < 0)
			{
				direction = 2;
			}

			else if(x[currentStep] - x[currentStep + 1] > 0)
			{
				direction = 1;
			}

			else if(y[currentStep] - y[currentStep + 1] < 0)
			{
				direction = 3;
			}

			else if(y[currentStep] - y[currentStep + 1] > 0)
			{
				direction = 4;
			}
			// printf("Comparing %d %d\n", currentStep, currentStep - 1);
		}

		else if(reverse == 1)
		{
			if(x[currentStep] - x[currentStep - 1] < 0)
			{
				direction = 2;
			}

			else if(x[currentStep] - x[currentStep - 1] > 0)
			{
				direction = 1;
			}

			else if(y[currentStep] - y[currentStep - 1] < 0)
			{
				direction = 3;
			}

			else if(y[currentStep] - y[currentStep - 1] > 0)
			{
				direction = 4;
			}
			// printf("Comparing %d %d\n", currentStep, currentStep - 1);
		}

		if(state == 1)
			state = 2;
		else if(state == 2)
			state = 1;
	}
}

void Soldier::detect()
{
	if(collide_detect(y[currentStep] - TILE_SIZE, y[currentStep] + 2 * TILE_SIZE, x[currentStep] - TILE_SIZE, x[currentStep] + 2 * TILE_SIZE, 
		player.getY() + 1, player.getY() + TILE_SIZE - 1, player.getX() + 1, player.getX() + TILE_SIZE - 1))
	{
		if(!foundPlayer)
			GoSplat = Time;
		foundPlayer = 1;
		
	}


}

struct Level
{
	int bomb_co_x, bomb_co_y;
	int player_x, player_y;
	int bomb_status;
	int map[GAME_WIDTH/TILE_SIZE][GAME_HEIGHT/TILE_SIZE];
}level;

void splashLogicNotMagic()
{

	//displayImage("introsapper.txt", 130, 130, SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2);
	glPushMatrix();
	glColor3f(101.0/255.0, 141.0/255.0, 209.0/255.0);
	char s[50];
	
	static int y1 = SCREEN_HEIGHT;
	if(y1 > SCREEN_HEIGHT/2)
		y1-=3;
	sprintf(s, "Logic Not Magic");
	glTranslatef(SCREEN_WIDTH/2 - 220,y1,0);
	
	glScalef(0.4, 0.4, 0.0);
	glLineWidth(5.0);
	void * font = GLUT_STROKE_ROMAN;
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();
	

	static int y2 = 0;
	if(y2 < SCREEN_HEIGHT/2 - 60)
		y2+=3;
	glPushMatrix();
	sprintf(s, "Productions");
	glTranslatef(SCREEN_WIDTH/2 - 80, y2, 0.0);
	glScalef(0.25, 0.25, 0.0);
	glLineWidth(5.0);
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();


	static int y3 = -40;
	if(y3 < SCREEN_HEIGHT/2 - 100)
		y3+=3;
	glPushMatrix();
	sprintf(s, "Presents...");
	glTranslatef(SCREEN_WIDTH/2 - 40, y3, 0.0);
	glScalef(0.25, 0.25, 0.0);
	glLineWidth(5.0);
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();


}
void splashSapper()
{
	displayImage("images\\introsapper.raw", 128, 128, SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2);

	glPushMatrix();
	glColor3f(22.0/255.0, 104.0/255.0, 255.0/255.0);
	char s[50];
	sprintf(s, "THE SAPPER");
	glTranslatef(SCREEN_WIDTH/2 - 170,SCREEN_HEIGHT/2,0);
	glScalef(0.4, 0.4, 0.0);
	glLineWidth(8.0);
	void * font = GLUT_STROKE_ROMAN;
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();
}

void splashInstructions()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, Textures[5]); 
	glPushMatrix();
	glTranslatef(0, 0, 0);
	glScalef(SCREEN_WIDTH, SCREEN_HEIGHT, 1);
	drawTexture();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);	
}
void mainMenu()
{
	glPushMatrix();
	glColor3f(22.0/255.0, 104.0/255.0, 255.0/255.0);
	char s[50];
	sprintf(s, "THE SAPPER");
	glTranslatef(SCREEN_WIDTH/2 - 170,SCREEN_HEIGHT/2 - 150,0);
	glRotatef(90, 0, 0, 1);
	glScalef(0.4, 0.4, 0.0);
	glLineWidth(8.0);
	void * font = GLUT_STROKE_ROMAN;
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();

	glPushMatrix();
	
	if(Choice == 0)
	{
		glTranslatef(SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2 + 50,0);
		glColor3f(119.0/255.0, 32.0/255.0, 65.0/255.0);
		glLineWidth(7.0);
		glScalef(0.3, 0.3, 0.0);
	}
	else
	{
		glTranslatef(SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2 + 50,0);
		glColor3f(57.0/255.0, 134.0/255.0, 96.0/255.0);
		glLineWidth(5.0);
		glScalef(0.2, 0.2, 0.0);
	}
	sprintf(s, "New Game");
	
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();

	glPushMatrix();
	
	if(Choice == 1)
	{
		glTranslatef(SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2,0);
		glColor3f(119.0/255.0, 32.0/255.0, 65.0/255.0);
		glLineWidth(7.0);
		glScalef(0.3, 0.3, 0.0);
	}
	else
	{
		glTranslatef(SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2,0);
		glColor3f(57.0/255.0, 134.0/255.0, 96.0/255.0);
		glLineWidth(5.0);
		glScalef(0.2, 0.2, 0.0);
	}

	sprintf(s, "Load Game");
	
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();

		glPushMatrix();
	
	if(Choice == 2)
	{
		glTranslatef(SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2-50,0);
		glColor3f(119.0/255.0, 32.0/255.0, 65.0/255.0);
		glLineWidth(7.0);
		glScalef(0.3, 0.3, 0.0);
	}
	else
	{
		glTranslatef(SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2-50,0);
		glColor3f(57.0/255.0, 134.0/255.0, 96.0/255.0);
		glLineWidth(5.0);
		glScalef(0.2, 0.2, 0.0);
	}

	sprintf(s, "Credits");
	
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();

	glPushMatrix();

	if(Choice == 3)
	{
		glTranslatef(SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2-100,0);
		glColor3f(119.0/255.0, 32.0/255.0, 65.0/255.0);
		glLineWidth(7.0);
		glScalef(0.3, 0.3, 0.0);
	}
	else
	{
		glTranslatef(SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2-100,0);
		glColor3f(57.0/255.0, 134.0/255.0, 96.0/255.0);
		glLineWidth(5.0);
		glScalef(0.2, 0.2, 0.0);
	}

	sprintf(s, "Exit");
	
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();
}
void drawCircle(GLint h, GLint k, GLint r)
{
	float angle;
	int i;
	glBegin(GL_POLYGON);
	for(i = 0; i < 100; i++)
	{
		angle = 3.14 / 100 * i * 2;
		glVertex2f(h + cos(angle) * r, k + sin(angle) * r);
	}
	glEnd();
}

void credits()
{
	glPushMatrix();
	glColor3f(22.0/255.0, 104.0/255.0, 255.0/255.0);
	char s[50];
	sprintf(s, "CREDITS");
	glTranslatef(SCREEN_WIDTH/2 - 170,SCREEN_HEIGHT/2 - 100,0);
	glRotatef(90, 0, 0, 1);
	glScalef(0.4, 0.4, 0.0);
	glLineWidth(8.0);
	void * font = GLUT_STROKE_ROMAN;
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();

	glPushMatrix();
	glTranslatef(SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 + 70,0);
	glColor3f(255.0/255.0, 43.0/255.0, 60.0/255.0);
	glLineWidth(6.0);
	glScalef(0.2, 0.2, 0.0);
	sprintf(s, "Team");
	
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();

	glPushMatrix();
	glTranslatef(SCREEN_WIDTH/2 - 40, SCREEN_HEIGHT/2 + 70,0);
	glColor3f(213.0/255.0, 255.0/255.0, 155.0/255.0);
	glLineWidth(5.0);
	glScalef(0.2, 0.2, 0.0);
	sprintf(s, "Krishna Bhargava M L");
	
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();

	glPushMatrix();
	glTranslatef(SCREEN_WIDTH/2 - 40, SCREEN_HEIGHT/2 + 40,0);
	glColor3f(213.0/255.0, 255.0/255.0, 155.0/255.0);
	glLineWidth(5.0);
	glScalef(0.2, 0.2, 0.0);
	sprintf(s, "K S Shrivara");
	
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();

	glPushMatrix();
	glTranslatef(SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 - 40,0);
	glColor3f(255.0/255.0, 43.0/255.0, 60.0/255.0);
	glLineWidth(6.0);
	glScalef(0.2, 0.2, 0.0);
	sprintf(s, "Under the Guidance of");
	
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();

	glPushMatrix();
	glTranslatef(SCREEN_WIDTH/2 - 40, SCREEN_HEIGHT/2 - 80,0);
	glColor3f(213.0/255.0, 255.0/255.0, 155.0/255.0);
	glLineWidth(5.0);
	glScalef(0.2, 0.2, 0.0);
	sprintf(s, "Mrs. Anitha Premkumar");
	
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();
	
}

void displayLevel()
{
	// Display Map
	DEBUG_FILE<<"--- display ---\n";
	for(int x = 0, i = 0; x <= GAME_WIDTH - TILE_SIZE; x += TILE_SIZE, i++)
	{
		for(int y = GAME_HEIGHT, j = 0; y > 0; y -= TILE_SIZE, j++)
		{
			DEBUG_FILE<<level.map[i][j];
			glEnable(GL_TEXTURE_2D);
			glBindTexture( GL_TEXTURE_2D, Textures[level.map[i][j]]); 
			glPushMatrix();
			glTranslatef(x, y - TILE_SIZE, 0);
			glScalef(32, 32, 1);
			drawTexture();
			glPopMatrix();
			glDisable(GL_TEXTURE_2D);
		}
		DEBUG_FILE<<endl;
	}
	DEBUG_FILE<<"--- display ---\n";

}
void displayBomb()
{
	static int radius = 0;
	static int direction = 1;
	if(direction == 1 && ++radius == TILE_SIZE/4)
		direction = -1;
	
	else if(direction == -1 && --radius == 0)
		direction = 1;

	glColor3f(255.0/255.0, 100.0/255.0, 89.0/255.0);
	drawCircle(level.bomb_co_x + TILE_SIZE/2, level.bomb_co_y + TILE_SIZE/2, radius);
	//drawCircle(level.bomb_co_x + TILE_SIZE/2, level.bomb_co_y + TILE_SIZE/2 + 10 + 32, radius);

	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, Textures[4]); 
	//glPushMatrix();
	//glTranslatef(level.bomb_co_x, level.bomb_co_y, 0);
	//glScalef(32, 32, 1);
	////glTranslatef(-level.bomb_co_x - TILE_SIZE/2, -level.bomb_co_y - TILE_SIZE/2, 0);
	//drawTexture();
	//glPopMatrix();
	//glDisable(GL_TEXTURE_2D);
}
void displayHUD()
{
	glPushMatrix();
	glColor3f(56.0/255.0, 178.0/255.0, 255.0/255.0);
	glBegin(GL_POLYGON);
		glVertex2f(0, SCREEN_HEIGHT);
		glVertex2f(0, SCREEN_HEIGHT - HUD_HEIGHT);
		glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT - HUD_HEIGHT);
		glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glColor3f(255.0/255.0, 100.0/255.0, 89.0/255.0);
	glBegin(GL_POLYGON);
		glVertex2f(0, SCREEN_HEIGHT);
		glVertex2f(0, SCREEN_HEIGHT - HUD_HEIGHT);
		glVertex2f(SCREEN_WIDTH*(level.bomb_status)/100, SCREEN_HEIGHT - HUD_HEIGHT);
		glVertex2f(SCREEN_WIDTH*(level.bomb_status)/100, SCREEN_HEIGHT);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0, 0, 0);
	char s[50];
	sprintf(s, "Level %d Time: %d", CurrentLevel + 1,(Time - LevelTime)/25);
	glTranslatef(10, SCREEN_HEIGHT - HUD_HEIGHT + 4,0);
	glScalef(0.2, 0.2, 0.0);
	glLineWidth(1.5);
	void * font = GLUT_STROKE_ROMAN;
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();
}
void displayFog()
{
	int playerY = player.getY();
	int playerX = player.getX();
	glPushMatrix();
	glColor4f(0, 0, 0, 140.0/255.0);
	glBegin(GL_POLYGON);
		glVertex2f(0, GAME_HEIGHT);
		glVertex2f(GAME_WIDTH, GAME_HEIGHT);
		glVertex2f(GAME_WIDTH, playerY + TILE_SIZE * 5);
		glVertex2f(0, playerY + TILE_SIZE * 5);
	glEnd();
	glBegin(GL_POLYGON);
		glVertex2f(0, 0);
		glVertex2f(GAME_WIDTH, 0);
		glVertex2f(GAME_WIDTH, playerY - TILE_SIZE * 4);
		glVertex2f(0, playerY - TILE_SIZE * 4);
	glEnd();
	glBegin(GL_POLYGON);
		glVertex2f(0, playerY + TILE_SIZE * 5);
		glVertex2f(0, playerY - TILE_SIZE * 4);
		glVertex2f(playerX - TILE_SIZE * 4, playerY - TILE_SIZE * 4);
		glVertex2f(playerX - TILE_SIZE * 4, playerY + TILE_SIZE * 5);
	glEnd();
	glBegin(GL_POLYGON);
		glVertex2f(GAME_WIDTH, playerY + TILE_SIZE * 5);
		glVertex2f(GAME_WIDTH, playerY - TILE_SIZE * 4);
		glVertex2f(playerX + TILE_SIZE * 5, playerY - TILE_SIZE * 4);
		glVertex2f(playerX + TILE_SIZE * 5, playerY + TILE_SIZE * 5);
	glEnd();
	glPopMatrix();
}
void displayLevelComplete()
{
	glPushMatrix();
	glColor3f(135.0/255.0, 255.0/255.0, 114.0/255.0);
	char s[50];
	sprintf(s, "LEVEL COMPLETE!");
	glTranslatef(SCREEN_WIDTH/2 - 220,SCREEN_HEIGHT/2 - 30,0);
	glRotatef(10, 0, 0, 1);
	glScalef(0.4, 0.4, 0.0);
	glLineWidth(8.0);
	void * font = GLUT_STROKE_ROMAN;
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();


}
void displaySelectLevel()
{
	glPushMatrix();
	glColor3f(22.0/255.0, 104.0/255.0, 255.0/255.0);
	char s[50];
	sprintf(s, "LOAD LEVEL");
	glTranslatef(SCREEN_WIDTH/2 - 170,SCREEN_HEIGHT/2 - 150,0);
	glRotatef(90, 0, 0, 1);
	glScalef(0.4, 0.4, 0.0);
	glLineWidth(8.0);
	void * font = GLUT_STROKE_ROMAN;
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();



	for(int i = 0; i < LevelCount; i++)
	{
		glPushMatrix();
	
		if(LevelChoice == i)
		{
			glTranslatef(SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2 + 120 - (i * 50) ,0);
			glColor3f(119.0/255.0, 32.0/255.0, 65.0/255.0);
			glLineWidth(7.0);
			glScalef(0.3, 0.3, 0.0);
		}
		else
		{
			glTranslatef(SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2 + 120 - (i * 50),0);
			glColor3f(57.0/255.0, 134.0/255.0, 96.0/255.0);
			glLineWidth(5.0);
			glScalef(0.2, 0.2, 0.0);
		}
		sprintf(s, "Level %d", i + 1);
	
		for (int i = 0; i <= strlen(s); i++)
		{
			glutStrokeCharacter(font, s[i]);
		}
		glPopMatrix();
	}

}
void displayGameOver()
{
	glPushMatrix();
	glColor3f(135.0/255.0, 255.0/255.0, 114.0/255.0);
	char s[50];
	sprintf(s, "GAME OVER!");
	glTranslatef(SCREEN_WIDTH/2 - 180,SCREEN_HEIGHT/2 - 30,0);
	glRotatef(10, 0, 0, 1);
	glScalef(0.4, 0.4, 0.0);
	glLineWidth(8.0);
	void * font = GLUT_STROKE_ROMAN;
	for (int i = 0; i <= strlen(s); i++)
	{
		glutStrokeCharacter(font, s[i]);
	}
	glPopMatrix();


}
void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Reset Colors
	glColor4f(1, 1, 1, 255);

	if(Time < SPLASH_LN_TIME)
		splashLogicNotMagic();
	else if(Time < SPLASH_SAPPER_TIME)
		splashSapper();
	else if(Time < SPLASH_INSTRUCTION_TIME)
		splashInstructions();
	else if(SelectLevel)
		displaySelectLevel();
	
	else if(LevelComplete == 1)
		displayLevelComplete();

	else if(GameOver)
		displayGameOver();
	
	else if(InCredits)
		credits();

	else if(InGame)
	{
		displayLevel();

		for(int i = 0; i < SoldierCount; i++)
		{
			soldier[i].detect();
			soldier[i].draw();
		}
		displayFog();
		displayBomb();
		player.draw();
		displayHUD();
	}

	else 
		mainMenu();
	
	glFlush();
	glutSwapBuffers();
}

void loadMap(char mapFileName[], char soldierFileName[])
{
	ifstream mapFile;
	mapFile.open(mapFileName, ios::binary); 

	// Try reading the next map
	if(!mapFile)
	{
		GameOver = 1;
		DEBUG_FILE<<"! Failed to open map.\n";
	}
	else
	{
		mapFile.read((char*)&level, sizeof(level));
		mapFile.close();
		LevelTime = Time;
		player.update(level.player_x, level.player_y, 'a');
	}
	// printf("Map Info: %d",level.map[0][0]);
	mapFile.close();
	// SOLDIER
	ifstream soldierFile;
	soldierFile.open(soldierFileName, ios::binary); 
	soldierFile.read((char*)&SoldierCount, sizeof(int));
	for(int i = 0; i < SoldierCount; i++)
	{
		soldierFile.read((char*)&soldierValues, sizeof(SoldierValues));
		soldier[i].init(soldierValues.x, soldierValues.y, soldierValues.steps);
	}
	// printf("%d", soldierValues.steps);
	mapFile.close();

	player.refresh();
}

void keyboard(unsigned char key, int x, int y)
{
	if(InGame)
	{
		if(!GoSplat)
		{
			if(key == 'w') 
			{
				if((level.map[player.getX() / TILE_SIZE][((GAME_HEIGHT - player.getY()) / TILE_SIZE) - 2]) == 1)
					player.update(player.getX(), player.getY() + TILE_SIZE, key);
				else
					player.update(player.getX(), player.getY(), key);
			}
			if(key == 'a') 
			{
				if(level.map[player.getX() / TILE_SIZE - 1][((GAME_HEIGHT - player.getY()) / TILE_SIZE) - 1] == 1)
					player.update(player.getX() - TILE_SIZE, player.getY(), key);
				else
					player.update(player.getX(), player.getY(), key);
			}

			if(key == 's') 
			{
				if(level.map[player.getX() / TILE_SIZE][((GAME_HEIGHT - player.getY()) / TILE_SIZE)] == 1)
					player.update(player.getX(), player.getY() - TILE_SIZE, key);
			
				else
					player.update(player.getX(), player.getY(), key);
			}
			if(key == 'd') 
			{
				if(level.map[player.getX() / TILE_SIZE + 1][((GAME_HEIGHT - player.getY()) / TILE_SIZE) - 1] == 1)
					player.update(player.getX() + TILE_SIZE, player.getY(), key);
			
				else
					player.update(player.getX(), player.getY(), key);
			}


			if(key == ' ')
			{
				player.update(player.getX(), player.getY(), key);
				if(player.getX() == level.bomb_co_x && player.getY() == level.bomb_co_y)
					level.bomb_status++;
				if(level.bomb_status == 100) 
				{
					InGame = 0;
					LevelComplete = 1;
				}
			}

		}
		
		if(key == 'q')
		{
			InGame = 0;
		}


		
	}

	if(LevelComplete == 1)
	{
		if(key == 'q')
		{
			CurrentLevel++;
			char s1[50];
			sprintf(s1, "maps\\map%d.bin", CurrentLevel);
			char s2[50];
			sprintf(s2, "maps\\soldier%d.bin", CurrentLevel);
			loadMap(s1, s2);
			GoSplat = 0;
			InGame = 1;
			LevelComplete = 0;
		}
	}
		

	else if(InCredits || InGame || GameOver)
	{
		if(key == 'q')
		{
			GameOver = 0;
			InCredits = 0;
			NewGame = 0;
			CurrentLevel = 0;
			GoSplat = 0;
		}
	}

	else if(SelectLevel)
	{
		if(key == 's' && LevelChoice != LevelCount - 1) LevelChoice++;
		if(key == 'w' && LevelChoice != 0) LevelChoice--;
		if(key == ' ')
		{
			char s1[50];
			sprintf(s1, "maps\\map%d.bin", LevelChoice);
			char s2[50];
			sprintf(s2, "maps\\soldier%d.bin", LevelChoice);
			loadMap(s1, s2);
			GoSplat = 0;
			InGame = 1;
			SelectLevel = 0;
			CurrentLevel = LevelChoice;
		}
		if(key == 'q')
		{
			GameOver = 0;
			InCredits = 0;
			GoSplat = 0;
			NewGame = 0;
			CurrentLevel = 0;
			SelectLevel = 0;
		}
	}

	else
	{
		if(key == 's' && Choice != 3) Choice++;
		if(key == 'w' && Choice != 0) Choice--;
		if(key == ' ') 
		{
			switch(Choice)
			{
				case 0:
					loadMap("maps\\map0.bin", "maps\\soldier0.bin");
					GoSplat = 0;
					CurrentLevel = 0;
					NewGame = 1;
					InGame = 1;
					break;
				case 1:
					SelectLevel = 1;
					break;
				case 2:
					InCredits = 1;
					break;
				case 3:
					exit(0);
					break;
			}
		}
	}
}

void Timer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(CLOCK_SPEED, Timer, 0);
	Time++;
}

void setupWindowAndLevelCountAndTextures()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, SCREEN_WIDTH, 0.0, SCREEN_HEIGHT);
	
	// Find the number of levels
	char mapFileName[50];
	for(LevelCount = 0;;LevelCount++)
	{
		sprintf(mapFileName, "maps\\map%d.bin", LevelCount);
		ifstream mapFile;
		mapFile.open(mapFileName, ios::binary); 
		if(!mapFile)
			break;
	}
	DEBUG_FILE<<"Level Count: "<<LevelCount<<".\n";

	DEBUG_FILE<<"Loading Textures.\n";
	Textures[0] = LoadTexture("images\\empty.raw", 32, 32);
	Textures[1] = LoadTexture("images\\floor.raw", 32, 32);
	Textures[2] = LoadTexture("images\\wall1.raw", 32, 32);
	Textures[3] = LoadTexture("images\\wall2.raw", 32, 32);
	Textures[4] = LoadTexture("images\\bomb.raw", 32, 32);
	Textures[5] = LoadTexture("images\\instructions.raw", 1376,768);

}

int main(int argc, char** argv)
{
	DEBUG_FILE.open("maps\\log.txt");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	char gameString[50];
	sprintf(gameString, "%dx%d:32@60", SCREEN_WIDTH, SCREEN_HEIGHT);
	DEBUG_FILE<<"Game String: "<<gameString<<".\n";
	glutGameModeString(gameString);
	glutEnterGameMode();
	//glutCreateWindow("The Sapper");
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	setupWindowAndLevelCountAndTextures();
	Timer(0);

	glutMainLoop();
	DEBUG_FILE.close();
}