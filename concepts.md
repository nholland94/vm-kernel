# Programs

Programs are compiled to a special bytecode format just for this OS. The bytecode format provides a simple stack-based machine interface with all the operations required for general purpose programming. All system calls to perform operations through the kernel are also encapsulated in the bytecode format.

Programs contain headers that declare various pieces of information about the program, such as what system modules and libraries it requires. This is read in order to verify the program can be run on the current system.

# Libraries

Libraries are collections of bytecode that can be called by programs and other libraries. They are formatted similarly to programs, but are loaded at boot time (at least for right now).

Libraries are identified by a developer id and a name. Functions in libraries are referred to by name and arity. The idea is that, eventually, the libraries can be signed with a developer's cert and be securely loaded during runtime. For now, the developer id is merely a form of organization.

# System Modules

System modules are an important part to the OS which have direct access to the hardware the OS is running on. They need to be written/compiled for each target hardware architecture and (at least for right now) they must be loaded into the kernel ahead of time.

System modules expose system interfaces. These interfaces can be accesed directly by programs running on the OS through the use of a special opcode. Each program's header declares which system modules it depends on and those dependencies are checked during the program's initialization. A program can then call any of the functions provided by the sytem modules using the module name and the function name (TODO: determine how args are handled).

Examples of system modules:

- net
- keyboard
- mouse
- graphics

Note that system modules should provide high level APIs rather than low level interfaces. For example, a graphics card interface should be more similar to OpenGL or Vulkan than what a linux graphics card driver would expose. As an example, a net interface should implement low level protocols such as IP and maybe even TCP. However, http should be implemented in a library instead of a system module.
