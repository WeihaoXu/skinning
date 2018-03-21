INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/pmdreader)
AUX_SOURCE_DIRECTORY(${CMAKE_SOURCE_DIR}/lib/pmdreader libpmdr_src)
ADD_LIBRARY(pmdreader STATIC ${libpmdr_src})
