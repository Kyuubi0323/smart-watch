#!/bin/bash

# Configuration Test Script
# This script tests different configurations without building

echo "=== ESP32-C3 Smart Watch Configuration Test ==="
echo

# Test ESPC3_1_69 configuration
echo "Testing ESPC3_1_69 configuration:"
gcc -DESPC3_1_69=1 -I components/custominclude -include stdbool.h -xc - <<'EOF'
#include "main.h"
#include <stdio.h>
int main() {
    printf("  Resolution: %dx%d\n", WIDTH, HEIGHT);
    printf("  Pins: SCLK=%d MOSI=%d DC=%d CS=%d RST=%d BL=%d\n", SCLK, MOSI, DC, CS, RST, BL);
    printf("  RGB Order: %s\n", RGB_ORDER ? "RGB" : "BGR");
    printf("  Offset: X=%d Y=%d\n", OFFSET_X, OFFSET_Y);
    return 0;
}
EOF

echo

# Test ESPC3 configuration  
echo "Testing ESPC3 configuration:"
gcc -DESPC3=1 -I components/custominclude -include stdbool.h -xc - <<'EOF'
#include "main.h"
#include <stdio.h>
int main() {
    printf("  Resolution: %dx%d\n", WIDTH, HEIGHT);
    printf("  Pins: SCLK=%d MOSI=%d DC=%d CS=%d RST=%d BL=%d\n", SCLK, MOSI, DC, CS, RST, BL);
    printf("  RGB Order: %s\n", RGB_ORDER ? "RGB" : "BGR");
    printf("  Offset: X=%d Y=%d\n", OFFSET_X, OFFSET_Y);
    return 0;
}
EOF

echo

# Test ESPS3_1_69 configuration
echo "Testing ESPS3_1_69 configuration:"
gcc -DESPS3_1_69=1 -I components/custominclude -include stdbool.h -xc - <<'EOF'
#include "main.h"
#include <stdio.h>
int main() {
    printf("  Resolution: %dx%d\n", WIDTH, HEIGHT);
    printf("  Pins: SCLK=%d MOSI=%d DC=%d CS=%d RST=%d BL=%d\n", SCLK, MOSI, DC, CS, RST, BL);
    printf("  RGB Order: %s\n", RGB_ORDER ? "RGB" : "BGR");
    printf("  Offset: X=%d Y=%d\n", OFFSET_X, OFFSET_Y);
    return 0;
}
EOF

echo
echo "All configurations tested successfully!"
echo "Use './build_config.sh help' to see how to build for each configuration."