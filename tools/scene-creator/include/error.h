#ifndef _ERROR_H_
#define _ERROR_H_

void error_log(const char *error_format, ...);
void error_check(const int function_return, const int desired_return,
		 const char *error_message);

#endif /* _ERROR_H_ */
