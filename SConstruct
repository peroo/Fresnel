env = Environment()
#env['CC'] = 'icc'
#env['CXX'] = 'icpc'
#env['LINKERFORPROGRAMS'] = 'icpc'

if ARGUMENTS.get('debug', 0):
    env['CCFLAGS'] = '-g'
elif ARGUMENTS.get('optimize', 0):
#    env['CCFLAGS'] = '-fast'
#    env['CCFLAGS'] = '-O9'
    env['CCFLAGS'] = '-O2 -ftree-vectorize -ftree-vectorizer-verbose=3 -ffast-math -mtune=k8-sse3'
elif ARGUMENTS.get('valgrind', 0):
    env['CCFLAGS'] = '-O0 -g'
else:
    env['CCFLAGS'] = '-Wno-write-strings'

env.Program(['Slingshot.cpp', 
         'JavaScript.cpp', 
         'HttpRequest.cpp', 
         'JSDatabase.cpp', 
         'Indexer.cpp',
         'Resource.cpp',
         'Database.cpp',
         'Audio/Audio.cpp',
         'Audio/AudioEncoder.cpp',
         'Audio/AudioDecoder.cpp',
         'Audio/FLACDecoder.cpp', 
         'Audio/VorbisEncoder.cpp', 
         'Audio/Metadata.cpp',
         'Image/Image.cpp', 
], 
LIBS=['v8_g', 
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
#     'profiler',
#     'tcmalloc'
])

