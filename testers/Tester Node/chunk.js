const http = require('http');

const options = {
    hostname: '127.1.0.0',
    port: 4242,
    path: '/',
    method: 'GET',
    headers: {
        'Transfer-Encoding': 'chunked',
    },
};

let body = '';
const req = http.request(options, (res) => {
    res.on('data', (chunk) => {
        body += chunk;
    });
	
	res.on('end', () => {
		console.log(body);
	});
});



req.write('Hello ');
req.write('World!');
req.end();