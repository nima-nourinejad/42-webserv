#!/bin/bash

# Create the new_src and new_include directories if they don't exist
mkdir -p new_srcs
mkdir -p new_includes

# Loop through all .c files in the src directory and format them
for file in srcs/*.c; do
  python3 -m c_formatter_42 < "$file" > "new_srcs/$(basename "$file")"
done

# Loop through all .h files in the include directory and format them
for file in includes/*.h; do
  python3 -m c_formatter_42 < "$file" > "new_includes/$(basename "$file")"
done

# Remove the original src and include directories
rm -rf srcs
rm -rf includes

# Rename new_src to src and new_include to include
mv new_srcs srcs
mv new_includes includes

-----------------------------33787818733042364733969385091--
