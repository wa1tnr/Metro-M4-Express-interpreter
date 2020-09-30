// Wed Sep 30 10:26:45 UTC 2020
// 4737-a3a-0ca-

/* Tiny interpreter,
   similar to myforth's Standalone Interpreter
   This example code is in the public domain */

typedef void (*func)(void);         // signature of functions in dictionary

/* Structure of a dictionary entry */
typedef struct {
  const char*    name;              // Pointer the Word Name in flash
  const func     function;          // Pointer to function
} entry;

/* Data stack for parameter passing
   This "stack" is circular,
   like a Green Arrays F18A data stack,
   so overflow and underflow are not possible
   Number of items must be a power of 2 */
const int STKSIZE = 8;
const int STKMASK = 7;
int stack[STKSIZE];
int p = 0;

int tickstate = 0; // used to differentiate keyboard input after a tick
int crlfstate = 0; // differentiate when ascii 13 is entered to the terminal

/* TOS is Top Of Stack */
#define TOS stack[p]
/* NAMED creates a string in flash */
#define NAMED(x, y) const char x[]=y

/* Terminal Input Buffer for interpreter */
const byte maxtib = 16;
char tib[maxtib];
/* buffer required for strings read from flash */
char namebuf[maxtib];
byte pos;

/* push n to top of data stack */
void push(int n) {
  p = (p + 1)& STKMASK;
  TOS = n;
}

/* return top of stack */
int pop() {
  int n = TOS;
  p = (p - 1)& STKMASK;
  return n;
}

/* new forward declaration: xtTOadrs */
NAMED(_xtTOadrs, "xt>adrs");
void xtTOadrs();

/* new word: tick */
NAMED(_tick, "\'");
void tick() {
  tickstate = -1; // true: tickstate flag has been raised
}

/* Global delay timer */
int spd = 44;

/* top of stack becomes current spd */
NAMED(_speed, "speed");
void speed() {
  spd = pop();
}

/* discard top of stack */
NAMED(_drop, "drop");
void drop() {
  pop();
}

/* recover dropped stack item */
NAMED(_back, "back");
void back() {
  for (int i = 1; i < STKSIZE; i++) drop();
}

/* exchange top two stack items */
NAMED(_swap, "swap");
void swap() {
  int a;
  int b;
  a = pop();
  b = pop();
  push(b);
}

/* copy second on stack to top */
NAMED(_over, "over");
void over() {
  int a;
  int b;
  a = pop();
  b = pop();
}

/* add top two items */
NAMED(_add, "+");
void add() {
  int a = pop();
  TOS = a + TOS;
}

/* bitwise and top two items */
NAMED(_and, "and");
void and_() {
  int a = pop();
  TOS = a & TOS;
}

/* inclusive or top two items */
NAMED(_or, "or");
void or_() {
  int a = pop();
  TOS = a | TOS;
}

/* exclusive or top two items */
NAMED(_xor, "xor");
void xor_() {
  int a = pop();
  TOS = a ^ TOS;
}

/* invert all bits in top of stack */
NAMED(_invert, "invert");
void invert() {
  TOS = ~(TOS);
}

/* negate top of stack */
NAMED(_negate, "negate");
void negate() {
  TOS = -(TOS);
}

NAMED(_wiggle, "wiggle");
void wiggle() {
  int a = pop();
  pinMode(a, OUTPUT);
  for (int i = 0; i < 20; i++) {
    digitalWrite(a, HIGH);
    delay(spd);
    digitalWrite(a, LOW);
    delay(spd);
  }
}

NAMED(_low, "low");
void low() {
  digitalWrite(pop(), LOW);
}

/* everybody loves a nop */
NAMED(_nopp, "nop");
void nopp() { } // pad the dictionary

/* table of names and function addresses in flash */
const entry dictionary[] = {
  {_low, low},
  {_nopp, nopp} // to pad dictionary
};

/* Number of words in the dictionary */
const int entries = sizeof dictionary / sizeof dictionary[0];

char ch;

void ok() {
  if (crlfstate == -1) {
    Serial.print(" ok\r\n");
    crlfstate = 0;
  }
  // ORIG:  // if (ch == '\r') Serial.println("ok");
}

void printing() {
  if (int(ch) == 13) {
    crlfstate = -1; // raise crlfstate TRUE
    Serial.print(" ");
  } else {
    if (int(ch) != 32) { // a space delimiter
      Serial.print(ch);
    } else { // it's a space
      Serial.print(' ');
    }
  }
}

/* Incrementally read command line from serial port */
/* support backspacing and other increased functionality */
byte reading() {
  if (!Serial.available()) return 1;
  ch = Serial.read();
  printing(); // alternate: Serial.print(ch); // char-by-char input, echo
  if (ch == '\n') {
    Serial.print("\r\n");
    return 1;
  }
  if (ch == '\r') return 0;
  if (ch == ' ') return 0;
  if (ch == '\010') { // backspace
    // if (pos == 0) throw_(); - error handler - see full version of this program
    tib[pos--] = 0;
    tib[pos] = 0;
    Serial.print(" ");
    Serial.print("\010");
    return 1;
  }
  if (pos < maxtib) {
    tib[pos++] = ch;
    tib[pos] = 0;
  }
  return 1;
}

/* Block on reading the command line from serial port */
/* then echo each word */
void readword() {
  pos = 0;
  tib[0] = 0;
  while (reading());
}

/* Run a word via its name */
/* support xt and tick */
void runword() {
  ok();
  // Serial.println("?");
}

/* Arduino main setup and loop */
void setup() {
  Serial.begin(38400);
  while (!Serial);
  Serial.println ("rev 30 Sep aab");
  Serial.println ("Forth-like interpreter:");
  // words();
  Serial.println();
}

void loop() {
  readword();
  runword();
}
// revised: 30 September 2020
// END.
