#ifndef __ADOOM_H__
#define __ADOOM_H__

class ADoom {
public:
	ADoom(DWORD);
	~ADoom();
	template<typename ReadType>
	ReadType rM(DWORD, DWORD);
	BOOL wM(DWORD, PVOID, SIZE_T);
	VOID sT(DWORD);
	DWORD aM(SIZE_T);

private:
	HANDLE	DH;
};



#endif
