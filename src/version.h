#define VERSION "1.5.7"

#ifdef DEBUG
    #define RELEASE VERSION " DEBUG"
#else
    #define RELEASE VERSION
#endif

#define COPYRIGHT_NOTICE "ccalc " RELEASE " Copyright (C) 2020 Philipp Hochmann, phil.hochmann@gmail.com\n"
