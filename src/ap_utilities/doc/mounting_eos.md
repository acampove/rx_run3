# Mount EOS in laptop

```bash
# install SSHF
...
# Check that it's installed
which sshfs

# Make directory to mount EOS

APDIR=/eos/lhcb/wg/dpa/wp2/ci/
sudo mkdir -p $APDIR
sudo chown $USER:$USER $APDIR 

# Mount EOS
sshfs -o idmap=user USERNAME@lxplus.cern.ch:$MNT_DIR $MNT_DIR
```

