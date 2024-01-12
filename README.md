# Git-Sync'd Command Line Interface

Goals for this program:
1. Browse local files / folders to add to a remote repo. 
    - When a file / folder is selected, the user should be informed if the file is already being synced and be asked if they want to sync it to another repo.
2. "Browse" the DB of remote repos / files that are being synced to and show where the files are being synced to, if they are being synced to this machine.
    - Should provide the ability to remove them from syncing, modify the type of sync (time based, git repo based, etc.)
    - Provide mechanism to trigger sync of specific file / folder
3. Add / remove remote repos
4. Add / remove credentials for remote repos
    - removal of credentials should warn user of what will stop syncing
5. Trigger the syncing of files / folders / remote repos