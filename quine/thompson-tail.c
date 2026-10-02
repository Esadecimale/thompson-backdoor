void payload() {
  // for a working quinbe keep this off stdout
  int c = 41 + 1;
}

int main() {
  int i;

  /* arbitrary stuff */
  payload();

  /* self-reproduction */
  printf("#include <stdio.h>\n\nchar s[] = {\n");
  for (i = 0; s[i]; i++)
    printf("\t%d,\n", s[i]);
  printf("%s", s);
}
