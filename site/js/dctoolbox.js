// function baseName(filePath) {
//     return filePath.split(/[\\/]/).pop();
// }

function Utf8ArrayToStr(array) {
    var out, i, len, c;
    var char2, char3;

    out = "";
    len = array.length;
    i = 0;
    while(i < len) {
    c = array[i++];
    switch(c >> 4)
    { 
      case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
        // 0xxxxxxx
        out += String.fromCharCode(c);
        break;
      case 12: case 13:
        // 110x xxxx   10xx xxxx
        char2 = array[i++];
        out += String.fromCharCode(((c & 0x1F) << 6) | (char2 & 0x3F));
        break;
      case 14:
        // 1110 xxxx  10xx xxxx  10xx xxxx
        char2 = array[i++];
        char3 = array[i++];
        out += String.fromCharCode(((c & 0x0F) << 12) |
                       ((char2 & 0x3F) << 6) |
                       ((char3 & 0x3F) << 0));
        break;
    }
    }

    return out;
}

Module['onRuntimeInitialized'] = function() {
    console.log("WASM LOADED");
}

const dropArea = document.getElementById('uploadfile-bg');

['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => {
    dropArea.addEventListener(eventName, preventDefaults, false);
});

function preventDefaults (e) {
    e.preventDefault();
    e.stopPropagation();
}

dropArea.addEventListener('drop', droppedFile, false);

function droppedFile(e) {
    let dt = e.dataTransfer;
    let files = dt.files;

    processFile(files[0]);
}

function openFileBrowser() {
    document.getElementById("yourinputname").click();
}

function selectedFile() {
    let input = document.getElementById("yourinputname");

    processFile(input.files[0]);
}

function drawMRImage(data_ptr) {
    let mr_width_ptr = Module._malloc(4);
    let mr_height_ptr = Module._malloc(4);
    
    // Get Width and Height
    Module._mr_decode_width_height(data_ptr, mr_width_ptr, mr_height_ptr);
    let mr_width = Module.getValue(mr_width_ptr, "i32");
    let mr_height = Module.getValue(mr_height_ptr, "i32");
    Module._free(mr_width_ptr); 
    Module._free(mr_height_ptr);

    // Decode to raw 24bpp
    let mr_raw = Module._malloc(mr_width*mr_height*3);
    Module._mr_decode_to_raw(data_ptr, mr_raw);

    let canvas = document.getElementById("myCanvas");
    let ctx = canvas.getContext("2d");
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    let imgData = ctx.createImageData(mr_width, mr_height);
    let cImageData = new Uint8ClampedArray(Module.HEAPU8.buffer, mr_raw, mr_width*mr_height*3);
    let offset = 0;
    for (let i = 0; i < mr_width*mr_height*3; i += 3) {
        imgData.data[offset + 0] = cImageData[i+0];  // R value
        imgData.data[offset + 1] = cImageData[i+1];    // G value
        imgData.data[offset + 2] = cImageData[i+2];  // B value
        imgData.data[offset + 3] = 255;  // A value
        offset = offset + 4;
    }

    // Center image in Canvas
    let x_offset = (canvas.width - mr_width)/2;
    let y_offset = (canvas.height - mr_height)/2;
    ctx.putImageData(imgData, x_offset, y_offset);

    Module._free(mr_raw);
}

function processFile(file) {
    console.log(file.name);
    const file_ext = file.name.split('.').pop().toLowerCase();
    
    let reader = new FileReader();
    reader.readAsArrayBuffer(file);
    reader.onprogress = function(e) {
        if (event.loaded && event.total) {
            const percent = (event.loaded / event.total) * 100;
            console.log(`Progress: ${Math.round(percent)}`);
        }
    }
    reader.onload = function() {
        let file_data = new Uint8Array(reader.result, 0, reader.result.byteLength);
        console.log(file_data.length);

        var data_ptr = Module._malloc(file_data.length);
        if(data_ptr == 0){	// error allocating memory
            console.log("Error allocating memory");
        }
        Module.HEAPU8.set(file_data, data_ptr);

        if(Module._ip_valid_file(data_ptr) >= 0) {
            console.log("Is valid IP.BIN");
            const MR_OFFSET = 14368
            // create a var containing the address of struct
            let ip_fields_pp = Module._malloc(4);
            // Create a struct 
            Module._ip_field_create(ip_fields_pp);

            // Get the actual location of the object
            let ip_fields_p = Module.getValue(ip_fields_pp, "i32");

            Module._ip_extract_fields(data_ptr, ip_fields_p);

            let buffer = Module._malloc(128);
            Module._ip_field_get_company_name(ip_fields_p, buffer, 128);
            document.getElementById("companyname").value = Utf8ArrayToStr(new Uint8Array(Module.HEAP8.buffer, buffer, 128));
            Module._ip_field_get_software_title(ip_fields_p, buffer, 128);
            document.getElementById("softwaretitle").value = Utf8ArrayToStr(new Uint8Array(Module.HEAP8.buffer, buffer, 128));
            Module._ip_field_get_product_version(ip_fields_p, buffer, 128);
            document.getElementById("productversion").value = Utf8ArrayToStr(new Uint8Array(Module.HEAP8.buffer, buffer, 128));
            Module._ip_field_get_release_date(ip_fields_p, buffer, 128);
            document.getElementById("releasedate").value = Utf8ArrayToStr(new Uint8Array(Module.HEAP8.buffer, buffer, 128));
            Module._ip_field_get_boot_filename(ip_fields_p, buffer, 128);
            document.getElementById("bootfilename").value = Utf8ArrayToStr(new Uint8Array(Module.HEAP8.buffer, buffer, 128));
            if(Module._mr_valid_file(data_ptr+MR_OFFSET) == 0)
            {
                drawMRImage(data_ptr+MR_OFFSET);
            }

            Module._ip_field_destroy(ip_fields_p);
            Module._free(ip_fields_pp);
            Module._free(buffer);
        }
        else if(Module._mr_valid_file(data_ptr) >= 0) {
            drawMRImage(data_ptr);
        }
        else {
            console.log("Unrecognized file");
        }
        
        Module._free(data_ptr);        
    }
}

// input -> bytes of the file (Uint8Array), name of the file
// creates and downloads a file with that data
// function download_binary_file(bynary_data,  file_name){
//     // creating blob
//     var mimetype = "application/octet-stream";
//     var blob = new Blob([bynary_data], {type: mimetype});
//     var url = window.URL.createObjectURL(blob);
    
//     // download 
//     var element = document.createElement('a');
//     element.setAttribute('href', url);
//     element.setAttribute('download', (file_name));
//     element.style.display = 'none';
//     document.body.appendChild(element);
//     element.click();
//     document.body.removeChild(element);
//     window.URL.revokeObjectURL(url);
// }