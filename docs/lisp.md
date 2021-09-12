We use a custom Lisp variant to encode resources. We anticipate an ingame programming capability and we need formats for various resource files, including models, materials, constructions etc. A Lisp could fulfill all of these roles. Some Lisp implementations exhibit the ability for incremental compilation - Lisp requires intepretation due to its runtime code evaluation capability but parts of programs can be compiled with no effect on program correctness. So it stands to reason that resource files, no matter whether they are programs, properties, or models, could be used equivalently in either text or byte-code format.

To work out the necessary properties of our Lisp flavour we explain Lisp in gradually increasing detail until a specification becomes evident.

The concept of Lisp starts very simply, with 2 fundamental types, the list and the atom. The list is a node of a singly-linked-list, containing only a data pointer and a next pointer. The atom is a symbol. The symbol supports 1 important operation: equality, checking whether 1 symbol is another. To the programmer they are represented by a name, and symbols are equal if and only if they have identical names. The list's pointers may each point to either an atom or a list. This presents the first implementation problem, because it is thus a dynamically typed system, which is unnatural in C++. There are 2 parts to this problem. The first is deciding how to get around C++'s static type correctness mechanisms (allowing us to use either a list or an atom in the same place), and the second is maintaining dynamic type correctness (allowing us to distinguish between a list and an atom).

void\*  
void\* pointers are a very blunt tool for the job. They can be casted from any pointer and to any pointer, allowing us to reject C++'s typing system entirely. They are not supposed to be used in most situations.

2 pointers
We could simply have 2 pointer fields for each link, making a total of 4 per list. This effectively doubles the size of list so is not ideal.

union  
A union is a data structure that holds 1 item, and that item is one of a specified selection of possible types. The union takes as much space as the biggest type. In this case we could use a union of atom\* and list\*. This has the advantage that the list is only the size of 2 pointers, which is as small as it can practically be. However, a union has no built-in way to determine what type it is currently holding. This could be solved by putting a variable alongside it, such as in the list structure, to signal the type. That way whenever we need to distinguish between them, we could use a simple conditional or switch on the flag variable. The way C++ works, however, this could make each list bigger by 2 bytes or more, even though we only need 2 bits. This would make up a significant portion of the list's size: being 2 pointers for a 32 bit system, the minimum size is 8 bytes.

There may be another way to signal type, using the fact that an atom could be smaller than a list, being one pointer if necessary. So what we could do is make the atom a struct of 2 pointers, with one of them being nullptr always.

It is sensible to make these atoms hold a pointer because the traditional implementation of the symbol equality operator is to checkk whether the symbols occupy the same memory - whether their pointers compare equal.

However, we need to consider what atoms do. Often they are simple symbols, but get any practical use out of a Lisp we require some extra variants. At a minimum we need built-in functions. This is technically all we need to make Lisp Turing-complete, but other types are very useful for practical purposes, such as numeric and string constants, and macro symbols. So, once we know that an item is an atom, we need to find out what kind of atom it is. So we need to store type information somewhere. So perhaps we could keep a type-flag that dually signals whether a pointer is an atom and what kind of atom it is. We could also do some trickery to keep this flag to 1 byte per list - namely by storing the flag for each of the 2 pointers in each half of the same byte.

What actually goes in a symbol though? We link to a symbol by pointing to some memory location, but what actually needs to be stored in that location. With the current system, we don't need anything to be stored there, but we do need each symbol's location to be unique so we have to allocate them anyway. However, there is another important thing to do with symbols, which is lookup. If a symbol is evaluated, it is supposed to evaluate to its lookup value. This doesn't technically have to be an operation on a symbol, because we can use the equality operator to implement lookup, but it is much more efficient to implement it as an operator. So, the first thing a symbol should contain is a pointer to its lookup value, which could be a list or an atom. Then we consider the textual representation of a symbol - symbols have names, and the name should be usable to access the symbol (for parsing), and the symbol should be usable to access the name (for printing). The first should probably be best implemented by a hash table, and the second by storing the string in the symbol.

So, a memory model presents itself. We have a hash table from names to symbols, where each symbol stores its name and a lookup pointer. We also have lists. But we don't know where to put them, or how to manage their memory. We could use the built in shared pointers, but they could add overhead.

Evidence suggests that compilers will tend to align data, with the consequence that there is no point in keeping extra data smaller than a pointer - the size of a list or atom will be a multiple of 4 bytes, so we may as well use the whole 4 bytes. With the list, we could use a byte for each type flag, which allows use to use the same flag type for the symbol's lookup value. This leaves 2 bytes in a list and 3 bytes in a symbol. The extra list bytes could be used for memory management purposes.

# binding
Symbols have differnt meanings in different contexts. How shall we implement this? And what about namespaces? First things first, we have a symbol table. One way to do things would be to store symbol lookup values on a stack.

# type system
It would be nice to give the option to use haskell-style type safety, while also allowing the traditional lisp scripting style of disregarding types. So, we could add optional qualifiers to functions. (I say qualifiers instead of types because I want to be able to use this system to guarantee function purity, which I am not sure counts as a type.) We're not doing C-style typing here, though. This is not about what something is but rather what can we do with it. This system has to be optional, but in a context with the system active, it is illegal for a function's return type qualifiers to not be guaranteed by the type qualifications of the functions in its body, or to pass into a function an argument that does not meet its parameter type qualifiers (the argument is either the result of a function, with qualifiers stated by that function, a parameter of a surrounding function wih qualifiers stated by *that* function, or a literal with qualifiers automatically generated).

The qualifiers themselves would be something akin to mathematical (group theory) statements, but influenced by haskell's system and possibly using lisp fragments, given in s-expression form. E.g.

(defun f (a b) (+ a b))

can be type qualified as

(defun (f natural pure) ((a natural) (b natural)) (+ a b))

In this case, we assume a built-in function + with a signature as if it were (defun (+ natural pure) ((a natural) (b natural)) ...) and the signatures trivially match. Note that since the function name and the parameters (call both 'labels' for conciseness) are atoms in usual lisp, but qualified labels are lists, we have a natural syntax for optional qualifiers. Note also that by putting the label first in the type specifier we deviate from C-style convention but gain a neat and intuitive way to combine multiple qualifiers on a label. It seems obvious for the actual combination to be conjuction, so (f natural pure) means that f satisfies both natural and pure.

The elephant in the room is the fact that the proposed signature of + is quite unhelpful for general purposes, because it would make (+ 5.6 2.4) illegal, which is rather annoying and suggests a need for (defun (+-real real pure) ((a real) (b real)) ...) allowing (+-real 5.6 2.4) which is both clunky and unsustainable (requiring specialised versions of each function for every possible argument type).

Haskell's type system seems to provide a better alternative - definitely typeclasses and perhaps monads should make things easier. However, Haskell has some serious conflicts with the principles of this language, such as requiring lists to be of uniform types, not to mention the execution model. Deciding how to make the system compatible will require serious thought. At any rate, it will most likely complicate the syntax, and I have an idea for that: use the other bracket types. Lisp only uses () brackets, which leaves [] and {} available for special syntax, such as type annotations. It would be neat to design the system in such a way that the content of such brackets becomes optional so they can simply be stripped in parsing to run in an unsafe 'scripting' mode. What if [] denoted 'proof guiding' (such as type annotation) while {} denoted 'compilation guiding' to suggest efficient ways for compilation processes to deal with particular pieces of code.

## an alternative
What about something closer to intelligent assertions? Something like (defun f (a b) (assume (a natural) (b natural)) (guarantee (f natural pure)) ...) where the qualification checker tries to prove the guarantee from the assumption. The syntax is clunkier but it allows labels to be linked e.g. we can now enforce a > b by adding (> a b) to the assumption. We can also use this system in other contexts, since with non-pure programs, function signatures are only half of the picture - we could put similar statements in objects or on variables or program state

## why?
Essential to programming is the interaction of assumptions and guarantees, which allows a programmer to narrow their focus. Without it, trying to reason about a program of any size is intractable. What we do instead is when writing a part of a program, we make assumptions about what we use in it and try to make something we can make a desired guarantee about, so that we can then make corresponding assumptions when using it in other program parts. Ideally this would allow us to know for sure that the program as a whole is as required. But programmers are fallible and we often oversimplify our assumptions, leading to the failure of guarantees. Hence we have type safety systems that attempt to prevent programmers from assuming what cannot be guaranteed. These same systems allow program transformers e.g. compilers/interpreters to make useful assumptions about code which allow them to do their work more effectively.

# Parsing

# application Interface
We could use class templates to define the management and lisp-operability of native objects, and then use these by instantiating a manager in the VM for each class we want (such as bignum and bool). Because we will most likely use garbage collection in general, the managers will have to work under the VM's main memory management system to track references. We will end up with a collection of memory containers, and for garbage detection, each container will have to track not only internal references, but also references between containers, and interface containers will have to also track references between the inside and outside of the VM.

Difficulties: an outwards reference cannot prevent the deletion of the entity it points to, so either the existence of the entity must be externally guaranteed (e.g. I think C++ functions are never deleted), or the lisp program can't assume the reference is valid. Smart pointers could potentially help with that but still, we should anticipate the possibility of lisp programs requiring correctness proofs beyond what external code can support (what if it throws exceptions?), so outwards references must be treated as suspect. Equally, external code can't assume the existence of a lisp entity, but a way comes to mind to deal with that: a system where external code deals with special Handle objects that can be casted to booleans like C++ pointers and null themselves when their target is deleted. Howver, this should only happen upon destruction of a VM, because a VM should not garbage collect entities that have external references to them.

# User Interface
I have an idea for user input interface. We could use a listener-stack. So, the base program has a stack frame of listeners, and entering a menu would push a frame, as would taking control of a vehicle. Exiting these contexts would pop their frames of course. Any input event starts at the top of the stack and filters down until it finds an appropriate listener. Thus higher frames can usually 'shadow' bindings of lower frames, which allows a menu or vehicle 'with focus' to easily take cursor 'focus' and use key bindings without worrying about lower levels capturing them first. However, frames can exert upwards control by reserving certain events, which prevents higher frames from capturing them. The Esc key might be a good candidate for this, to allow the user to escape any context without relying on the context itself to provide that functionality. The implementation for this feature suggests itself as a preliminary upward stack traversal followed by a downward one. Then, the natural progression for that is to maintain a duplex stack and prefer binding to the second (outwards) stack. This may present performance issues. Now, the issue of transparency - does an event get passed on from a particular frame? I think it makes sense for a handler to return a boolean value, say false means pass on (or 'throw') and true means the event is consumed.

Performance: We can take advantage of the fact that if a frame has inwards opacity to a particular event then any outwards binding on that event in that frame is shadowed. *But* the inwards binding may throw the event and break our assumption. So, that would require tracking whether each binding can throw, which seems impractical. I think then we could perhaps use the 'fake stack' trick used in the Lisp VM: instead of a stack of lookup tables, we keep a lookup table of stacks, and a tracking stack determining which lookup stacks need to be popped and when. It would be more complicated here because of the duplex stacks, but should keep lookup time low (the alternative poses a potential performance problem because it requires potentially, for each key press and mouse or analogue stick update, around 10 failed table lookups).

Now, how much of this should be in Lisp and how much native? And what should the structure be? 

# First-Class Functions
The language should emphasise functional programming, so we must support using functions as first-class objects. This raises the issue of non-local variables. Consider:

01> (define sumc
      (lambda (x)
        (lambda (y) (+ x y))))

This curried-sum function should be usable thusly:

02> (define add5 (sumc 5))
--> ?
03> (add5 4)
--> 9

Now how might the evaluation proceed?
(define add5 (sumc 5))
(define add5 ((lambda (x) (lambda (y) (+ x y))) 5))
(define add5 (lambda (y) (+ 5 y)))

(add5 4)
((lambda (y) (+ 5 y)) 4)
(+ 5 4)
9
9

Note how this requires the evaluation of y to 5, but does not require + to be evaluated at the same time.

# Elfin
Because lisp-like languages allow data and programs to have the same structure, there are interesting possibilites for a lisp version of JSON that could potentially be much more flexible. It could be called something like Lisp Functional Notation (LFN), pronounced 'Elfin' because why not. A range of names could justify that pronunciation - e.g. Extensible Linked Function Notation (ELFN). Actually, that's a decent name for the language itself.

# Code formats
Lisp-like languages present a particular challenge for compilation: Usually compilation converts text into an AST and then into bytecode or machine code for execution, but lisp expects the AST to be available at runtime, not to mention an interpreter. (Arguably, a lisp AST, if serialised, is itself bytecode.)

## 