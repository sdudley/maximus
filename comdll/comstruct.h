/** Communications handle. Completes foward declaration in ntcomm.h.
 *  Fields below handleType are private to the module. Fields above
 *  (and including) handleType are "public" to the com module stub.
 */

typedef void * commHandle_t;

struct _hcomm
{
  /****** Public members available to *any* COMMAPI driver */
  COMMHANDLE            h;                      /**< "Windows" COMMHANDLE -- contains UNIX fd */   
  BOOL                  burstMode;              /**< FALSE = Nagle algorithm enabled (TCP_NODELAY) */
  const char            *device;                /**< Name of tcp service or port number (ASCIZ) */
  BOOL                  fDCD;                   /**< Current status of DCD */
  commHandle_t          handleType;             /**< Type of handle / dl_open stub */
  COMMTIMEOUTS          ct;                     /**< Timeout values */

  /****** Private members for this file below */
  struct sockaddr_un    *saddr_p;               /**< Address we're bound to andlistening on or NULL */
  int                   listenfd;               /**< File descriptor for saddr_p  or -1 */
  signed int            peekHack;               /**< Character we've ComPeek()ed but not ComRead(); or -1 */
  BOOL                  burstModePending;       /**< Next write's burst mode */
  size_t                txBufSize;              /**< Size of the transmit buffer */
#ifdef TELNET
  telnet_moption_t      telnetPendingOptions;   /**< Unprocessed option requests from remote */
  telnet_moption_t      telnetOptions;          /**< Current telnet options (bitmask) */
#endif
} _hcomm;


