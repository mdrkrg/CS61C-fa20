from distutils.core import setup, Extension
import sysconfig

def main():
    CFLAGS = ['-g', '-Wall', '-std=c99', '-fopenmp', '-mavx', '-mfma', '-pthread', '-O3']
    LDFLAGS = ['-fopenmp']
    # Use the setup function we imported and set up the modules.
    # You may find this reference helpful: https://docs.python.org/3.6/extending/building.html
    # TODO: YOUR CODE HERE
    numc = Extension('numc', sources=['numc.c', 'matrix.c'], extra_compile_args=CFLAGS, extra_link_args=LDFLAGS)
    setup(name = "numc",
          version = '0.0a',
          ext_modules = [numc]
          )

if __name__ == "__main__":
    main()
