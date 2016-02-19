# preview
image preview in your terminal

*preview* is a small tool to display images inside you terminal.
It is build using C++14 and use internally the awesome library [CImg](https://github.com/dtschump/CImg "CImg") 
to read images, iterates overs pixels.

## Screenshot
Look what a banana looks like with *preview*
![Screenshot](/img/screenshot.png "Banana")

## Installation
*preview* is built using cmake which will download autonomously the required library locally, you just 
need to type from the root repository:
```Shell
> mkidr build && cd build
> cmake ..
> make
```
Depending of the libraries installed on you computer, *preview* will be able to support different 
file format

## Examples
Some example images are available inside the repository, we can use them this way (assuming you're 
inside the build folder):
```Shell
> ./preview ../img/banana.png
```
This should display a banana inside your browser

## License

*preview* is licensed under the MIT license, you can find a copy of the license at the root of the repository.
