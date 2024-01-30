# Use NVIDIA CUDA base image
FROM ubuntu:23.04

ARG DEBIAN_FRONTEND=noninteractive

ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES compute,graphics,utility

RUN apt-get update && apt-get install -y \
    curl \
    wget  \
    gnupg


# Get the Nvidia container toolkit
RUN curl -fsSL https://nvidia.github.io/libnvidia-container/gpgkey | gpg --dearmor -o /usr/share/keyrings/nvidia-container-toolkit-keyring.gpg \
    && curl -s -L https://nvidia.github.io/libnvidia-container/stable/deb/nvidia-container-toolkit.list | \
    sed 's#deb https://#deb [signed-by=/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg] https://#g' | \
    tee /etc/apt/sources.list.d/nvidia-container-toolkit.list

# Update packages and install required dependencies
RUN apt-get update && apt-get install -y \
    xauth \
    libglfw3 \
    libglfw3-dev \
    libglm-dev \
    xorg-dev \
    libglu1-mesa-dev \
    libx11-dev \
    libxcb1 \
    libxcb1-dev \
    mesa-utils \
    libxext6 \
    nvidia-container-toolkit \
    nvidia-container-runtime \
    libqt5x11extras5
    
RUN wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | tee /etc/apt/trusted.gpg.d/lunarg.asc && \
    wget -qO /etc/apt/sources.list.d/lunarg-vulkan-jammy.list http://packages.lunarg.com/vulkan/lunarg-vulkan-jammy.list
    
RUN apt update && apt install -y \
    libvulkan1 \
    libvulkan-dev \
    vulkan-sdk \
    vulkan-tools \
    pcmanfm \
    vim \
    git


# Install gcc, g++, gdb, cmake
RUN apt update && apt install -y build-essential cmake

# Nvidia driver details
ENV VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/nvidia_icd.x86_64.json
COPY nvidia_icd.json /usr/share/vulkan/icd.d/nvidia_icd.x86_64.json

# Install vulkan memory allocator
RUN git clone https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
WORKDIR /VulkanMemoryAllocator/
RUN cmake -S . -B build/
RUN cmake --install build --prefix build/install

#ENV CMAKE_PREFIX_PATH+=";/VulkanMemoryAllocator/build/install/;"
ENV VulkanMemoryAllocator_DIR=/VulkanMemoryAllocator/build/install/share/cmake/VulkanMemoryAllocator/

# Set app directory
COPY CMakeLists.txt /app/
COPY .gitignore /app/
COPY Dockerfile /app/
COPY src/ /app/src/
COPY include/ /app/include/
WORKDIR /app

ENV QT_DEBUG_PLUGINS=1
#RUN export QT_PLUGIN_PATH=/usr/lib/qt/plugins


# X11 Server
ENV DISPLAY=:0
ENV PULSE_SERVER=/tmp/PulseServer

# Entry point or default command to run when the container starts
CMD ["/bin/bash"]