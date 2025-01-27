const axios = require('axios');

// Replace with your actual URL
const url = 'http://127.1.0.0:4242/uploads/map';

// Send DELETE request
axios.delete(url)
  .then(response => {
    console.log('File deleted successfully:', response.data);
  })
  .catch(error => {
    console.error('Error deleting file:', error);
  });
