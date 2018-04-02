//
//  main.cpp
//  OpenGL_BaseCode
//
//  Created by 吴宇飞 on 2018/4/2.
//  Copyright © 2018年 gyd. All rights reserved.
//

#include "GLShaderManager.h"

#include "GLTools.h"

#include <glut/glut.h>
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"
#include "GLFrame.h"
#include "GLFrustum.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLBatch pyramidBatch;

GLShaderManager shaderManager;
GLFrame cameraFrame;
GLFrame objectFrame;
GLMatrixStack modelviewStack;
GLMatrixStack projectStack;
GLFrustum viewFrustum;
GLGeometryTransform transformPipline;

GLuint textureID;

//窗口大小改变时接受新的宽度和高度，其中0,0代表窗口中视口的左下角坐标，w，h代表像素

void ChangeSize(int w,int h)

{
    glViewport(0,0, w, h);
    
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0, 500);
    projectStack.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipline.SetMatrixStacks(modelviewStack, projectStack);
}

//为程序作一次性的设置

bool loadTGATexture(const char *szfileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pbits;
    int nwidth, nheight, ncomponents;
    GLenum eformat;
    
    //读取纹理像素
    pbits = gltReadTGABits(szfileName, &nwidth, &nheight, &ncomponents, &eformat);
    if (pbits == NULL) {
        return false;
    }
    
    //设置s,t的环绕方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    
    //设置放大缩小的过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    //精密包装像素数据
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    //载入纹理
    glTexImage2D(GL_TEXTURE_2D, 0, ncomponents, nwidth, nheight, 0, eformat, GL_UNSIGNED_BYTE, pbits);
    
    free(pbits);
    
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR ||
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        
        //加载Mip,纹理生成所有的Mip层
        //参数：GL_TEXTURE_1D、GL_TEXTURE_2D、GL_TEXTURE_3D
        glGenerateMipmap(GL_TEXTURE_2D);
    
    return true;
}



void makePyramid(GLBatch& pyramidBatch)
{
    //1.pyramidBatch
    //6 * 3 = 18
    //通过三角形批次类pyramidBatch 来组件图形
    //参数1:类型
    //参数2:顶点数 6 * 3 = 18
    //参数3:需要使用到纹理,默认为0,可以不写这个参数
    pyramidBatch.Begin(GL_TRIANGLES, 18,1);
    
    
    //2.金字塔底部
    //底部四边形 = 三角形X + 三角形Y
    //三角形X
    
    //----------vBlackLeft------------
    //设置法线坐标-光照
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    
    //设置顶点对应的纹理坐标
    // MultiTexCoord2f(GLuint texture, GLclampf s, GLclampf t);
    //参数1:纹理的层次
    //参数2:s坐标
    //参数3:t坐标
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    
    //设置顶点坐标
    pyramidBatch.Vertex3f(-1.0f, -1.0f, -1.0f);
    
    //错误地方!!顶点设置错误!修改成如下
    //------vBlackRight------------
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3f(1.0f, -1.0f, -1.0f);
    
    //vFrontRight点
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    pyramidBatch.Vertex3f(1.0f, -1.0f, 1.0f);
    
    
    //三角形Y
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    pyramidBatch.Vertex3f(-1.0f, -1.0f, 1.0f);
    
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3f(-1.0f, -1.0f, -1.0f);
    
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    pyramidBatch.Vertex3f(1.0f, -1.0f, 1.0f);
    
    //塔顶
    M3DVector3f vApex = {0.0f,1.0f,0.0f};
    M3DVector3f vFrontLeft = {-1.0f,-1.0f,1.0f};
    M3DVector3f vFrontRight = {1.0f,-1.0f,1.0f};
    M3DVector3f vBackLeft = {-1.0f,-1.0f,-1.0f};
    M3DVector3f vBackRight = {1.0f,-1.0f,-1.0f};
    
    //目的:为了临时存储法线向量
    M3DVector3f n;
    
    //金字塔的前面
    //三角形:(Apex,vFrontLeft,vFrontRight)
    
    //找法线
    //m3dFindNormal(M3DVector3f result, const M3DVector3f point1, const M3DVector3f point2,const M3DVector3f point3);
    //参数1:结果
    //参数2-4:3个顶点
    m3dFindNormal(n, vApex, vFrontLeft, vFrontRight);
    
    //vApex
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    //vFrontLeft
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontLeft);
    
    //vFrongRight
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontRight);
    
    
    //金字塔的左边
    //三角形:(Apex,vBackLeft,vFrontLeft)
    m3dFindNormal(n, vApex, vBackLeft, vFrontLeft);
    
    //vApex
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    //vBackLeft
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackLeft);
    
    //vFrontLeft
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontLeft);
    
    //金字塔右边
    //三角形：（vApex, vFrontRight, vBackRight）
    m3dFindNormal(n, vApex, vFrontRight, vBackRight);
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontRight);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackRight);
    
    //金字塔后边
    //三角形：（vApex, vBackRight, vBackLeft）
    m3dFindNormal(n, vApex, vBackRight, vBackLeft);
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackRight);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackLeft);
    
    //结束批次设置
    pyramidBatch.End();
}

void SetupRC()
{
    glClearColor(0.7, 0.7, 0.7, 1);
    glEnable(GL_DEPTH_TEST);
    shaderManager.InitializeStockShaders();
    
    //分配纹理对象
    glGenTextures(1, &textureID);
    
    //绑定纹理
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    //加载TGA文件
    loadTGATexture("brick.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    
    //设置渲染图像的顶点
    makePyramid(pyramidBatch);
    cameraFrame.MoveForward(-15);
}

//开始渲染

void RenderScene(void)
{
    GLfloat vlightPos[] = {1.0f, 1.0f, 0.0f};
    GLfloat vlinghtColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    modelviewStack.PushMatrix();
    
    M3DMatrix44f cameraF;
    cameraFrame.GetCameraMatrix(cameraF);
    modelviewStack.MultMatrix(cameraF);
    
    M3DMatrix44f proF;
    objectFrame.GetMatrix(proF);
    modelviewStack.MultMatrix(proF);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,transformPipline.GetModelViewMatrix(),transformPipline.GetProjectionMatrix(),vlightPos,vlinghtColor,0);
    
    pyramidBatch.Draw();
    modelviewStack.PopMatrix();
    glutSwapBuffers();
}

// 清理…例如删除纹理对象
void ShutdownRC(void)
{
    glDeleteTextures(1, &textureID);
    
}

void specialKeyClick(int key, int x, int y)
{
    if (key == GLUT_KEY_UP) {
        objectFrame.RotateWorld(m3dDegToRad(-5), 1.0f, 0, 0);
    }
    
    if (key == GLUT_KEY_DOWN) {
        objectFrame.RotateWorld(m3dDegToRad(5), 1.0f, 0, 0);
    }
    
    if (key == GLUT_KEY_LEFT) {
        objectFrame.RotateWorld(m3dDegToRad(-5), 0, 1.0f, 0);
    }
    
    if (key == GLUT_KEY_RIGHT) {
        objectFrame.RotateWorld(m3dDegToRad(5), 0, 1.0f, 0);
    }
    
    glutPostRedisplay();
}

int main(int argc,char* argv[])

{
    //设置当前工作目录，针对MAC OS X
    
    gltSetWorkingDirectory(argv[0]);
    
    //初始化GLUT库
    
    glutInit(&argc, argv);
    
    /*初始化双缓冲窗口，其中标志GLUT_DOUBLE、GLUT_RGBA、GLUT_DEPTH、GLUT_STENCIL分别指
     
     双缓冲窗口、RGBA颜色模式、深度测试、模板缓冲区*/
    
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
    
    //GLUT窗口大小，标题窗口
    
    glutInitWindowSize(800,600);
    
    glutCreateWindow("Triangle");
    
    //注册回调函数
    
    glutReshapeFunc(ChangeSize);
    
    glutDisplayFunc(RenderScene);
    
    glutSpecialFunc(specialKeyClick);
    
    //驱动程序的初始化中没有出现任何问题。
    
    GLenum err = glewInit();
    
    
    
    if(GLEW_OK != err) {
        
        fprintf(stderr,"glew error:%s\n",glewGetErrorString(err));
        
        return 1;
        
    }
    
    //调用SetupRC
    
    SetupRC();
    
    glutMainLoop();
    
    ShutdownRC();
    
    return 0;
    
}



