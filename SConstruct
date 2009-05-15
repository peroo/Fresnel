env = Environment()
#env['CC'] = 'icc'
#env['CXX'] = 'icpc'
#env['LINKERFORPROGRAMS'] = 'icpc'

if ARGUMENTS.get('debug', 0):
    env['CCFLAGS'] = '-g'
elif ARGUMENTS.get('optimize', 0):
#    env['CCFLAGS'] = '-fast'
#    env['CCFLAGS'] = '-O9'
    env['CCFLAGS'] = '-O2 -ftree-vectorize -ftree-vectorizer-verbose=3 -ffast-math'
else:
    env['CCFLAGS'] = '-Wno-write-strings'

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
      'pthread',
      'profiler',
      'tcmalloc'
])

