if(ANDROID)
    set(Qt5AndroidSupport_FOUND TRUE)
    add_library(Qt5::AndroidSupport INTERFACE IMPORTED)
endif()
