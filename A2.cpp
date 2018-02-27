// to run g++ tut1.cpp -lGL -lGLU -lglfw
// ./a.out

// Older OpenGL

#include <GLFW/glfw3.h>	// Windows
#include <glm/glm.hpp>
#include <GL/gl.h>	     // OpenGL
#include <GL/glu.h>	  // utility library
#include <iostream>
#include <vector>
using namespace std;

GLFWwindow *window;       // pointer to GLFW window
int w, h;                 // width and height values
double mouseX, mouseY;    // hold the X and Y pos of the mouse
vector<glm::vec2> cps;    // Control Point array
vector<glm::vec2> bspline;// BSpline Point array
vector<float> weights;    // Control Point Weight array
vector<int> sks;          // Standard Knot Sequence
vector<float> U;          // Normalize Standard Knot Sequence
int selected = -1;        // hold the index of the point we've clicked. -1 means we dont have anything selected
float selectDistance = 0.03f; // size of the point so it's easier to click it
float pt_u = 0.0f;           // u parameter
float delta_u = 0.0001f;     // u step size
glm::vec2 u_coord;           // u coordinate

int k = 2;  // Order (degree + 1)
int m = 0;  // Number of control points

void updateKnotSeq() {
  vector<int> knots;
  // calculate knot seq info
  int numKnots = k + m;   // total number of knots
  int rKnots = numKnots - 2*k;  // remaining knots (standard knot has multiplicity at the knots)
  for (int i = 0; i < k; i++) {
    knots.push_back(0);
  }
  for (int i = 0; i < rKnots; i++) {
    knots.push_back(i+1);
  }
  for (int i = 0; i < k; i++) {
    if (rKnots + 1 < 1) {
      knots.push_back(1);
    } else {
      knots.push_back(rKnots+1);
    }
  }
  sks = knots;
}

void normalizeKnotSeq() {
  vector<float> knots;
  float uniStepSize = m - k + 1;
  for (int i = 0; i < sks.size(); i++) {
    knots.push_back(sks[i]/uniStepSize);
  }

  U = knots;
}

int delta(float u, int k, int m) {
  for (int i = 0; i < m+k-2; i++) {
    if (u >= U[i] && u < U[i+1]) {
      return i;
    }
  }
  return -1;
}

glm::vec2 E_delta_1(float u, int k, int m) {
  vector<glm::vec2> c;
  vector<float> w;
  float sumWeight = 0.0;
  int d = delta(u, k, m);
  for (int i = 0; i <= k-1; i++) {
    c.push_back(cps[d-i] * weights[d-i]);
    w.push_back(weights[d-i]);
  }
  for (int r = k; r >= 2; r--) {
    int i = d;
    for (int s = 0; s <= r-2; s++) {
      float omega = (u-U[i])/(U[i+r-1]-U[i]);
      w[s] = (omega * w[s] + (1-omega) * w[s+1]);
      c[s] = (omega * c[s] + (1-omega) * c[s+1]);
      i = i-1;
    }
  }
  return c[0]/w[0];
}

void renderBSpline() {
  vector<glm::vec2> curvepoints;
  float u = 0.0f;
  while (u < 1.0f) {
    glm::vec2 p = E_delta_1(u, k, m);
    curvepoints.push_back(p);
    u += delta_u;
  }
  bspline = curvepoints;
}

void updateBSpline() {
  m = cps.size();
  updateKnotSeq();
  normalizeKnotSeq();
  renderBSpline();
}

/*************************************
************** RENDER ****************
*************************************/
void render() {
  // OpenGL calls go here
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glMatrixMode(GL_PROJECTION);  // scene - how it gets projected onto the screen
  glLoadIdentity();
  glOrtho(-1, 1, -1, 1, -1, 1); // view volume is a cube, which goes from -1 to 1 in all directions

  if (m >= k) {
    renderBSpline();
    u_coord = E_delta_1(pt_u, k, m);
  }

  glPointSize(7);
  glBegin(GL_POINTS); // GL_POINTS, GL_QUADS, GL_LINES
  for(int i = 0; i < cps.size(); i++) {
    glColor3f(1.0f, 1.0f, 1.0f);
    if (i == selected) {
      glColor3f(1.0f, 0.0f, 0.0f);
    }
    glVertex2f(cps[i].x, cps[i].y);
  }
  glEnd();

  glBegin(GL_LINE_STRIP);
  glColor3f(1.0f, 1.0f, 1.0f);
  for (int i = 0; i < bspline.size(); i++) {
    glVertex2f(bspline[i].x, bspline[i].y);
  }
  glEnd();

  if (m >= k) {
    glPointSize(10);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex2f(u_coord.x, u_coord.y);
    glEnd();
  }

}


/**********************************
********* USER INPUT CODE *********
**********************************/
void keyboard(GLFWwindow *sender, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS | GLFW_REPEAT) {
    if (key == GLFW_KEY_ESCAPE) {
      glfwSetWindowShouldClose(window, 1);
    }
    if (key == GLFW_KEY_W) {
      if (k >= m) {
        k = m;
      } else {
        k += 1;
      }
      cout << "k: " << k << endl;
      updateBSpline();
    }
    if (key == GLFW_KEY_S) {
      if (k-1 < 2) {
        k = 2;
      } else {
        k-=1;
      }
      cout << "k: " << k << endl;
      updateBSpline();
    }
    if (key == GLFW_KEY_D) {
      if (pt_u+0.005f >= 0.9999f) {
        pt_u = 0.9999f;
      } else {
        pt_u += 0.005f;
      }
      printf("u: %.3f\n", pt_u);
    }
    if (key == GLFW_KEY_A) {
      if (pt_u-0.005f < 0.0f) {
        pt_u = 0.0f;
      } else {
        pt_u -= 0.005f;
      }
      printf("u: %.3f\n", pt_u);
    }
    if (key == GLFW_KEY_Q && selected != -1) {
      weights[selected] += 0.1f;
      printf("weights[%d] = %.1f\n", selected, weights[selected]);
    }
    if (key == GLFW_KEY_E && selected != -1) {
      if (weights[selected] - 0.1f < 0.0f) {
        weights[selected] = 0.0f;
      } else {
        weights[selected] -= 0.1f;
      }
      printf("weights[%d] = %.1f\n", selected, weights[selected]);
    }
  }
}

void mouseClick(GLFWwindow *sender, int button, int action, int mods) {
  if (action == GLFW_PRESS) {
    for(int i = 0; i < cps.size(); i++) {
      if (abs(cps[i].x - mouseX) < selectDistance && abs(cps[i].y - mouseY) < selectDistance) {
        selected = i;
      }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && selected == -1) {
      cps.push_back(glm::vec2(mouseX, mouseY));
      weights.push_back(1.0f);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && selected != -1) {
      cps.erase(cps.begin() + selected);
      weights.erase(weights.begin() + selected);
      selected = -1;
    }
    m = cps.size();
    updateKnotSeq();
    normalizeKnotSeq();
  }
  if (action == GLFW_RELEASE) {
    selected = -1;
  }
}

// mX and mY are in screen coordinates mX [0,width], mY [0, height] convert to [-1,1]
void mousePos(GLFWwindow *sender, double mX, double mY) {
  mouseX = (2 * mX / w) - 1;
  mouseY = (-2 * mY / h) + 1; // want -1 at h, and 1 at 0.

  if (selected != -1) {
      cps[selected] = glm::vec2(mouseX, mouseY);
  }
}
/********************************
************ MAIN ***************
********************************/
int main() {
  if(!glfwInit()) {
    return 1;
  }

  window = glfwCreateWindow(640, 480, "Assignment 2 - CPSC587", NULL, NULL);  // width, height, window title
  if(!window) { // check if window was created properly
    return 1;
  }

  glfwMakeContextCurrent(window); // tell OpenGL what window we're working with
  glfwSetKeyCallback(window, keyboard);
  glfwSetMouseButtonCallback(window, mouseClick);
  glfwSetCursorPosCallback(window, mousePos);

  while(!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &w, &h); // get attributes of the window size and store into w and h
    glViewport(0, 0, w, h); // pixel coordinate of top left --> tell where to draw
    render();

    glfwSwapBuffers(window);  // double buffer -> work on 1 buffer, display the other
    glfwPollEvents(); // detect input events --> mouse, keyboard. this detects when the mouse clicks the X in the top right
  }

  glfwDestroyWindow(window);  // destroy the window
  glfwTerminate();
  return 0;
}
