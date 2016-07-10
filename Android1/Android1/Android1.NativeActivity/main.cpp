
#include "app_engine.h"


void android_main(struct android_app* state) {
  AppEngine engine(state);

  float touch_x = 0;
  float touch_y = 0;

  while (1) {
    updateEvents(&engine, state);

    // ‰æ–Ê’†S‚ğ’†“_‚Æ‚µ‚½À•WŒn‚É•ÏŠ·
    touch_x = (engine.touch_x - engine.width * 0.5f) / (engine.width * 0.5f);
    touch_y = ((engine.touch_y - engine.height * 0.5f) / (engine.height * 0.5f)) * -1.0f;

    if (engine.active) {
      if (engine.display == NULL) return;
      glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      // renderer hear

      GLfloat vtx[] = {
        touch_x, touch_y
      };

      glVertexPointer(2, GL_FLOAT, 0, vtx);
      glEnableClientState(GL_VERTEX_ARRAY);
      glPointSize(100);
      glDrawArrays(GL_POINTS, 0, 1);
      glDisableClientState(GL_VERTEX_ARRAY);

      eglSwapBuffers(engine.display, engine.surface);
    }
  }
}
