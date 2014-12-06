
PsyProjectGameExe( solution().name )
	files { "./AutoGenRegisterReflection.cpp", "./**.h", "./**.inl", "./**.cpp" }
   debugdir "../Dist"
   includedirs { 
      "./", 
      "../Psybrus/Engine/Source/Shared/", 
   }
	-- External includes.
	includedirs { 
      "../Psybrus/External/jsoncpp/include", 
   }

   PsyAddEngineLinks {
      "Engine",
      "System_Sound",
      "System_Scene",
      "System_Renderer",
      "System_Os",
      "System_Network",
      "System_Debug",
      "System_Content",
      "System_File",
      "System",
      "Serialisation",
      "Reflection",
      "Math",
      "Import",
      "Events",
      "Base",
   }
   
   PsyAddExternalLinks {
      "BulletPhysics",
      "freetype",
      "jsoncpp",
      "libb64",
      "pcre",
      "png",
      "SoLoud",
      "zlib",
   }

   PsyAddBoostLibs {
      "regex",
      "filesystem",
      "system",
      "wave",
   }

   configuration { "windows-* or linux-*" }
      PsyAddExternalLinks {
         "assimp",
         "assimp_contrib",
         "enet",
         "HLSLCrossCompiler",
         "hlsl2glslfork",
         "glew",
         "squish",
         "webby",
      }

   configuration "linux*"
      includedirs {
         "../Psybrus/Engine/Source/Platforms/Linux/",
      }

      prebuildcommands {
         --"python ../../Psybrus/reflection_parse.py " .. solution().name
      }

   configuration "windows-*"
      includedirs {
         "../Psybrus/Engine/Source/Platforms/Windows/",
         BOOST_INCLUDE_PATH
      }

      prebuildcommands {
            "C:\\Python27\\python.exe ../../Psybrus/reflection_parse.py " .. solution().name
      }

      libdirs {
           BOOST_LIB_PATH
      }

