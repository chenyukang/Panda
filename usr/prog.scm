#t
-123
"adsf"
"asdf"
(quote ())
(quote (0 . 1))
(quote (0 1 2 3))
(quote asdf)
(define a 1)
a
(set! a 2)
a
(if #t 1 2)
(+ 1 2 3)
((lambda (x) x) 1)
(define (add x y) (+ x y))
(add 1 2)
add
(define c ((lambda (x) (lambda () x)) 3))
(c)
(begin 1 2)
(cond (#f          1)
       ((eq? #t #t) 2)
       (else        3))

(let ((x (+ 1 1))
       (y (- 5 2)))
   (+ x y))

(and 1 2 #f 3)

(or #f #f 3 #f)
(apply + '(1 2 3))
(define env (environment))
(eval '(define z 25) env)
(eval 'z env)

