const axios = require('axios');
const fs = require('fs');
const FormData = require('form-data');

async function uploadFile() {
    // Create a form and append a file to it
    const form = new FormData();
    form.append('file', fs.createReadStream('../Hello.pdf')); // Replace with the actual file path

    try {
        const response = await axios.post('http://127.1.0.0:4242/upload', form, {
            headers: {
                ...form.getHeaders() // Set the appropriate headers for multipart form-data
            }
        });

        console.log('File uploaded successfully:', response.data);
    } catch (error) {
        console.error('Error uploading file:', error.message);
    }
}

uploadFile();
