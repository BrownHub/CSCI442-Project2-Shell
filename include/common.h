#ifndef _COMMON_H

/**
 * STATIC_ASSERT_INLINE - Take advantage of division by zero to
 * indicate a failure condition.
 *
 * If usage is guaranteed to be inside of a function, a braced group
 * is recommended instead.
 */
#undef STATIC_ASSERT_INLINE
#define STATIC_ASSERT_INLINE(value, condition) ((value) / !!(condition))

/**
 * ARRAY_SIZE - Get the number of elements in an array.
 *
 * For type safety, this function will cause a division-by-zero
 * compile-time check failure if a pointer is passed instead of an
 * array.
 */
#undef ARRAY_SIZE
#define ARRAY_SIZE(arr)                                                 \
	STATIC_ASSERT_INLINE(sizeof(arr) / sizeof((arr)[0]),            \
			     !__builtin_types_compatible_p(typeof(arr), \
							   typeof(&(arr)[0])))

#endif /* _COMMON_H */
