# Apps meta-component
# This component doesn't have source files itself, but depends on all app components

set(app_components qmi8658c navigation contacts piobot)

idf_component_register(INCLUDE_DIRS "."
                       REQUIRES ${app_components})
