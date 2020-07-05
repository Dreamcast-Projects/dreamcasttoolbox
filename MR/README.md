# MR Logo

The file format is composed of three sections: Header, Palette, and the Image Data. All of the data written in the file is in little-endian format.

A MR file starts off with a 30 byte header.

**Header** 

<table>
  <tr>
    <th>Size</th>
    <th>Contents</th>
  </tr>
  <tr>
    <td>2 bytes</td>
    <td>"MR"</td>
  </tr>
  <tr>
    <td>4 bytes</td>
    <td>Total file size</td>
  </tr>
  <tr>
    <td>4 bytes</td>
    <td>Crap (fill with 0's)</td>
  </tr>
  <tr>
    <td>4 bytes</td>
    <td>Image data offset in bytes (Header size + Palette size)</td>
  </tr>
  <tr>
    <td>4 bytes</td>
    <td>Image width</td>
  </tr>
  <tr>
    <td>4 bytes</td>
    <td>Image height</td>
  </tr>
  <tr>
    <td>4 bytes</td>
    <td>Crap (fill with 0's)</td>
  </tr>
  <tr>
    <td>4 bytes</td>
    <td>Amount of colors in palette</td>
  </tr>
</table>

**Palette**

The header is followed by the image's palette. The palette is composed of palette entries where each palette entry is 4 bytes long and is stored as BGRA. The alpha byte goes unused. The maximum number of palette entries is 128.

<table>
  <tr>
    <th colspan=4>Palette Entry</th>
  </tr>
  <tr>
    <td>B</td>
    <td>G</td>
    <td>R</td>
    <td>A</td>
  </tr>
</table>

In total, the byte size of your palette should be number of colors * 4.

**Image Data**

Lastly, the image data. The image data is basically an array of indices, each a byte long, that refers to a palette entry in the palette. The image data is compressed using a form of Run-Length Encoding (RLE).
