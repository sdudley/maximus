#ifdef DeclareCommand
/* Possible Telnet Commands
**
** Note: format strings should be
**       - int, float, string types
**       - usable by either scanf or printf
*/

DeclareCommand(SE,		240,	"End of subnegotiation parameters")
DeclareCommand(NOP,		241,	"No operation")
DeclareCommand(DM,		242,	"Data mark")
DeclareCommand(BRK,		243,	"Break")
DeclareCommand(IP,		244,	"Interrupt")
DeclareCommand(AO,		245,	"Abort Output")
DeclareCommand(AYT,		246,	"Are you there?")
DeclareCommand(EC,		247,	"Erase Character")
DeclareCommand(EL,		248,	"Erase Line")
DeclareCommand(GA,		249,	"Go Ahead")
DeclareCommand(SB,		250,	"Subnegotiation Follows")
DeclareCommand(WILL,		251,	"Will ")
DeclareCommand(WONT,		252,	"Will not")
DeclareCommand(DO,		253,	"Please do")
DeclareCommand(DONT,		254,	"Please do not")
DeclareCommand(IAC,		255,	"Interpret as Command")
#endif

#ifdef DeclareOption
DeclareOption(TRANSMIT_BINARY,	0,	0,	"Transmit Binary")
DeclareOption(ECHO,		1,	1,	"echo")
DeclareOption(SGA,		3,	2,	"suppress go ahead")
DeclareOption(STATUS,		5,	3,	"status")
DeclareOption(TIMING,		6,	4,	"timing mark")
DeclareOption(TERMTYPE,		24,	5,	"terminal type")
DeclareOption(NAWS,		31,	6,	"window size")
DeclareOption(TERMSPEED,	32,	7,	"terminal speed")
DeclareOption(FLOW, 		33,	8,	"remote flow control")
DeclareOption(LINEMODE,		34,	9,	"line mode")
DeclareOption(ENVIRON,		36,	10,	"environment variables")
#endif

#if !defined(DeclareCommand) && !defined(DeclareOption)
typedef enum
{
#define DeclareCommand(a,b,c)	cmd_ ## a = b,
#include __FILE__
#undef DeclareCommand
} telnet_command_t;

#if 0
static struct 
{
  telnet_command_t	cmdEnum;
  const char		*commandName;
} telnet_Commands[]=
{
#define DeclareCommand(a,b,c)		{ cmd_ ## a, c },
#include __FILE__
#undef DeclareCommand
};
#endif

typedef enum
{
#define DeclareOption(a,b,c,d)	mopt_ ## a = 1 << c,
#include __FILE__
#undef DeclareOption
} telnet_moption_t;

typedef enum
{
#define DeclareOption(a,b,c,d)	opt_ ## a = b,
#include __FILE__
#undef DeclareOption
} telnet_option_t;

static struct telnet_option_info_t
{
  telnet_option_t	optEnum;
  telnet_moption_t	optBit;
  const char		*optionName;
  char			optionValue;
} telnet_OptionList[]=
{
#define DeclareOption(a,b,c,d)	{ opt_ ## a, 1 << c, d, b },
#include __FILE__
#undef DeclareOption
};

#endif /* top-level include */
