#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

main(int argc,char **argv)
{
  char *domain; int domainlen;
  int iterations;
  char *hexsalt; int hexsaltlen;
  char *salt; int saltlen;
  char *guess; int guesslen;
  char *x; int xlen;
  char *y; int ylen;
  int i;
  int j;
  unsigned int r;
  int rbits;

  srandom(time(0));

  if (!*argv) return 100;

  if (!*++argv) return 100;
  domain = *argv;
  domainlen = strlen(domain);

  if (!*++argv) return 100;
  iterations = atoi(*argv);

  if (!*++argv) return 100;
  hexsalt = *argv;
  hexsaltlen = strlen(hexsalt);

  saltlen = hexsaltlen / 2;
  salt = malloc(saltlen);
  if (!salt) return 111;
  for (i = 0;i < saltlen;++i) {
    int d;
    sscanf(hexsalt + 2 * i,"%2x",&d);
    salt[i] = d;
  }

  guesslen = 10 + domainlen;
  guess = malloc(guesslen);
  if (!guess) return 111;
  for (i = 0;i < 8;++i) guess[i] = 'x';
  guess[8] = '.';
  for (i = 0;i < domainlen;++i) guess[9 + i] = domain[i];
  guess[9 + domainlen] = '.';

  xlen = guesslen + 1 + saltlen;
  x = malloc(xlen);
  if (!x) return 111;
  j = 0;
  for (i = 0;i < guesslen;++i)
    if (guess[i] == '.') {
      x[j] = i - j;
      while (j < i) { ++j; x[j] = guess[j - 1]; }
      ++j;
    }
  x[j] = 0;
  for (i = 0;i < saltlen;++i) x[guesslen + 1 + i] = salt[i];

  ylen = 20 + saltlen;
  y = malloc(ylen);
  if (!y) return 111;
  for (i = 0;i < saltlen;++i) y[20 + i] = salt[i];

  for (;;) {
    r = random();
    x[1] = guess[0] = "etaoinshrdlucmfw"[r & 15]; r >>= 4;
    x[2] = guess[1] = "etaoinshrdlucmfw"[r & 15]; r >>= 4;
    x[3] = guess[2] = "etaoinshrdlucmfw"[r & 15]; r >>= 4;
    x[4] = guess[3] = "etaoinshrdlucmfw"[r & 15]; r >>= 4;
    x[5] = guess[4] = "etaoinshrdlucmfw"[r & 15]; r >>= 4;
    x[6] = guess[5] = "etaoinshrdlucmfw"[r & 15]; r >>= 4;
    x[7] = guess[6] = "etaoinshrdlucmfw"[r & 15]; r >>= 4;
    x[8] = guess[7] = "etaoinshrdlucmfw"[r & 15];

    SHA1(x,xlen,y);
    for (i = 0;i < iterations;++i) SHA1(y,ylen,y);

    r = 0;
    rbits = 0;
    for (i = 0;i < 20;++i) {
      r = r * 256 + (unsigned int) (unsigned char) y[i];
      rbits += 8;
      while (rbits >= 5) {
        rbits -= 5;
        putchar("0123456789abcdefghijklmnopqrstuv"[r >> rbits]);
	r -= (r >> rbits) << rbits;
      }
    }
    putchar(' ');
    for (i = 0;i < guesslen;++i) putchar(guess[i]);
    putchar('\n');
    if (ferror(stdout)) return 111;
  }
}
