/*
 * This program is a simple PoC to test out the thompon backdoor
 * developed for the RomHack Camp 2026 Thompson workshop "Trusting
 * Trust, Hands On".
 *
 * It is not a full replica of /bin/login, but it contains some of the
 * logic involved for authentication.
 *
 * Highly inspired from login.v7.c (Bell Labs Research Unix Seventh Edition).
 *
 */
#include <crypt.h>  /* crypt */
#include <stdio.h>  /* printf, puts, fputs, fgets, fflush, snprintf, perror, stdin, stdout */
#include <string.h> /* strcmp, strcspn, strlen, explicit_bzero */
#include <unistd.h> /* isatty, getpass, execl, STDIN_FILENO */

struct user {
  const char *name;
  const char *hash; 
};

/*
 * hard-coded DES. first two bytes are for salt.
 */
static const struct user USERS[] = {
  { "alice", "aacoGZy0DVVSw" }, /* demo */
  { "bob",   "bbXi4Hst6ytRc" }, /* secret */
  { "root",  "aacoGZy0DVVSw" }, /* demo */
  { NULL, NULL }
};

static const struct user *find_user(const char *name) {
  const struct user *u;
  for (u = USERS; u->name; u++)
    if (strcmp(u->name, name) == 0)
      return u;
  return NULL;
}

static char *read_password(void) {
  /* when interactive prompt for password */
  if (isatty(STDIN_FILENO))
    return getpass("Password: ");

  static char buf[128];
  fputs("Password: ", stdout);
  fflush(stdout);
  if (!fgets(buf, sizeof buf, stdin))
    return NULL;
  buf[strcspn(buf, "\n")] = '\0';
  return buf;
}

int main(int argc, char **argv)
{
  char name[64];
  char *pw, *namep;
  const struct user *u;

  if (argc > 1) {
    snprintf(name, sizeof name, "%s", argv[1]);
  } else {
    fputs("login: ", stdout);
    fflush(stdout);
    if (!fgets(name, sizeof name, stdin))
      return 0;
    name[strcspn(name, "\n")] = '\0';
  }

  /* Always prompt even if user doesn't exist. */  
  u = find_user(name);
  pw = read_password();
  if (!pw)
    return 1;

  /* DES is always used as first two chars are always the same */
  namep = crypt(pw, u ? u->hash : "xx");
  if (isatty(STDIN_FILENO))
    explicit_bzero(pw, strlen(pw));

  /* this is the actual check */
  if (!u || !namep || strcmp(namep, u->hash) != 0) {
    puts("Login incorrect");
    return 1;
  }

  /* mock proper login procedure */  
  printf("Welcome %s\n", u->name);
  if (strcmp(u->name, "root") == 0)
    puts("you have root privileges");
  fflush(stdout);

  /* drop into a shell when interactive */
  if (isatty(STDIN_FILENO)) {
    execl("/bin/sh", "-sh", (char *)NULL);
    perror("execl");
    return 1;
  }
  
  return 0;
}
