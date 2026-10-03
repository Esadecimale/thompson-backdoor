# Trusting Trust, Hands On

In 1984 Ken Thompson, during his Turing Award acceptance speech, introduced to the world a very interesting kind of backdoor, described by himself as "the cutest program I have ever wrote". The speech has later been written in an article called "Reflections on Trusting Trust".

The idea is simple: modify a C compiler in order to insert a self-propagating backdoor that will replicate itself whenever the compromised compiler is used to compile a new compiler.

This workshop is all about compilers and backdoors!

## Get started

Download and compile original tinyCC

```sh
git clone https://repo.or.cz/tinycc.git
cd tinycc
./configure
make -j$(nproc)
./tcc -v
cd ..
```

Compile the login program with the tinyCC compiler we just built

```sh
cd login
make tcc
```

You can test out the login program

```sh
./login 
login: alice
Password:              # write 'demo' for password
Welcome alice

./login
login: ken
Password:              # write 'demo' for password
Login incorrect
```

## Step 1 - Basic login backdoor

The first patch showcases how to change the code that is compiled by
modifying the text in memory. Lets hard-code a backdoor that is
triggered everytime we execute the `login.c` program.

```sh
cd tinycc
patch -p1 < ../patch/1-login-backdoor.patch
./configure
make -j$(nproc)
cd ..
```

Once we have a dirty compiler we can test it on the login program

```sh
cd login
make tcc

./login
login: ken
Password:                                     # write 'demo' for password
you have root privileges
-sh-5.3$ 
```

## Step 2 - Quine

A quine is a program that prints its own code. We need to generate a quine in order to build the self-reproducing logic. To generate the quine we start from a tail `thompson-tail.c`.

```sh
cd ./quine
python gen.py
wrote thompson.c (1857 bytes) from thompson-tail.c
```

Compile and execute the quine. Notice that it will print itself, exactly itself! 

```sh
make tcc
./thompson > thompson2.c
diff thompson.c thompson2.c     # no stdout, equal file
```

When using `tcc` to compile even the ELF produced will be the same across the two sources.

```sh
make tcc2
diff thompson thompson2         # no stdout, equal file
```

## Step 3 - Self-propagating logic

The second patch adds the self-replicating logic into the
compiler. Remember to undo last patch before applying the new one.

```sh
cd tinycc
git checkout .                                   # remove last patch
patch -p1 < ../patch/2-self-propagation.patch    # apply new patch
./configure
make -j$(nproc)
cd ..
```

Compile login and introduce the backdoor

```sh
cd login
make tcc

./login
login: ken
Password:              # write 'demo' for password
you have root privileges
-sh-5.3$
```

Test the self-propagation mechanism by cloning a brand new version of
the compiler bug free and compiling it with the poisoned compiler.

```sh
git clone https://repo.or.cz/tinycc.git clean

cd clean && ./configure && make tccdefs_.h && cd ..

./tinycc/tcc -B./tinycc \
    -I./clean -I./clean/include \
    ./clean/tcc.c -o tcc2 -lm -ldl -lpthread
```

The poison compiler will compile the login backdoor.

```sh
./tcc2 -B./tinycc \
    -I./clean/include \
    -o login2 ./login/login.c -lcrypt

printf 'x\n' | ./login2 ken
```

## References

- [Reflections on Trusting Trust](https://www.cs.cmu.edu/~rdriley/487/papers/Thompson_1984_ReflectionsonTrustingTrust.pdf)
- [Multics Security Evaluation: Vulnerability Analysis](https://conferences.computer.org/sp/pdfs/early/karg74.pdf)
- [Countering Trusting Trust through Diverse Double-Compiling (DDC)](https://dwheeler.com/trusting-trust/)
- [Thirty Years Later: Lessons from the Multics Security Evaluation](https://www.acsac.org/2002/papers/classic-multics.pdf)
- [That Time Ken Thompson Wrote a Backdoor into the C Compiler](https://micahkepe.com/blog/thompson-trojan-horse/)
- [Running the "Reflections on Trusting Trust" Compiler](https://research.swtch.com/nih)