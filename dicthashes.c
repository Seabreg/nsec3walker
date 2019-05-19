#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

char line[1000]; int linelen;
int linealpha[1000];

#define ALPHABET 37
char alphabet[ALPHABET + 1] = "abcdefghijklmnopqrstuvwxyz0123456789-";
unsigned int trigraph[ALPHABET][ALPHABET][ALPHABET];
double trigraphcount;

char *domain; int domainlen;
int iterations;
char *hexsalt; int hexsaltlen;
char *salt; int saltlen;
char *guess; int guesslen;
char *x; int xlen;
char *y; int ylen;

void printhash(int linelen)
{
  int i;
  int j;
  unsigned int r;
  int rbits;

  guesslen = linelen + 2 + domainlen;
  for (i = 0;i < linelen;++i) guess[i] = line[i];
  guess[linelen] = '.';
  for (i = 0;i < domainlen;++i) guess[linelen + 1 + i] = domain[i];
  guess[linelen + 1 + domainlen] = '.';

  xlen = guesslen + 1 + saltlen;
  j = 0;
  for (i = 0;i < guesslen;++i)
    if (guess[i] == '.') {
      x[j] = i - j;
      while (j < i) { ++j; x[j] = guess[j - 1]; }
      ++j;
    }
  x[j] = 0;
  for (i = 0;i < saltlen;++i) x[guesslen + 1 + i] = salt[i];

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
  if (ferror(stdout)) exit(111);
}

int maxcost = 0;

void doit(int pos,int cost)
{
  int r;
  int cost2;
  for (r = 0;r < ALPHABET;++r) {
    cost2 = cost + trigraph[linealpha[pos - 2]][linealpha[pos - 1]][r];
    if (cost2 <= maxcost) {
      linealpha[pos] = r;
      line[pos] = alphabet[r];
      if (cost2 > maxcost - 10) printhash(pos + 1);
      if (pos < 50) doit(pos + 1,cost2);
    }
  }
}

void doit1(void)
{
  int r;
  for (r = 0;r < ALPHABET;++r) {
    linealpha[1] = r;
    line[1] = alphabet[r];
    if (0 > maxcost - 10) printhash(2);
    doit(2,0);
  }
}

void doit0(void)
{
  int r;
  for (r = 0;r < ALPHABET;++r) {
    linealpha[0] = r;
    line[0] = alphabet[r];
    if (0 > maxcost - 10) printhash(1);
    doit1();
  }
}

main(int argc,char **argv)
{
  int i;
  int j;
  int k;

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

  guesslen = 1002 + domainlen;
  guess = malloc(guesslen);
  if (!guess) return 111;

  xlen = guesslen + 1 + saltlen;
  x = malloc(xlen);
  if (!x) return 111;

  ylen = 20 + saltlen;
  y = malloc(ylen);
  if (!y) return 111;
  for (i = 0;i < saltlen;++i) y[20 + i] = salt[i];

  while (fgets(line,sizeof line,stdin)) {
    linelen = strlen(line);
    if (linelen >= 1 && line[linelen - 1] == '\n') line[linelen - 1] = 0;
    linelen = strlen(line);

    for (i = 0;i < linelen;++i) {
      if (line[i] >= 'A' && line[i] <= 'Z') line[i] += 32;
      for (j = 0;j < ALPHABET;++j)
        if (alphabet[j] == line[i]) break;
      if (j == ALPHABET) break;
      linealpha[i] = j;
    }
    if (i < linelen) continue;

    printhash(linelen);

    for (i = 2;i < linelen;++i) {
      ++trigraph[linealpha[i - 2]][linealpha[i - 1]][linealpha[i]];
      ++trigraphcount;
    }
  }

  for (i = 0;i < ALPHABET;++i)
    for (j = 0;j < ALPHABET;++j)
      for (k = 0;k < ALPHABET;++k)
        if (trigraph[i][j][k] == 0)
          trigraph[i][j][k] = 30;
        else {
          trigraph[i][j][k] = ceil(log(trigraphcount / trigraph[i][j][k]) / log(2.0));
          if (trigraph[i][j][k] < 1) trigraph[i][j][k] = 1;
          if (trigraph[i][j][k] > 30) trigraph[i][j][k] = 30;
        }

  for (maxcost = 0;maxcost < 10000;maxcost += 10) {
    doit0();
  }

  return 0;
}
