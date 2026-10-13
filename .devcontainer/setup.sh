#!/usr/bin/env bash

set -euo pipefail

# Add an inputX group with the host /dev/input GID and add current user to it
if [[ ! -e /dev/input/event0 ]]; then
    echo "Error: /dev/input/event0 does not exist"
    exit 1
fi

HOST_INPUT_GID=$(stat -c %g /dev/input/event0)
INPUT_GROUP_NAME="input${HOST_INPUT_GID}"

if ! getent group "$INPUT_GROUP_NAME" > /dev/null; then
    sudo groupadd "$INPUT_GROUP_NAME" --non-unique --gid "$HOST_INPUT_GID"
fi

if ! id -nG "$USER" | tr ' ' '\n' | grep -qx "$INPUT_GROUP_NAME"; then
    sudo usermod -a -G "$INPUT_GROUP_NAME" "$USER"
fi

# Add a line to .bashrc to automatically switch to the input group on new shell sessions. This is necessary because the
# terminal you run in VS Code runs on the same shell as the one that runs this script, meaning a single newgrp call 
# would not suffice in a case where the user opens another separate terminal in VS Code. So we'll just add that newgrp
# call to .bashrc and it should be executed on every new shell session.
BASHRC_PATH="$HOME/.bashrc"
MARKER="# devcontainer: apply $INPUT_GROUP_NAME group"
if ! grep -qF "$MARKER" "$BASHRC_PATH"; then
   printf '\n%s\nif ! id -nG | grep -qw "%s"; then newgrp "%s"; fi\n' \
       "$MARKER" "$INPUT_GROUP_NAME" "$INPUT_GROUP_NAME" >> "$BASHRC_PATH"
fi

# Add gallery demo app symlink for easier access
sudo ln -s '/usr/lib/x86_64-linux-gnu/qt6/examples/widgets/gallery/bin/gallery' '/usr/local/bin/qt-gallery-demo'
