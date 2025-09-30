# Router Documentation

## Overview

The Router module provides HTTP request routing functionality for the webserver. It maps incoming HTTP requests to appropriate handlers based on server configuration, HTTP method, and URL path patterns.

## Architecture

### Core Components

- **Router**: Main routing class that manages route mappings
- **RequestProcessor**: Handles request execution and fallback logic
- **Handlers**: Specific implementations for different request types (GET, POST, DELETE, CGI, redirect)

### Route Storage Structure

```bash
server_id → path → HTTP_method → Handler_function
```

## Key Features

### Route Matching Strategies

1. **Exact Match**: Highest priority for precise path matches
2. **Extension Match**: Matches file extensions (e.g., `.py`, `.php`)
3. **Prefix Match**: Matches directory prefixes (e.g., `/uploads`, `/admin`)

### Handler Types

#### GET Handler

- **Purpose**: Serve static files and handle directory requests
- **Process**:
  1. Extract and validate file path from request
  2. Resolve path relative to server root directory
  3. Handle directory requests with autoindex support
  4. Serve static files with proper content type
  5. Handle index files (`index.html`) for root requests
- **Features**:
  - Directory listing with autoindex
  - MIME type detection
  - Error handling for missing files/directories

#### POST Handler

- **Purpose**: Handle file uploads via multipart/form-data
- **Process**:
  1. Find location with upload configuration
  2. Validate multipart/form-data content type
  3. Parse boundary and extract file data
  4. Validate filename and file size (1MB limit)
  5. Create upload directory if needed
  6. Write file to upload path
- **Features**:
  - Chunked request body support
  - File size validation
  - Directory creation
  - Duplicate filename handling

#### DELETE Handler

- **Purpose**: Remove files from upload directories
- **Process**:
  1. Find location with upload configuration
  2. Extract filename from request path
  3. Validate file existence
  4. Remove file from filesystem
  5. Return success/error response
- **Features**:
  - File existence validation
  - Safe file deletion
  - Proper error responses

#### CGI Handler

- **Purpose**: Execute CGI scripts and return results
- **Process**:
  1. **Validation Phase**: Validate location, path, and file
  2. **Path Resolution**: Determine CGI script path
  3. **File Validation**: Check if file exists and is executable
  4. **CGI Execution**: Set up environment and execute script
  5. **Response Parsing**: Parse CGI output (headers, status, body)
  6. **Response Building**: Set HTTP response from CGI result
- **Features**:
  - RFC 3875 compliant CGI implementation
  - Environment variable setup
  - Chunked body support
  - Timeout handling (504 Gateway Timeout)
  - Header parsing from CGI output

#### Redirect Handler

- **Purpose**: Handle HTTP redirects to configured URLs
- **Process**:
  1. Find location with redirect configuration
  2. Extract redirect URL from location
  3. Set 302 Found status code
  4. Set Location header
  5. Provide fallback HTML for non-automatic redirects
- **Features**:
  - Configurable redirect URLs
  - Proper HTTP status codes
  - Fallback HTML content
  - Keep-alive connection handling

## Handler Selection Logic

The router automatically selects the appropriate handler based on location configuration and HTTP method:

### Selection Priority

1. **Redirect Handler**: If `return_url` is configured in location
2. **CGI Handler**: If both `cgi_path` and `cgi_ext` are configured
3. **POST Handler**: If method is POST and `upload_path` is configured
4. **DELETE Handler**: If method is DELETE and `upload_path` is configured
5. **GET Handler**: Default fallback for all other cases

### Configuration-Based Routing

```cpp
if (!location.return_url.empty()) {
    // Use redirect handler
} else if (!location.cgi_path.empty() && !location.cgi_ext.empty()) {
    // Use CGI handler
} else if (method == http::POST && !location.upload_path.empty()) {
    // Use POST handler for uploads
} else if (method == http::DELETE && !location.upload_path.empty()) {
    // Use DELETE handler for file removal
} else {
    // Use GET handler as default
}
```

## Configuration Integration

The router integrates with server configuration files to automatically register routes based on:

- Server locations and their allowed methods
- CGI configurations (`cgi_path`, `cgi_ext`)
- Upload directories (`upload_path`)
- Redirect URLs (`return_url`)
- Autoindex settings for directory listing

## Usage

### Initialization

```cpp
Router router;
router.setupRouter(server_configs);
```

### Request Processing

```cpp
router.handleRequest(server, request, response);
```

## Route Resolution Priority

The `findHandler()` method implements a sophisticated route matching algorithm with the following priority order:

1. **Exact path match** (highest priority)
2. **Extension-based match** (e.g., `.py` files)
3. **Longest prefix match** (most specific directory)

### Handler Resolution Process

1. **Server Lookup**: Find the server in the routing table by server ID
2. **Exact Match**: Try to find exact path match first
3. **Advanced Matching**: If no exact match, find the best advanced match:
   - **Extension Matching**: Routes starting with `.` match file extensions
   - **Prefix Matching**: Routes match directory prefixes with validation
4. **Method Validation**: Ensure the route supports the requested HTTP method
5. **Best Match Selection**: Choose the most specific match (longest prefix or extension)

## Error Handling

- Invalid requests return appropriate HTTP error responses
- Missing routes fall back to default handlers
- Parser errors are handled gracefully

## Debug Features

- Route listing functionality for development
- Comprehensive logging of route registrations

## Dependencies

- Request/Response classes
- Server configuration
- HTTP constants
- Utility functions for path normalization and response building
