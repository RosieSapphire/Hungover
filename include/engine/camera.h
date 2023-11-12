#ifndef ENGINE_CAMERA_H_
#define ENGINE_CAMERA_H_

#include "engine/types.h"

/**
 * struct camera - Camera Structure
 * @pitch: Pitch of Camera
 * @pitch_last: Last Pitch of Camera
 * @yaw: Yaw of Camera
 * @yaw_last: Last Yaw of Camera
 * @pitch_smooth: Smoothed Pitch of Camera
 * @yaw_smooth: Smoothed Yaw of Camera
 * @eye_last: Last Eye Postion
 * @eye: Eye Postion
 * @foc_last: Last Focus Postion
 * @foc: Focus Postion
 */
struct camera
{
	f32 pitch, pitch_last, yaw, yaw_last, pitch_smooth, yaw_smooth;
	f32 eye_last[3], eye[3], foc_last[3], foc[3];
};

void camera_init(struct camera *c);
void camera_get_focus(struct camera *c);

#endif /* ENGINE_CAMERA_H_ */
