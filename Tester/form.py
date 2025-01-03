import requests

# Function to upload a file
def upload_file(file_path, url):
    # Open the file in binary mode
    with open(file_path, 'rb') as file:
        # Create a dictionary of the file to be sent in the form-data
        files = {'file': (file_path, file)}
        
        # Send the POST request
        response = requests.post(url, files=files)

        # Check if the upload was successful
        if response.status_code == 200:
            print("File uploaded successfully:", response.text)
        else:
            print(f"Failed to upload file. Status code: {response.status_code}")
            print("Response:", response.text)

# Example usage
if __name__ == "__main__":
    file_path = 'Hello.pdf'  # Replace with the path to your file
    url = 'http://127.1.0.0:4242/uploads'  # Replace with your server's upload URL
    upload_file(file_path, url)
