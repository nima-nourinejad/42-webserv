const axios = require('axios');
const fs = require('fs');
const FormData = require('form-data');

// Ensure you are using cors on the server side, not here
// const cors = require('cors');
// app.use(cors());

async function uploadFile() {
    const filePath = '../Hello.pdf'; // Ensure this file path is correct

    // Check if file exists before proceeding
    if (!fs.existsSync(filePath)) {
        console.error('File does not exist:', filePath);
        return;
    }

    // Get file stats to check its size
    const fileStats = fs.statSync(filePath);
    console.log(`Preparing to upload file: ${filePath}`);
    console.log(`File size: ${fileStats.size} bytes`);

    // Create a form and append the file to it
    const form = new FormData();
    const fileStream = fs.createReadStream(filePath);
    form.append('file', fileStream);

    try {
        // Logging the headers being sent to the server
        const headers = {
            ...form.getHeaders(),
            'Content-Length': fileStats.size // Explicitly set the content-length header if necessary
        };
        console.log('Request Headers:', headers);

        // Start file upload with axios
        const response = await axios.post('http://127.1.0.0:4242/upload', form, { headers });

        // Log the response from the server
        console.log('File uploaded successfully:', response.data);
    } catch (error) {
        // Detailed error handling
        if (error.response) {
            console.error('Server responded with error:', error.response.status, error.response.data);
        } else if (error.request) {
            console.error('No response received from server. Request details:', error.request);
        } else {
            console.error('Error in setting up the request:', error.message);
        }
    }
}

uploadFile();
