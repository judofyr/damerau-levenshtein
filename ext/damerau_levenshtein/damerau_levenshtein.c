#include "ruby.h"

VALUE DamerauLevenshteinBinding = Qnil;

void Init_damerau_levenshtein();

VALUE method_internal_distance(VALUE self, VALUE _s, VALUE _t, VALUE _block_size, VALUE _max_distance, VALUE cost_function);

void Init_damerau_levenshtein() {
	DamerauLevenshteinBinding = rb_define_module("DamerauLevenshteinBinding");
	rb_define_method(DamerauLevenshteinBinding, "internal_distance", method_internal_distance, 5);
}

VALUE method_internal_distance(VALUE self, VALUE _s, VALUE _t, VALUE _block_size, VALUE _max_distance, VALUE cost_function){
  VALUE *sv = RARRAY_PTR(_s);
  VALUE *tv = RARRAY_PTR(_t);
  int i, i1, j, j1, k, half_tl, block;
  float cost, *d, distance, del, ins, subs, transp;
  int half_sl;
  int stop_execution = 0;
  float min = 0;
  float current_distance = 0;
  int pure_levenshtein = 0;
  int block_size = NUM2INT(_block_size);
  float max_distance = NUM2INT(_max_distance);
  int sl = (int) RARRAY_LEN(_s);
  int tl = (int) RARRAY_LEN(_t);
  long long s[sl];
  long long t[tl];
  
  if (block_size == 0) {
    pure_levenshtein = 1;
    block_size = 1;
  }


  if (sl == 0) return INT2NUM(tl);
  if (tl == 0) return INT2NUM(sl);
  //case of lengths 1 must present or it will break further in the code
  if (sl == 1 && tl == 1 && sv[0] != tv[0]) return INT2NUM(1);


  for (i=0; i < sl; i++) s[i] = NUM2LL(sv[i]);
  for (i=0; i < tl; i++) t[i] = NUM2LL(tv[i]);

  sl++;
  tl++;

  //one-dimentional representation of 2 dimentional array len(s)+1 * len(t)+1
  d = malloc((sizeof(float))*(sl)*(tl));
  //populate 'vertical' row starting from the 2nd position (first one is filled already)
  for(i = 0; i < tl; i++){
    d[i*sl] = i;
  }

  //fill up array with scores
  for(i = 1; i<sl; i++){
    d[i] = i;
    if (stop_execution == 1) break;
    current_distance = 10000;
    for(j = 1; j<tl; j++){

      if (NIL_P(cost_function)) {
        cost = (s[i-1] == t[j-1]) ? 0 : 1;
      } else {
        VALUE cost_val = rb_funcall(cost_function, rb_intern("call"), 2, INT2NUM(s[i-1]), INT2NUM(t[j-1]));
        Check_Type(cost_val, T_FLOAT);
        cost = RFLOAT_VALUE(cost_val);
      }

      half_sl = (sl - 1)/2;
      half_tl = (tl - 1)/2;

      block = block_size < half_sl || half_sl == 0 ? block_size : half_sl;
      block = block < half_tl || half_tl == 0 ? block : half_tl;

      while (block >= 1){
        int swap1 = 1;
        int swap2 = 1;
        i1 = i - (block * 2);
        j1 = j - (block * 2);
        for (k = i1; k < i1 + block; k++) {
          if (s[k] != t[k + block]){
            swap1 = 0;
            break;
          }
        }
        for (k = j1; k < j1 + block; k++) {
          if (t[k] != s[k + block]){
            swap2 = 0;
            break;
          }
        }

        del = d[j*sl + i - 1] + 1;
        ins = d[(j-1)*sl + i] + 1;
        min = del;
        if (ins < min) min = ins;
        //if (i == 2 && j==2) return INT2NUM(swap2+5);
        if (pure_levenshtein == 0 && i >= 2*block && j >= 2*block && swap1 == 1 && swap2 == 1){
          transp = d[(j - block * 2) * sl + i - block * 2] + cost + block -1;
          if (transp < min) min = transp;
          block = 0;
        } else if (block == 1) {
          subs = d[(j-1)*sl + i - 1] + cost;
          if (subs < min) min = subs;
        }
        block--;
      }
      d[j*sl+i]=min;
      if (current_distance > d[j*sl+i]) current_distance = d[j*sl+i];
    }
    if (current_distance > max_distance) {
      stop_execution = 1;
    }
  }
  distance=d[sl * tl - 1];
  if (stop_execution == 1) distance = current_distance;

  free(d);
  return DBL2NUM(distance);
}
