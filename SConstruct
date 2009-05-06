if ARGUMENTS.get('debug', 0):
    env = Environment(CCFLAGS = '-g')
else:
    env = Environment()
#    env = Environment(CCFLAGS = '-O9')




env.Program(['Slingshot.cpp', 
         'JavaScript.cpp', 
         'HttpRequest.cpp', 
         'JSDatabase.cpp', 
         'FLAC.cpp', 
         'OggEncode.cpp', 
         'Image.cpp', 
         'Indexer.cpp',
         'AudioFile.cpp',
         'Database.cpp'
], 
LIBS=['v8', 
      'microhttpd', 
      'sqlite3', 
      'boost_filesystem', 
      'FLAC++', 
      'vorbisenc', 
      'jpeg', 
      'png',
      'libtag',
      'libzebra',
      'pthread'])

