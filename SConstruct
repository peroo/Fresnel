if ARGUMENTS.get('debug', 0):
    env = Environment(CCFLAGS = '-g')
else:
    env = Environment(CCFLAGS = '-Wno-write-strings')
#    env = Environment(CCFLAGS = '-O9')

env.Program(['Slingshot.cpp', 
         'JavaScript.cpp', 
         'HttpRequest.cpp', 
         'JSDatabase.cpp', 
         'Image.cpp', 
         'Indexer.cpp',
         'Database.cpp',
         'Resource.cpp',
         'Audio/Audio.cpp',
         'Audio/AudioEncoder.cpp',
         'Audio/AudioDecoder.cpp',
         'Audio/FLACDecoder.cpp', 
         'Audio/VorbisEncoder.cpp', 
         'Audio/Metadata.cpp',
], 
LIBS=['v8', 
      'microhttpd', 
      'sqlite3', 
      'boost_filesystem', 
      'FLAC++', 
      'vorbisenc', 
      'jpeg', 
      'png',
      'tag',
      'zbar',
      'pthread'])

