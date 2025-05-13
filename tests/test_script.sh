#!/bin/bash

# Test script for ASIC Diagnostics Driver

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Function to check if a command succeeded
check_status() {
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ $1${NC}"
    else
        echo -e "${RED}✗ $1${NC}"
        exit 1
    fi
}

echo "Starting ASIC Diagnostics Driver Tests..."

# Check if module is loaded
if lsmod | grep -q "asic_diag"; then
    echo "Removing existing module..."
    sudo rmmod asic_diag
    check_status "Remove existing module"
fi

# Build and load the module
echo "Building and loading module..."
make
check_status "Build module"
sudo insmod asic_diag.ko
check_status "Load module"

# Check if debugfs entry exists
if [ ! -d "/sys/kernel/debug/asic_diag" ]; then
    echo -e "${RED}Debugfs entry not found${NC}"
    exit 1
fi

# Test register access through sysfs
echo "Testing register access..."
for reg in 00 04 08 0c; do
    if [ -f "/sys/bus/pci/devices/*/asic_diag/$reg" ]; then
        echo -e "${GREEN}✓ Register $reg accessible${NC}"
    else
        echo -e "${RED}✗ Register $reg not accessible${NC}"
        exit 1
    fi
done

# Test event buffer
echo "Testing event buffer..."
echo "1" > /sys/bus/pci/devices/*/asic_diag/04  # Trigger an event
sleep 1
if [ -s "/sys/kernel/debug/asic_diag/events" ]; then
    echo -e "${GREEN}✓ Event captured${NC}"
else
    echo -e "${RED}✗ No events captured${NC}"
    exit 1
fi

# Unload the module
echo "Unloading module..."
sudo rmmod asic_diag
check_status "Unload module"

echo -e "${GREEN}All tests passed!${NC}" 