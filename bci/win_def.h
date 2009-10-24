﻿/**
	win_def
	Main windows application include, always include first
	@classes	(WinMem, WinStr, CStr, WinFlag, WinBit, WinTimer, WinErrorCheck, Uncopyable, WinCOM)
	@author		Copyright © 2009 Grechkin Andrew
	@link		()
	@link		(ole32) for WinCom
**/

#ifndef WIN_DEF_HPP
#define WIN_DEF_HPP

#if (_WIN32_WINNT < 0x0501)
#undef _WIN32_WINNT
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501		// Windows 2000 or later
#endif

#if (WINVER < 0x0501)
#undef WINVER
#endif
#ifndef WINVER
#define WINVER 0x0501
#endif

#if (_WIN32_IE < 0x0600)
#undef _WIN32_IE
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <windows.h>

///===================================================================================== definitions
#define MAX_PATH_LENGTH			32768
#define SPACE					L' '
#define PATH_SEPARATOR			L"\\" // Path separator in the file system
#define PATH_SEPARATOR_C		L'\\' // Path separator in the file system
#define LONG_FILENAME_PREFIX	L"\\\\?\\" // Prefix to put ahead of a long path for Windows API

#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

typedef const TCHAR			*PCTSTR;
typedef const void			*PCVOID;

#ifdef DynamicLink
typedef	void*(__cdecl *dl_memset)(void*, int, size_t);
typedef	void*(__cdecl *dl_memcpy)(void*, const void*, size_t);
typedef	long long(__cdecl *dl__atoi64)(const char *);
typedef	long long(__cdecl *dl__wtoi64)(const wchar_t *);
typedef	int(__cdecl *dl__wcsicmp)(const wchar_t *, const wchar_t *);
typedef	int(__cdecl *dl_wcscmp)(const wchar_t *, const wchar_t *);
typedef	size_t(__cdecl *dl_wcslen)(const wchar_t *);
typedef	wchar_t*(__cdecl *dl_wcscpy)(wchar_t *, const wchar_t *);
typedef	wchar_t*(__cdecl *dl_wcsncpy)(wchar_t *, const wchar_t *, size_t);
typedef	int(__cdecl *dl__snwprintf)(wchar_t *, size_t, const wchar_t *, ...);
namespace	DynLink {
	extern HINSTANCE		hNtDll;
	extern dl__snwprintf	my_snprintf;
	bool					InitLib();
	void					FreeLib();
}
#define my_snwprintf DynLink::my_snprintf
#else
namespace	DynLink {
	bool	inline			InitLib() {
		return	true;
	}
	void	inline			FreeLib() {}
}
#define my_snwprintf _snwprintf
#endif

///=================================================================================================
inline void						XchgByte(WORD &inout) {
	inout = inout >> 8 | inout << 8;
}
inline void						XchgWord(DWORD &inout) {
	inout = inout >> 16 | inout << 16;
}

template <typename Type>
inline	const Type&				Min(const Type &a, const Type &b) {
	return	(a < b) ? a : b;
}
template <typename Type>
inline	const Type&				Max(const Type &a, const Type &b) {
	return	(a < b) ? b : a;
}
template <typename Type>
inline	void					Swp(Type &x, Type &y) {
	Type tmp(x);
	x = y;
	y = tmp;
}

///====================================================================================== Uncopyable
class		Uncopyable {
	Uncopyable(const Uncopyable&);
	Uncopyable& operator=(const Uncopyable&);
protected:
	~Uncopyable() {
	}
	Uncopyable() {
	}
};

///=================================================================================== WinErrorCheck
class		WinErrorCheck {
	HRESULT	mutable	m_err;
protected:
	~WinErrorCheck() {
	}
	WinErrorCheck(): m_err(NO_ERROR) {
	}
public:
	HRESULT			err() const {
		return	m_err;
	}
	HRESULT			err(HRESULT err) const {
		return	m_err = err;
	}
	/*
		HRESULT			GetErr() const {
			return	m_err;
		}
		HRESULT			SetErr() const {
			return	m_err = ::GetLastError();
		}
		HRESULT			SetErr(HRESULT err) const {
			return	m_err = err;
		}
	*/
	bool			IsOK() const {
		return	m_err == NO_ERROR;
	}
	bool			ChkSucc(bool in) const {
		if (!in)
			err(::GetLastError());
		else
			err(NO_ERROR);
		return	in;
	}
	template<typename Type>
	void			SetIfFail(Type &in, const Type &value) {
		if (m_err != NO_ERROR)
			in = value;
	}
};

///========================================================================================== WinMem
namespace	WinMem {
	inline PVOID				Alloc(size_t size) {
		return	::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
	}
	inline PVOID				Realloc(PCVOID in, size_t size) {
		return	::HeapReAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, (PVOID)in, size);
	}
	inline void					Free(PCVOID in) {
		::HeapFree(::GetProcessHeap(), 0, (PVOID)in);
	}
	inline size_t				Size(PCVOID in) {
		return	::HeapSize(::GetProcessHeap(), 0, in);
	}

	inline PVOID				Copy(PVOID dest, PCVOID sour, size_t size) {
#ifdef DynamicLink
		dl_memcpy ProcAdd = (dl_memcpy) ::GetProcAddress(DynLink::hNtDll, "memcpy");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load memcpy", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(dest, sour, size) : NULL;
#else
		return	::CopyMemory(dest, sour, size);
#endif
	}
	inline PVOID				Fill(PVOID in, size_t size, char fill) {
#ifdef DynamicLink
		dl_memset ProcAdd = (dl_memset) ::GetProcAddress(DynLink::hNtDll, "memset");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load _memset", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in, fill, size) : NULL;
#else
		return	::FillMemory(in, size, (BYTE)fill);
#endif
	}
	inline void					Zero(PVOID in, size_t size) {
		Fill(in, size, 0);
	}
	template<typename Type>
	inline void					Fill(Type &in, char fill) {
		Fill(&in, sizeof(in), fill);
	}
	template<typename Type>
	inline void					Zero(Type &in) {
		Fill(&in, sizeof(in), 0);
	}
}

///========================================================================================== WinStr
namespace	WinStr {
	inline size_t				Len(PCSTR in) {
		return	::strlen(in);
	}
	inline size_t				Len(PCWSTR in) {
#ifdef DynamicLink
		dl_wcslen ProcAdd = (dl_wcslen) ::GetProcAddress(DynLink::hNtDll, "wcslen");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load wcslen", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in) : 0;
#else
		return	::wcslen(in);
#endif
	}
	inline int					Cmp(PCSTR in1, PCSTR in2) {
		return	::strcmp(in1, in2);
	}
	inline int					Cmp(PCWSTR in1, PCWSTR in2) {
#ifdef DynamicLink
		dl_wcscmp ProcAdd = (dl_wcscmp) ::GetProcAddress(DynLink::hNtDll, "wcscmp");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load _wcscmp", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in1, in2) : 0;
#else
		return	::wcscmp(in1, in2);
#endif
	}
	inline int					Cmpi(PCSTR in1, PCSTR in2) {
		return	::_stricmp(in1, in2);
	}
	inline int					Cmpi(PCWSTR in1, PCWSTR in2) {
#ifdef DynamicLink
		dl__wcsicmp ProcAdd = (dl__wcsicmp) ::GetProcAddress(DynLink::hNtDll, "_wcsicmp");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load _wcsicmp", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in1, in2) : 0;
#else
		return	::_wcsicmp(in1, in2);
#endif
	}
	inline bool					Eq(PCSTR in1, PCSTR in2) {
		return	Cmp(in1, in2) == 0;
	}
	inline bool					Eq(PCWSTR in1, PCWSTR in2) {
		return	Cmp(in1, in2) == 0;
	}
	inline bool					Eqi(PCSTR in1, PCSTR in2) {
		return	Cmpi(in1, in2) == 0;
	}
	inline bool					Eqi(PCWSTR in1, PCWSTR in2) {
		return	Cmpi(in1, in2) == 0;
	}
	inline PSTR					Copy(PSTR dest, PCSTR src) {
		return	::strcpy(dest, src);
	}
	inline PWSTR				Copy(PWSTR dest, PCWSTR src) {
#ifdef DynamicLink
		dl_wcscpy ProcAdd = (dl_wcscpy) ::GetProcAddress(DynLink::hNtDll, "wcscpy");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load wcscpy", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(dest, src) : (PWSTR)L"";
#else
		return	::wcscpy(dest, src);
#endif
	}
	inline PSTR					Copy(PSTR dest, PCSTR src, size_t size) {
		return	::strncpy(dest, src, size);
	}
	inline PWSTR				Copy(PWSTR dest, PCWSTR src, size_t size) {
#ifdef DynamicLink
		dl_wcsncpy ProcAdd = (dl_wcsncpy) ::GetProcAddress(DynLink::hNtDll, "wcsncpy");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load wcsncpy", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(dest, src, size) : (PWSTR)L"";
#else
		return	::wcsncpy(dest, src, size);
#endif
	}

	inline PCWSTR				Assign(PCWSTR &dest, PCWSTR src) {
		dest = (PCWSTR)WinMem::Alloc((WinStr::Len(src) + 1) * sizeof(WCHAR));
		WinStr::Copy((PWSTR)dest, src);
		return	dest;
	}
	inline void					Free(PCWSTR in) {
		return	WinMem::Free(in);
	}

	inline PCSTR				CharFirst(PCSTR in, CHAR ch) {
		return	::strchr(in, ch);
	}
	inline PCWSTR				CharFirst(PCWSTR in, WCHAR ch) {
		return	::wcschr(in, ch);
	}
	inline PSTR					CharFirst(PSTR in, CHAR ch) {
		return	::strchr(in, ch);
	}
	inline PWSTR				CharFirst(PWSTR in, WCHAR ch) {
		return	::wcschr(in, ch);
	}

	inline PCSTR				CharLast(PCSTR in, CHAR ch) {
		return	::strrchr(in, ch);
	}
	inline PCWSTR				CharLast(PCWSTR in, WCHAR ch) {
		return	::wcsrchr(in, ch);
	}
	inline PSTR					CharLast(PSTR in, CHAR ch) {
		return	::strrchr(in, ch);
	}
	inline PWSTR				CharLast(PWSTR in, WCHAR ch) {
		return	::wcsrchr(in, ch);
	}

	inline PWSTR				CharFirstOf(PCWSTR in, PCWSTR mask) {
		size_t	lin = Len(in);
		size_t	len = Len(mask);
		for (size_t i = 0; i < lin; ++i) {
			for (size_t j = 0; j < len; ++j) {
				if (in[i] == mask[j])
					return	(PWSTR)&in[i];
			}
		}
		return	NULL;
	}
	inline PWSTR				CharFirstNotOf(PCWSTR in, PCWSTR mask) {
		size_t	lin = Len(in);
		size_t	len = Len(mask);
		for (size_t i = 0; i < lin; ++i) {
			for (size_t j = 0; j < len; ++j) {
				if (in[i] == mask[j])
					break;
				if (j == len - 1)
					return	(PWSTR)&in[i];
			}
		}
		return	NULL;
	}
	inline PWSTR				CharLastOf(PCWSTR in, PCWSTR mask) {
		size_t	len = Len(mask);
		for (size_t i = Len(in) - 1; i >= 0; --i) {
			for (size_t j = 0; j < len; ++j) {
				if (in[i] == mask[j])
					return	(PWSTR)&in[i];
			}
		}
		return	NULL;
	}
	inline PWSTR				CharLastNotOf(PCWSTR in, PCWSTR mask) {
		size_t	len = Len(mask);
		for (size_t i = Len(in) - 1; i >= 0; --i) {
			for (size_t j = 0; j < len; ++j) {
				if (in[i] == mask[j])
					break;
				if (j == len - 1)
					return	(PWSTR)&in[i];
			}
		}
		return	NULL;
	}

	inline PSTR					Fill(PSTR in, CHAR ch) {
		return	::_strset(in, ch);
	}
	inline PWSTR				Fill(PWSTR in, WCHAR ch) {
		return	::_wcsset(in, ch);
	}

	inline PSTR					Reverse(PSTR in) {
		return	::_strrev(in);
	}
	inline PWSTR				Reverse(PWSTR in) {
		return	::_wcsrev(in);
	}

	inline long long			AsLongLong(PCSTR in) {
#ifdef DynamicLink
		dl__atoi64 ProcAdd = (dl__atoi64) ::GetProcAddress(DynLink::hNtDll, "_atoi64");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load _atoi64", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in) : 0LL;
#else
		return	_atoi64(in);
#endif
	}
	inline unsigned long		AsULong(PCSTR in) {
		return	(unsigned long)AsLongLong(in);
	}
	inline long					AsLong(PCSTR in) {
		return	(long)AsLongLong(in);
	}
	inline unsigned int			AsUInt(PCSTR in) {
		return	(unsigned int)AsLongLong(in);
	}
	inline int					AsInt(PCSTR in) {
		return	(int)AsLongLong(in);
	}

	inline long long			AsLongLong(PCWSTR in) {
#ifdef DynamicLink
		dl__wtoi64 ProcAdd = (dl__wtoi64) ::GetProcAddress(DynLink::hNtDll, "_wtoi64");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load _wtoi64", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in) : 0LL;
#else
		return	_wtoi64(in);
#endif
	}
	inline unsigned long		AsULong(PCWSTR in) {
		return	(unsigned long)AsLongLong(in);
	}
	inline long					AsLong(PCWSTR in) {
		return	(long)AsLongLong(in);
	}
	inline unsigned int			AsUInt(PCWSTR in) {
		return	(unsigned int)AsLongLong(in);
	}
	inline int					AsInt(PCWSTR in) {
		return	(int)AsLongLong(in);
	}

	/*
		inline bool					ToFile(PCWSTR path, PCWSTR in, bool bom = true) {
			CHAR	bom16le[] = {0xFE, 0xFF, 0x0};
			DWORD	dwWritten = 0;
			HANDLE	hFile = ::CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
			if (bom)
				::WriteFile(hFile, bom16le, Len(bom16le), &dwWritten, NULL);
			::WriteFile(hFile, in, Len(in), &dwWritten, NULL);
			::CloseHandle(hFile);
			return	dwWritten != 0;
		}
	*/
}

///============================================================================================ CStr
class		CStrA {
	typedef			CHAR	cType;
	typedef			cType*	pType;
	typedef	const	cType*	pcType;
	size_t	mutable			sz_;
	pType	mutable			st_;

	void				Assign(pcType in, size_t size = 0) {
		Free();
		size = Max(size, WinStr::Len(in));
		Alloc(size);
		WinStr::Copy(st_, in, sz_);
	}

	void				Alloc(size_t size) {
		sz_ = size + 1;
		st_ = (pType)WinMem::Alloc((sz_) * sizeof(cType));
	}
	void				Realloc(size_t size) const {
		sz_ = size + 1;
		st_ = (pType)WinMem::Realloc(st_, (sz_) * sizeof(cType));
	}
	void				Free() {
		if (st_)
			WinMem::Free(st_);
		st_ = NULL;
		sz_ = 0;
	}
public:
	~CStrA() {
		Free();
	}
	CStrA(size_t size = MAX_PATH): st_(NULL) {
		Alloc(size);
	}
	CStrA(const CStrA &in): st_(NULL) {
		Assign(in.st_);
	}
	CStrA(const CStrA &in, size_t size): st_(NULL) {
		Assign(in.st_, size);
	}
	CStrA(pcType in): st_(NULL) {
		Assign(in);
	}
	CStrA(const wchar_t* in, UINT cp_ = CP_ACP): st_(NULL) {
		Alloc(WinStr::Len(in));
		cp(in, cp_);
	}
	const CStrA&		operator=(const CStrA &in) {
		Assign(in.st_, in.sz_);
		return	*this;
	}
	const CStrA&		operator=(pcType in) {
		Assign(in, WinStr::Len(in));
		return	*this;
	}

	const CStrA&		operator+=(pcType in) {
		if (st_ != in) {
			size_t len1 = WinStr::Len(st_), len2 = WinStr::Len(in);
			CheckSize(len1 + len2);
			WinStr::Copy(st_ + len1, in, sz_ - len1);
		}
		return	*this;
	}
	CStrA				operator+(const CStrA &in) const {
		CStrA Result(*this);
		return	Result += in;
	}

	bool				operator==(const CStrA &in) const {
		return	WinStr::Eq(st_, in.st_);
	}
	bool				operator!=(const CStrA &in) const {
		return	!operator==(in);
	}
	bool				operator<(const CStrA &in) const {
		return	WinStr::Cmp(st_, in.st_) < 0;
	}
	bool				operator>(const CStrA &in) const {
		return	WinStr::Cmp(st_, in.st_) > 0;
	}
	bool				operator<=(const CStrA &in) const {
		return	operator==(in) || operator<(in);
	}
	bool				operator>=(const CStrA &in) const {
		return	operator==(in) || operator>(in);
	}

	bool				CheckSize(size_t size) const {
		if (size >= sz_) {
			sz_ = size;
			Realloc(sz_);
			return	false;
		}
		return	true;
	}

	cType&				operator[](int in) {
		return	st_[in];
	}
	const cType&		operator[](int in) const {
		return	st_[in];
	}

	operator			char*() {
		return	st_;
	}
	operator			const char*() const {
		return	st_;
	}
	size_t				Len() const {
		return	WinStr::Len(st_);
	}
	size_t				Size() const {
		return	sz_;
	}
	pcType				c_str() const {
		return	(pcType)st_;
	}
	pType				Data() const {
		return	(pType)st_;
	}

	const CStrA&		cp(const wchar_t* in, UINT cp = CP_ACP) {
		CheckSize(::WideCharToMultiByte(cp, 0, in, -1, NULL, 0, NULL, NULL));
		::WideCharToMultiByte(cp, 0, in, -1, st_, sz_, NULL, NULL);
		return	*this;
	}
	const CStrA&		utf(const wchar_t* in) {
		return	cp(in, CP_UTF8);
	}
	const CStrA&		oem(const wchar_t* in) {
		return	cp(in, CP_OEMCP);
	}

};
class		CStrW {
	typedef			WCHAR	cType;
	typedef			PWSTR	pType;
	typedef			PCWSTR	pcType;
	size_t	mutable			sz_;
	pType	mutable			st_;

	void				Assign(pcType in, size_t size = 0) {
		Free();
		size = Max(size, WinStr::Len(in));
		Alloc(size);
		WinStr::Copy(st_, in, sz_);
	}

	void				Alloc(size_t size) {
		sz_ = size + 1;
		st_ = (pType)WinMem::Alloc((sz_) * sizeof(cType));
	}
	void				Realloc(size_t size) const {
		sz_ = size + 1;
		st_ = (pType)WinMem::Realloc(st_, (sz_) * sizeof(cType));
	}
	void				Free() {
		if (st_)
			WinMem::Free(st_);
		st_ = NULL;
		sz_ = 0;
	}
public:
	~CStrW() {
		Free();
	}
	CStrW(size_t size = MAX_PATH): st_(NULL) {
		Alloc(size);
	}
	CStrW(const CStrW &in): st_(NULL) {
		Assign(in.st_);
	}
	CStrW(const CStrW &in, size_t size): st_(NULL) {
		Assign(in.st_, size);
	}
	CStrW(pcType in): st_(NULL) {
		Assign(in);
	}
	CStrW(const char* in, UINT cp_ = CP_ACP): st_(NULL) {
		Alloc(WinStr::Len(in));
		cp(in, cp_);
	}
	const CStrW&		operator=(const CStrW &in) {
		Assign(in.st_, in.sz_);
		return	*this;
	}
	const CStrW&		operator=(pcType in) {
		Assign(in, WinStr::Len(in));
		return	*this;
	}

	const CStrW&		operator+=(pcType in) {
		if (st_ != in) {
			size_t len1 = WinStr::Len(st_);
			CheckSize(len1 + WinStr::Len(in));
			WinStr::Copy(st_ + len1, in, sz_ - len1);
		}
		return	*this;
	}
	CStrW				operator+(const CStrW &in) const {
		CStrW Result(*this);
		return	Result += in;
	}

	bool				operator==(const CStrW &in) const {
		return	WinStr::Eq(st_, in.st_);
	}
	bool				operator!=(const CStrW &in) const {
		return	!operator==(in);
	}
	bool				operator<(const CStrW &in) const {
		return	WinStr::Cmp(st_, in.st_) < 0;
	}
	bool				operator>(const CStrW &in) const {
		return	WinStr::Cmp(st_, in.st_) > 0;
	}
	bool				operator<=(const CStrW &in) const {
		return	operator==(in) || operator<(in);
	}
	bool				operator>=(const CStrW &in) const {
		return	operator==(in) || operator>(in);
	}

	bool				CheckSize(size_t size) const {
		if (size >= sz_) {
			sz_ = size;
			Realloc(sz_);
			return	false;
		}
		return	true;
	}

	cType&				operator[](int in) {
		return	st_[in];
	}
	const cType&		operator[](int in) const {
		return	st_[in];
	}

	operator			wchar_t*() {
		return	st_;
	}
	operator			const wchar_t*() const {
		return	st_;
	}
	size_t				Len() const {
		return	WinStr::Len(st_);
	}
	size_t				Size() const {
		return	sz_;
	}
	pcType				c_str() const {
		return	(pcType)st_;
	}
	pType				Data() const {
		return	(pType)st_;
	}

	const CStrW&		cp(const char* in, UINT cp = CP_ACP) {
		CheckSize(::MultiByteToWideChar(cp, 0, in, -1, NULL, 0));
		::MultiByteToWideChar(cp, 0, in, -1, st_, sz_);
		return	*this;
	}
	const CStrW&		utf(const char* in) {
		return	cp(in, CP_UTF8);
	}
	const CStrW&		oem(const char* in) {
		return	cp(in, CP_OEMCP);
	}
};

#ifdef UNICODE
typedef	CStrW	CStr;
#else
typedef	CStrA	CStr;
#endif

template<typename Type>
class		WinBuffer {
	size_t	m_ctr;
	Type	m_buf;
	bool				Free() {
		if (--m_ctr == 0) {
			WinMem::Free(m_buf);
			m_buf = NULL;
		}
		return	true;
	}
public:
	~WinBuffer() {
		Free();
	}
	WinBuffer(size_t size): m_ctr(0), m_buf(NULL) {
		m_buf = (Type)WinMem::Alloc(size);
		if (m_buf)
			++m_ctr;
	}
	operator			Type() {
		return	m_buf;
	}
	Type				operator->() const {
		return	m_buf;
	}
	const WinBuffer&	operator=(const WinBuffer &in) {
		if (this != &in) {
			Free();
			size_t	size = in.Size();
			m_buf = (Type)WinMem::Alloc(size);
			if (m_buf) {
				WinMem::Copy(m_buf, in.m_buf, size);
				m_ctr = 1;
			}
		}
		return	*this;
	}
	size_t				Size() {
		return	WinMem::Size(m_buf);
	}
	void				Inc() {
		++m_ctr;
	}
	void				Dec() {
		Free();
	}
	Type				Data() const {
		return	m_buf;
	}
};

class		BOM {
public:
	static	PCSTR	utf8() {
		static	const char bom_utf8[] = {'\xEF', '\xBB', '\xBF', '\xAA'};
		return	bom_utf8;
	}
	static	PCSTR	utf16le() {
		static	const char bom_utf16le[] = {'\xFF', '\xFE', '\xAA'};
		return	bom_utf16le;
	}
	static	PCSTR	utf16be() {
		static	const char bom_utf16be[] = {'\xFE', '\xFF', '\xAA'};
		return	bom_utf16be;
	}
	static	PCSTR	utf32le() {
		static	const char bom_utf32le[] = {'\xFF', '\xFE', '\0', '\0', '\xAA'};
		return	bom_utf32le;
	}
	static	PCSTR	utf32be() {
		static	const char bom_utf32be[] = {'\0', '\0', '\xFE', '\xFF', '\xAA'};
		return	bom_utf32be;
	}
	/*
	00 00 FE FF	UTF-32, big-endian
	FF FE 00 00	UTF-32, little-endian
	FE FF	UTF-16, big-endian
	FF FE	UTF-16, little-endian
	EF BB BF	UTF-8
	*/
};

///========================================================================================= WinFlag
template<typename Type>
class		WinFlag {
public:
	static bool			Check(Type in, Type flag) {
		return	flag == (in & flag);
	}
	static Type&		Set(Type &in, Type flag) {
		return	in |= flag;
	}
	static Type&		UnSet(Type &in, Type flag) {
		return	in &= ~flag;
	}
};

///========================================================================================= WinFlag
template<typename Type>
class		WinBit {
	static size_t		BIT_LIMIT() {
		return	sizeof(Type) * 8;
	}
	static bool			BadBit(size_t in) {
		return	(in < 0) || (in >= BIT_LIMIT());
	}
public:
	static bool			Check(Type in, size_t bit) {
		if (BadBit(bit))
			return	false;
		Type tmp = 1;
		tmp <<= bit;
		return	(in & tmp);
	}
	static Type&		Set(Type &in, size_t bit) {
		if (BadBit(bit))
			return	in;
		Type	tmp = 1;
		tmp <<= bit;
		in |= tmp;
		return	in;
	}
	static Type&		UnSet(Type &in, size_t bit) {
		if (BadBit(bit))
			return	in;
		Type	tmp = 1;
		tmp <<= bit;
		in &= ~tmp;
		return	in;
	}
};

///======================================================================================== WinTimer
class		WinTimer {
	HANDLE			hTimer;
	LARGE_INTEGER	liUTC;
	long			lPeriod;

	void			Open() {
		Close();
		hTimer = ::CreateWaitableTimer(NULL, false, NULL);
	}
	void			Close() {
		if (hTimer) {
			::CloseHandle(hTimer);
			hTimer = NULL;
		}
	}
public:
	~WinTimer() {
		Stop();
		Close();
	}
	WinTimer(): hTimer(NULL) {
		Open();
		liUTC.QuadPart = 0LL;
		lPeriod = 0L;
	}
	WinTimer(LONGLONG time, long period = 0): hTimer(NULL) {
		Open();
		Set(time, period);
	}
	void			Set(LONGLONG time, long period = 0) {
		if (time == 0LL) {
			liUTC.QuadPart = -period * 10000LL;
		} else {
			liUTC.QuadPart = time;
		}
		lPeriod = period;
	}
	void			Start() {
		if (hTimer)
			::SetWaitableTimer(hTimer, &liUTC, lPeriod, NULL, NULL, false);
	}
	void			Stop() {
		if (hTimer)
			::CancelWaitableTimer(hTimer);
	}
	operator		HANDLE() const {
		return	hTimer;
	}
	void			StartTimer() {
		// объявляем свои локальные переменные
		FILETIME ftLocal, ftUTC;
		LARGE_INTEGER liUTC;

// таймер должен сработать в первый раз 1 января 2002 года в 1:00 PM но местному времени
		SYSTEMTIME st;
		::GetSystemTime(&st);
//		st.wOayOfWeek = 0;			// игнорируется
//		st.wHour = 0;				// 0 PM
//		st.wMinute = 0;				// 0 минут
		st.wSecond = 0;				// 0 секунд
		st.wMilliseconds = 0;		// 0 миллисекунд
		::SystemTimeToFileTime(&st, &ftLocal);
//		::LocalFileTimeToFilelime(&ttLocal, &ftUTC);

// преобразуем FILETIME в LARGE_INTEGER из-за различий в выравнивании данных
		liUTC.LowPart = ftUTC.dwLowDateTime;
		liUTC.HighPart = ftUTC.dwHighDateTime;
	}
};

///========================================================================================== WinEnv
namespace	WinEnv {
	inline CStrW	Get(PCWSTR name) {
		CStrW	buf(::GetEnvironmentVariable(name, NULL, 0));
		::GetEnvironmentVariable(name, buf, buf.Size());
		return	buf;
	}
	inline bool		Set(PCWSTR name, PCWSTR val) {
		return	::SetEnvironmentVariable(name, val);
	}
	inline bool		Add(PCWSTR name, PCWSTR val) {
		CStrW	buf(::GetEnvironmentVariable(name, NULL, 0) + WinStr::Len(val));
		::GetEnvironmentVariable(name, buf, buf.Size());
		buf += val;
		return	::SetEnvironmentVariable(name, buf.c_str());
	}
	inline bool		Del(PCWSTR name) {
		return	::SetEnvironmentVariable(name, NULL);
	}
}

///=========================================================================================== WinFS
class		WinFilePos {
	LARGE_INTEGER	m_pos;
public:
	WinFilePos(LONGLONG pos = 0LL) {
		m_pos.QuadPart = pos;
	}
	operator		LONGLONG() const {
		return	m_pos.QuadPart;
	}
	operator		LARGE_INTEGER() const {
		return	m_pos;
	}
	operator		PLARGE_INTEGER() const {
		return	(PLARGE_INTEGER)&m_pos;
	}
};

namespace	WinFS {
	typedef		WIN32_FILE_ATTRIBUTE_DATA	FileInfo;
	inline bool		ValidName(PCWSTR path) {
		return	!(WinStr::Eq(path, L".") || WinStr::Eq(path, L"..") || WinStr::Eq(path, L"..."));
	}
	inline bool		IsExist(PCWSTR	path) {
		::GetFileAttributesW(path);
		return	::GetLastError() != ERROR_FILE_NOT_FOUND;
	}
	inline bool		IsDir(PCWSTR	path) {
		return	::GetFileAttributesW(path) & FILE_ATTRIBUTE_DIRECTORY;
	}
	inline bool		IsJunc(PCWSTR	path) {
		return	::GetFileAttributesW(path) & FILE_ATTRIBUTE_REPARSE_POINT;
	}

	inline bool		Expand(PCWSTR path, CStrW&	buf) {
		buf.CheckSize(::ExpandEnvironmentStringsW(path, NULL, 0));
		return	::ExpandEnvironmentStringsW(path, buf, buf.Size());
	}
	inline DWORD	GetAttr(PCWSTR	path) {
		return	::GetFileAttributesW(path);
	}
	inline FileInfo	GetInfo(PCWSTR	path) {
		FileInfo	info;
		::GetFileAttributesEx(path, GetFileExInfoStandard, &info);
		return	info;
	}
	inline LONGLONG	GetSize(PCWSTR	path) {
		FileInfo	info;
		::GetFileAttributesEx(path, GetFileExInfoStandard, &info);
		return	((LONGLONG)info.nFileSizeHigh) << 32 | info.nFileSizeLow;
	}
	inline LONGLONG	GetSize(HANDLE	hFile) {
		LARGE_INTEGER	size;
		::GetFileSizeEx(hFile, &size);
		return	size.QuadPart;
	}
	inline FILETIME	GetTimeCreate(PCWSTR	path) {
		FileInfo	info;
		::GetFileAttributesEx(path, GetFileExInfoStandard, &info);
		return	info.ftCreationTime;
	}
	inline FILETIME	GetTimeAccess(PCWSTR	path) {
		FileInfo	info;
		::GetFileAttributesEx(path, GetFileExInfoStandard, &info);
		return	info.ftLastAccessTime;
	}
	inline FILETIME	GetTimeWrite(PCWSTR	path) {
		FileInfo	info;
		::GetFileAttributesEx(path, GetFileExInfoStandard, &info);
		return	info.ftLastWriteTime;
	}

	inline bool		SetAttr(PCWSTR	path, DWORD attr) {
		return	::SetFileAttributes(path, attr);
	}

	inline bool		HardLink(PCWSTR path, PCWSTR newfile) {
		return	::CreateHardLinkW(newfile, path, NULL);
	}
	inline bool		Copy(PCWSTR path, PCWSTR dest) {
		return	::CopyFileW(path, dest, true);
	}
	inline bool		Move(PCWSTR path, PCWSTR dest) {
		return	::MoveFileExW(path, dest, MOVEFILE_COPY_ALLOWED);
	}
	inline bool		DelDir(PCWSTR	path) {
		::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL);
		return	::RemoveDirectoryW(path);
	}
	inline bool		DelFile(PCWSTR	path) {
		::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL);
		return	::DeleteFileW(path);
	}

	inline bool		FileOpenRead(PCWSTR	path, HANDLE &hFile) {
		hFile = ::CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
							  FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		return	hFile && hFile != INVALID_HANDLE_VALUE;
	}
	inline bool		FileClose(HANDLE hFile) {
		return	::CloseHandle(hFile);
	}
	inline bool		FilePos(HANDLE hFile, const WinFilePos &pos, DWORD m = FILE_BEGIN) {
		return	::SetFilePointerEx(hFile, pos, NULL, m);
	}
	inline LONGLONG	FilePos(HANDLE hFile) {
		WinFilePos	pos;
		return	::SetFilePointerEx(hFile, WinFilePos(0LL), pos, FILE_CURRENT);
	}
	inline bool		FileRead(HANDLE hFile, PBYTE buf, DWORD &size) {
		return	::ReadFile(hFile, buf, size, &size, NULL);
	}
	inline bool		FileRead(PCWSTR	path, CStrA &buf) {
		bool	Result = false;
		HANDLE	hFile = ::CreateFileW(path, GENERIC_READ, 0, NULL, OPEN_EXISTING,
									 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			DWORD	size = GetSize(hFile);
			buf.CheckSize(size);
			Result = ::ReadFile(hFile, buf, buf.Size() - 1, &size, NULL);
			::CloseHandle(hFile);
		}
		return	Result;
	}
	inline bool		FileWrite(PCWSTR	path, const CStrA &buf) {
		bool	Result = false;
		HANDLE	hFile = ::CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
									 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			DWORD	written;
			Result = ::WriteFile(hFile, buf, buf.Len(), &written, NULL);
			::CloseHandle(hFile);
		}
		return	Result;
	}
}

///========================================================================================== WinReg
/*

class	WinReg {
	HKEY	mutable	hKeyOpend;
	HKEY			hKeyReq;
	CStrW			sPath;

	void			CloseKey() const {
		if (hKeyOpend) {
			::RegCloseKey(hKeyOpend);
			hKeyOpend = 0;
		}
	}
	bool			OpenKey(UINT access) const {
		return	OpenKey(hKeyReq, sPath, access);
	}
	bool			OpenKey(HKEY hkey, PCWSTR path, UINT access) const {
		CloseKey();
		bool	Result = false;
		if (access == KEY_READ)
			Result = ::RegOpenKeyEx(hkey, path, 0, access, &hKeyOpend) == ERROR_SUCCESS;
		else
			Result = ::RegCreateKeyEx(hkey, path, 0, NULL, 0, access, 0, &hKeyOpend, 0) == ERROR_SUCCESS;
		return	Result;
	}

	template <typename Type>
	void			SetRaw(PCWSTR name, const Type &value) const {
		if (OpenKey(KEY_WRITE)) {
			::RegSetValueEx(hKeyOpend, name, NULL, REG_BINARY, (PBYTE)(&value), sizeof(Type));
			CloseKey();
		}
	}
	template <typename Type>
	bool			GetRaw(PCWSTR name, Type &value) const {
		bool	Result = OpenKey(KEY_READ);
		if (Result) {
			DWORD len = sizeof(Type);
			Result = ::RegQueryValueEx(hKeyOpend, name, NULL, NULL, (PBYTE)(&value), &len) == ERROR_SUCCESS;
			CloseKey();
		}
		return	Result;
	}
public:
	~WinReg() {
		CloseKey();
	}
	WinReg(): hKeyOpend(0), hKeyReq(HKEY_CURRENT_USER), sPath(TEXT("")) {
	}
	WinReg(PCWSTR path): hKeyOpend(0), hKeyReq(0), sPath(path) {
		hKeyReq = HKEY_CURRENT_USER;
		stringt	tmp = TEXT("HKEY_CURRENT_USER\\");
		if (StrUtil::Find(sPath, tmp)) {
			StrUtil::Cut(sPath, tmp);
			return;
		}
		tmp = TEXT("HKCU\\");
		if (StrUtil::Find(sPath, tmp)) {
			StrUtil::Cut(sPath, tmp);
			return;
		}
		tmp = TEXT("HKEY_LOCAL_MACHINE\\");
		if (StrUtil::Find(sPath, tmp)) {
			StrUtil::Cut(sPath, tmp);
			hKeyReq = HKEY_LOCAL_MACHINE;
			return;
		}
		tmp = TEXT("HKLM\\");
		if (StrUtil::Find(sPath, tmp)) {
			StrUtil::Cut(sPath, tmp);
			hKeyReq = HKEY_LOCAL_MACHINE;
			return;
		}
		tmp = TEXT("HKEY_USERS\\");
		if (StrUtil::Find(sPath, tmp)) {
			StrUtil::Cut(sPath, tmp);
			hKeyReq = HKEY_USERS;
			return;
		}
		tmp = TEXT("HKU\\");
		if (StrUtil::Find(sPath, tmp)) {
			StrUtil::Cut(sPath, tmp);
			hKeyReq = HKEY_USERS;
			return;
		}
		tmp = TEXT("HKEY_CLASSES_ROOT\\");
		if (StrUtil::Find(sPath, tmp)) {
			StrUtil::Cut(sPath, tmp);
			hKeyReq = HKEY_CLASSES_ROOT;
			return;
		}
		tmp = TEXT("HKCR\\");
		if (StrUtil::Find(sPath, tmp)) {
			StrUtil::Cut(sPath, tmp);
			hKeyReq = HKEY_CLASSES_ROOT;
			return;
		}
	}
	WinReg(HKEY hkey, PCWSTR path): hKeyOpend(0), hKeyReq(hkey), sPath(path) {
	}

	void			SetPath(PCWSTR path) {
		sPath = path;
	}
	void			SetKey(HKEY hkey) {
		hKeyReq = hkey;
	}

	bool			Add(PCWSTR name) const {
		bool	Result = OpenKey(KEY_WRITE);
		if (Result) {
			HKEY tmp = 0;
			Result = (::RegCreateKey(hKeyOpend, name, &tmp) == ERROR_SUCCESS);
			if (Result) {
				::RegCloseKey(tmp);
			}
			CloseKey();
		}
		return	Result;
	}
	bool			Del(PCWSTR name) const {
		bool	Result = OpenKey(KEY_WRITE);
		if (Result) {
			Result = (::RegDeleteValue(hKeyOpend, name) == ERROR_SUCCESS);
			CloseKey();
		}
		return	Result;
	}

	void			Set(PCWSTR name, PCWSTR value) const {
		if (OpenKey(KEY_WRITE)) {
			::RegSetValueEx(hKeyOpend, name, NULL, REG_SZ, (PBYTE)value, WinStr::Len(value) * sizeof(WCHAR));
			CloseKey();
		}
	}

	CStrW			Get(PCWSTR name) const {
		CStrW	buf(4096);
		Get(name, buf, L"");
		return	buf;
	}
	CStrW			Get(PCWSTR name, PCWSTR def) const {
		CStrW	buf(4096);
		Get(name, buf, def);
		return	buf;
	}
	bool			Get(PCWSTR name, CStrW &value, PCWSTR def) const {
		bool	Result = OpenKey(KEY_READ);
		value = def;
		if (Result) {
			CStr	buf(4096);
			DWORD	size = buf.Size();
			DWORD	type = 0;
			if (::RegQueryValueEx(hKeyOpend, name, NULL, &type, (PBYTE)(buf.Data()), &size) == ERROR_SUCCESS) {
				switch (type) {
					case REG_DWORD:
						value = n2s(*(PDWORD)buf.Data());
						break;
					case REG_QWORD:
						value = n2s(*(LONGLONG*)buf.Data());
						break;
					default:
						value = buf;
				}
				Result = true;
			}
			CloseKey();
		}
		return	Result;
	}

	CStrW			GetPath() const {
		return	sPath;
	}
};

*/
///========================================================================================== WinCom
class		WinCOM {
	bool		m_err;

	WinCOM(): m_err(true) {
		if (m_err) {
			if (SUCCEEDED(::CoInitializeEx(NULL, COINIT_MULTITHREADED)))
				m_err = false;
		}
	}
	WinCOM(const WinCOM&);
public:
	~WinCOM() {
		Close();
	}
	static WinCOM&	the() {
		static WinCOM com;
		return	com;
	}
	static bool		Init() {
		return	the().IsOK();
	}
	static bool		Close() {
		if (!the().m_err) {
			::CoUninitialize();
			the().m_err = true;
		}
		return	true;
	}
	static bool		IsOK() {
		return	!the().m_err;
	}
};

///============================================================================================ Item
template<typename Type>
class		WinItem {		// Item of the collection. Each entry it of type Item
	Type		m_data;
	WinItem*	m_next;
public:
	~WinItem() {
	}
	WinItem(const Type &data, WinItem* next): m_data(data), m_next(next) {
	}
	Type&		data() {
		return	m_data;
	}
	Type*		next() {
		return	m_next;
	}
};

template<typename Type>
class		WinStack {
	WinItem<Type>*	head;
public:
	~WinStack() {
		while (head)
			pop();
	}
	WinStack(): head(NULL) {
	}
	void		push(const Type &in) {
		head = new WinItem<Type>(in, head);
	}
	Type		pop() {
		Type	Result = head->data();
		WinItem<Type>*	tmp = head;
		head = head->next();
		delete	tmp;
		return	Result;
	}
};

#endif // WIN_DEF_HPP
