# Testing ARM Compute Library on QNX 7.1

```bash
# Move neon test binaries to your QNX target
scp ${QNX_TARGET}/aarch64le/usr/bin/neon_* root@<target-ip-address>:/usr/bin

# Move the ARM Compute Library to your QNX target
scp ${QNX_TARGET}/aarch64le/usr/lib/libarm_compute.so root@<target-ip-address>:/usr/lib

# ssh into your QNX target and run tests
# All tests should pass
ssh root@<target-ip-address>
neon_cnn
neon_copy_objects
neon_gemm_qasymm8
neon_permute
neon_scale
neon_sgemm
```
