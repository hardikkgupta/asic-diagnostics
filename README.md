# ASIC Diagnostics Kernel Driver

A high-performance Linux kernel module for ASIC diagnostics that provides efficient register access and PCIe event capture capabilities. This driver is designed for network ASIC diagnostics with a focus on performance, reliability, and ease of debugging.

## Overview

The ASIC Diagnostics driver provides a robust interface for accessing ASIC registers and capturing PCIe events through sysfs and debugfs interfaces. It implements interrupt-driven event capture with optimized buffer management to minimize CPU overhead.

## Key Features

- Register access through sysfs/debugfs interfaces
- Interrupt-driven PCIe event capture
- Optimized buffer management using kfifo
- Comprehensive test suite
- Automated regression testing
- Low CPU utilization design

## Project Structure

```
.
├── src/                    # Source code
│   ├── asic_diag.c        # Main driver implementation
│   ├── asic_diag.h        # Driver header
│   ├── pcie_events.c      # PCIe event handling
│   └── sysfs_interface.c  # Sysfs/debugfs interface
├── tests/                  # Test suite
│   ├── test_driver.c      # Kernel module tests
│   └── test_script.sh     # Test automation script
├── Makefile               # Build system
└── README.md             # This file
```

## Requirements

- Linux kernel headers
- GCC compiler
- Make build system
- Root access for module loading

## Building

1. Install required dependencies:
   ```bash
   sudo apt-get install linux-headers-$(uname -r)
   ```

2. Build the kernel module:
   ```bash
   make
   ```

3. Load the module:
   ```bash
   sudo insmod asic_diag.ko
   ```

## Testing

The project includes two levels of testing:

1. Basic functionality tests:
   ```bash
   cd tests
   ./test_script.sh
   ```

2. Comprehensive kernel tests:
   ```bash
   cd tests
   make
   sudo insmod test_driver.ko
   ```

## Usage

### Register Access

Access ASIC registers through sysfs:
```bash
# Read register
cat /sys/bus/pci/devices/*/asic_diag/00

# Write register
echo "0x12345678" > /sys/bus/pci/devices/*/asic_diag/04
```

### Event Monitoring

Monitor captured events through debugfs:
```bash
cat /sys/kernel/debug/asic_diag/events
```

## Performance Optimization

The driver implements several optimizations:
- Interrupt-driven event capture
- Lock-free buffer management
- Efficient register access
- Minimal CPU overhead

## Development

### Adding New Features

1. Add register definitions in `asic_diag.h`
2. Implement functionality in appropriate source file
3. Add sysfs/debugfs interface if needed
4. Update test suite

### Debugging

1. Check kernel logs:
   ```bash
   dmesg | grep asic_diag
   ```

2. Monitor debugfs entries:
   ```bash
   cat /sys/kernel/debug/asic_diag/events
   ```
4. Push to the branch
5. Create a Pull Request

## Support

For issues and feature requests, please create an issue in the repository.
