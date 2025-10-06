#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#define BB_IN(mx, my, x, y, w, h) ((mx) >= (x) && (my) >= (y) && \
                                   (mx) < (x)+(w) && (my) < (y)+(h))

#endif
