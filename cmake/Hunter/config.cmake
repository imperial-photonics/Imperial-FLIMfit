hunter_config(OpenCV VERSION ${HUNTER_OpenCV_VERSION} CMAKE_ARGS 
   WITH_CUDA=OFF 
   WITH_OPENEXR=OFF 
   WITH_EIGEN=OFF 
   WITH_JASPER=OFF 
   WITH_AVFOUNDATION=OFF 
   BUILD_opencv_python=OFF 
   WITH_LAPACK=OFF)
hunter_config(Boost VERSION 1.61.0) # override manually for the moment - v1.64 not building correctly on VS2017