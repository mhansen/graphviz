#include <assert.h>
#include <common/memory.h>
#include <common/types.h>
#include <common/utils.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// variant of `isalpha` that assumes a C locale
static bool isalpha_no_locale(char c) {
  if (c >= 'a' && c <= 'z')
    return true;
  if (c >= 'A' && c <= 'Z')
    return true;
  return false;
}

/* return true if *s points to &[A-Za-z]+;      (e.g. &Ccedil; )
 *                          or &#[0-9]*;        (e.g. &#38; )
 *                          or &#x[0-9a-fA-F]*; (e.g. &#x6C34; )
 */
static int xml_isentity(const char *s)
{
    s++;			/* already known to be '&' */
    if (*s == ';') { // '&;' is not a valid entity
	return 0;
    }
    if (*s == '#') {
	s++;
	if (*s == 'x' || *s == 'X') {
	    s++;
	    while (isxdigit(*s))
		s++;
	} else {
	    while (isdigit(*s))
		s++;
	}
    } else {
	while (isalpha_no_locale(*s))
	    s++;
    }
    if (*s == ';')
	return 1;
    return 0;
}

// options to tweak the behavior of XML escaping
typedef struct {
  // assume no embedded escapes, and escape "\n" and "\r"
  unsigned raw : 1;
  // escape '-'
  unsigned dash : 1;
  // escape consecutive ' '
  unsigned nbsp : 1;
} xml_flags_t;

/** XML-escape a character
 *
 * \param previous The source character preceding the current one or '\0' if
 *   there was no prior character.
 * \param current Pointer to the current position in a source string being
 *   escaped.
 * \param flags Options for configuring behavior.
 * \param cb User function for emitting escaped data. This is expected to take a
 *   caller-defined state type as the first parameter and the string to emit as
 *   the second, and then return an opaque value that is passed back to the
 *   caller.
 * \param state Data to pass as the first parameter when calling `cb`.
 * \return The return value of a call to `cb`.
 */
static int xml_core(char previous, const char *current, xml_flags_t flags,
                    int (*cb)(void *state, const char *s), void *state) {

  char c = *current;

  // escape '&' only if not part of a legal entity sequence
  if (c == '&' && (flags.raw || !xml_isentity(current)))
    return cb(state, "&amp;");

  // '<' '>' are safe to substitute even if string is already UTF-8 coded since
  // UTF-8 strings won't contain '<' or '>'
  if (c == '<')
    return cb(state, "&lt;");

  if (c == '>')
    return cb(state, "&gt;");

  // '-' cannot be used in XML comment strings
  if (c == '-' && flags.dash)
    return cb(state, "&#45;");

  if (c == ' ' && previous == ' ' && flags.nbsp)
    // substitute 2nd and subsequent spaces with required_spaces
    return cb(state, "&#160;"); // Inkscape does not recognize &nbsp;

  if (c == '"')
    return cb(state, "&quot;");

  if (c == '\'')
    return cb(state, "&#39;");

  if (c == '\n' && flags.raw)
    return cb(state, "&#10;");

  if (c == '\r' && flags.raw)
    return cb(state, "&#13;");

  // otherwise, output the character as-is
  char buffer[2] = {c, '\0'};
  return cb(state, buffer);
}

char *xml_string(char *s)
{
    return xml_string0 (s, FALSE);
}

// a dynamically resizable string
typedef struct {
  char *base;
  size_t length;
  size_t capacity;
} buffer_t;

/** Write string data to a buffer
 *
 * \param dst A `buffer_t` to write to, but `void*` typed to align with the
 *   callback type `xml_core` expects.
 * \param src String to append.
 * \return Number of characters written.
 */
static int buffer_put(void *dst, const char *src) {

  buffer_t *buffer = dst;
  size_t length = strlen(src);

  // do we need to expand this buffer?
  assert(buffer->base != NULL && "buffer not initialized in xml_string0?");
  while (length > buffer->capacity ||
         buffer->capacity - length <= buffer->length) {
    size_t capacity = buffer->capacity == 0 ? 64 : (buffer->capacity * 2);
    char *base = grealloc(buffer->base, capacity);
    buffer->base = base;
    buffer->capacity = capacity;
  }

  // write source data into the buffer
  strcpy(buffer->base + buffer->length, src);
  buffer->length += length;

  // `xml_core` should only have given us short data
  assert(length <= INT_MAX && "too large XML escape sequence");
  return (int)length;
}

/* xml_string0:
 * Encode input string as an xml string.
 * If raw is true, the input is interpreted as having no
 * embedded escape sequences, and \n and \r are changed
 * into &#10; and &#13;, respectively.
 * Uses a static buffer, so non-re-entrant.
 */
char *xml_string0(char *s, boolean raw) {
  static char *buf = NULL;
  static size_t bufsize = 0;
  char prev = '\0';

  const xml_flags_t flags = {.raw = raw != FALSE, .dash = 1, .nbsp = 1};

  if (!buf) {
    bufsize = 64;
    buf = gmalloc(bufsize);
  }

  // generate an escaped version of this string into `buf`
  buffer_t buffer = {.base = buf, .capacity = bufsize};
  while (s && *s) {
    (void)xml_core(prev, s, flags, buffer_put, &buffer);
    prev = *s;
    s++;
  }
  assert(buffer.length < buffer.capacity && "no room for NUL");
  buffer.base[buffer.length] = '\0';

  // save the static buffer (it may have been realloced) for reuse next time
  buf = buffer.base;
  bufsize = buffer.capacity;

  return buf;
}

/* a variant of xml_string for urls in hrefs */
char *xml_url_string(char *s)
{
    static char *buf = NULL;
    static int bufsize = 0;
    char *p, *sub;
    int len, pos = 0;

    if (!buf) {
	bufsize = 64;
	buf = gmalloc(bufsize);
    }

    p = buf;
    while (s && *s) {
	if (pos > (bufsize - 8)) {
	    bufsize *= 2;
	    buf = grealloc(buf, bufsize);
	    p = buf + pos;
	}
	/* escape '&' only if not part of a legal entity sequence */
	if (*s == '&' && !(xml_isentity(s))) {
	    sub = "&amp;";
	    len = 5;
	}
	/* '<' '>' are safe to substitute even if string is already UTF-8 coded
	 * since UTF-8 strings won't contain '<' or '>' */
	else if (*s == '<') {
	    sub = "&lt;";
	    len = 4;
	}
	else if (*s == '>') {
	    sub = "&gt;";
	    len = 4;
	}
	else if (*s == '"') {
	    sub = "&quot;";
	    len = 6;
	}
	else if (*s == '\'') {
	    sub = "&#39;";
	    len = 5;
	}
	else {
	    sub = s;
	    len = 1;
	}
	while (len--) {
	    *p++ = *sub++;
	    pos++;
	}
	s++;
    }
    *p = '\0';
    return buf;
}
