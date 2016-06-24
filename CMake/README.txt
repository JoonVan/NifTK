Notes:
------

ExternalProjects - CMake files to drive the Superbuild process which builds dependencies as 'External' projects.
PackageDepends   - CMake files about packages with settings that are required by the modules depending on them.
cuda             - CMake files for cuda. This can probably be replaced, as it is included in CMake 2.8.3 and onwards.
Find*            - CMake files for finding packages. If you see @variables, these get substituted at cmake time.
mitkCompiler*    - CMake compiler settings borrowed from MITK project.
StartVS*         - Batch file that gets generated using the build into the build folder to assist with launching 
                   Visual Studio with correct paths for development work. Windows only ... obviously.
SuperBuild*      - CMake file to drive the Superbuild process
CPack*           - Stuff to setup CPack, which we use for generating all packages.
CTestCustom*     - CTest setup, including filtering of error messages.
