### Apply for the Reusable and Available Badges 
 
We are applying for the *Available* and *Reusable* *badges* for the following reasons.
- We have made our prototype tool available via Github, a publicly accessible archival repository. (*Available badge*)
- We have provided two different ways to build and run our artifact (*Available badge*):
    - (i) We provided a pre-built docker image (compressed size: 1.56 GB) that contains our prototype tool, all the benchmark programs, and our scripts, which should work out-of-the-box. We also provided the `dockerfile`, thus you can automatically build our tool through `docker build` if you fail to download the pre-built Docker image.
    - (ii) We provided instructions to build our prototype tool on your host system, step by step. This makes it convenient for secondary development.
- We carefully documented and structured the artifact to the extent that reuse and repurposing are facilitated.  (*Reusable badge*)
- In particular, we also made our experimental benchmark available and provided some shell scripts to automatically build and perform controlled concurrency testing on the benchmark programs. This should allow you to reproduce our experimental evaluation. (*Reusable badge*)