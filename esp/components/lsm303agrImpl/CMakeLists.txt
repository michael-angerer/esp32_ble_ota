set(srcs "lsm303agrImpl.c")

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES "lsm303agr-pid driver")
