### Apply for the Functional, Reusable, and Available Badges 
 
We are applying the functional, reusable, and available badges for the following reason.
- we have made our prototype tool available via Github; we placed it on the publicly accessible archival repository.
- we carefully documented and well-structured to the extent that reuse and repurposing are facilitated.
- we have provided two different ways to build and run our artifact: 
    - (i) we provide a pre-built docker image (3.9G) that contains our prototype tool, all the benchmark programs, and our scripts, which should work out-of-the-box; we still provide the `dockerfile`, thus you can automatically build our tool through `docker build` if you fail to download the pre-built Docker image (3.9G).
    - (iii) We also provide instructions to build our prototype tool on your host system, step by step. This makes it convenient for the second development.
- In particular, we also made our experimental benchmark available and provided some shell scripts to automatically build and perform controlled concurrency testing on the benchmark programs. This should allow you to reproduce our experimental evaluation.