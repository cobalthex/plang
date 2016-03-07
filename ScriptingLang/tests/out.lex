      Comment //comment
      Comment /* also a comment */
      Comment //#operator ~ left 15
      Comment //defines a custom binary operator
      Comment //with left-associativity and precidence of 15
      Comment //prefix and postfix are both urnary operators
      Comment //ternary operators can be emulated two part binary operators
   RegionOpen (
       Number 1
   Identifier ,
       Number 2
   Identifier ,
       Number 3
  RegionClose )
   Terminator ;
   RegionOpen (
   Identifier a
  RegionClose )
   Terminator ;
   RegionOpen {
  RegionClose }
   Terminator ;
   RegionOpen (
  RegionClose )
   RegionOpen {
  RegionClose }
   Terminator ;
   RegionOpen (
   Identifier a
  RegionClose )
   RegionOpen {
   Identifier x
   Identifier +
       Number 1
   Terminator ;
  RegionClose }
   Terminator ;
   Terminator ;
   Terminator ;
   RegionOpen [
       Number 1
   Identifier ,
       Number 2
  RegionClose ]
   Identifier +
       Number 3
   Terminator ;
   RegionOpen [|
       Number 1
  RegionClose |]
   Terminator ;
   RegionOpen (
   RegionOpen [
   RegionOpen [|
   RegionOpen {
   Identifier a
   Identifier :
   Identifier b
  RegionClose }
   Identifier ,
       Number 2
  RegionClose |]
  RegionClose ]
   Identifier ,
   Identifier $
  RegionClose )
   Terminator ;
   Identifier many
   Terminator ;
   Identifier statements
   Terminator ;
   Identifier on
   Terminator ;
   Identifier one
   Terminator ;
   Identifier line
   Terminator ;
   Identifier c
     Accessor .
   Identifier d
   Terminator ;
   Identifier a
     Accessor .
     Accessor .
   Identifier b
   Terminator ;
      Comment //z + q = n + p;
      Comment //a = b = c + d = e;
      Comment //z = y:4;
      Comment //x + y z;
      Comment //f a + b c + d;
      Comment //x + 5 = 4 + 1, 2, 3;
      Comment // d, e, f, g, h i;
      Comment // j;
      Comment // q + 4, d z l, 44;
      Comment // .;
      Comment // ..;
      Comment // ...;
      Comment // a.b;
      Comment // a..b;
      Comment // a...b;
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
*/
