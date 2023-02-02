set(srcs "bmi160Impl.c")

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "." PRIV_REQUIRES "BMI160_driver")