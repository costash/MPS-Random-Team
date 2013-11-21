Random-Team
===========

Semigrupa 2013, Vineri, 12-14

Team structure
--------------

__Bam 1__

* Constantin-Alexandru Tudorică - Developement
* Mircea Gîlcă - Research
* Alina Marcu - Testing
  
__Bam 2__
* Cristian Pălășanu
* Alin Preda

__VBam__
* Constantin Serban-Rădoi - Development
* Gabriel Ivănică - Testing
* Radu Iacob - Research


Management structure
--------------------

* Constantin-Alexandru Tudorică - Project Manager
* __TODO__

Basic Git Workflow
------------------

We always work in a local branch different from the `master` branch.

In order to create a local branch we use `git checkout -b <branch_name>`. If you don't want to branch out of the current branch be sure that you are on the `master` branch.
We can easily check on what branch we are using `git branch`. An example of output is:

```
* feature1
  master
```
As you can see the current branch is marked by an asterix.

If you want to merge your code you first `git checkout master`(not sure if it's necessary anymore), then you pull the latest changes using `git pull`.
You then move on your own branch using `git checkout <branch_name>` and the rebase this branch onto the master branch using `git rebase master`. If it fails fix the errors, commit the modifications and continue the rebase using `git rebase --continue`.
When the rebase succedes you just need to switch back to the master branch and do a `git merge <branch_name>` it should do a fast forward merge (menaning that it just merges the commits from your branch onto the master branch).


