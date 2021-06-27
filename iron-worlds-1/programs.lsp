(define 'nil 'nil)
(define 't 't)
(define 'not
  '(lambda (a)
    (cond
      (a nil)
      (t t))))
(define 'or
  '(lambda (a b)
    (cond
      (a t)
      (t b))))
(define 'and
  '(lambda (a b)
    (cond
      (a b)
      (t nil))))
(define '-and '(lambda (a) (lambda (b) (and a b))))
(define 't-and '(-and t))
(define 'n-and '(-and nil))
