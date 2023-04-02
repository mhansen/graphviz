/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/


#include <assert.h>
#include <cgraph/startswith.h>
#include <common/geomprocs.h>
#include <common/render.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define EPSILON .0001

/* standard arrow length in points */
#define ARROW_LENGTH 10.

#define NUMB_OF_ARROW_HEADS  4
/* each arrow in 8 bits.  Room for NUMB_OF_ARROW_HEADS arrows in 32 bit int. */

#define BITS_PER_ARROW_TYPE 4
/// arrow types
enum {
  ARR_TYPE_NONE = ARR_NONE,
  ARR_TYPE_NORM = 1,
  ARR_TYPE_CROW = 2,
  ARR_TYPE_TEE = 3,
  ARR_TYPE_BOX = 4,
  ARR_TYPE_DIAMOND = 5,
  ARR_TYPE_DOT = 6,
  ARR_TYPE_CURVE = 7,
  ARR_TYPE_GAP = 8
};

/// an arrow type with optional modifications
typedef struct {
  uint8_t type : BITS_PER_ARROW_TYPE;
  bool mod_open : 1;
  bool mod_inv : 1;
  bool mod_left : 1;
  bool mod_right : 1;
} arrowflag_t;

/// does this arrow have any modifications set?
static bool flag_has_mods(arrowflag_t f) {
  if (f.mod_open) return true;
  if (f.mod_inv) return true;
  if (f.mod_left) return true;
  if (f.mod_right) return true;
  return false;
}

/// a collection of up to 4 arrows
typedef struct {
  arrowflag_t flags[NUMB_OF_ARROW_HEADS];
} arrowflags_t;

/// serialize an arrow collection
static uint32_t flags_to_int(arrowflags_t f) {
  assert(sizeof(f) <= sizeof(uint32_t));
  uint32_t u = 0;
  memcpy(&u, &f, sizeof(f));
  return u;
}

/// deserialize an arrow collection
static arrowflags_t int_to_flags(uint32_t f) {
  assert(sizeof(f) <= sizeof(uint32_t));
  arrowflags_t flags = {0};
  memcpy(&flags, &f, sizeof(flags));
  return flags;
}

typedef struct {
    char *dir;
    uint8_t stype : BITS_PER_ARROW_TYPE;
    uint8_t etype : BITS_PER_ARROW_TYPE;
} arrowdir_t;

static const arrowdir_t Arrowdirs[] = {
    {"forward", ARR_TYPE_NONE, ARR_TYPE_NORM},
    {"back", ARR_TYPE_NORM, ARR_TYPE_NONE},
    {"both", ARR_TYPE_NORM, ARR_TYPE_NORM},
    {"none", ARR_TYPE_NONE, ARR_TYPE_NONE},
    {0}
};

typedef struct {
    char *name;
    arrowflag_t type;
} arrowname_t;

static const arrowname_t Arrowsynonyms[] = {
    /* synonyms for deprecated arrow names - included for backward compatibility */
    /*  evaluated before primary names else "invempty" would give different results */
    {"invempty", {.type = ARR_TYPE_NORM, .mod_inv = true, .mod_open = true}}, // oinv
    {0}
};

static const arrowname_t Arrowmods[] = {
    {"o", {.mod_open = true}},
    {"r", {.mod_right = true}},
    {"l", {.mod_left = true}},
    /* deprecated alternates for backward compat */
    {"e", {.mod_open = true}}, // o  - needed for "ediamond"
    {"half", {.mod_left = true}}, // l  - needed for "halfopen"
    {0}
};

static const arrowname_t Arrownames[] = {
    {"normal", {.type = ARR_TYPE_NORM}},
    {"crow", {.type = ARR_TYPE_CROW}},
    {"tee", {.type = ARR_TYPE_TEE}},
    {"box", {.type = ARR_TYPE_BOX}},
    {"diamond", {.type = ARR_TYPE_DIAMOND}},
    {"dot", {.type = ARR_TYPE_DOT}},
    {"none", {.type = ARR_TYPE_GAP}},
    // mod_inv is used only here to define two additional shapes
    // since not all types can use it
    {"inv", {.type = ARR_TYPE_NORM, .mod_inv = true}},
    {"vee", {.type = ARR_TYPE_CROW, .mod_inv = true}},
    /* WARNING ugly kludge to deal with "o" v "open" conflict */
    // Define "open" as just "pen" since "o" already taken as mod_open
    // Note that mod_open has no meaning for ARR_TYPE_CROW shape
    {"pen", {.type = ARR_TYPE_CROW, .mod_inv = true}},
    /* WARNING ugly kludge to deal with "e" v "empty" conflict */
    // Define "empty" as just "mpty" since "e" already taken as mod_open
    // Note that mod_open has expected meaning for ARR_TYPE_NORM shape
    {"mpty", {.type = ARR_TYPE_NORM}},
    {"curve", {.type = ARR_TYPE_CURVE}},
    {"icurve", {.type = ARR_TYPE_CURVE, .mod_inv = true}},
    {0}
};

typedef struct {
    uint8_t type : BITS_PER_ARROW_TYPE;
    double lenfact;		/* ratio of length of this arrow type to standard arrow */
    pointf (*gen)(GVJ_t *job, pointf p, pointf u, double arrowsize,
                  double penwidth, arrowflag_t flag); ///< generator function
                                                      ///< for type
    double (*len)(double lenfact, double arrowsize, double penwidth,
                  arrowflag_t flag); ///< penwidth dependent length
} arrowtype_t;

/* forward declaration of functions used in Arrowtypes[] */
static pointf arrow_type_normal(GVJ_t * job, pointf p, pointf u, double arrowsize, double penwidth, arrowflag_t flag);
static pointf arrow_type_crow(GVJ_t * job, pointf p, pointf u, double arrowsize, double penwidth, arrowflag_t flag);
static pointf arrow_type_tee(GVJ_t * job, pointf p, pointf u, double arrowsize, double penwidth, arrowflag_t flag);
static pointf arrow_type_box(GVJ_t * job, pointf p, pointf u, double arrowsize, double penwidth, arrowflag_t flag);
static pointf arrow_type_diamond(GVJ_t * job, pointf p, pointf u, double arrowsize, double penwidth, arrowflag_t flag);
static pointf arrow_type_dot(GVJ_t * job, pointf p, pointf u, double arrowsize, double penwidth, arrowflag_t flag);
static pointf arrow_type_curve(GVJ_t * job, pointf p, pointf u, double arrowsize, double penwidth, arrowflag_t flag);
static pointf arrow_type_gap(GVJ_t * job, pointf p, pointf u, double arrowsize, double penwidth, arrowflag_t flag);

static double arrow_length_generic(double lenfact, double arrowsize, double penwidth, arrowflag_t flag);
static double arrow_length_normal(double lenfact, double arrowsize, double penwidth, arrowflag_t flag);
static double arrow_length_tee(double lenfact, double arrowsize, double penwidth, arrowflag_t flag);
static double arrow_length_box(double lenfact, double arrowsize, double penwidth, arrowflag_t flag);
static double arrow_length_diamond(double lenfact, double arrowsize, double penwidth, arrowflag_t flag);
static double arrow_length_dot(double lenfact, double arrowsize, double penwidth, arrowflag_t flag);

static const arrowtype_t Arrowtypes[] = {
    {ARR_TYPE_NORM, 1.0, arrow_type_normal, arrow_length_normal},
    {ARR_TYPE_CROW, 1.0, arrow_type_crow, arrow_length_generic},
    {ARR_TYPE_TEE, 0.5, arrow_type_tee, arrow_length_tee},
    {ARR_TYPE_BOX, 1.0, arrow_type_box, arrow_length_box},
    {ARR_TYPE_DIAMOND, 1.2, arrow_type_diamond, arrow_length_diamond},
    {ARR_TYPE_DOT, 0.8, arrow_type_dot, arrow_length_dot},
    {ARR_TYPE_CURVE, 1.0, arrow_type_curve, arrow_length_generic},
    {ARR_TYPE_GAP, 0.5, arrow_type_gap, arrow_length_generic},
};

static const size_t Arrowtypes_size =
  sizeof(Arrowtypes) / sizeof(Arrowtypes[0]);

/// perform something like bitwise OR on two arrow flags
static arrowflag_t arrow_or(arrowflag_t a, arrowflag_t b) {
  return (arrowflag_t){
    .type = a.type == ARR_TYPE_NONE ? b.type : a.type,
    .mod_open = a.mod_open | b.mod_open,
    .mod_inv = a.mod_inv | b.mod_inv,
    .mod_left = a.mod_left | b.mod_left,
    .mod_right = a.mod_right | b.mod_right
  };
}

static char *arrow_match_name_frag(char *name, const arrowname_t *arrownames,
                                   arrowflag_t *flag) {
    size_t namelen = 0;
    char *rest = name;

    for (const arrowname_t *arrowname = arrownames; arrowname->name;
         arrowname++) {
	namelen = strlen(arrowname->name);
	if (startswith(name, arrowname->name)) {
	    *flag = arrow_or(*flag, arrowname->type);
	    rest += namelen;
	    break;
	}
    }
    return rest;
}

static char *arrow_match_shape(char *name, arrowflag_t *flag) {
    char *next, *rest;
    arrowflag_t f = {.type = ARR_TYPE_NONE};

    rest = arrow_match_name_frag(name, Arrowsynonyms, &f);
    if (rest == name) {
	do {
	    next = rest;
	    rest = arrow_match_name_frag(next, Arrowmods, &f);
	} while (next != rest);
	rest = arrow_match_name_frag(rest, Arrownames, &f);
    }
    if (flag_has_mods(f) && f.type == ARR_TYPE_NONE)
	f = arrow_or(f, (arrowflag_t){.type = ARR_TYPE_NORM});
    *flag = f;
    return rest;
}

static void arrow_match_name(char *name, arrowflags_t *flag) {
    char *rest = name;
    char *next;
    int i;

    memset(flag, 0, sizeof(*flag));
    for (i = 0; *rest != '\0' && i < NUMB_OF_ARROW_HEADS; ) {
	arrowflag_t f = {.type = ARR_TYPE_NONE};
	next = rest;
        rest = arrow_match_shape(next, &f);
	if (f.type == ARR_TYPE_NONE && !flag_has_mods(f)) {
	    agerr(AGWARN, "Arrow type \"%s\" unknown - ignoring\n", next);
	    return;
	}
	if (f.type == ARR_TYPE_GAP && !flag_has_mods(f) && i == NUMB_OF_ARROW_HEADS - 1)
	    f.type = ARR_TYPE_NONE;
	if (f.type == ARR_TYPE_GAP && !flag_has_mods(f) && i == 0 && *rest == '\0')
	    f.type = ARR_TYPE_NONE;
	if (f.type != ARR_TYPE_NONE || flag_has_mods(f))
	    flag->flags[i] = arrow_or(flag->flags[i], f);
    }
}

void arrow_flags(Agedge_t *e, uint32_t *sflag, uint32_t *eflag) {
    char *attr;

    arrowflags_t sf = {.flags = {{.type = ARR_TYPE_NONE}}};
    arrowflags_t ef = {.flags = {{.type = agisdirected(agraphof(e)) ? ARR_TYPE_NORM : ARR_TYPE_NONE}}};
    if (E_dir && ((attr = agxget(e, E_dir)))[0]) {
	for (const arrowdir_t *arrowdir = Arrowdirs; arrowdir->dir; arrowdir++) {
	    if (streq(attr, arrowdir->dir)) {
		sf.flags[0].type = arrowdir->stype;
		ef.flags[0].type = arrowdir->etype;
		break;
	    }
	}
    }
    if (ef.flags[0].type == ARR_TYPE_NORM) {
	/* we cannot use the pre-constructed E_arrowhead here because the order in
	 * which edge attributes appear and are thus parsed into a dictionary mean
	 * E_arrowhead->id potentially points at a stale attribute value entry
	 */
	Agsym_t *arrowhead = agfindedgeattr(agraphof(e), "arrowhead");
	if (arrowhead != NULL && ((attr = agxget(e, arrowhead)))[0])
		arrow_match_name(attr, &ef);
    }
    if (sf.flags[0].type == ARR_TYPE_NORM) {
	/* similar to above, we cannot use E_arrowtail here */
	Agsym_t *arrowtail = agfindedgeattr(agraphof(e), "arrowtail");
	if (arrowtail != NULL && ((attr = agxget(e, arrowtail)))[0])
		arrow_match_name(attr, &sf);
    }
    *sflag = flags_to_int(sf);
    *eflag = flags_to_int(ef);
    if (ED_conc_opp_flag(e)) {
	edge_t *f;
	uint32_t s0, e0;
	/* pick up arrowhead of opposing edge */
	f = agfindedge(agraphof(aghead(e)), aghead(e), agtail(e));
	arrow_flags(f, &s0, &e0);
	*eflag |= s0;
	*sflag |= e0;
    }
}

static double arrow_length(edge_t * e, uint32_t flag) {
    double length = 0.0;
    int i;

    const double penwidth = late_double(e, E_penwidth, 1.0, 0.0);
    const double arrowsize = late_double(e, E_arrowsz, 1.0, 0.0);

    if (arrowsize == 0) {
	return 0;
    }

    arrowflags_t f = int_to_flags(flag);

    for (i = 0; i < NUMB_OF_ARROW_HEADS; i++) {
        /* we don't simply index with flag because arrowtypes are not necessarily sorted */
        uint8_t type = f.flags[i].type;
        for (size_t j = 0; j < Arrowtypes_size; ++j) {
	    const arrowtype_t *arrowtype = &Arrowtypes[j];
	    if (type == arrowtype->type) {
		const arrowflag_t arrow_flag = f.flags[i];
		length += (arrowtype->len)(arrowtype->lenfact, arrowsize, penwidth, arrow_flag);
	        break;
	    }
        }
    }
    return length;
}

/* inside function for calls to bezier_clip */
static bool inside(inside_t * inside_context, pointf p)
{
    return DIST2(p, inside_context->a.p[0]) <= inside_context->a.r[0];
}

int arrowEndClip(edge_t* e, pointf * ps, int startp,
                 int endp, bezier *spl, uint32_t eflag) {
    inside_t inside_context;
    pointf sp[4];
    double elen, elen2;

    elen = arrow_length(e, eflag);
    elen2 = elen * elen;
    spl->eflag = eflag;
    spl->ep = ps[endp + 3];
    if (endp > startp && DIST2(ps[endp], ps[endp + 3]) < elen2) {
	endp -= 3;
    }
    sp[3] = ps[endp];
    sp[2] = ps[endp + 1];
    sp[1] = ps[endp + 2];
    sp[0] = spl->ep;	/* ensure endpoint starts inside */

    if (elen > 0) {
	inside_context.a.p = &sp[0];
	inside_context.a.r = &elen2;
	bezier_clip(&inside_context, inside, sp, true);
    }

    ps[endp] = sp[3];
    ps[endp + 1] = sp[2];
    ps[endp + 2] = sp[1];
    ps[endp + 3] = sp[0];
    return endp;
}

int arrowStartClip(edge_t* e, pointf * ps, int startp,
                   int endp, bezier *spl, uint32_t sflag) {
    inside_t inside_context;
    pointf sp[4];
    double slen, slen2;

    slen = arrow_length(e, sflag);
    slen2 = slen * slen;
    spl->sflag = sflag;
    spl->sp = ps[startp];
    if (endp > startp && DIST2(ps[startp], ps[startp + 3]) < slen2) {
	startp += 3;
    }
    sp[0] = ps[startp + 3];
    sp[1] = ps[startp + 2];
    sp[2] = ps[startp + 1];
    sp[3] = spl->sp;	/* ensure endpoint starts inside */

    if (slen > 0) {
	inside_context.a.p = &sp[3];
	inside_context.a.r = &slen2;
	bezier_clip(&inside_context, inside, sp, false);
    }

    ps[startp] = sp[3];
    ps[startp + 1] = sp[2];
    ps[startp + 2] = sp[1];
    ps[startp + 3] = sp[0];
    return startp;
}

/* arrowOrthoClip:
 * For orthogonal routing, we know each Bezier of spl is a horizontal or vertical
 * line segment. We need to guarantee the B-spline stays this way. At present, we shrink
 * the arrows if necessary to fit the last segment at either end. Alternatively, we could
 * maintain the arrow size by dropping the 3 points of spl, and adding a new spl encoding
 * the arrow, something "ex_0,y_0 x_1,y_1 x_1,y_1 x_1,y_1 x_1,y_1", when the last line
 * segment is x_1,y_1 x_2,y_2 x_3,y_3 x_0,y_0. With a good deal more work, we could guarantee
 * that the truncated spl clips to the arrow shape.
 */
void arrowOrthoClip(edge_t *e, pointf *ps, int startp, int endp, bezier *spl,
                    uint32_t sflag, uint32_t eflag) {
    pointf p, q, r, s, t;
    double d, tlen, hlen, maxd;

    if (sflag && eflag && endp == startp) { /* handle special case of two arrows on a single segment */
	p = ps[endp];
	q = ps[endp+3];
	tlen = arrow_length (e, sflag);
	hlen = arrow_length (e, eflag);
        d = DIST(p, q);
	if (hlen + tlen >= d) {
	    hlen = tlen = d/3.0;
	}
	if (p.y == q.y) { // horizontal segment
	    s.y = t.y = p.y;
	    if (p.x < q.x) {
		t.x = q.x - hlen;
		s.x = p.x + tlen;
	    }
	    else {
		t.x = q.x + hlen;
		s.x = p.x - tlen;
	    }
	}
	else {            // vertical segment
	    s.x = t.x = p.x;
	    if (p.y < q.y) {
		t.y = q.y - hlen;
		s.y = p.y + tlen;
	    }
	    else {
		t.y = q.y + hlen;
		s.y = p.y - tlen;
	    }
	}
	ps[endp] = ps[endp + 1] = s;
	ps[endp + 2] = ps[endp + 3] = t;
	spl->sflag = sflag, spl->sp = p;
	spl->eflag = eflag, spl->ep = q;
	return;
    }
    if (eflag) {
	hlen = arrow_length(e, eflag);
	p = ps[endp];
	q = ps[endp+3];
        d = DIST(p, q);
	maxd = 0.9*d;
	if (hlen >= maxd) {   /* arrow too long */
	    hlen = maxd;
	}
	if (p.y == q.y) { // horizontal segment
	    r.y = p.y;
	    if (p.x < q.x) r.x = q.x - hlen;
	    else r.x = q.x + hlen;
	}
	else {            // vertical segment
	    r.x = p.x;
	    if (p.y < q.y) r.y = q.y - hlen;
	    else r.y = q.y + hlen;
	}
	ps[endp + 1] = p;
	ps[endp + 2] = ps[endp + 3] = r;
	spl->eflag = eflag;
	spl->ep = q;
    }
    if (sflag) {
	tlen = arrow_length(e, sflag);
	p = ps[startp];
	q = ps[startp+3];
        d = DIST(p, q);
	maxd = 0.9*d;
	if (tlen >= maxd) {   /* arrow too long */
	    tlen = maxd;
	}
	if (p.y == q.y) { // horizontal segment
	    r.y = p.y;
	    if (p.x < q.x) r.x = p.x + tlen;
	    else r.x = p.x - tlen;
	}
	else {            // vertical segment
	    r.x = p.x;
	    if (p.y < q.y) r.y = p.y + tlen;
	    else r.y = p.y - tlen;
	}
	ps[startp] = ps[startp + 1] = r;
	ps[startp + 2] = q;
	spl->sflag = sflag;
	spl->sp = p;
    }
}

// See https://www.w3.org/TR/SVG2/painting.html#TermLineJoinShape for the
// terminology

static pointf miter_point(pointf base_left, pointf P, pointf base_right,
                          double penwidth) {
  if ((base_left.x == P.x && base_left.y == P.y) ||
      (base_right.x == P.x && base_right.y == P.y)) {
    // the stroke shape is really a point so we just return this point without
    // extending it with penwidth in any direction, which seems to be the way
    // SVG renderers render this.
    return P;
  }
  const pointf A[] = {base_left, P};
  const double dxA = A[1].x - A[0].x;
  const double dyA = A[1].y - A[0].y;
  const double hypotA = hypot(dxA, dyA);
  const double cosAlpha = dxA / hypotA;
  const double sinAlpha = dyA / hypotA;
  const double alpha = dyA > 0 ? acos(cosAlpha) : -acos(cosAlpha);

  const pointf P1 = {P.x - penwidth / 2.0 * sinAlpha,
                     P.y + penwidth / 2.0 * cosAlpha};

  const pointf B[] = {P, base_right};
  const double dxB = B[1].x - B[0].x;
  const double dyB = B[1].y - B[0].y;
  const double hypotB = hypot(dxB, dyB);
  const double cosBeta = dxB / hypotB;
  const double beta = dyB > 0 ? acos(cosBeta) : -acos(cosBeta);

  // angle between the A segment and the B segment in the reverse direction
  const double beta_rev = beta - M_PI;
  const double theta = beta_rev - alpha + (beta_rev - alpha <= -M_PI ? 2 * M_PI : 0);
  assert(theta >= 0 && theta <= M_PI && "theta out of range");

  // check if the miter limit is exceeded according to
  // https://www.w3.org/TR/SVG2/painting.html#StrokeMiterlimitProperty
  const double stroke_miterlimit = 4.0;
  const double normalized_miter_length = 1.0 / sin(theta / 2.0);

  if (normalized_miter_length > stroke_miterlimit)  {
    // fall back to bevel
    const double sinBeta = dyB / hypotB;
    const double sinBetaMinusPi = -sinBeta;
    const double cosBetaMinusPi = -cosBeta;
    const pointf P2 = {P.x + penwidth / 2.0 * sinBetaMinusPi,
                       P.y - penwidth / 2.0 * cosBetaMinusPi};

    // the bevel is the triangle formed from the three points P, P1 and P2 so
    // a good enough approximation of the miter point in this case is the
    // crossing of P-P3 with P1-P2 which is the same as the midpoint between
    // P1 and P2
    const pointf Pbevel = {(P1.x + P2.x) / 2, (P1.y + P2.y) / 2};

    return Pbevel;
  }

  // length between P1 and P3 (and between P2 and P3)
  const double l = penwidth / 2.0 / tan(theta / 2.0);

  const pointf P3 = {P1.x + l * cosAlpha,
                     P1.y + l * sinAlpha};

  return P3;
}

static pointf arrow_type_normal0(pointf p, pointf u, double penwidth,
                                 arrowflag_t flag, pointf *a) {
    pointf q, v;
    double arrowwidth;

    arrowwidth = 0.35;
    if (penwidth > 4)
        arrowwidth *= penwidth / 4;

    v.x = -u.y * arrowwidth;
    v.y = u.x * arrowwidth;
    q.x = p.x + u.x;
    q.y = p.y + u.y;

    pointf delta_base = {0, 0};

    const pointf origin = {0, 0};
    const pointf v_inv = {-v.x, -v.y};
    const pointf normal_left = flag.mod_right ? origin : v_inv;
    const pointf normal_right = flag.mod_left ? origin : v;
    const pointf base_left = flag.mod_inv ? normal_right : normal_left;
    const pointf base_right = flag.mod_inv ? normal_left : normal_right;
    const pointf normal_tip = {-u.x, -u.y};
    const pointf inv_tip = u;
    const pointf P = flag.mod_inv ? inv_tip : normal_tip ;

    const pointf P3 = miter_point(base_left, P, base_right, penwidth);

    const point delta_tip = {P3.x - P.x, P3.y - P.y};

    if (u.x != 0 || u.y != 0) {
	// phi = angle of arrow
	const double cosPhi = P.x / hypot(P.x, P.y);
	const double sinPhi = P.y / hypot(P.x, P.y);
	delta_base = (pointf) {penwidth / 2.0 * cosPhi, penwidth / 2.0 * sinPhi};
    }

    if (flag.mod_inv) {
	p.x += delta_base.x;
	p.y += delta_base.y;
	q.x += delta_base.x;
	q.y += delta_base.y;
	a[0] = a[4] = p;
	a[1].x = p.x - v.x;
	a[1].y = p.y - v.y;
	a[2] = q;
	a[3].x = p.x + v.x;
	a[3].y = p.y + v.y;
	q.x += delta_tip.x;
	q.y += delta_tip.y;
    } else {
	p.x -= delta_tip.x;
	p.y -= delta_tip.y;
	q.x -= delta_tip.x;
	q.y -= delta_tip.y;
	a[0] = a[4] = q;
	a[1].x = q.x - v.x;
	a[1].y = q.y - v.y;
	a[2] = p;
	a[3].x = q.x + v.x;
	a[3].y = q.y + v.y;
	q.x -= delta_base.x;
	q.y -= delta_base.y;
    }

    return q;
}

static pointf arrow_type_normal(GVJ_t *job, pointf p, pointf u,
                                double arrowsize, double penwidth,
                                arrowflag_t flag) {
    (void)arrowsize;

    pointf a[5];

    pointf q = arrow_type_normal0(p, u, penwidth, flag, a);

    if (flag.mod_left)
	gvrender_polygon(job, a, 3, !flag.mod_open);
    else if (flag.mod_right)
	gvrender_polygon(job, &a[2], 3, !flag.mod_open);
    else
	gvrender_polygon(job, &a[1], 3, !flag.mod_open);

    return q;
}

static pointf arrow_type_crow(GVJ_t *job, pointf p, pointf u, double arrowsize,
                              double penwidth, arrowflag_t flag) {
    pointf m, q, v, w, a[9];
    double arrowwidth, shaftwidth;

    arrowwidth = 0.45;
    if (penwidth > 4 * arrowsize && flag.mod_inv)
        arrowwidth *= penwidth / (4 * arrowsize);

    shaftwidth = 0;
    if (penwidth > 1 && flag.mod_inv)
	shaftwidth = 0.05 * (penwidth - 1) / arrowsize;   /* arrowsize to cancel the arrowsize term already in u */

    v.x = -u.y * arrowwidth;
    v.y = u.x * arrowwidth;
    w.x = -u.y * shaftwidth;
    w.y = u.x * shaftwidth;
    q.x = p.x + u.x;
    q.y = p.y + u.y;
    m.x = p.x + u.x * 0.5;
    m.y = p.y + u.y * 0.5;
    if (flag.mod_inv) {  /* vee */
	a[0] = a[8] = p;
	a[1].x = q.x - v.x;
	a[1].y = q.y - v.y;
	a[2].x = m.x - w.x;
	a[2].y = m.y - w.y;
	a[3].x = q.x - w.x;
	a[3].y = q.y - w.y;
	a[4] = q;
	a[5].x = q.x + w.x;
	a[5].y = q.y + w.y;
	a[6].x = m.x + w.x;
	a[6].y = m.y + w.y;
	a[7].x = q.x + v.x;
	a[7].y = q.y + v.y;
    } else {                     /* crow */
	a[0] = a[8] = q;
	a[1].x = p.x - v.x;
	a[1].y = p.y - v.y;
	a[2].x = m.x - w.x;
	a[2].y = m.y - w.y;
	a[3].x = p.x;
	a[3].y = p.y;
	a[4] = p;
	a[5].x = p.x;
	a[5].y = p.y;
	a[6].x = m.x + w.x;
	a[6].y = m.y + w.y;
	a[7].x = p.x + v.x;
	a[7].y = p.y + v.y;
    }
    if (flag.mod_left)
	gvrender_polygon(job, a, 6, 1);
    else if (flag.mod_right)
	gvrender_polygon(job, &a[3], 6, 1);
    else
	gvrender_polygon(job, a, 9, 1);

    return q;
}

static pointf arrow_type_gap(GVJ_t *job, pointf p, pointf u, double arrowsize,
                             double penwidth, arrowflag_t flag) {
    (void)arrowsize;
    (void)penwidth;
    (void)flag;

    pointf q, a[2];

    q.x = p.x + u.x;
    q.y = p.y + u.y;
    a[0] = p;
    a[1] = q;
    gvrender_polyline(job, a, 2);

    return q;
}

static pointf arrow_type_tee(GVJ_t *job, pointf p, pointf u, double arrowsize,
                             double penwidth, arrowflag_t flag) {
    (void)arrowsize;

    pointf m, n, q, v, a[4];

    v.x = -u.y;
    v.y = u.x;
    q.x = p.x + u.x;
    q.y = p.y + u.y;
    m.x = p.x + u.x * 0.2;
    m.y = p.y + u.y * 0.2;
    n.x = p.x + u.x * 0.6;
    n.y = p.y + u.y * 0.6;

    const double length = hypot(u.x, u.y);
    const double polygon_extend_over_polyline = penwidth / 2 - 0.2 * length;
    if (length > 0 && polygon_extend_over_polyline > 0) {
	// the polygon part of the 'tee' arrow will visually overlap the
	// 'polyline' part so we need to move the whole arrow in order not to
	// overlap the node
	const pointf P = {-u.x, -u.y};
	// phi = angle of arrow
	const double cosPhi = P.x / hypot(P.x, P.y);
	const double sinPhi = P.y / hypot(P.x, P.y);
	const pointf delta = {polygon_extend_over_polyline * cosPhi, polygon_extend_over_polyline * sinPhi};

	// move the arrow backwards to not visually overlap the node
	p = sub_pointf(p, delta);
	m = sub_pointf(m, delta);
	n = sub_pointf(n, delta);
	q = sub_pointf(q, delta);
    }

    a[0].x = m.x + v.x;
    a[0].y = m.y + v.y;
    a[1].x = m.x - v.x;
    a[1].y = m.y - v.y;
    a[2].x = n.x - v.x;
    a[2].y = n.y - v.y;
    a[3].x = n.x + v.x;
    a[3].y = n.y + v.y;
    if (flag.mod_left) {
	a[0] = m;
	a[3] = n;
    } else if (flag.mod_right) {
	a[1] = m;
	a[2] = n;
    }
    gvrender_polygon(job, a, 4, 1);
    a[0] = p;
    a[1] = q;
    gvrender_polyline(job, a, 2);

    // A polyline doesn't extend visually beyond its starting point, so we
    // return the starting point as it is, without taking penwidth into account

    return q;
}

static pointf arrow_type_box(GVJ_t *job, pointf p, pointf u, double arrowsize,
                             double penwidth, arrowflag_t flag) {
    (void)arrowsize;
    (void)penwidth;

    pointf m, q, v, a[4];

    v.x = -u.y * 0.4;
    v.y = u.x * 0.4;
    m.x = p.x + u.x * 0.8;
    m.y = p.y + u.y * 0.8;
    q.x = p.x + u.x;
    q.y = p.y + u.y;

    pointf delta = {0, 0};

    if (u.x != 0 || u.y != 0) {
	const pointf P = {-u.x, -u.y};
	// phi = angle of arrow
	const double cosPhi = P.x / hypot(P.x, P.y);
	const double sinPhi = P.y / hypot(P.x, P.y);
	delta = (pointf) {penwidth / 2.0 * cosPhi, penwidth / 2.0 * sinPhi};
    }

    // move the arrow backwards to not visually overlap the node
    p.x -= delta.x;
    p.y -= delta.y;
    m.x -= delta.x;
    m.y -= delta.y;
    q.x -= delta.x;
    q.y -= delta.y;

    a[0].x = p.x + v.x;
    a[0].y = p.y + v.y;
    a[1].x = p.x - v.x;
    a[1].y = p.y - v.y;
    a[2].x = m.x - v.x;
    a[2].y = m.y - v.y;
    a[3].x = m.x + v.x;
    a[3].y = m.y + v.y;
    if (flag.mod_left) {
	a[0] = p;
	a[3] = m;
    } else if (flag.mod_right) {
	a[1] = p;
	a[2] = m;
    }
    gvrender_polygon(job, a, 4, !flag.mod_open);
    a[0] = m;
    a[1] = q;
    gvrender_polyline(job, a, 2);

    // A polyline doesn't extend visually beyond its starting point, so we
    // return the starting point as it is, without taking penwidth into account

    return q;
}

static pointf arrow_type_diamond0(pointf p, pointf u, double penwidth,
                                  arrowflag_t flag, pointf *a) {
    pointf q, r, v;

    v.x = -u.y / 3.;
    v.y = u.x / 3.;
    r.x = p.x + u.x / 2.;
    r.y = p.y + u.y / 2.;
    q.x = p.x + u.x;
    q.y = p.y + u.y;

    const pointf origin = {0, 0};
    const pointf unmod_left = sub_pointf(scale(-0.5, u), v);
    const pointf unmod_right = add_pointf(scale(-0.5, u), v);
    const pointf base_left = flag.mod_right ? origin : unmod_left;
    const pointf base_right = flag.mod_left ? origin : unmod_right;
    const pointf tip = scale(-1, u);
    const pointf P = tip;

    const pointf P3 = miter_point(base_left, P, base_right, penwidth);

    const pointf delta = sub_pointf(P3, P);

    // move the arrow backwards to not visually overlap the node
    p = sub_pointf(p, delta);
    r = sub_pointf(r, delta);
    q = sub_pointf(q, delta);

    a[0] = a[4] = q;
    a[1].x = r.x + v.x;
    a[1].y = r.y + v.y;
    a[2] = p;
    a[3].x = r.x - v.x;
    a[3].y = r.y - v.y;

    // return the visual starting point of the arrow outline
    q = sub_pointf(q, delta);

    return q;
}

static pointf arrow_type_diamond(GVJ_t *job, pointf p, pointf u,
                                 double arrowsize, double penwidth,
                                 arrowflag_t flag) {
    (void)arrowsize;

    pointf a[5];

    pointf q = arrow_type_diamond0(p, u, penwidth, flag, a);

    if (flag.mod_left)
	gvrender_polygon(job, &a[2], 3, !flag.mod_open);
    else if (flag.mod_right)
	gvrender_polygon(job, a, 3, !flag.mod_open);
    else
	gvrender_polygon(job, a, 4, !flag.mod_open);

    return q;
}

static pointf arrow_type_dot(GVJ_t *job, pointf p, pointf u, double arrowsize,
                             double penwidth, arrowflag_t flag) {
    (void)arrowsize;
    (void)penwidth;

    double r;
    pointf AF[2];

    r = hypot(u.x, u.y) / 2.;

    pointf delta = {0, 0};

    if (u.x != 0 || u.y != 0) {
	const pointf P = {-u.x, -u.y};
	// phi = angle of arrow
	const double cosPhi = P.x / hypot(P.x, P.y);
	const double sinPhi = P.y / hypot(P.x, P.y);
	delta = (pointf) {penwidth / 2.0 * cosPhi, penwidth / 2.0 * sinPhi};

	// move the arrow backwards to not visually overlap the node
	p.x -= delta.x;
	p.y -= delta.y;
    }


    AF[0].x = p.x + u.x / 2. - r;
    AF[0].y = p.y + u.y / 2. - r;
    AF[1].x = p.x + u.x / 2. + r;
    AF[1].y = p.y + u.y / 2. + r;
    gvrender_ellipse(job, AF, !flag.mod_open);

    pointf q = {p.x + u.x, p.y + u.y};

    // return the visual starting point of the arrow outline
    q.x -= delta.x;
    q.y -= delta.y;

    return q;
}


/* Draw a concave semicircle using a single cubic bezier curve that touches p at its midpoint.
 * See http://digerati-illuminatus.blogspot.com.au/2008/05/approximating-semicircle-with-cubic.html for details.
 */
static pointf arrow_type_curve(GVJ_t *job, pointf p, pointf u, double arrowsize,
                               double penwidth, arrowflag_t flag) {
    (void)arrowsize;

    double arrowwidth = penwidth > 4 ? 0.5 * penwidth / 4 : 0.5;
    pointf q, v, w;
    pointf AF[4], a[2];

    q.x = p.x + u.x;
    q.y = p.y + u.y; 
    v.x = -u.y * arrowwidth; 
    v.y = u.x * arrowwidth;
    w.x = v.y; // same direction as u, same magnitude as v.
    w.y = -v.x;
    a[0] = p;
    a[1] = q;

    AF[0].x = p.x + v.x + w.x;
    AF[0].y = p.y + v.y + w.y;

    AF[3].x = p.x - v.x + w.x;
    AF[3].y = p.y - v.y + w.y;

    if (flag.mod_inv) {  /* ----(-| */
        AF[1].x = p.x + 0.95 * v.x + w.x + w.x * 4.0 / 3.0;
        AF[1].y = AF[0].y + w.y * 4.0 / 3.0;

        AF[2].x = p.x - 0.95 * v.x + w.x + w.x * 4.0 / 3.0;
        AF[2].y = AF[3].y + w.y * 4.0 / 3.0;
    }
    else {  /* ----)-| */
        AF[1].x = p.x + 0.95 * v.x + w.x - w.x * 4.0 / 3.0;
        AF[1].y = AF[0].y - w.y * 4.0 / 3.0;

        AF[2].x = p.x - 0.95 * v.x + w.x - w.x * 4.0 / 3.0;
        AF[2].y = AF[3].y - w.y * 4.0 / 3.0;
    }

    gvrender_polyline(job, a, 2);
    if (flag.mod_left)
	Bezier(AF, 3, 0.5, NULL, AF);
    else if (flag.mod_right)
	Bezier(AF, 3, 0.5, AF, NULL);
    gvrender_beziercurve(job, AF, sizeof(AF) / sizeof(pointf), FALSE);

    return q;
}


static pointf arrow_gen_type(GVJ_t *job, pointf p, pointf u, double arrowsize,
                             double penwidth, arrowflag_t flag) {
    uint8_t f = flag.type;
    for (size_t i = 0; i < Arrowtypes_size; ++i) {
	const arrowtype_t *arrowtype = &Arrowtypes[i];
	if (f == arrowtype->type) {
	    u.x *= arrowtype->lenfact * arrowsize;
	    u.y *= arrowtype->lenfact * arrowsize;
	    p = arrowtype->gen(job, p, u, arrowsize, penwidth, flag);
	    break;
	}
    }
    return p;
}

boxf arrow_bb(pointf p, pointf u, double arrowsize)
{
    double s;
    boxf bb;
    double ax,ay,bx,by,cx,cy,dx,dy;
    double ux2, uy2;

    /* generate arrowhead vector */
    u.x -= p.x;
    u.y -= p.y;
    /* the EPSILONs are to keep this stable as length of u approaches 0.0 */
    s = ARROW_LENGTH * arrowsize / (hypot(u.x, u.y) + EPSILON);
    u.x += (u.x >= 0.0) ? EPSILON : -EPSILON;
    u.y += (u.y >= 0.0) ? EPSILON : -EPSILON;
    u.x *= s;
    u.y *= s;

    /* compute all 4 corners of rotated arrowhead bounding box */
    ux2 = u.x / 2.;
    uy2 = u.y / 2.;
    ax = p.x - uy2;
    ay = p.y - ux2;
    bx = p.x + uy2;
    by = p.y + ux2;
    cx = ax + u.x;
    cy = ay + u.y;
    dx = bx + u.x;
    dy = by + u.y;

    /* compute a right bb */
    bb.UR.x = MAX(ax, MAX(bx, MAX(cx, dx)));
    bb.UR.y = MAX(ay, MAX(by, MAX(cy, dy)));
    bb.LL.x = MIN(ax, MIN(bx, MIN(cx, dx)));
    bb.LL.y = MIN(ay, MIN(by, MIN(cy, dy)));
 
    return bb;
}

void arrow_gen(GVJ_t *job, emit_state_t emit_state, pointf p, pointf u,
               double arrowsize, double penwidth, uint32_t flag) {
    obj_state_t *obj = job->obj;
    double s;
    int i;
    emit_state_t old_emit_state;

    old_emit_state = obj->emit_state;
    obj->emit_state = emit_state;

    /* Dotted and dashed styles on the arrowhead are ugly (dds) */
    /* linewidth needs to be reset */
    gvrender_set_style(job, job->gvc->defaultlinestyle);

    gvrender_set_penwidth(job, penwidth);

    /* generate arrowhead vector */
    u.x -= p.x;
    u.y -= p.y;
    /* the EPSILONs are to keep this stable as length of u approaches 0.0 */
    s = ARROW_LENGTH / (hypot(u.x, u.y) + EPSILON);
    u.x += (u.x >= 0.0) ? EPSILON : -EPSILON;
    u.y += (u.y >= 0.0) ? EPSILON : -EPSILON;
    u.x *= s;
    u.y *= s;

    /* the first arrow head - closest to node */
    arrowflags_t flags = int_to_flags(flag);
    for (i = 0; i < NUMB_OF_ARROW_HEADS; i++) {
        arrowflag_t f = flags.flags[i];
	if (f.type == ARR_TYPE_NONE && !flag_has_mods(f))
	    break;
        p = arrow_gen_type(job, p, u, arrowsize, penwidth, f);
    }

    obj->emit_state = old_emit_state;
}

static double arrow_length_generic(double lenfact, double arrowsize,
                                   double penwidth, arrowflag_t flag) {
  (void)penwidth;
  (void)flag;

  return lenfact * arrowsize * ARROW_LENGTH;
}

static double arrow_length_normal(double lenfact, double arrowsize,
                                  double penwidth, arrowflag_t flag) {
  pointf a[5];
  // set arrow end point at origin
  const pointf p = {0, 0};
  // generate an arrowhead vector along x-axis
  const pointf u = {lenfact * arrowsize * ARROW_LENGTH, 0};

  // arrow start point
  pointf q = arrow_type_normal0(p, u, penwidth, flag, a);

  const pointf base1 = a[1];
  const pointf base2 = a[3];
  const pointf tip = a[2];
  const double full_length = q.x;
  assert(full_length > 0 && "non-positive full length");
  const double nominal_length = fabs(base1.x - tip.x);
  const double nominal_base_width = base2.y - base1.y;
  assert(nominal_base_width > 0 && "non-positive nominal base width");
  // the full base width is proportionally scaled with the length
  const double full_base_width =
      nominal_base_width * full_length / nominal_length;
  assert(full_base_width > 0 && "non-positive full base width");

  // we want a small overlap between the edge path (stem) and the arrow to avoid
  // gaps between them in case the arrow has a corner towards the edge path
  const double overlap_at_base = penwidth / 2;
  // overlap the tip to a point where its width is equal to the penwidth.
  const double length_where_width_is_penwidth =
      full_length * penwidth / full_base_width;
  const double overlap_at_tip = length_where_width_is_penwidth;

  const double overlap = flag.mod_inv ? overlap_at_tip : overlap_at_base;

  // arrow length is the x value of the start point since the arrow points along
  // the positive x axis and ends at origin
  return full_length - overlap;
}

static double arrow_length_tee(double lenfact, double arrowsize,
                               double penwidth, arrowflag_t flag) {
    (void)flag;

    // The `tee` arrow shape normally begins and ends with a polyline which
    // doesn't extend visually beyond its starting point, so we only have to
    // take penwidth into account if the polygon part visually extends the
    // polyline part at the start or end points.

    const double nominal_length = lenfact * arrowsize * ARROW_LENGTH;
    double length = nominal_length;

    // see the 'arrow_type_tee' function for the magical constants used below

    const double polygon_extend_over_polyline_at_start = penwidth / 2 - (1 - 0.6) * nominal_length;
    if (polygon_extend_over_polyline_at_start > 0) {
	length += polygon_extend_over_polyline_at_start;
    }

    const double polygon_extend_over_polyline_at_end = penwidth / 2 - 0.2 * nominal_length;
    if (polygon_extend_over_polyline_at_start > 0) {
	length += polygon_extend_over_polyline_at_end;
    }

    return length;
}

static double arrow_length_box(double lenfact, double arrowsize,
                               double penwidth, arrowflag_t flag) {
  (void)flag;

  // The `box` arrow shape begins with a polyline which doesn't extend
  // visually beyond its starting point, so we only have to take penwidth
  // into account at the end point.

  return lenfact * arrowsize * ARROW_LENGTH + penwidth / 2;
}

static double arrow_length_diamond(double lenfact, double arrowsize,
                                   double penwidth, arrowflag_t flag) {
  pointf a[5];
  // set arrow end point at origin
  const pointf p = {0, 0};
  // generate an arrowhead vector along x-axis
  const pointf u = {lenfact * arrowsize * ARROW_LENGTH, 0};

  // arrow start point
  pointf q = arrow_type_diamond0(p, u, penwidth, flag, a);

  // calculate overlap using a triangle with its base at the left and right
  // corners of the diamond and its tip at the end point
  const pointf base1 = a[3];
  const pointf base2 = a[1];
  const pointf tip = a[2];
  const double full_length = q.x / 2;
  assert(full_length > 0 && "non-positive full length");
  const double nominal_length = fabs(base1.x - tip.x);
  const double nominal_base_width = base2.y - base1.y;
  assert(nominal_base_width > 0 && "non-positive nominal base width");
  // the full base width is proportionally scaled with the length
  const double full_base_width =
      nominal_base_width * full_length / nominal_length;
  assert(full_base_width > 0 && "non-positive full base width");

  // we want a small overlap between the edge path (stem) and the arrow to avoid
  // gaps between them in case the arrow has a corner towards the edge path

  // overlap the tip to a point where its width is equal to the penwidth
  const double length_where_width_is_penwidth =
      full_length * penwidth / full_base_width;
  const double overlap_at_tip = length_where_width_is_penwidth;

  const double overlap = overlap_at_tip;

  // arrow length is the x value of the start point since the arrow points along
  // the positive x axis and ends at origin
  return 2 * full_length - overlap;
}

static double arrow_length_dot(double lenfact, double arrowsize,
                               double penwidth, arrowflag_t flag) {
  (void)flag;

  return lenfact * arrowsize * ARROW_LENGTH + penwidth;
}
