# Bitmap Upscaler

Tool to upscale any .bmp by any amount, simply making every 1 pixel in the input to n by n in the output.

## To Compile
```
gcc upscaler.c -o upscaler
```
## Example Usage

To upscale a 16x16 bitmap called `sprite.bmp` by a factor of 3 to a 48x48 bitmap called `output.bmp`.
```
./upscaler 3 sprite.bmp output.bmp
```
