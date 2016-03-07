      Comment //comment @ tests/simple.plang (1, 1)
      Comment /* also a comment */ @ tests/simple.plang (2, 1)
      Comment //#operator ~ left 15 @ tests/simple.plang (4, 1)
      Comment //defines a custom binary operator @ tests/simple.plang (5, 1)
      Comment //with left-associativity and precidence of 15 @ tests/simple.plang (6, 1)
      Comment //prefix and postfix are both urnary operators @ tests/simple.plang (7, 1)
      Comment //ternary operators can be emulated two part binary operators @ tests/simple.plang (8, 1)
   RegionOpen ( @ tests/simple.plang (10, 1)
       Number 1 @ tests/simple.plang (10, 2)
   Identifier , @ tests/simple.plang (10, 3)
       Number 2 @ tests/simple.plang (10, 5)
   Identifier , @ tests/simple.plang (10, 6)
       Number 3 @ tests/simple.plang (10, 8)
  RegionClose ) @ tests/simple.plang (10, 9)
   Terminator ; @ tests/simple.plang (10, 10)
   RegionOpen ( @ tests/simple.plang (12, 1)
   Identifier a @ tests/simple.plang (12, 2)
  RegionClose ) @ tests/simple.plang (12, 3)
   Terminator ; @ tests/simple.plang (12, 4)
   RegionOpen { @ tests/simple.plang (14, 1)
  RegionClose } @ tests/simple.plang (14, 2)
   Terminator ; @ tests/simple.plang (14, 3)
   RegionOpen ( @ tests/simple.plang (16, 1)
  RegionClose ) @ tests/simple.plang (16, 2)
   RegionOpen { @ tests/simple.plang (16, 3)
  RegionClose } @ tests/simple.plang (16, 4)
   Terminator ; @ tests/simple.plang (16, 5)
   RegionOpen ( @ tests/simple.plang (17, 1)
   Identifier a @ tests/simple.plang (17, 2)
   Identifier , @ tests/simple.plang (17, 3)
   Identifier b @ tests/simple.plang (17, 5)
  RegionClose ) @ tests/simple.plang (17, 6)
   RegionOpen { @ tests/simple.plang (17, 8)
   Identifier a @ tests/simple.plang (17, 10)
   Identifier - @ tests/simple.plang (17, 12)
   Identifier b @ tests/simple.plang (17, 14)
  RegionClose } @ tests/simple.plang (17, 16)
   Terminator ; @ tests/simple.plang (17, 17)
   RegionOpen ( @ tests/simple.plang (18, 1)
   Identifier a @ tests/simple.plang (18, 2)
  RegionClose ) @ tests/simple.plang (18, 3)
   RegionOpen { @ tests/simple.plang (18, 5)
   Identifier x @ tests/simple.plang (18, 7)
   Identifier + @ tests/simple.plang (18, 9)
       Number 1 @ tests/simple.plang (18, 11)
   Terminator ; @ tests/simple.plang (18, 12)
   Identifier y @ tests/simple.plang (18, 14)
   Identifier + @ tests/simple.plang (18, 16)
       Number 2 @ tests/simple.plang (18, 18)
   Terminator ; @ tests/simple.plang (18, 19)
  RegionClose } @ tests/simple.plang (18, 21)
   Terminator ; @ tests/simple.plang (18, 22)
   Terminator ; @ tests/simple.plang (20, 1)
   Terminator ; @ tests/simple.plang (20, 2)
   RegionOpen [ @ tests/simple.plang (22, 1)
       Number 1 @ tests/simple.plang (22, 2)
   Identifier , @ tests/simple.plang (22, 3)
       Number 2 @ tests/simple.plang (22, 4)
  RegionClose ] @ tests/simple.plang (22, 5)
   Identifier + @ tests/simple.plang (22, 7)
       Number 3 @ tests/simple.plang (22, 9)
   Terminator ; @ tests/simple.plang (22, 10)
   RegionOpen [| @ tests/simple.plang (24, 1)
       Number 1 @ tests/simple.plang (24, 3)
  RegionClose |] @ tests/simple.plang (24, 4)
   Terminator ; @ tests/simple.plang (24, 6)
   RegionOpen ( @ tests/simple.plang (26, 1)
   RegionOpen [ @ tests/simple.plang (26, 2)
   RegionOpen [| @ tests/simple.plang (26, 3)
   RegionOpen { @ tests/simple.plang (26, 6)
   Identifier a @ tests/simple.plang (26, 8)
   Identifier : @ tests/simple.plang (26, 9)
   Identifier b @ tests/simple.plang (26, 11)
  RegionClose } @ tests/simple.plang (26, 13)
   Identifier , @ tests/simple.plang (26, 14)
       Number 2 @ tests/simple.plang (26, 16)
  RegionClose |] @ tests/simple.plang (26, 18)
  RegionClose ] @ tests/simple.plang (26, 20)
   Identifier , @ tests/simple.plang (26, 21)
   Identifier $ @ tests/simple.plang (26, 23)
  RegionClose ) @ tests/simple.plang (26, 24)
   Terminator ; @ tests/simple.plang (26, 25)
   Identifier many @ tests/simple.plang (28, 1)
   Terminator ; @ tests/simple.plang (28, 5)
   Identifier statements @ tests/simple.plang (28, 6)
   Terminator ; @ tests/simple.plang (28, 16)
   Identifier on @ tests/simple.plang (28, 17)
   Terminator ; @ tests/simple.plang (28, 19)
   Identifier one @ tests/simple.plang (28, 20)
   Terminator ; @ tests/simple.plang (28, 23)
   Identifier line @ tests/simple.plang (28, 24)
   Terminator ; @ tests/simple.plang (28, 28)
   Identifier c @ tests/simple.plang (30, 1)
     Accessor . @ tests/simple.plang (30, 2)
   Identifier d @ tests/simple.plang (30, 3)
   Terminator ; @ tests/simple.plang (30, 4)
   Identifier a @ tests/simple.plang (31, 1)
     Accessor . @ tests/simple.plang (31, 2)
     Accessor . @ tests/simple.plang (31, 3)
   Identifier b @ tests/simple.plang (31, 4)
   Terminator ; @ tests/simple.plang (31, 5)
      Comment //z + q = n + p; @ tests/simple.plang (33, 1)
      Comment //a = b = c + d = e; @ tests/simple.plang (35, 1)
      Comment //z = y:4; @ tests/simple.plang (37, 1)
      Comment //x + y z; @ tests/simple.plang (38, 1)
      Comment //f a + b c + d; @ tests/simple.plang (39, 1)
      Comment //x + 5 = 4 + 1, 2, 3; @ tests/simple.plang (40, 1)
      Comment // d, e, f, g, h i; @ tests/simple.plang (41, 1)
      Comment // j; @ tests/simple.plang (42, 1)
      Comment // q + 4, d z l, 44; @ tests/simple.plang (43, 1)
      Comment // .; @ tests/simple.plang (45, 1)
      Comment // ..; @ tests/simple.plang (46, 1)
      Comment // ...; @ tests/simple.plang (47, 1)
      Comment // a.b; @ tests/simple.plang (49, 1)
      Comment // a..b; @ tests/simple.plang (50, 1)
      Comment // a...b; @ tests/simple.plang (51, 1)
      Comment /*
#operator =!= infix left 15

;;

vあr = 3 + 5.0 * 4;
str = "test 不確かな text";


z = .5;

noq = zee.5;
floq = (1).3;

prop = 4000.5.5;
proq = 'string';

a::b;

foo_bar = baz.bonk;
z4d = 1'000'000;
quux = (1, "2", 3); //tuple
floop = 4, '3', 6; //tuple ---- missing 6
nugget = [7, 8, 9]; //list
donk = [| a, b, c |]; //array
gork = x, y, z; //tuple ---- missing z

a = (x: 1, y: 2, z: 3); //named tuple
b = u: 4, v: 5, w: 6; //named tuple

dotted = lal..lol;
more = zoo...mop * 5;

b13 = (jello)
{
	x = 10.5;
	x.5 =!= 'fuck';
}
zzz = true;
eor =
{
	a3sdfm: 5;
	z3fsdf: "6";
}
lolo = null;

if (x == 5)
{
	v313;
}

zoop = !5 + 3;

(1), 2, 3;
!5, 6, 7; 	
!(8 9 10);
!(8, 9, 10);
(a, {b:c}, (q) { d = e; }, f);

nest = ()
{
	deeper: ()
	{
		deepest: 5;
	}
	otherd:
	{
		otherst: "blah";
	}
}

back = out;

rest = (a, b, (c)) { }
*/ @ tests/simple.plang (53, 1)
