
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
      // 現在の状態を保存するようシステムによって要求されました。保存してください。
      // 保存するデータが無いので今は保留
      break;
      case APP_CMD_INIT_WINDOW:
      // ウィンドウが表示されています。準備してください。
      if (engine->app->window != NULL) {
        init_display(engine);
        draw_frame(engine);
      }
      break;
      case APP_CMD_TERM_WINDOW:
      // ウィンドウが非表示または閉じています。クリーン アップしてください。
      term_display(engine);
      break;
      case APP_CMD_GAINED_FOCUS:
      // アプリがフォーカスを取得すると、加速度計の監視を開始します。
      if (engine->accelerometerSensor != NULL) {
        ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                       engine->accelerometerSensor);
        // 目標は 1 秒ごとに 60 のイベントを取得することです (米国)。
        ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                       engine->accelerometerSensor, (1000L / 60) * 1000);
      }
      break;
      case APP_CMD_LOST_FOCUS:
      // アプリがフォーカスを失うと、加速度計の監視を停止します。
      // これにより、使用していないときのバッテリーを節約できます。
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
    // OpenGL ES と EGL の初期化

    /*
    * 目的の構成の属性をここで指定します。
    * 以下で、オンスクリーン ウィンドウと
    * 互換性のある、各色最低 8 ビットのコンポーネントの EGLConfig を選択します
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

    /* ここで、アプリケーションは目的の構成を選択します。このサンプルでは、
    * 抽出条件と一致する最初の EGLConfig を
    * 選択する単純な選択プロセスがあります */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID は、ANativeWindow_setBuffersGeometry() に
    * よって受け取られることが保証されている EGLConfig の属性です。
    * EGLConfig を選択したらすぐに、ANativeWindow バッファーを一致させるために
    * EGL_NATIVE_VISUAL_ID を使用して安全に再構成できます。*/
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

    // GL の状態を初期化します。
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    return 0;
  }

  static void draw_frame(AppEngine* engine) {
    if (engine->display == NULL) {
      // ディスプレイがありません。
      return;
    }

    // 色で画面を塗りつぶします。
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

    // 加速度計の監視の準備
    sensorManager = ASensorManager_getInstance();
    accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    sensorEventQueue = ASensorManager_createEventQueue(sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

    active = 1;
  }
};

void android_main(struct android_app* state) {
  AppEngine app(state);

  while (1) {
    // 保留中のすべてのイベントを読み取ります。
    int ident;
    int events;
    struct android_poll_source* source;

    // Activeでない場合、無期限にブロックしてイベントが発生するのを待ちます。
    while ((ident = ALooper_pollAll(app.active ? 0 : -1, NULL, &events,
                                    (void**)&source)) >= 0)
    {
      // このイベントを処理します。
      if (source != NULL) {
        source->process(state, source);
      }

      // センサーにデータがある場合、今すぐ処理します。
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

      // 終了するかどうか確認します。
      if (state->destroyRequested != 0) {
        app.term_display(&app);
        return;
      }
    }

    if (app.active) {
      // 描画は画面の更新レートに合わせて調整されているため、
      // ここで時間調整をする必要はありません。
      app.draw_frame(&app);
    }
  }
}
