

PROGRAM(catboost-python)

PEERDIR(
    contrib/python/six
    contrib/python/numpy
    contrib/python/pandas
    library/python/pymain
)

PY_SRCS(
    TOP_LEVEL
    __init__.py
)

PY_MAIN(library.python.pymain:run)

NO_LINT()

NO_CHECK_IMPORTS()

END()
