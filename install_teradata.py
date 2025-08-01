# Install Teradata Dynamic Libraries
import os
import tarfile
import urllib.request

def install_linux_amd64():
    # Make a temporary directory for the installation
    temp_dir = "teradata_installation"
    if not os.path.exists(temp_dir):
        os.makedirs(temp_dir)

    os.chdir(temp_dir)
    print("Installing Teradata Dynamic Libraries for linux_amd64...")
    # Download the tar.gz file from the specified URL

    url = "https://test-bucket-ceiveran.s3.eu-west-1.amazonaws.com/td_linux.tar.gz"
    response = urllib.request.urlopen(url)
    if response.status != 200:
        print(f"Failed to download file from {url}. Status code: {response.status}")
        exit(1)

    print(f"Downloading Teradata Dynamic Libraries from {url}...")

    # Save the downloaded file
    tar_file_path = "td_linux.tar.gz"
    with open(tar_file_path, "wb") as file:
        file.write(response.read())

    print(f"Downloaded Teradata Dynamic Libraries to {tar_file_path}.")

    # Extract the tar.gz file
    with tarfile.open(tar_file_path, "r:gz") as tar:
        tar.extractall(path=".")

    # Clean up the tar.gz file
    os.remove(tar_file_path)
    print("Extracted Teradata Dynamic Libraries.")

    os.chdir("TeradataToolsAndUtilitiesBase")

    print("Run setup script to complete the installation.")

    setup_script_path = "setup.sh"
    if not os.path.exists(setup_script_path):
        print(f"Setup script {setup_script_path} not found. Please ensure the tar.gz file contains it.")
        exit(1)

    code = os.system(f"bash {setup_script_path} a")
    if code != 0:
        print("Failed to run the setup script. Please check the script and try again.")
        exit(1)

    print("Teradata Dynamic Libraries installed successfully for linux_amd64.")

def install_linux_arm64():
    pass

def main():
    # Get DUCKDB_PLATFORM environment variable
    platform = os.getenv("DUCKDB_PLATFORM")
    # Check if running in Docker
    in_docker = os.getenv("LINUX_CI_IN_DOCKER", "0") == "1"

    if platform is None:
        print("DUCKDB_PLATFORM environment variable is not set.")
        return

    print("Installing Teradata Dynamic Libraries for platform:", platform)

    if platform == "linux_amd64":
        if not in_docker:
            print("This script is intended to be run in a Docker container.")
            return

        install_linux_amd64()
        return

    print("Unsupported platform:", platform)

if __name__ == "__main__":
    main()