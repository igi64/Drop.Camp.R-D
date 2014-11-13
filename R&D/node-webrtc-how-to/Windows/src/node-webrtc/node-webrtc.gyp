{    
    'includes': 
    [
        '../talk/build/common.gypi',
    ],
    'targets': [
		{
		  'target_name': 'node-webrtc',
		  'type': 'loadable_module',
#		  'standalone_static_library': 0,
		  'product_name': 'wrtc',
		  'product_extension': 'node',
		  'product_prefix': '',
		  'variables': {
			'node_root_dir%': '../../../node-android',
		  },
		  'defines': [ 'BUILDING_NODE_EXTENSION',
#        'TRACING',
        'LARGEFILE_SOURCE',
        '_FILE_OFFSET_BITS=64',
#        'WEBRTC_TARGET_PC',
#        'WEBRTC_LINUX',
#        'WEBRTC_THREAD_RR',
#        'EXPAT_RELATIVE_PATH',
#        'GTEST_RELATIVE_PATH',
#        'JSONCPP_RELATIVE_PATH',
#        'WEBRTC_RELATIVE_PATH',
##        'POSIX',
#        '__STDC_FORMAT_MACROS',
#        'DYNAMIC_ANNOTATIONS_ENABLED=0'
                             ],
		  'dependencies': [
			'<(DEPTH)/talk/libjingle.gyp:libjingle_peerconnection',
			'<(DEPTH)/talk/libjingle.gyp:libjingle_p2p',
#			'<(DEPTH)/talk/libjingle.gyp:libjingle_media',
			'<(DEPTH)/talk/libjingle.gyp:libjingle',
		  ],
		  'include_dirs': [
			'<(node_root_dir)/src',
			'<(node_root_dir)/deps/uv/include',
			'<(node_root_dir)/deps/v8/include',
		  ],
		  'cflags': [
#        '-pthread',
#        '-fno-exceptions',
#        '-fno-strict-aliasing',
#        '-Wall',
#        '-Wno-unused-parameter',
#        '-Wno-missing-field-initializers',
#        '-Wextra',
#        '-Wno-unused-local-typedefs',
#        '-Wno-uninitialized',
#        '-Wno-unused-variable',
#        '-Wno-unused-but-set-variable',
#        '-pipe',
#        '-fno-ident',
#        '-fdata-sections',
#        '-ffunction-sections',
#        '-fPIC',
#        '-fpermissive',

#			'-pthread',
#			'-Wall',
#			'-fPIC',
#			'-fvisibility=default',
          ],
          'link_settings': {
            'ldflags': [
#  		      '-fvisibility=default',
            ],
            'libraries': [
              '../<(node_root_dir)/build/Release/lib/v8_base.lib',
              '../<(node_root_dir)/build/Release//lib/v8_nosnapshot.lib',
              '../<(node_root_dir)/Release/lib/libuv.lib',
              '../<(node_root_dir)/Release/node.lib',
            ],
          },
          'conditions': [
            ['OS=="win"', {
              'msvs_disabled_warnings': [4996, 4506, 4005],
            }],
          ],
		  'sources': [
			'binding.cc',
			'create-offer-observer.cc',
			'create-answer-observer.cc',
			'set-local-description-observer.cc',
			'set-remote-description-observer.cc',
			'peerconnection.cc',
			'datachannel.cc',
		  ],
		}, # end node-webrtc
    ], # end targets
}
