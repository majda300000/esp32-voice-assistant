set(COMPONENT_SRCS
    main.c
    recsrc.c
    speech_commands_action.c
    led.c
    mqtt_connect.c
    wifi_connect.c
    wake_button.c
    )

set(COMPONENT_ADD_INCLUDEDIRS 
    include
    ${CMAKE_SOURCE_DIR}
    )

set(COMPONENT_REQUIRES
    esp-sr
    hardware_driver
    esp-mqtt
    )

register_component()



