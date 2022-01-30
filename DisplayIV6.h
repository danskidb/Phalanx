#include "DisplayBase.h"

// Time position for when we don't display seconds.
// Rotating between these options gives the components some downtime and should extend lifetime.
enum TimePosition {
    Middle,
    Left,
    Right,
    Split,
    MAX
};

/*
    DisplayIV6 - Phalanx
    - IV-6 is a seven-segment display with dot, so fits perfectly in a byte. Last byte is the dot.
    - 6 tubes.
*/
class DisplayIV6 : public DisplayBase
{
protected:
    const int Digits = 6;

    TimePosition currentTimePosition = TimePosition::Middle;    // for when there's less numbers on screen than there are tubes.
    int lastHour = -1;
    int shiftOutIndex = 0;
    volatile byte displayData[6];

public:
    bool Initialize();
    void IRAM_ATTR OnTick();
    void ShiftCurrentTimeFull(int hour, int minute, int second, bool displayZeroFirstDigit);
    void ShiftCurrentTime(int hour, int minute, int second, bool displayZeroFirstDigit);
    void ShiftRaw(byte data[]);
    void ShiftText(String text);
    void ShiftBlank();

private:
    void InternalShiftTimeComponent(int number, bool displayZeroFirstDigit, bool dotOnSecondDigit);
    void InternalShiftOut(byte data);   // emulates shiftOut() with shiftOutIndex on displayData.
};

static byte TubeDigit[10] = {
    B11111100, // 0
    B01100000, // 1
    B11011010, // 2
    B11110010, // 3
    B01100110, // 4
    B10110110, // 5
    B10111110, // 6
    B11100000, // 7
    B11111110, // 8
    B11110110  // 9
};

static byte CharMap[128] = {
    B00000000, //	0	NUL
    B00000000, //	1	SOH
    B00000000, //	2	STX
    B00000000, //	3	ETX
    B00000000, //	4	EOT
    B00000000, //	5	ENQ
    B00000000, //	6	ACK
    B00000000, //	7	BEL
    B00000000, //	8	BS
    B00000000, //	9	HT
    B00000000, //	10	LF
    B00000000, //	11	VT
    B00000000, //	12	FF
    B00000000, //	13	CR
    B00000000, //	14	SO
    B00000000, //	15	SI
    B00000000, //	16	DLE
    B00000000, //	17	DC1
    B00000000, //	18	DC2
    B00000000, //	19	DC3
    B00000000, //	20	DC4
    B00000000, //	21	NAK
    B00000000, //	22	SYN
    B00000000, //	23	ETB
    B00000000, //	24	CAN
    B00000000, //	25	EM
    B00000000, //	26	SUB
    B00000000, //	27	ESC
    B00000000, //	28	FS
    B00000000, //	29	GS
    B00000000, //	30	RS
    B00000000, //	31	US
    B00000000, //	32	
    B01100001, //	33	!
    B00000000, //	34	"
    B00000000, //	35	#
    B00000000, //	36	$
    B00000000, //	37	%
    B00000000, //	38	&
    B00000000, //	39	'
    B11110000, //	40	(
    B10011100, //	41	)
    B00000000, //	42	*
    B00000000, //	43	+
    B00000001, //	44	,
    B00000010, //	45	-
    B00000001, //	46	.
    B00000000, //	47	/
    B11111100, //	48	0
    B01100000, //	49	1
    B11011010, //	50	2
    B11110010, //	51	3
    B01100110, //	52	4
    B10110110, //	53	5
    B10111110, //	54	6
    B11100000, //	55	7
    B11111110, //	56	8
    B11110110, //	57	9
    B00000000, //	58	:
    B00000000, //	59	;
    B00000000, //	60	<
    B00010010, //	61	=
    B00000000, //	62	>
    B11100001, //	63	?
    B00000000, //	64	@
    B11101110, //	65	A
    B00111110, //	66	B
    B10011100, //	67	C
    B11110010, //	68	D
    B10011110, //	69	E
    B10001110, //	70	F
    B11110110, //	71	G
    B01101110, //	72	H
    B11000000, //	73	I
    B11100000, //	74	J
    B00101110, //	75	K
    B00011100, //	76	L
    B11101100, //	77	M
    B00101010, //	78	N
    B11111100, //	79	O
    B11001110, //	80	P
    B11100110, //	81	Q
    B00001010, //	82	R
    B10110110, //	83	S
    B00011110, //	84	T
    B01111100, //	85	U
    B01111100, //	86	V
    B01111100, //	87	W
    B00110000, //	88	X
    B01110110, //	89	Y
    B11011010, //	90	Z
    B11110000, //	91	[
    B00000000, //	92	backslash
    B10011100, //	93	]
    B00000000, //	94	^
    B00010000, //	95	_
    B00000000, //	96	`
    B11101110, //	97	a
    B00111110, //	98	b
    B00011010, //	99	c
    B11110010, //	100	d
    B11011110, //	101	e
    B10001110, //	102	f
    B11110110, //	103	g
    B00101110, //	104	h
    B01000000, //	105	i
    B01100000, //	106	j
    B00101110, //	107	k
    B00011100, //	108	l
    B11101100, //	109	m
    B00101010, //	110	n
    B00111010, //	111	o
    B11001110, //	112	p
    B11100110, //	113	q
    B00001010, //	114	r
    B10110110, //	115	s
    B00011110, //	116	t
    B00111000, //	117	u
    B00111000, //	118	v
    B01111100, //	119	w
    B00110000, //	120	x
    B01110110, //	121	y
    B11011010, //	122	z
    B11110000, //	123	{
    B11000000, //	124	|
    B10011100, //	125	}
    B00000000, //	126	~
    B00000000, //	127	DEL
};