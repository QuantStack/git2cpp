cmake_minimum_required(VERSION 3.28)
project(git2cpp-wasm)

add_custom_target(build-serve COMMAND npm install)

# Serve both cockle and JupyterLite deployments.
add_custom_target(serve COMMAND npm run serve)
add_custom_target(serve-with-cors COMMAND npm run serve-with-cors)
