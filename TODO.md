* add backwards search
* fix wide char (utf8) delete during input
* replace libmill with vanilla bsd sockets
* input tab scroller with zipper
* add cursor
    + add walker that keeps track of previous and next file
      ```c
      struct walker {
          void *prev; /* iter to previous ent */
          void *curr; /* iter to current ent */
          void *next; /* iter to next ent */
          int count;  /* number of files to zip */
      };
      ```
