INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)

set(EVOPLEX_GUI_H
  maingui.h
  attributeswidget.h
  #contextmenutable.h
  experimentwidget.h
  graphwidget.h
  projectwidget.h
  projectswindow.h
  queuewidget.h
  tablewidget.h
  settingswidget.h
  welcomewidget.h
)

set(EVOPLEX_GUI_CXX
  maingui.cpp
  attributeswidget.cpp
  #contextmenutable.cpp
  experimentwidget.cpp
  graphwidget.cpp
  projectwidget.cpp
  projectswindow.cpp
  queuewidget.cpp
  tablewidget.cpp
  settingswidget.cpp
  welcomewidget.cpp
)

qt5_wrap_ui(EVOPLEX_GUI_UI
  forms/attributeswidget.ui
  forms/queuewidget.ui
  forms/projectwidget.ui
  forms/settingswidget.ui
  forms/welcomewidget.ui
)

add_library(EvoplexGUILib SHARED ${EVOPLEX_GUI_CXX} ${EVOPLEX_GUI_UI})

target_link_libraries(EvoplexGUILib EvoplexCore Qt5::Core Qt5::Widgets)