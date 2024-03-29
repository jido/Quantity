# Quantity
Number format to represent quantities for display and storage

* Efficient conversion to and from text
* What you see is what you get: it's a decimal format which means _no internal rounding, no hidden digits_
* Arbitrary precision
* Supports native equality and comparison of like numbers

## Small quantity (32 bit)

**Bit layout**

~~~
sxmmmmmmmmmmkkkkkkkkkkuuuuuuuuuu
~~~

s
  = sign

x
  = extension bit (0)

m
  = millions

k
  = thousands

u
  = units

* Numbers from _0_ to _999,999,999_
* Infinity has all bits set except for sign
* Negative quantities stored using two's complement
* If only the sign bit is set it indicates an invalid quantity (NaN)
* Decimal digits are stored in groups of three digits from **000** to **999**
* The _extension_ bit allows to represent quantities from a billion onwards as described below

### Examples

One
~~~
00000000000000000000000000000001
                      \___1____/
~~~

One thousand
~~~
00000000000000000000010000000000
            \___1____/\__000___/
~~~

Speed of light (299,792,458 m/s)
~~~
00010010101111000110000111001010
  \__299___/\__792___/\__458___/
~~~

Positive infinity
~~~
01111111111111111111111111111111
~~~

## Large quantity (variable length)

When the _extension bit_ is set a larger quantity is stored using as many bits as required.

The extended format starts with a 48-bit **header**:

~~~
sxxxnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
~~~

s
  = sign

x
  = extension bits (100)

n
  = number of _chunks_ (44 bits)

A _chunk_ consists of eight groups of three digits for a total of **24 digits**. Each chunk occupies _80 bits = 10 bytes_ in memory.

As previously, each group of three digits is stored as a 10-bit integer up to **999** (_in binary:_ 1111100111).

A special value of **1023** means that the group is not in use (_in binary:_ 1111111111). That is useful when the number of groups is not known in advance. This special value marks the last group of digits of the quantity.

A special value of **1022** or **1021** mean that one digit and two digits respectively are not in use in the preceding group (_in binary:_ 1111111110 and 1111111101). The unused digits should be all **0**.

The chunks follow immediately the header. The number of bytes occupied by the quantity is rounded up to the nearest multiple of four, which means that two bytes padding are added when _n_ is even.

The padding is normally filled with ones. The first 10 bits of the padding can be used for special values _1022_ or _1021_ when the last one or two digits of the previous chunk should be discarded.

## Extensions

The default extension value is **100** (binary).

The number of chunks is encoded using _44 bits_ which corresponds to a quantity with up to **4×10¹⁴ digits**.

Other extension values put some of those bits to a different use:

~~~
100 : default
101 : with exponent
110 : 64 bit floating point
111 : variable length floating point
~~~

### Exponent extension

The format recognises a special case where all the end digits of the quantity are _zeros_.

Instead of using more chunks to write these zeros, the first 16 bits of _n_ are used to store an _exponent_ which indicates the number of trailing zeros.

**Header bit layout**

~~~
sxxxeeeeeeeeeeeeeeee nnnnnnnnnnnnnnnnnnnnnnnnnnnn
~~~

s
  = sign

x
  = extension

e
  = exponent (16 bits)

n
  = number of _chunks_ (28 bits)

**Example**

Advogadro constant (6.02214076×10²³)
~~~
01010000000000001000 0000000000000000000000000001
    \__exponent____/ \_____number of chunks_____/
10010110100011010110000100110000000000000000000000000000000000000000000000000000
\__602___/\__214___/\__076___/
~~~

### Floating point extension

The above _exponent extension_ could also be used to write floating point quantities by deciding a maximum number of figures after the decimal point and adjust the exponent by the same amount (_exponent bias_). 

However, that is not an efficient method if the quantities have a wide range of magnitude.

Instead it is preferable to fix the position of the decimal point _after the first digit_ and use chunks for the fractional part only.

In the floating point extension, the exponent is immediately followed by the first digit of the quantity.

**Bit layout**

~~~
64 bit:
sxxxeeeeeeeeeeeeeeeedddd mmmmmmmmmmµµµµµµµµµµnnnnnnnnnnpppppppppp

Variable length header:
sxxxeeeeeeeeeeeeeeeedddd nnnnnnnnnnnnnnnnnnnnnnnn
~~~

s
  = sign

x
  = extension (110 or 111)

e
  = exponent (16 bits)

d
  = first digit (1 to 9)

m
  = millis

µ
  = micros

p
  = picos

n
  = nanos _(64 bit)_ or number of chunks _(variable length)_

**Example**

Electron mass at rest (9.1093837015×10⁻³¹ kg)
~~~
011001111111111000011001 0001101101010111111110101111010111110100
    \___exponent___/\9_/ \__109___/\__383___/\__701___/\__500___/
~~~

### Variations

An implementation could allow only _fixed length_ quantities, which are **32 bit small quantity** and **64 bit floating point quantity**. In case of a 32 bit quantity a second value could be added to write a _fraction (x/y)_.

If _28 bits_ are always enough to store the number of chunks, the default extended format without exponent can be dropped altogether. Then quantities can have up to **6×10⁹ non-zero digits**.

If there is no need for an _explicit fractional part_, the variable length floating point extension can be dropped. The 64 bit floating point extension can be made to behave like the exponent extension by using _exponent bias +12_.

## Practical considerations

### Endianness

Quantities should always be in **big endian** format for storage or exchange. This is because the textual representation of a quantity puts the most significant digit first.

However, it is acceptable to use **little endian** format for local usage. The byte length of a quantity is always a multiple of _four_, so it is practical to handle the data as a sequence of _32 bit values_ (four bytes) using the platform natural endianness. The extension bits and number of chunks may need to be duplicated or relocated when handling a quantity in _little endian_ format.

### Compression

The format does not define a compression scheme.

If the data contains patterns (repeated digits or sequence of digits), it is likely it will benefit from compression. This can be applied on top of the format as seen fit.

### Normalisation

To make comparison more efficient quantities should be normalised, which generally means they should be written using as less bytes as possible.

The options below should be evaluated in order and the first fit be taken.

**_Integer quantity_**

Option | Extension bits | Number of chunks | Length | Application
---|---|---|---|---
_Small quantity_ | 0 | - | 32 bit | value up to 999,999,999 and special values
_With exponent_ | 101 | 0 | 48 bit | powers of 10
_64 bit floating point_ | 110 | - | 64 bit | up to 13 significant figures
_With exponent_ | 101 | any | 128 bit+ | values ending with zeros
_Default extension_ | 100 | any | 128 bit+ | other values

**_Fractional quantity_**

Option | Extension bits | Number of chunks | Length | Application
---|---|---|---|---
_Small quantity_ | 0 | - | 32 bit | round value up to 999,999,999 and special values
_Floating point_ | 111 | 0 | 48 bit | single significant figure
_64 bit floating point_ | 110 | - | 64 bit | up to 13 significant figures
_Floating point_ | 111 | any | 128 bit+ | other values

### Infinitesimal quantity

Although the first digit of a floating point value would normally be between one and nine, nothing prevents from making it zero instead.
A floating point value which first digit is zero and with zero chunks denotes a quantity which is equivalent to zero, such as _1/x when x ⟶ ∞_. It can be positive or negative.

**Example**

+ε
~~~
011100000000000000000000 000000000000000000000000
~~~

Another use for zero as first digit is to recover from a parsing error. For example, when converting from text to a quantity, floating point values may be detected by looking at a finite number of characters. When a decimal point appears after the detection point the quantity may already be written as a sequence of chunks without a first digit. It can be easily converted to a (non normalised) floating point value with _first digit = 0_.

### Operations

Quantities do not define any operation other than equality, comparison and conversion to and from other formats such as text.

One reason is that combining quantities with very different magnitude can result in extreme memory consumption, for example _10¹⁰⁰⁰⁰⁰ + 1_ does not have a space-efficient representation.
