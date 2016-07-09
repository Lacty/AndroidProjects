
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))


class AppEngine {
public:
  android_app* app;

  ASensorManager* sensorManager;
  const ASensor* accelerometerSensor;
  ASensorEventQueue* sensorEventQueue;

  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  int32_t width;
  int32_t height;

  int32_t touch_x;
  int32_t touch_y;

  int active;

  static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    AppEngine* engine = (AppEngine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
      engine->touch_x = AMotionEvent_getX(event, 0);
      engine->touch_y = AMotionEvent_getY(event, 0);
      return 1;
    }
    return 0;
  }

  static void handle_cmd(struct android_app* app, int32_t cmd) {
    AppEngine* engine = (AppEngine*)app->userData;
    switch (cmd) {
      case APP_CMD_SAVE_STATE:
      // ���݂̏�Ԃ�ۑ�����悤�V�X�e���ɂ���ėv������܂����B�ۑ����Ă��������B
      // �ۑ�����f�[�^�������̂ō��͕ۗ�
      break;
      case APP_CMD_INIT_WINDOW:
      // �E�B���h�E���\������Ă��܂��B�������Ă��������B
      if (engine->app->window != NULL) {
        init_display(engine);
        draw_frame(engine);
      }
      break;
      case APP_CMD_TERM_WINDOW:
      // �E�B���h�E����\���܂��͕��Ă��܂��B�N���[�� �A�b�v���Ă��������B
      term_display(engine);
      break;
      case APP_CMD_GAINED_FOCUS:
      // �A�v�����t�H�[�J�X���擾����ƁA�����x�v�̊Ď����J�n���܂��B
      if (engine->accelerometerSensor != NULL) {
        ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                       engine->accelerometerSensor);
        // �ڕW�� 1 �b���Ƃ� 60 �̃C�x���g���擾���邱�Ƃł� (�č�)�B
        ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                       engine->accelerometerSensor, (1000L / 60) * 1000);
      }
      break;
      case APP_CMD_LOST_FOCUS:
      // �A�v�����t�H�[�J�X�������ƁA�����x�v�̊Ď����~���܂��B
      // ����ɂ��A�g�p���Ă��Ȃ��Ƃ��̃o�b�e���[��ߖ�ł��܂��B
      if (engine->accelerometerSensor != NULL) {
        ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                        engine->accelerometerSensor);
      }

      engine->active = 0;
      draw_frame(engine);
      break;
    }
  }

  static int init_display(AppEngine* engine) {
    // OpenGL ES �� EGL �̏�����

    /*
    * �ړI�̍\���̑����������Ŏw�肵�܂��B
    * �ȉ��ŁA�I���X�N���[�� �E�B���h�E��
    * �݊����̂���A�e�F�Œ� 8 �r�b�g�̃R���|�[�l���g�� EGLConfig ��I�����܂�
    */
    const EGLint attribs[] = {
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_BLUE_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_RED_SIZE, 8,
      EGL_NONE
    };
    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* �����ŁA�A�v���P�[�V�����͖ړI�̍\����I�����܂��B���̃T���v���ł́A
    * ���o�����ƈ�v����ŏ��� EGLConfig ��
    * �I������P���ȑI���v���Z�X������܂� */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID �́AANativeWindow_setBuffersGeometry() ��
    * ����Ď󂯎���邱�Ƃ��ۏ؂���Ă��� EGLConfig �̑����ł��B
    * EGLConfig ��I�������炷���ɁAANativeWindow �o�b�t�@�[����v�����邽�߂�
    * EGL_NATIVE_VISUAL_ID ���g�p���Ĉ��S�ɍč\���ł��܂��B*/
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
      LOGW("Unable to eglMakeCurrent");
      return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->active = 1;

    // GL �̏�Ԃ����������܂��B
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    return 0;
  }

  static void draw_frame(AppEngine* engine) {
    if (engine->display == NULL) {
      // �f�B�X�v���C������܂���B
      return;
    }

    // �F�ŉ�ʂ�h��Ԃ��܂��B
    glClearColor(((float)engine->touch_x) / engine->width, 0,
                 ((float)engine->touch_y) / engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(engine->display, engine->surface);
  }

  static void term_display(AppEngine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
      eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      if (engine->context != EGL_NO_CONTEXT) {
        eglDestroyContext(engine->display, engine->context);
      }
      if (engine->surface != EGL_NO_SURFACE) {
        eglDestroySurface(engine->display, engine->surface);
      }
      eglTerminate(engine->display);
    }

    engine->active = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
  }

public:
  AppEngine() = delete;
  AppEngine(android_app* state) {
    memset(this, 0, sizeof(this));
    state->userData = this;
    state->onAppCmd = handle_cmd;
    state->onInputEvent = handle_input;
    app = state;

    // �����x�v�̊Ď��̏���
    sensorManager = ASensorManager_getInstance();
    accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    sensorEventQueue = ASensorManager_createEventQueue(sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

    active = 1;
  }
};

void android_main(struct android_app* state) {
  AppEngine app(state);

  while (1) {
    // �ۗ����̂��ׂẴC�x���g��ǂݎ��܂��B
    int ident;
    int events;
    struct android_poll_source* source;

    // Active�łȂ��ꍇ�A�������Ƀu���b�N���ăC�x���g����������̂�҂��܂��B
    while ((ident = ALooper_pollAll(app.active ? 0 : -1, NULL, &events,
                                    (void**)&source)) >= 0)
    {
      // ���̃C�x���g���������܂��B
      if (source != NULL) {
        source->process(state, source);
      }

      // �Z���T�[�Ƀf�[�^������ꍇ�A�������������܂��B
      if (ident == LOOPER_ID_USER) {
        if (app.accelerometerSensor != NULL) {
          ASensorEvent event;
          while (ASensorEventQueue_getEvents(app.sensorEventQueue,
                 &event, 1) > 0) {
            LOGI("accelerometer: x=%f y=%f z=%f",
                 event.acceleration.x, event.acceleration.y,
                 event.acceleration.z);
          }
        }
      }

      // �I�����邩�ǂ����m�F���܂��B
      if (state->destroyRequested != 0) {
        app.term_display(&app);
        return;
      }
    }

    if (app.active) {
      // �`��͉�ʂ̍X�V���[�g�ɍ��킹�Ē�������Ă��邽�߁A
      // �����Ŏ��Ԓ���������K�v�͂���܂���B
      app.draw_frame(&app);
    }
  }
}
