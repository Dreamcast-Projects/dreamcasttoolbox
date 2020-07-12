// function baseName(filePath) {
//     return filePath.split(/[\\/]/).pop();
// }

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
            if(Module._mr_valid_file(data_ptr+MR_OFFSET) == 0)
            {
                drawMRImage(data_ptr+MR_OFFSET);
            }
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