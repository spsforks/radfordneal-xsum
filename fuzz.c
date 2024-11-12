/*
Fuzz xsum against Shewchuck and addition-with-bigints.
Does not check -Infinity / Infinity / NaN in inputs.
Also does not check the case where inputs are all -0.
*/

/* Copyright 2024 Kevin Gibbons

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "double-utils.h"
#include "shewchuk.h"
#include "addition-with-bigints.h"
#include "xsum.h"

int main() {
  xsum_small_accumulator sacc;
  xsum_large_accumulator lacc;

  shewchuk adder;
  shewchuk_init(&adder);

  bigint bigint_sum;
  bigint bigint_addend;

  int seed = time(NULL);
  printf("seed: %d\n", seed);
  srand(seed);

  double addends[10];
  for (int i = 0; i < 1e6; ++i) {
    xsum_small_init(&sacc);
    xsum_large_init(&lacc);
    shewchuk_reinit(&adder);
    bigint_init(&bigint_sum);

    int count = (rand() % 7) + 2;
    for (int j = 0; j < count; ++j) {
      double v = random_double();
      addends[j] = v;
      xsum_small_add1(&sacc, v);
      xsum_large_add1(&lacc, v);
      shewchuk_add(&adder, v);

      double_to_bigint(v, &bigint_addend);
      add_bigints(&bigint_sum, &bigint_addend, &bigint_sum);
    }

    double result_s = xsum_small_round(&sacc);
    double result_l = xsum_large_round(&lacc);
    double result_shewchuk = shewchuk_compute(&adder);
    double result_bigint = bigint_to_double(&bigint_sum);

    if (result_shewchuk != result_bigint) {
      printf("!!! bigint / shewchuk disagreement !!!\n");
      printf("addends: ");
      for (int j = 0; j < count; ++j) {
        printf("%.17g, ", addends[j]);
      }
      printf("\n");
      printf("bigint:     %.17g\n", result_bigint);
      printf("shewchuk:   %.17g\n", result_shewchuk);
      return 1;
    }

    if (result_s != result_shewchuk || result_l != result_shewchuk) {
      printf("!!! xsum / shewchuk disagreement !!!\n");
      printf("addends: ");
      for (int j = 0; j < count; ++j) {
        printf("%.17g, ", addends[j]);
      }
      printf("\n");
      printf("shewchuk:   %.17g\n", result_shewchuk);
      printf("small xsum: %.17g\n", result_s);
      printf("large xsum: %.17g\n", result_l);
      return 1;
    }
  }
}
