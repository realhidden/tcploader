This is the source code for the windows gui. It's c style, but was created in c++ mode so there is no gurantees that it is valid c code.

Do note that there is a few extra .lib files in the linking, be sure to not screw those up if you don't use the provided project.

Do note that winmain does some important work, all of the work is not done by the dialog handler.

Cli is broken because I plain didn't have the time to get a single exe to do both console and windows. Make a new project or something and add it to the solution if you so desperatly want cli instead of the gui, but I personaly say: "don't run an exe, link with a library", so please avoid using a cli executable for c/c++ applications.

Icon and initial work done by dasda, the rest by henke37.

