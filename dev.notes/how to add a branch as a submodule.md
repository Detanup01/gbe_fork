1. create an orphan branch
   ```shell
   git checkout --orphan 'third-party/my-branch'
   ```
2. make sure no files are staged yet
   ```shell
   git rm -r -f --cached .
   ```
3. copy some new files (or add ones that already exist)
   ```shell
   cp ~/myfile.txt ./
   ```
4. stage the required files
   ```shell
   git add myfile.txt
   ```
   you can also stage all files in the current directory
   ```shell
   git add .
   ```
5. commit the files
   ```shell
   git commit -m 'my commit msg'
   ```
6. add the branch as submodule  
   ```shell
   git -c protocol.file.allow=always submodule add -f -b 'third-party/my-branch' file://"$(pwd)" 'my-relative-dir/without/dot/at/beginning'
   ```
   git by default disallow local repos, this option `protocol.file.allow=always` forces git to allow it  
   this will:  
   - look for a **local** repo in the directory shown by `pwd` (current folder),  
     notice how we don't simply use `./` because if we did that git will try to use the `origin` of the repo,  
     and since the origin (github/gitlab/etc...) doesn't have this branch yet it will fail, using the file protocol (`file://absolute_path`) forces git to use the local repo files  
     you can of course push the branch to origin before doing this step  
   - look for a branch named `third-party/my-branch`
   - create a submodule pointing at this branch inside a new folder `my-relative-dir/without/dot/at/beginning`  
     notice that the new folder does **not** start with `./` as usual  
7. fix the submodule path  
   after the last command, the file `.gitmodules` will point at the absolute path of the repo on disk, fix it to be relative
   ```shell
   git -c protocol.file.allow=always submodule add -f -b 'third-party/my-branch' ./ 'my-relative-dir/without/dot/at/beginning'
   ```
   this time git won't try to grab the data from origin, it will just edit `.gitmodules`  
8. new git management objects/files will be staged, you can view them
   ```shell
   git status
   ```
   possible output
   ```shell
   On branch third-party/my-branch

   Changes to be committed:
   (use "git restore --staged <file>..." to unstage)
         modified:   .gitmodules
         new file:   third-party/my-branch
   ```  
9. commit these 2 files
   ```shell
   git commit -m 'add branch third-party/my-branch as submodule'
   ```
