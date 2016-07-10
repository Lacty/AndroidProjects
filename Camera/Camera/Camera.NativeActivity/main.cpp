
#include "app_engine.h"
#include <cmath>
#include <Eigen/Core>


// degrees -> radians
float toRadians(float degrees) {
  return degrees * M_PI / 180.0f;
}

// radians -> degrees
float toDegrees(float radians) {
  return radians * 180.0f / M_PI;
}

float calcFovy(const float fov, const float aspect, const float near_z) {
  if (aspect >= 1.0f) return fov;

  // ‰æ–Ê‚ªc’·‚É‚È‚Á‚½‚çA•Šî€‚Åfov‚ğ‹‚ß‚é
  // fov‚Ænear_z‚©‚ç“Š‰e–Ê‚Ì•‚Ì”¼•ª‚ğ‹‚ß‚é
  float half_w = std::tan(toRadians(fov / 2.0f)) * near_z;

  // •\¦‰æ–Ê‚Ìc‰¡”ä‚©‚çA“Š‰e–Ê‚Ì‚‚³‚Ì”¼•ª‚ğ‹‚ß‚é
  float half_h = half_w / aspect;

  // “Š‰e–Ê‚Ì‚‚³‚Ì”¼•ª‚Ænear_z‚©‚çAfov‚ª‹‚Ü‚é
  return toDegrees(std::atan(half_h / near_z) * 2.0f);
}

Eigen::Matrix4f perspectiveView(float fovy,
                                float aspect,
                                float near_z,
                                float far_z) {
  Eigen::Matrix4f m;  // 4x4‚Ìs—ñ‚ğì¬

  float fovy_rad = toRadians(fovy);
  float f = 1.0f / (std::tan(fovy_rad / 2.0f));
  float d = (far_z - near_z);

  m << f / aspect, 0.0f, 0.0f, 0.0f,
    0.0f, f, 0.0f, 0.0f,
    0.0f, 0.0f, -(far_z + near_z) / d, -(2.0f * far_z * near_z) / d,
    0.0f, 0.0f, -1.0f, 0.0f;

  return m;
}

void android_main(struct android_app* state) {
  AppEngine engine(state);

  float angle = 0;

  while (1) {
    updateEvents(&engine, state);
    
    if (engine.active) {
      if (engine.display == NULL) return;
      glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      glViewport(0, 0, engine.width, engine.height);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      float aspect = engine.width / float(engine.height);
      float fov = 35.0f;
      float near_z = 0.5f;

      float fovy = calcFovy(fov, aspect, near_z);
      Eigen::Matrix4f m = perspectiveView(fovy, aspect, near_z, 50.0f);
      glMultMatrixf(m.data());

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      
      // renderer hear
      glPushMatrix();
      glTranslatef(0, 0, -2);
      glRotatef(angle++, 0, 1, 0);

      GLfloat vtx[] = {
        0.0f, 0.433f, 0.0f,
        -0.5f, -0.433f, 0.0f,
        0.5f, -0.433f, 0.0f
      };
      glVertexPointer(3, GL_FLOAT, 0, vtx);

      glEnableClientState(GL_VERTEX_ARRAY);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glDisableClientState(GL_VERTEX_ARRAY);
      
      glPopMatrix();
      eglSwapBuffers(engine.display, engine.surface);
    }
  }
}
