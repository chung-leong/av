#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_av.h"

int av_get_element_bool(zval *array, const char *key, int *p_value) {
	if(array) {
		if(Z_TYPE_P(array) == IS_ARRAY) {
			zval **p_data;
			if(zend_hash_find(Z_ARRVAL_P(array), key, strlen(key) + 1, (void **) &p_data) == SUCCESS) {
				convert_to_boolean(*p_data);
				*p_value = Z_LVAL_PP(p_data) != 0;
				return TRUE;
			}
		}
	}
	return FALSE;
}

int av_get_element_long(zval *array, const char *key, long *p_value) {
	if(array) {
		if(Z_TYPE_P(array) == IS_ARRAY) {
			zval **p_data;
			if(zend_hash_find(Z_ARRVAL_P(array), key, strlen(key) + 1, (void **) &p_data) == SUCCESS) {
				convert_to_long(*p_data);
				*p_value = Z_LVAL_PP(p_data);
				return TRUE;
			}
		}
	}
	return FALSE;
}

int av_get_element_double(zval *array, const char *key, double *p_value) {
	if(array) {
		if(Z_TYPE_P(array) == IS_ARRAY) {
			zval **p_data;
			if(zend_hash_find(Z_ARRVAL_P(array), key, strlen(key) + 1, (void **) &p_data) == SUCCESS) {
				convert_to_double(*p_data);
				*p_value = Z_DVAL_PP(p_data);
				return TRUE;
			}
		}
	}
	return FALSE;
}

int av_get_element_string(zval *array, const char *key, char **p_value) {
	if(array) {
		if(Z_TYPE_P(array) == IS_ARRAY) {
			zval **p_data;
			if(zend_hash_find(Z_ARRVAL_P(array), key, strlen(key) + 1, (void **) &p_data) == SUCCESS) {
				convert_to_string(*p_data);
				*p_value = Z_STRVAL_PP(p_data);
				return TRUE;
			}
		}
	}
	return FALSE;
}

int av_get_element_stringl(zval *array, const char *key, char **p_value, long *p_len) {
	if(array) {
		if(Z_TYPE_P(array) == IS_ARRAY) {
			zval **p_data;
			if(zend_hash_find(Z_ARRVAL_P(array), key, strlen(key) + 1, (void **) &p_data) == SUCCESS) {
				convert_to_string(*p_data);
				*p_value = Z_STRVAL_PP(p_data);
				*p_len = Z_STRLEN_PP(p_data);
				return TRUE;
			}
		}
	}
	return FALSE;
}

int av_get_element_hash(zval *array, const char *key, HashTable **p_hash) {
	if(array) {
		if(Z_TYPE_P(array) == IS_ARRAY) {
			zval **p_data;
			if(zend_hash_find(Z_ARRVAL_P(array), key, strlen(key) + 1, (void **) &p_data) == SUCCESS) {
				convert_to_array(*p_data);
				*p_hash = Z_ARRVAL_PP(p_data);
				return TRUE;
			}
		}
	}
	return FALSE;
}

int av_get_element_resource(zval *array, const char *key, zval **p_res) {
	if(array) {
		if(Z_TYPE_P(array) == IS_ARRAY) {
			zval **p_data;
			if(zend_hash_find(Z_ARRVAL_P(array), key, strlen(key) + 1, (void **) &p_data) == SUCCESS) {
				if(Z_TYPE_PP(p_data) == IS_RESOURCE) {
					*p_res = *p_data;
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

void av_set_element_bool(zval *array, const char *key, int value) {
	zval *element;
	MAKE_STD_ZVAL(element);
	ZVAL_BOOL(element, value);
	zend_hash_update(HASH_OF(array), key, strlen(key) + 1, (void *) &element, sizeof(zval *), NULL);
}

void av_set_element_long(zval *array, const char *key, long value) {
	zval *element;
	MAKE_STD_ZVAL(element);
	ZVAL_LONG(element, value);
	zend_hash_update(HASH_OF(array), key, strlen(key) + 1, (void *) &element, sizeof(zval *), NULL);
}

void av_set_element_double(zval *array, const char *key, double value) {
	zval *element;
	MAKE_STD_ZVAL(element);
	ZVAL_DOUBLE(element, value);
	zend_hash_update(HASH_OF(array), key, strlen(key) + 1, (void *) &element, sizeof(zval *), NULL);
}

void av_set_element_string(zval *array, const char *key, const char *value) {
	zval *element;
	MAKE_STD_ZVAL(element);
	ZVAL_STRING(element, (value) ? value : "", TRUE);
	zend_hash_update(HASH_OF(array), key, strlen(key) + 1, (void *) &element, sizeof(zval *), NULL);
}

void av_set_element_stringl(zval *array, const char *key, const char *value, long value_length) {
	zval *element;
	MAKE_STD_ZVAL(element);
	ZVAL_STRINGL(element, (value) ? value : "", value_length, TRUE);
	zend_hash_update(HASH_OF(array), key, strlen(key) + 1, (void *) &element, sizeof(zval *), NULL);
}

zval *av_create_gd_image(uint32_t width, uint32_t height TSRMLS_DC) {
	zval *z_width, *z_height, *z_function_name, *z_retval = NULL;
	zval **params[2];

	ALLOC_INIT_ZVAL(z_width);
	ALLOC_INIT_ZVAL(z_height);
	ALLOC_INIT_ZVAL(z_function_name);
	ZVAL_LONG(z_width, width);
	ZVAL_LONG(z_height, height);
	ZVAL_STRING(z_function_name, "imagecreate", TRUE);
	params[0] = &z_width;
	params[1] = &z_height;
	call_user_function_ex(CG(function_table), NULL, z_function_name, &z_retval, 2, params, TRUE, NULL TSRMLS_CC);
	zval_ptr_dtor(&z_width);
	zval_ptr_dtor(&z_height);
	zval_ptr_dtor(&z_function_name);
	return z_retval;
}

#ifdef USE_CUSTOM_MALLOC

void *custom_malloc(size_t size) {
	void *p = emalloc(size);
	return p;
}

void *custom_realloc(void *ptr, size_t size) {
	void *p = erealloc(ptr, size);
	return p;
}

void custom_free(void *ptr) {
	if(ptr) {
		efree(ptr);
	}
}

void av_install_custom_malloc(void) {
	av_set_custom_malloc(custom_malloc, custom_realloc, custom_free);
}

#endif
