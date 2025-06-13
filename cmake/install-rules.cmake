install(
    TARGETS httpfileserver_exe
    RUNTIME COMPONENT httpfileserver_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
