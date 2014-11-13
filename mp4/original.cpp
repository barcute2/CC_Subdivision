
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>


int nFPS = 30;
float fRotateAngle = 0.f;
GLfloat x = 0, y = 0;
GLenum rotate = GL_FALSE;
double p0x = 7, p0y = 7, p0z = -9;
double p1x = 3, p1y = 4, p1z = -5, t = 0;
double p2x = -2, p2y = -3, p2z = -7;
double p3x = -7, p3y = -7, p3z = -9;
double camerax=0, cameray=0, cameraz=4;
int timeron = 1;

//this struct will hold pairs of point indexes that make an edge
struct pair{
	int first, second;
};

//this struct will hold the coordinates of each point
struct coordinate{
        float x,y,z;
        coordinate(float a,float b,float c) : x(a),y(b),z(c) {};
};
//this struct will be a face made from a quad
struct face{
        int faceNumber;
        int faces[4];
        std::vector<int> edgePoints;
        face(int facen,int f1,int f2,int f3,int f4){ 
                faceNumber = facen;
                faces[0]=f1;
                faces[1]=f2;
                faces[2]=f3;
                faces[3]=f4;
        }
};

struct edge{
	int face1, face2;
	int point1, point2, middlePoint;
	edge(int p1, int p2, int mp, int f1, int f2){
		point1 = p1;
		point2 = p2;
		middlePoint = mp;
		face1 = f1;
		face2 = f2;
	}
};
std::vector<coordinate> facePoints;

class Mesh{
	public:
	std::vector<coordinate> points;
	std::vector<face> faces;
	std::vector<coordinate> normals;
	std::vector<coordinate> edgePoints;
	std::vector<coordinate> newPoints;
	std::vector<edge> edges;
	
	int beginFace, beginPoint;
	int oldPointSize, oldFaceSize;

	
	

	void set_values(std::vector<coordinate> *pts, std::vector<face> *fcs, std::vector<coordinate> *nrmls){
			points = *pts;
			faces = *fcs;
			normals = *nrmls;
			beginFace = 0;
			beginPoint = 0;
	}


	void setUpEdges(){
		int point1, point2, face1, face2, middlePoint;
		for(int i = beginPoint; i < points.size() -1; i++)
			for(int j = i + 1; j < points.size(); j++){
				int counter = 0;
				for(int k = 0; k < faces.size(); k++){
					point1 = i;
					point2 = j;
					int yesi = 0, yesj = 0;
					for(int m = 0; m < 4; m++){
						if(faces[k].faces[m] == i)
							yesi = 1;
						if(faces[k].faces[m] == j)
							yesj = 1;
					}
					if((yesi == 1) && (yesj == 1)){
						if(counter == 0){
							face1 = k;
							counter++;
						}
						else{
							face2 = k;
							edges.push_back(edge(point1, point2, 0, face1, face2));
						}
					}
				}
			}
		setUpTheMidpoints();
	}

	void setUpTheMidpoints(){
		for(int i = 0; i < edges.size(); i++){
			float avgx, avgy, avgz;
			avgx = points[edges[i].point1].x + points[edges[i].point2].x + facePoints[edges[i].face1].x + facePoints[edges[i].face2].x;
			avgy = points[edges[i].point1].y + points[edges[i].point2].y + facePoints[edges[i].face1].y + facePoints[edges[i].face2].y;
			avgz = points[edges[i].point1].z + points[edges[i].point2].z + facePoints[edges[i].face1].z + facePoints[edges[i].face2].z;
			edgePoints.push_back(coordinate(avgx/4, avgy/4, avgz/4));
			edges[i].middlePoint = edgePoints.size() - 1;
		}
	}

	void makeNewFaces(){
		int tempBeginPoint = points.size();
		int tempBeginFace = faces.size();
		for(int i = beginPoint; i < tempBeginPoint; i++){
			//printf("i=%d\n", i);
			std::vector<edge> pointEdges;
			//get the edges that have that point
			for(int j = 0; j < edges.size(); j++)
				if((edges[j].point1 == i) || (edges[j].point2 == i))
					pointEdges.push_back(edges[j]);
			//compute q and r
			float qx = 0, qy = 0, qz = 0, rx = 0, ry = 0, rz = 0;
			for(int j = 0; j < pointEdges.size(); j++){
				qx += facePoints[pointEdges[j].face1].x + facePoints[pointEdges[j].face2].x;
				qy += facePoints[pointEdges[j].face1].y + facePoints[pointEdges[j].face2].y;
				qz += facePoints[pointEdges[j].face1].z + facePoints[pointEdges[j].face2].z;
				rx += edgePoints[pointEdges[j].middlePoint].x;
				ry += edgePoints[pointEdges[j].middlePoint].y;
				rz += edgePoints[pointEdges[j].middlePoint].z;
			}
			qx /= pointEdges.size() * 2;
			qy /= pointEdges.size() * 2;
			qz /= pointEdges.size() * 2;
			rx /= pointEdges.size();
			ry /= pointEdges.size();
			rz /= pointEdges.size();
			//we got our new vertex
			float newx = qx/pointEdges.size() + 2*rx/pointEdges.size() + points[i].x * (pointEdges.size() -3)/pointEdges.size();
			float newy = qy/pointEdges.size() + 2*ry/pointEdges.size() + points[i].y * (pointEdges.size() -3)/pointEdges.size();
			float newz = qz/pointEdges.size() + 2*rz/pointEdges.size() + points[i].z * (pointEdges.size() -3)/pointEdges.size();
			points.push_back(coordinate(newx, newy, newz));
			//compute the faces associated with that point
			int tempFaceSize = faces.size(), newPointIndex = points.size() - 1;
			for(int j = 0; j < pointEdges.size() -1; j++){
				int ok = 0;
				for(int k = j + 1; ((k < pointEdges.size())); k++){
					//printf("size=%d j=%d k=%d\n", pointEdges.size(), j, k);
					int tempFaceNumber;
					if((pointEdges[j].face1 == pointEdges[k].face1) || (pointEdges[j].face1 == pointEdges[k].face2)){
						ok = 1;
						tempFaceNumber = pointEdges[j].face1;
						//see if these points already exist, otherwise add them
						int edgePoint1 = pointExists(edgePoints[pointEdges[j].middlePoint].x, edgePoints[pointEdges[j].middlePoint].y, edgePoints[pointEdges[j].middlePoint].z);
						int edgePoint2 = pointExists(edgePoints[pointEdges[k].middlePoint].x, edgePoints[pointEdges[k].middlePoint].y, edgePoints[pointEdges[k].middlePoint].z);
						int facePoint = pointExists(facePoints[tempFaceNumber].x, facePoints[tempFaceNumber].y, facePoints[tempFaceNumber].z);
						if(edgePoint1 == -1){
							edgePoint1 = points.size();
							points.push_back(coordinate(edgePoints[pointEdges[j].middlePoint].x, edgePoints[pointEdges[j].middlePoint].y, edgePoints[pointEdges[j].middlePoint].z));
						}
						if(edgePoint2 == -1){
							edgePoint2 = points.size();
							points.push_back(coordinate(edgePoints[pointEdges[k].middlePoint].x, edgePoints[pointEdges[k].middlePoint].y, edgePoints[pointEdges[k].middlePoint].z));
						}
						if(facePoint == -1){
							facePoint = points.size();
							points.push_back(coordinate(facePoints[tempFaceNumber].x, facePoints[tempFaceNumber].y, facePoints[tempFaceNumber].z));
						}
						//add the new face
						faces.push_back(face(tempFaceSize, newPointIndex, edgePoint1, facePoint, edgePoint2));
					}
					if((pointEdges[j].face2 == pointEdges[k].face1) || (pointEdges[j].face2 == pointEdges[k].face2)){
						ok = 1;
						tempFaceNumber = pointEdges[j].face2;
						//see if these points already exist, otherwise add them
						int edgePoint1 = pointExists(edgePoints[pointEdges[j].middlePoint].x, edgePoints[pointEdges[j].middlePoint].y, edgePoints[pointEdges[j].middlePoint].z);
						int edgePoint2 = pointExists(edgePoints[pointEdges[k].middlePoint].x, edgePoints[pointEdges[k].middlePoint].y, edgePoints[pointEdges[k].middlePoint].z);
						int facePoint = pointExists(facePoints[tempFaceNumber].x, facePoints[tempFaceNumber].y, facePoints[tempFaceNumber].z);
						if(edgePoint1 == -1){
							edgePoint1 = points.size();
							points.push_back(coordinate(edgePoints[pointEdges[j].middlePoint].x, edgePoints[pointEdges[j].middlePoint].y, edgePoints[pointEdges[j].middlePoint].z));
						}
						if(edgePoint2 == -1){
							edgePoint2 = points.size();
							points.push_back(coordinate(edgePoints[pointEdges[k].middlePoint].x, edgePoints[pointEdges[k].middlePoint].y, edgePoints[pointEdges[k].middlePoint].z));
						}
						if(facePoint == -1){
							facePoint = points.size();
							points.push_back(coordinate(facePoints[tempFaceNumber].x, facePoints[tempFaceNumber].y, facePoints[tempFaceNumber].z));
						}
						//add the new face
						faces.push_back(face(tempFaceSize, newPointIndex, edgePoint1, facePoint, edgePoint2));
					}
				}
			}
			//printf("For loop in j completed successfully\n");
		}
		//printf("For loop in i completed successfully\n");
		//at this point we will be dealing with new points and faces, don't need the old ones anymore
		beginPoint = tempBeginPoint;
		beginFace = tempBeginFace;
	}

	int pointExists(float x, float y, float z){
		for(int i = 0; i < points.size(); i++)
			if((points[i].x == x) && (points[i].y == y) && (points[i].z) == z)
				return i;
		return -1;
	}

	void draw(){
			float difamb[] = {1.0,0.5,0.3,1.0};
			for(int i = beginFace; i < faces.size(); i++){
				glBegin(GL_QUADS);
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, difamb);
					//glNormal3f(normals[faces[i].faceNumber].x, normals[faces[i].faceNumber].y,normals[faces[i].faceNumber].z);
					glVertex3f(points[faces[i].faces[0]].x,points[faces[i].faces[0]].y,points[faces[i].faces[0]].z);
					glVertex3f(points[faces[i].faces[1]].x,points[faces[i].faces[1]].y,points[faces[i].faces[1]].z);
					glVertex3f(points[faces[i].faces[2]].x,points[faces[i].faces[2]].y,points[faces[i].faces[2]].z);
					glVertex3f(points[faces[i].faces[3]].x,points[faces[i].faces[3]].y,points[faces[i].faces[3]].z);
				glEnd();
			}
	}
};


std::vector<coordinate> tempPoints;
std::vector<face> tempFaces;
std::vector<coordinate> tempNormals;

//std::vector<coordinate> edgePoints;
std::vector<pair> edges;
Mesh mesh;


void addFacePoints(){
	for(int i = mesh.beginFace; i < mesh.faces.size(); i++){
		float avgx, avgy, avgz;
		avgx = mesh.points[mesh.faces[i].faces[0]].x + mesh.points[mesh.faces[i].faces[1]].x + mesh.points[mesh.faces[i].faces[2]].x+ mesh.points[mesh.faces[i].faces[3]].x;
		avgy = mesh.points[mesh.faces[i].faces[0]].y + mesh.points[mesh.faces[i].faces[1]].y + mesh.points[mesh.faces[i].faces[2]].y+ mesh.points[mesh.faces[i].faces[3]].y;
		avgz = mesh.points[mesh.faces[i].faces[0]].z + mesh.points[mesh.faces[i].faces[1]].z + mesh.points[mesh.faces[i].faces[2]].z+ mesh.points[mesh.faces[i].faces[3]].z;
		avgx /= 4;
		avgy /= 4;
		avgz /= 4;
		facePoints.push_back(coordinate(avgx, avgy, avgz));
	}
}


void subdivision(){
	int tempSizeOfPoints = mesh.points.size();
	int tempSizeOfFaces = mesh.faces.size();
	int tempSizeOfNormals = mesh.normals.size();
	addFacePoints();
	printf("Face points created\n");
	mesh.setUpEdges();
	printf("Edges created successfully\n");
	mesh.makeNewFaces();
	printf("Faces created successfully\n");
}

void init(void)
{
	// init your data, setup OpenGL environment here
	glClearColor( 0.0, 0.0, 0.0, 1.0 );
	
	glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45,640.0/480.0,1.0,500.0);
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
//      glEnable(GL_COLOR_MATERIAL);
        float dif[] = {1.0,1.0,1.0,1.0};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
        float amb[] = {0.2,0.2,0.2,1.0};
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb);


	//set up the points

	//the points on the z=0 plane
	tempPoints.push_back(coordinate(-.6, 1, 0));
	tempPoints.push_back(coordinate(-.6,.6,0));
	tempPoints.push_back(coordinate(-.2,.6 ,0));
	tempPoints.push_back(coordinate(-.2, 1 ,0));
	tempPoints.push_back(coordinate(.2,.6 ,0));
	tempPoints.push_back(coordinate(.2,1 ,0));
	tempPoints.push_back(coordinate(.6,.6 ,0));
	tempPoints.push_back(coordinate(.6, 1 ,0));//
	tempPoints.push_back(coordinate(-.2,-.6 ,0));
	tempPoints.push_back(coordinate(.2,-.6 ,0));
	tempPoints.push_back(coordinate(-.6,-.6 ,0));
	tempPoints.push_back(coordinate(-.6, -1 ,0));
	tempPoints.push_back(coordinate(-.2,-1 ,0));
	tempPoints.push_back(coordinate(.2,-1 ,0));
	tempPoints.push_back(coordinate(.6,-1 ,0));
	tempPoints.push_back(coordinate(.6,-.6 ,0));
	//the points on the z=1 plane
	tempPoints.push_back(coordinate(-.6, 1, 1));
	tempPoints.push_back(coordinate(-.6,.6,1));
	tempPoints.push_back(coordinate(-.2,.6 ,1));
	tempPoints.push_back(coordinate(-.2, 1 ,1));
	tempPoints.push_back(coordinate(.2,.6 ,1));
	tempPoints.push_back(coordinate(.2,1 ,1));
	tempPoints.push_back(coordinate(.6,.6 ,1));
	tempPoints.push_back(coordinate(.6, 1 ,1));//
	tempPoints.push_back(coordinate(-.2,-.6 ,1));
	tempPoints.push_back(coordinate(.2,-.6 ,1));
	tempPoints.push_back(coordinate(-.6,-.6 ,1));
	tempPoints.push_back(coordinate(-.6, -1 ,1));
	tempPoints.push_back(coordinate(-.2,-1 ,1));
	tempPoints.push_back(coordinate(.2,-1 ,1));
	tempPoints.push_back(coordinate(.6,-1 ,1));
	tempPoints.push_back(coordinate(.6,-.6 ,1));

	//set up the faces

	tempFaces.push_back(face(0,0, 1, 2, 3));
	tempFaces.push_back(face(1,3, 2, 4, 5));
	tempFaces.push_back(face(2,5, 4, 6, 7));
	tempFaces.push_back(face(3,2, 8, 9, 4));
	tempFaces.push_back(face(4,10, 11, 12, 8));
	tempFaces.push_back(face(5,8, 12, 13, 9));
	tempFaces.push_back(face(6,9, 13, 14, 15));

	tempFaces.push_back(face(7,16, 17, 18, 19));
	tempFaces.push_back(face(8,19, 18, 20, 21));
	tempFaces.push_back(face(9,21, 20, 22, 23));
	tempFaces.push_back(face(10,18, 24, 25, 20));
	tempFaces.push_back(face(11,26, 27, 28,24));
	tempFaces.push_back(face(12,24, 28, 29, 25));
	tempFaces.push_back(face(13,25, 29, 30, 31));

	tempFaces.push_back(face(14,3, 0, 16, 19));
	tempFaces.push_back(face(15,5, 3, 19, 21));
	tempFaces.push_back(face(16,7, 5, 21, 23));
	tempFaces.push_back(face(17,0, 1, 17, 16));
	tempFaces.push_back(face(18,7, 6, 22, 23));
	tempFaces.push_back(face(19,1, 2, 18, 17));
	tempFaces.push_back(face(20,4, 6, 22, 20));

	tempFaces.push_back(face(21,11, 12, 28, 27));
	tempFaces.push_back(face(22,12, 13, 29, 28));
	tempFaces.push_back(face(23,13, 14, 30, 29));
	tempFaces.push_back(face(24,10, 11, 27, 26));
	tempFaces.push_back(face(25,14, 15, 31, 30));
	tempFaces.push_back(face(26,8,10, 26, 24));
	tempFaces.push_back(face(27,9, 15, 31, 25));

	tempFaces.push_back(face(28,2, 8, 24, 18));
	tempFaces.push_back(face(29,4, 9, 25, 20));

	//set up the normals

	tempNormals.push_back(coordinate(0, 0, -1));
	tempNormals.push_back(coordinate(0, 0, -1));
	tempNormals.push_back(coordinate(0, 0, -1));
	tempNormals.push_back(coordinate(0, 0, -1));
	tempNormals.push_back(coordinate(0, 0, -1));
	tempNormals.push_back(coordinate(0, 0, -1));
	tempNormals.push_back(coordinate(0, 0, -1));

	tempNormals.push_back(coordinate(0, 0, 1));
	tempNormals.push_back(coordinate(0, 0, 1));
	tempNormals.push_back(coordinate(0, 0, 1));
	tempNormals.push_back(coordinate(0, 0, 1));
	tempNormals.push_back(coordinate(0, 0, 1));
	tempNormals.push_back(coordinate(0, 0, 1));
	tempNormals.push_back(coordinate(0, 0, 1));

	tempNormals.push_back(coordinate(0, 1, 0));
	tempNormals.push_back(coordinate(0, 1, 0));
	tempNormals.push_back(coordinate(0, 1, 0));
	tempNormals.push_back(coordinate(-1, 0, 0));
	tempNormals.push_back(coordinate(1, 0, 0));
	tempNormals.push_back(coordinate(0,-1, 0));
	tempNormals.push_back(coordinate(0,-1, 0));

	tempNormals.push_back(coordinate(0,-1, 0));
	tempNormals.push_back(coordinate(0,-1, 0));
	tempNormals.push_back(coordinate(0,-1, 0));
	tempNormals.push_back(coordinate(-1, 0, 0));
	tempNormals.push_back(coordinate(1, 0, 0));
	tempNormals.push_back(coordinate(0,1, 0));
	tempNormals.push_back(coordinate(0,1, 0));

	tempNormals.push_back(coordinate(-1, 0, 0));
	tempNormals.push_back(coordinate(1, 0, 0));

	mesh.set_values(&tempPoints, &tempFaces, &tempNormals);	
}

void display(void)
{
	
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity ();             /* clear the matrix */	
	float pos[] = {-2.0,2.0,-3.0,1.0};
        glLightfv(GL_LIGHT0, GL_POSITION, pos);
        //gluLookAt(0, 0, 4, 0, 0, 0, 0, 1, 0);
	gluLookAt(camerax, cameray, cameraz, 0, 0, 0, 0, 1, 0);
	
	mesh.draw();

	glutSwapBuffers();
	glFlush ();

	glutPostRedisplay();	
}



void keyboard(unsigned char key, int x, int y)
{
	//keyboard control goes here
	if (key == 27) 
	{
		// ESC hit, so quit
		printf("demonstration finished.\n");
		exit(0);
	}
	
	if((key == 83) || (key == 115)){
		// s or S was hit, create subdivision
		subdivision();				
	}	
	
	if((key == 119) || (key == 87)){	
		// w or W was hit, display only the outlined triangles
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	
	if((key == 70) || (key == 102)){
		// f or F was hit, display the filled triangles
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}	
}


void timer(int v)
{
	camerax = (1-t)*(1-t)*(1-t)*p0x + 3*(1-t)*(1-t)*t*p1x + 3*(t-1)*t*t*p2x + t*t*t*p3x;
	cameray = (1-t)*(1-t)*(1-t)*p0y + 3*(1-t)*(1-t)*t*p1y + 3*(t-1)*t*t*p2y + t*t*t*p3y;
	cameraz = (1-t)*(1-t)*(1-t)*p0z + 3*(1-t)*(1-t)*t*p1z + 3*(t-1)*t*t*p2x + t*t*t*p3z;
	if(timeron == 1)
		t += 0.01;
	if(t >= 1)
		t = 0;
	glutTimerFunc(1000/nFPS,timer,v); // restart timer again
}

void idle(){	
	glutPostRedisplay();
}

void movement(int key, int x, int y){	
	switch (key){		
		case GLUT_KEY_UP:
			camerax	+= 0.1;
			break;
		case GLUT_KEY_DOWN:
			camerax -= 0.1;
			break;
		case GLUT_KEY_RIGHT:
			cameray += 0.1;
			break;
		case GLUT_KEY_LEFT:
			cameray -= 0.1;
			break;
	}
}

int main(int argc, char* argv[])
{
	glutInit(&argc, (char**)argv);

	// set up for double-buffering & RGB color buffer
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB); 
	glutInitWindowSize (500, 500); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow ((const char*)"mp4");
	
	init(); // setting up user data & OpenGL environment
	
	// set up the call-back functions 
	glutDisplayFunc(display);  // called when drawing 	
	glutKeyboardFunc(keyboard); // called when received keyboard interaction	
	glutSpecialFunc(movement);
	glutIdleFunc(idle);
	glutTimerFunc(100,timer,nFPS); // a periodic timer. Usually used for updating animation
	
	glutMainLoop(); // start the main message-callback loop

	return 0;
}


