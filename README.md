### Introduction
node-ndll is a small native addon which allows you to use NDLLs from Node.js applications.
I created this module to build Lime/OpenFL applications that runs on Node's V8 JavaScript engine.

### Installation
You need to have some C++ compiler installed to build this addon with node-gyp. After that you can simply install with npm.

To install node-ndll globally, use:

    npm -g install node-ndll

Note that you also need to set NODE_PATH manually to load globally installed modules.
On windows, set this to

    C:\Users\Your_User_Name\AppData\Roaming\npm\node_modules
