{    
    'includes': 
    [
        '../talk/build/common.gypi',
    ],
    'targets': [
		{
		  'target_name': 'webrtc-cmd',
		  'type': 'executable',
		  'dependencies': [
			'<(DEPTH)/talk/libjingle.gyp:libjingle_peerconnection',
#			'<(DEPTH)/talk/libjingle.gyp:libjingle_p2p',
#			'<(DEPTH)/talk/libjingle.gyp:libjingle_media',
			'<(DEPTH)/talk/libjingle.gyp:libjingle',
		  ],
		  'cflags': [
#			'-pthread',
#			'-Wall',
#			'-fPIC',
#			'-fvisibility=default',
          ],
		  'sources': [
			'webrtc-cmd.cc',
                        'simpleaudiodevice.cc',
 		  ],
		}, # end webrtc-cmd
		{
		  'target_name': 'webrtc-cmd_so',
		  'type': 'loadable_module',
		  'dependencies': [
			'<(DEPTH)/talk/libjingle.gyp:libjingle_peerconnection',
#			'<(DEPTH)/talk/libjingle.gyp:libjingle_p2p',
#			'<(DEPTH)/talk/libjingle.gyp:libjingle_media',
			'<(DEPTH)/talk/libjingle.gyp:libjingle',
		  ],
		  'cflags': [
#			'-pthread',
#			'-Wall',
#			'-fPIC',
#			'-fvisibility=default',
          ],
		  'sources': [
			'webrtc-cmd-jni.cc',
                        'simpleaudiodevice.cc',
		  ],
		}, # end webrtc-cmd_so
    ], # end targets
}
