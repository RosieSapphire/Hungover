#ifndef ENGINE_CAMERA_H_
#define ENGINE_CAMERA_H_

typedef struct
{
	float pitch, pitch_last, yaw, yaw_last, pitch_smooth, yaw_smooth;
	float eye_last[3], eye[3], foc_last[3], foc[3];
} camera_t;

void camera_init(camera_t *c);
void camera_get_focus(camera_t *c);

#endif /* ENGINE_CAMERA_H_ */
