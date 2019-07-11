* File: cqo1.mps
NAME          CQO1 EXAMPLE
OBJSENSE
    MIN
ROWS
 N  obj
 E  c1      
COLUMNS
    x1        obj       0.0
    x1        c1        1.0
    x2        obj       0.0
    x2        c1        1.0
    x3        obj       0.0
    x3        c1        2.0
    x4        obj       1.0
    x5        obj       1.0
    x6        obj       1.0
RHS
    rhs       c1        1.0
BOUNDS
 FR bound     x4      
 FR bound     x5      
 FR bound     x6      
CSECTION      k1        0.0            QUAD
    x4      
    x1      
    x2      
CSECTION      k2        0.0            RQUAD
    x5      
    x6      
    x3      
ENDATA