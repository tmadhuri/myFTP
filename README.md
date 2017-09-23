> This was made as part of a course assignment, and has not been changed or maintained since.

​In this assignment, we created an **Application Level File­Sharing­Protocol** ​with support for download and upload for files and indexed searching. 

The client of one peer connects to the server of the other.

Then both of them are ready to accept commands from the user, whose result is displayed.

```
IndexGet --shortlist <timestamp1> <timestamp2>
IndexGet --longlist
IndexGet --regex *
```

All timestamps are in seconds ellapsed since epoch time.

IndexGet works by reading the directory structure of the other peer's "share" folder and displaying it.

```
FileDownload <filename>
FileUpload <filename>
```

These work by creating a file of the same name at the other peer, and then transfering the file data.
