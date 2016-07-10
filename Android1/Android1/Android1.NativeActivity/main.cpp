
#include "app_engine.h"


void android_main(struct android_app* state) {
  AppEngine engine(state);

  while (1) {
    updateEvents(&engine, state);

    if (engine.active) {
      if (engine.display == NULL) return;
      glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      // renderer hear

      eglSwapBuffers(engine.display, engine.surface);
    }
  }
}
