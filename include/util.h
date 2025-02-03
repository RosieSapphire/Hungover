#ifndef _UTIL_H_
#define _UTIL_H_

/* macro for printing T3DVec3 to `debugf` */
#define debugf_tv3(VEC, LABEL) \
	debugf("%s: (%f, %f, %f)\n", LABEL, (VEC).v[0], (VEC).v[1], (VEC).v[2])

#endif /* _UTIL_H_ */
