To install the software, please follow the instructions in the INSTALL file.

To prepare the data, create a directory (whose path has to be specified in the
configuration files) and put in it 5 files:
- a list of paths for training images (absolutes of relatives to the current
  execution directory)
- a list of paths for training masks (absolutes of relatives to the current
  execution directory)
- a list of paths for training ground-truths (absolutes of relatives to the
  current execution directory)
- a list of paths for test images (absolutes of relatives to the current
  execution directory)
- a list of paths for test masks (absolutes of relatives to the current execution
  directory)
The filenames are free (you have, however, to specify them in the configuration
files), and the name of the images too,
but the order of the paths in the files is not: the i-th image has to match with
the i-th mask and the i-th ground-truth --- that is, all the three files for a
given image have to be in the same row number on the three different list files.
They also have to match in size.

The paths of the datasets MUST be specified in the configuration files
(train_config.json and test_config.json)

To execute the software, enter the directory build/src/train and execute
./train_movable <PUT A SIMULATION NAME HERE>
Training options can be altered in the train_config.json file

Once the classifier has been trained, it can be used to test on images by
entering build/src/test and executing
./test_movable <PUT A SIMULATION NAME HERE> <PATH TO THE CLASSIFIER>

If you have any question, or you wish to contribute, please do not hesitate to
send a message to roberto <dot> rigamonti <at> heig-vd <dot> ch.
