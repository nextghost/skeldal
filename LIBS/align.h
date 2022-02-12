#ifdef __cplusplus
extern "C" {
#endif
	static __inline short uaGetShort(const void *ptr) {
		unsigned char *c = (unsigned char *)ptr;
		return (short) (c[0] | (c[1] << 8));
	}

	static __inline unsigned short uaGetUShort(const void *ptr) {
		unsigned char *c = (unsigned char *)ptr;
		return (short) (c[0] | (c[1] << 8));
	}

	static __inline int uaGetInt(const void *ptr) {
		unsigned char *c = (unsigned char *)ptr;
		return (int) (c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24));
	}

	static __inline unsigned int uaGetUInt(const void *ptr) {
		unsigned char *c = (unsigned char *)ptr;
		return (unsigned int) (c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24));
	}


	static __inline const void *uaSeekPtr(const void ** ptr, int bytes) {
		const void *res = *ptr;
		*ptr = (void *const )((const char *)(*ptr) + bytes);
		return res;
	}

	static __inline short uaReadShort(const void **  ptr) {
		return uaGetShort(uaSeekPtr(ptr,2));
		
	}

	static __inline unsigned short uaReadUShort(const void ** ptr) {
		return uaGetUShort(uaSeekPtr(ptr,2));
	}


	static __inline int uaReadInt(const void **  ptr) {
		return uaGetInt(uaSeekPtr(ptr,4));
	}

	static __inline unsigned int uaReadUInt(const void **  ptr) {
		return uaGetUInt(uaSeekPtr(ptr,4));	
	}



#ifdef __cplusplus
}
#endif