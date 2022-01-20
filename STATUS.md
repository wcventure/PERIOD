### Apply for the Functional, Reusable, and Available Badges 
 
We are applying the *Available* and *Reusable* *badges* for the following reasons.
- We have made our prototype tool available via Github, a publicly accessible archival repository. (*Available badges*)
- We have provided two different ways to build and run our artifact (*Available badges*):
    - (i) We provided a pre-built docker image (compressed size: 1.56 GB) that contains our prototype tool, all the benchmark programs, and our scripts, which should work out-of-the-box. We also provided the `dockerfile`, thus you can automatically build our tool through `docker build` if you fail to download the pre-built Docker image.
    - (ii) We provided instructions to build our prototype tool on your host system, step by step. This makes it convenient for the second development.
- We carefully documented and structured the artifact to the extent that reuse and repurposing are facilitated.  (*Reusable badges*)
- In particular, we also made our experimental benchmark available and provided some shell scripts to automatically build and perform controlled concurrency testing on the benchmark programs. This should allow you to reproduce our experimental evaluation. (*Reusable badges*)