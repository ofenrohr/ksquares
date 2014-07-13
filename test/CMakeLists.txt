cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE ( KDE4       REQUIRED            )
FIND_PACKAGE ( Qt4        REQUIRED QT_USE_QT* )

INCLUDE(   ${QT_USE_FILE}   )
include(KDE4Defaults)

set( ksquarestest_SRCS regression.cpp )

kde4_add_unit_test( ksquarestest
  TESTNAME regression-ksquarestest
  ${ksquarestest_SRCS}
)

target_link_libraries( ksquarestest
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  kdegames
)
