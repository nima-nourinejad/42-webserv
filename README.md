# 42 Webserv

This repository contains the `Webserv` project from the 42 school curriculum, which involves developing a simple `HTTP` web server in C++. The objective is to understand and implement the fundamental components of a web server, including handling `HTTP` requests, managing client connections, and serving static and dynamic content.

---

## Table of Contents

1. [Introduction](#introduction)
2. [Features](#features)
3. [Installation](#installation)
4. [Usage](#usage)
5. [Configuration](#configuration)
6. [Project Structure](#project-structure)
7. [License](#license)

---

## Introduction

The `Webserv` project is part of the 42 school's curriculum, aiming to provide students with hands-on experience in building a web server from scratch. The project emphasizes understanding network protocols, socket programming, and managing concurrent connections, providing a strong foundation in web server functionality.

---

## Features

- **HTTP Request Handling**: Supports various `HTTP` methods, including `GET`, `POST`, and `DELETE`.
- **Static Content Serving**: Serves static files such as HTML, CSS, and JavaScript.
- **Dynamic Content via CGI**: Supports executing external programs using the Common Gateway Interface (`CGI`) to generate dynamic content.
- **Configuration Parsing**: Parses configuration files to customize server behavior.
- **Concurrent Client Handling**: Manages multiple client connections concurrently using non-blocking I/O and the `select()` system call.

---

## Installation

1. **Clone the Repository**

	```bash
	git clone https://github.com/Sepahsalar/42-webserv.git
	```

2. **Navigate to the Project Directory**

	```bash
	cd 42-webserv
	```

3. **Compile the Program**

	```bash
	make
	```

---

## Usage
After compilation, run the server with an optional configuration file:

```bash
./webserv [config_file]
```

If no configuration file is specified, the server will use a default configuration.

Example:

```bash
./webserv config/webserv.conf
```

Once the server is running, you can access it via a web browser or any `HTTP` client by navigating to the server's address and port specified in the configuration.

---

## Configuration

The server behavior can be customized using a configuration file. The configuration file allows you to define settings such as:

- **Listening Port**: Specify the port on which the server listens for incoming connections.
- **Server Name**: Define the server's hostname.
- **Root Directory**: Set the directory from which static files will be served.
- **Error Pages**: Configure custom error pages for different `HTTP` error codes.
- **CGI Scripts**: Define the file extensions and corresponding executables for handling dynamic content.

An example configuration file is provided in the config directory of the repository.

---

## Project Structure

The project is structured as follows:

```bash
42-webserv/
├── cgi/                # CGI scripts
├── config/             # Configuration files
├── html/               # Static HTML files
├── includes/           # Header files
├── srcs/               # Source code files
├── testers/            # Testing scripts and tools
├── var/www/            # Web root directory
├── LICENSE             # License information
├── Makefile            # Build configuration
└── README.md           # Project documentation
```

---

##License

This project is licensed under the MIT License. See the [LICENSE](https://github.com/Sepahsalar/42-webserv/blob/main/LICENSE) file for details.
