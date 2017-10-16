/*
 * triangle.c
 *
 *  Created on: Aug 28, 2017
 *      Author: Thumrongsak Kosiyatrakul
 */

#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "initShader.h"

#include "LabAssignment1.h"


#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))



vec4 vertices[100000];
vec4 shapes[100000];

int fragCounter = 0;
vec4* colors;
mat4 identity;
GLfloat scaleFactor = 1.0;
mat4 scaleTM;
int b_isDragging;
vec4 initDrag;
vec4 diffVectors[2];
int screenHeight = 512;
int screenWidth = 512;
b_canSpin = 0;
mat4 lastRotationMatrix;
mat4 currentTransform;
mat4 rotationMatrix;
mat4 appliedRotation;
int num_vertices = 0;
GLuint scale_loc;
GLuint rotate_loc;

float calcZ(float x, float y ) {
    float hypXY, zTheta, z;
    hypXY = sqrtf(x*x + y*y);
    zTheta = acosf(hypXY);
    z = sinf(zTheta);
    return z;
}

void getZRotationMatrix(float theta, mat4 *rotateMatrix) {
    vec4 v0, v1, v2, v3;
    defineVector(cos(theta), sin(theta), 0, 0, &v0);
    defineVector(-1*sin(theta), cos(theta), 0, 0, &v1);
    defineVector(0, 0, 1, 0, &v2);
    defineVector(0, 0, 0, 1, &v3);
    defineMatrix(v0, v1, v2, v3, rotateMatrix);
}

void getYRotationMatrix(float ax, float d, mat4 *rotateMatrix) {
    vec4 v0, v1, v2, v3;
    defineVector(d, 0, ax, 0, &v0);
    defineVector(0, 1, 0, 0, &v1);
    defineVector(-1*ax, 0, d, 0,  &v2);
    defineVector(0, 0, 0, 1, &v3);
    defineMatrix(v0, v1, v2, v3, rotateMatrix);
}

void getXRotationMatrix(float ay, float az, float d, mat4 *rotateMatrix) {
    vec4 v0, v1, v2, v3;
    defineVector(1, 0, 0, 0, &v0);
    defineVector(0, az/d, ay/d, 0,  &v1);
    defineVector(0, -1*ay/d, az/d, 0, &v2);
    defineVector(0, 0, 0, 1, &v3);
    defineMatrix(v0, v1, v2, v3, rotateMatrix);
}

void calculateRotationMatrix(vec4 *init, vec4 *currentVec) {
    printf("currentVec mag: %f\n", magnitude(currentVec));
    vec4 axisOfRotation, tempVec;
    mat4 temp, xRotateNeg, yRotateNeg, xRotate, yRotate, zRotate;
    float theta, xAngle, yAngle, zAngle;
    
    crossProduct( init, currentVec,  &axisOfRotation);
    
    if(axisOfRotation.x != axisOfRotation.x || magnitude(&axisOfRotation) < .00001) {
        // Is NaN
        printf("can't spin\n\n");
        b_canSpin = 0;
        return;
    }
    b_canSpin = 1;
    float axisMag = magnitude(&axisOfRotation);
    theta = dotProduct(currentVec, init);
    theta = acosf(theta/(magnitude(currentVec)*magnitude(init)));
    
    axisOfRotation.x /= axisMag;
    axisOfRotation.y /= axisMag;
    axisOfRotation.z /= axisMag;
    
    printf("crossProduct: %f \n", magnitude(&axisOfRotation));
    
//    printVector(&axisOfRotation);
    float d = sqrtf(axisOfRotation.y*axisOfRotation.y + axisOfRotation.z*axisOfRotation.z);
    
    xAngle = asinf(axisOfRotation.y/d);
    getXRotationMatrix(axisOfRotation.y, axisOfRotation.z, d, &xRotate);
//    printMatrix(&xRotate);
    transposeMatrix(&xRotate, &xRotateNeg);
//    printMatrix(&xRotateNeg);
    yAngle = acosf(d/1.0);
    getYRotationMatrix(axisOfRotation.x, d, &yRotate);
    transposeMatrix(&yRotate, &yRotateNeg);
    
    getZRotationMatrix(-1*theta, &zRotate);
    
    
    multiplyMatrices( &xRotateNeg,&yRotateNeg, &temp);
    multiplyMatrices(&temp,&zRotate, &xRotateNeg);
    
    //xRotateNeg serves as temp variable
    multiplyMatrices(&xRotateNeg, &yRotate,  &temp);
    multiplyMatrices(&temp, &xRotate,  &rotationMatrix);
    multiplyMatrices(&rotationMatrix, &identity, &lastRotationMatrix);

}

void init(void)
{
    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);
    GLuint vao;
    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*fragCounter + sizeof(vec4)*fragCounter, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4)*fragCounter, shapes);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*fragCounter, sizeof(vec4)*fragCounter, colors);
    
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    
    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (sizeof(vec4)*fragCounter));
    
    
    scale_loc = glGetUniformLocation(program, "scale");
    rotate_loc = glGetUniformLocation(program, "rotate");
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1,0);
    
}

void readPlyFile()  {
    char filename[10] = "cow1.ply";
    double i;
    char line[256];
    FILE *in;
    
    in = fopen(filename, "r+");
    i = fscanf(in, "%s\n", line);
    int state = 0;
    while(i != EOF) {
        if(state == 0 ) {
            i = fscanf(in, "%s\n", line);
            if(strcmp(line, "end_header") == 0) {
                state++;
                i = fscanf(in, "%f %f %f\n", &vertices[num_vertices].x, &vertices[num_vertices].y, &vertices[num_vertices].z);
                num_vertices++;
            }
        } else if(state == 1) {
            i = fscanf(in, "%f %f %f\n", &vertices[num_vertices].x, &vertices[num_vertices].y, &vertices[num_vertices].z);
            vertices[num_vertices].w = 1.0;
            num_vertices++;
            if(num_vertices==2903) {
                state++;
            }
        } else if(state == 2) {
            int shapeNum, i1, i2, i3;
            i = fscanf(in, "%d %d %d %d\n", &shapeNum,&i1, &i2, &i3);
            shapes[fragCounter++] = vertices[i1];
            shapes[fragCounter++] = vertices[i2];
            shapes[fragCounter++] = vertices[i3];
        }
    }
    
    printf("\n%d", num_vertices);
    fclose(in);
    printf("\n\n%s\n", filename);
    
}

void initColors() {
    colors = malloc(sizeof(vec4) * fragCounter);
    for(int i = 0; i < fragCounter; i++) {
        colors[i].x = (float)rand()/(float)(RAND_MAX);
        colors[i].y = (float)rand()/(float)(RAND_MAX);
        colors[i].z = (float)rand()/(float)(RAND_MAX);
        colors[i].w = 1.0;
    }
    printf("\n%f\n", colors[fragCounter-1].z);

}

void display(void)
{
    mat4 appliedRotationTemp;
    multiplyMatrices( &rotationMatrix, &currentTransform, &appliedRotation);
//    multiplyMatrices( &appliedRotation, &identity, &currentTransform);
//    multiplyMatrices( &identity, &identity, &rotationMatrix);
    printMatrix(&appliedRotation);
    glClearColor(0.0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniformMatrix4fv(scale_loc, 1, GL_TRUE, &scaleTM);
    glUniformMatrix4fv(rotate_loc, 1, GL_TRUE, &appliedRotation);
    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, fragCounter);
    glutSwapBuffers();
    
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    // This is where I'm doing scaling
    if(key == 'u') {
        // scale up / out
        scaleFactor -= .02;
        printf("\nScrollin' up | %f\n\n", scaleFactor);
        
    } else if (key == 'j') {
        // scale down / in
        scaleFactor += .02;
        printf("\nScrollin' down | %f\n\n", scaleFactor);
    }
    scalarMultMatrix(&identity, scaleFactor, &scaleTM);
//    printMatrix(&scaleTM);
    
    if(key == 'q')
        exit(0);
    
    glutPostRedisplay();
}


void mouseFunction(int button, int state, int x, int y) {
    
    printf("\n button: %d\n", button);
     printf("\n state: %d\n", state);
    // I have a mac so this doesn't work
    if(button == 3) {
        // Scroll up
        printf("\nScrollin' up\n\n");
    } else if(button == 4) {
        // Scroll down
        printf("\nScrollin' down\n\n");
    }
    
    if(button == 0) { //left - click
        if(state == 0) {
            b_isDragging = 1;
            float xCVM = (x - ((float) screenWidth)/2.0f)/256.0f;
            float yCVM = -1 * (y - ((float) screenHeight)/2.0f)/256.0f;
            float zCVM = calcZ(xCVM, yCVM);
            vec4 temp;
            defineVector(xCVM, yCVM, zCVM, 1, &temp);
            
            multMatrixVector(&currentTransform, &temp, &initDrag);
            
            printf("magnitude: %f\n\n", magnitude(&initDrag));
            printVector(&initDrag);
            diffVectors[0] = initDrag;
            diffVectors[1] = initDrag;
            diffVectors[1].x = -500; // flag to show initialization
        } else {
            b_isDragging = 0;
            mat4 temp;
//            printMatrix(&currentTransform);
            multiplyMatrices(&currentTransform, &identity, &temp);
            printf("\n\nTemp:\n ");
//          printMatrix(&temp);
//            printMatrix(&lastRotationMatrix);
            multiplyMatrices(&rotationMatrix, &temp,  &currentTransform);
            multiplyMatrices(&identity, &identity, &rotationMatrix);
            if(b_canSpin) {
                calculateRotationMatrix(&diffVectors[0], &diffVectors[1]);
                multiplyMatrices(&identity, &identity, &rotationMatrix);
            }
//            printMatrix(&currentTransform);
            
        }
    }
}

//This calls for the x and y to be scaled in accordance with the screen around the origin.
float calcAngles(float x, float y) {
    return 0.0;
}



void dragFunction(int x, int y) {
    vec4 currentPos;
    if(b_isDragging) {
        if(diffVectors[1].x != -500) {
            diffVectors[0] = diffVectors[1];
        }
        float xCVM = (x - (float) screenWidth/2.0)/256.0;
        float yCVM = -1 * (y - (float) screenHeight/2.0)/256.0;
        float zCVM = calcZ(xCVM, yCVM);
        vec4 temp;
        defineVector(xCVM, yCVM, zCVM, 1, &temp);
        multMatrixVector(&currentTransform, &temp, &currentPos);
        diffVectors[1] = currentPos;
        calculateRotationMatrix(&initDrag, &diffVectors[1]);
//        multiplyMatrices(&currentTransform, &identity, &temp);
//        printf("\n\nTemp:\n ");
//        //          printMatrix(&temp);
//        //            printMatrix(&lastRotationMatrix);
//        multiplyMatrices(&rotationMatrix, &temp,  &currentTransform);
//        multiplyMatrices(&identity, &identity, &rotationMatrix);
//        printMatrix(&rotationMatrix);
//        printf("we draggin\n: %d, %d\n", x, y);
    }
    glutPostRedisplay();
}

void idle() {
    if(b_isDragging != 1 && b_canSpin) {
        mat4 temp;
        //            printMatrix(&currentTransform);
        multiplyMatrices(&currentTransform, &identity, &temp);
        printf("\n\nidler:\n ");
        //          printMatrix(&temp);
        //            printMatrix(&lastRotationMatrix);
        
        multiplyMatrices(&lastRotationMatrix, &temp,  &currentTransform);
        multiplyMatrices(&identity, &identity, &rotationMatrix);
        glutPostRedisplay();
    }
    
    
}

int main(int argc, char **argv)
{
    
    vec4 v0, v1, v2, v3;
    
    defineVector(1, 0, 0, 0, &v0);
    defineVector(0, 1, 0, 0, &v1);
    defineVector(0, 0, 1, 0, &v2);
    defineVector(0, 0, 0, 1, &v3);
    
    defineMatrix(v0, v1, v2, v3, &identity);
    defineMatrix(v0, v1, v2, v3, &scaleTM);
    defineMatrix(v0, v1, v2, v3, &rotationMatrix);
    defineMatrix(v0, v1, v2, v3, &currentTransform);
    defineMatrix(v0, v1, v2, v3, &appliedRotation);
    
    
    
    
    readPlyFile();
  
    srand(time(NULL));   // should only be called once
      initColors();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(100,100);
    glutCreateWindow("not triangle");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseFunction);
    glutMotionFunc(dragFunction);
    glutIdleFunc(idle);
    glutMainLoop();
    
    
    return 0;
}

