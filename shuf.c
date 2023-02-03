#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <err.h>

#ifdef __linux__
#include <bsd/stdlib.h>
#else
#include <stdlib.h>
#endif

#define VERSION "bsd-2 licensed shuf [https://github.com/krzysckh/shuf]"
#define MIN(x, y) ((x < y) ? x : y)
#define MAX(x, y) ((x > y) ? x : y)

int nlen(int x) {
  int ret = 1; /* passed straight to malloc() */

  if (x < 0)
    ret++;
  x = abs(x);

  do {
    x /= 10;
    ++ret;
  } while (x);

  return ret;
}

void swap(void **a, void **b) {
  void *tmp = *a;
  *a = *b;
  *b = tmp;
}

/* https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle */
void shuf(void **a, int len) {
  int i;

  for (i = len - 1; i > 0; --i)
    swap(&a[(unsigned)arc4random() % len], &a[i]);
}

void usage() {
  errx(1, "shuf [-erzhv] [-i LO-HI] [-n COUNT] [-o FILE] [--random-source FILE]"
      " [FILE]\nversion " VERSION);
}

int getnl(char *s) {
  int ret = 0;
  while (*s++)
    if (*s == '\n')
      ret++;

  return ret;
}

char **tolines(char *s) {
  int nl = getnl(s),
      sz,
      i = 0;
  char **ret;

  ret = malloc(sizeof(char *) * (nl + 1));

  while (*s) {
    sz = 0;
    while (*s++ != '\n')
      sz++;

    ret[i] = malloc(sz + 1);
    strncpy(ret[i], s - sz - 1, sz);
    ret[i][sz] = 0;
    i++;
  }

  return ret;
}

int main(int argc, char *argv[]) {
  int eflag    = 0,
      iflag    = 0,
      nflag    = 0,
      irang[2] = { 0, 0 },
      hcount   = 0,
      /* end flags */
      ch,
      i_sz = 0,
      bufsz = 0,
      i,
      nl = 0,
      ctr = 0,
      delim = '\n';
  char *ofname = NULL,
       *buf = NULL,
       **s;
  FILE *fp = stdin,
       *ofp = stdout;

  struct option lo[] = {
    { "echo",            no_argument,       NULL, 'e' },
    { "input-range",     required_argument, NULL, 'i' },
    { "head-count",      required_argument, NULL, 'n' },
    { "output",          required_argument, NULL, 'o' },
    { "random-source",   required_argument, NULL, 's' },
    { "repeat",          no_argument,       NULL, 'e' },
    { "zero-terminated", no_argument,       NULL, 'z' },
    { "help",            no_argument,       NULL, 'h' },
    { "version",         no_argument,       NULL, 'v' }
  };

  while ((ch = getopt_long(argc, argv, "ei:n:o:s:ezhv", lo, NULL)) != -1) {
    switch (ch) {
      case 'e':
        eflag = 1;
        break;
      case 'i':
        iflag = 1;
        buf = optarg;
        if (buf[0] == '-')
          i_sz++, buf++;
        /* skip '-' sign if < 0 */
        while ((ch = *buf++)) {
          if (ch != '-')
            i_sz++;
          else
            break;
        }

        if (i_sz == (int)strlen(optarg))
          usage();

        optarg[i_sz] = 0;
        irang[0] = atoi(optarg);
        irang[1] = atoi(optarg + i_sz + 1);
        break;
      case 'n':
        nflag = 1;
        hcount = atoi(optarg);
        if (hcount < 0)
          errx(1, "head-count < 0.");
        break;
      case 'o':
        ofname = optarg;
        break;
      case 's':
        warnx("--random-source ignored.");
        /* ignore */
        break;
      case 'r':
        warnx("--repeat ignored.");
        /* ignore */
        break;
      case 'z':
        delim = '\0';
        break;
      default:
        usage();
    }
  }

  if (eflag) {
    nl = argc - optind;
    s = malloc(sizeof(char*) * nl);
    for (i = optind; i < argc; ++i)
      s[i - optind] = argv[i];

  } else if (iflag) {
    nl = 1 + MAX(irang[0], irang[1]) - MIN(irang[0], irang[1]);
    s = malloc(sizeof(char *) * nl);
    for (i = MIN(irang[0], irang[1]); i <= MAX(irang[0], irang[1]); i++) {
      s[ctr] = malloc(nlen(i));
      sprintf(s[ctr++], "%d", i);
    }
  } else {
    if (argv[optind] && strcmp(argv[optind], "-") != 0) {
      fp = fopen(argv[optind], "r");
      if (!fp)
        err(2, "%s", argv[optind]);
    }

    if (ofname) {
      ofp = fopen(ofname, "w");
      if (!fp)
        err(2, "%s", ofname);
    }

    do {
      buf = realloc(buf, bufsz + 1024);
      bufsz += fread(buf + bufsz, 1, 1024, fp);
    } while (!feof(fp));
    buf[bufsz] = 0;

    s = tolines(buf);
    nl = getnl(buf);
  }

  shuf((void **)s, nl);

  for (i = 0; i < (nflag && (nl > hcount) ? hcount : nl); i++)
    fprintf(ofp, "%s%c", s[i], delim);

  if (!eflag)
    for (i = 0; i < nl; ++i)
      free(s[i]);

  if (fp != stdin)
    fclose(fp);

  if (!iflag && !eflag)
    free(buf);

  free(s);
  return 0;
}
