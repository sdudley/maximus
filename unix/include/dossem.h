#ifndef _WES_DOSSEM_H
#define _WES_DOSSEM_H
struct _hsem;
typedef struct _hsem * HSEM;
typedef struct _hsem * HSYSSEM;
typedef struct _hsem ** PHSEM;
typedef struct _hsem * HMTX;
typedef struct _hsem ** PHMTX;

#define CSEM_NONE		0
#define CSEM_PUBLIC		1 << 0

#define SEM_IMMEDIATE_RETURN	0
#define SEM_INDEFINITE_WAIT	-1

/* WARP return values, hope they're okay from 1.x functions */
#define NO_ERROR		0
#define ERROR_ALREADY_EXISTS    1 /* 'wrong' value */
#define ERROR_INVALID_HANDLE	6
#define ERROR_NOT_ENOUGH_MEMORY 8
#define ERROR_INVALID_PARAMETER	87
#define ERROR_INTERRUPT		95
#define ERROR_TOO_MANY_SEMAPHORES 100
#define ERROR_TOO_MANY_SEM_REQUESTS 103
#define ERROR_SEM_OWNER_DIED	105
#define ERROR_INVALID_NAME	123
#define ERROR_SEM_NOT_FOUND	187
#define ERROR_DUPLICATE_HANDLE	284
#define ERROR_DUPLICATE_NAME	285
#define ERROR_TOO_MANY_HANDLES	290
#define ERROR_TOO_MANY_OPENS	291	
#define ERROR_WRONG_TYPE	292	
#define ERROR_TIMEOUT		640

#define DC_NONE			0
#define DC_SEM_SHARED		1 << 0

int DosCreateSem(int NoExclusive, PHSEM SemHandle, const char *SemName);
int DosOpenSem(PHSEM SemHandle, const char *SemName);
int DosSemRequest(HSEM SemHandle, int how);
int DosSemClear(HSEM SemHandle);
int DosCloseSem(HSEM SemHandle);
int DosSemWait(HSEM SemHandle, int how);
int DosCreateMutexSem(const char *SemName, PHMTX hmtx_p, int AttributeFlags, int InitialState);
int DosRequestMutexSem(HMTX hmtx, int how);
#endif
