# Nuclear Reactor Simulator

![Banner](https://repository-images.githubusercontent.com/868880307/6d972a54-2261-4929-aa6b-a401a33f1025)
Built by _XnipS_.

# Compiling

## Windows (Untested, Yet)

- Clone with submodules `git clone --recurse-submodules https://github.com/XnipS/FluidisedBed.git`
- Download [SDL2](https://github.com/libsdl-org/SDL/releases/latest) `SDL2-devel-2.28.1-VC.zip` version and extract contents to NuclearReactor/vclib/SDL2
- If you are running in a VM (missing opengl3), then download [opengl32.dll](https://fdossena.com/?p=mesa/index.frag) and move to build output folder.
- May need to copy and paste relevant .dll from FluidisedBed/vclib/SDL2/lib to your build output folder.

## Linux

- Requires SDL2 from relevant package manager
