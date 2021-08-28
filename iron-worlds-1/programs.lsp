(def nil 'nil)
(def t 't)
(def not
  (lambda (a)
    (cond
      (a nil)
      (t t))))
(def or
  (lambda (a b)
    (cond
      (a t)
      (t b))))
(def and
  (lambda (a b)
    (cond
      (a b)
      (t nil))))
