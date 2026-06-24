Simple RTSP recording library for my MediaRecorder project (https://github.com/Eap2468/MediaRecorder/tree/main)

Main idea is to abstract away handling functionality for multiple streams to a easy to interface with library with the minimum functionality required to do the job
Built on top of FFmpeg which admittedly Claude helped with the streaming functionality (everything in the recording thread) to speed things up so I could focus on the rest of the MediaRecorder.
Not thread safe for the moment but if the need comes up in the future I'll come back and add it. Same for any bugs however those will likely be fixed as they come up from use with the main application.

CreateSimpleMediaRecorder - Create a SIMPLE_MEDIA_RECORDER context for future operations
CleanupSimpleMediaRecorder - Cleanup a SIMPLE_MEDIA_RECORDER context 
AddSimpleMediaInput - Add an input to the media recording context
RemoveSimpleMediaInput - Removes an input to the media recording context by the SIMPLE_MEDIA_INPUT structure
RemoveSimpleMediaInputByName - Removes an input by the name, this is meant as the main way to remove inputs
SetSimpleMediaOutputDirectory - Sets the output directory for video files to be written to
GetSimpleMediaOutputDirectory - Gets the currently registered output directory for the recorder
StartSimpleMediaStream - Starts recording all inputs registered to a recorder
StopSimpleMediaStream - Stops all recording stream registered to a recorder
StartSimpleMediaInputStream - Starts a stream by input, this is mostly meant for internal use
StopSimpleMediaInputStream - Stops a stream by input, this is mostly meant for internal use
