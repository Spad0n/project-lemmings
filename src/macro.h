#ifndef RAYMACRO_H_
#define RAYMACRO_H_

#define Drawing \
    for (int _break = (BeginDrawing(), 1); _break; _break = 0, EndDrawing())

#define Mode2D(camera) \
    for (int _break = (BeginMode2D(camera), 1); _break; _break = 0, EndMode2D())

#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func)           \
    static inline void func##p(type *p) {                 \
	if (*p)                                           \
	    func(*p);                                     \
    }                                                     \
    struct __useless_struct_to_allow_trailing_semicolon__

#endif // RAYMACRO_H_
