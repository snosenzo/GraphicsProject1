//
//  scraps.c
//  Project1
//
//  Created by Sam Nosenzo on 10/12/17.
//  Copyright © 2017 Sam Nosenzo. All rights reserved.
//


void init(void)
{
    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);
    
    const float DegreesToRadians = M_PI / 180.0; // M_PI = 3.1415...
    
    
    
    int k = 0;
    for(float phi = -80.0; phi < 80.0; phi += 20.0) {
        
        float phir = phi * DegreesToRadians;
        float phir20 = (phi + 20.0)*DegreesToRadians;
        
        for(float theta = -180.0; theta <= 180.0; theta+=20.0) {
            float thetar = theta*DegreesToRadians;
            quad_data[k].x = sinf(thetar) * cosf(phir);
            quad_data[k].y = cosf(thetar)*cosf(phir);
            quad_data[k].z = sinf(phir);
            quad_data[k].w = 1.0;
            k++;
            quad_data[k].x = sin(thetar)*cos(phir20);
            quad_data[k].y = cos(thetar)*cos(phir20);
            quad_data[k].z = sin(phir20);
            quad_data[k].w = 1.0;
            k++;
            
        }
    }
    k = 0;
    
    strip_data[k].x = 0.0;
    strip_data[k].y = 0.0;
    strip_data[k].z = 1.0;
    strip_data[k].w = 1.0;
    k++;
    
    float sin80 = sinf(80.0 * DegreesToRadians);
    float cos80 = cosf(80.0 * DegreesToRadians);
    
    for(float theta = -180.0; theta <= 180.0; theta += 20.0) {
        float thetar = theta * DegreesToRadians;
        strip_data[k].x = sinf(thetar) * cos80;
        strip_data[k].y = cosf(thetar) * cos80;
        strip_data[k].z = sin80;
        strip_data[k].w = 1.0;
        k++;
    }
    
    strip_data[k].x = 0.0;
    strip_data[k].y = 0.0;
    strip_data[k].z = -1.0;
    strip_data[k].w = 1.0;
    k++;
    
    for(float theta = -180.0; theta <= 180.0; theta += 20.0) {
        float thetar = theta;
        strip_data[k].x = sinf(thetar) * cos80;
        strip_data[k].y = cosf(thetar) * cos80;
        strip_data[k].z = sin80;
        strip_data[k].w = 1.0;
        k++;
    }
    
    
    
    
    GLuint vao;
    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);
    
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 382 * 2 * sizeof(vec4), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 342*sizeof(vec4), quad_data);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*342, (sizeof(vec4)*40)/2, strip_data);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*342 + (sizeof(vec4)*40)/2, (sizeof(vec4)*40)/2, strip_data + (sizeof(vec4)*40)/2);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*382, sizeof(vec4)*382, colors);
    
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    
    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (sizeof(vec4)*382));
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1,0);
    
    
    scale_loc = glGetUniformLocation(program, "scale");
    rotate_loc = glGetUniformLocation(program, "rotate");
    
}
