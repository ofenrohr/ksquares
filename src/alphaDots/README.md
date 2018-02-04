# Alpha Dots

This part of KSquares is used to generate training data for Alpha Dots.

## Setup

Please make sure to corectly configure Alpha Dots in KSquares by entering the
Alpha Dots directory in KSquares -> Settings -> Configure KSquares -> Computer Player.

Clone this git repository to get Alpha Dots:
```
git clone https://gitlab.informatik.uni-bremen.de/ofenrohr/alphaDots.git
```

## Data generators

You can generate training data by running ksquares with certain command line 
arguments. To explore some of the datasets with a graphics user interface, run this:

```
ksquares --show-generate
```

You can only have one board size per dataset. Models can be trained on many datasets.

### First Try

Converts random Dots and Boxes games played by the Hard AI to images.
One training example consists of two images: 

* The current state of the game in the input image
* The next line chosen by the Hard AI in the target image

All data is written to disk as actual .PNG images. 
Input and target images share the same sample UUID.

After generating the training data images, a python script needs to be run 
to convert the images to a numpy array. 

```
mkdir firstTryDataset
ksquares --generate 1000 --dataset-generator firstTry --dataset-dest firstTryDataset
../alphaDots/datasetConverter/convert.py --dataset-dir firstTryDataset --output-file firstTry.npz --debug
```

### Stage One

The input and target images are no longer generated as individual image files but
instead sent directly to the python script. Communication is facilitated with
zeroMQ, sending images with the protobuf library.

```
ksquares --generate 1000 --dataset-type stageOne --dataset-dest /mnt/DATA/stageOneDataset
```

### Basic Strategy

During the first phase of a Dots and Boxes game, there is a very large amount of 
acceptable lines. Usually the Hard AI just selects one of those many lines at
random. The trained neural network does not know, that there are many alternatives
to the shown target image. This dataset aims to remedy this by putting all
viable lines in one target image.

```
ksquares --generate 1000 --dataset-type basicStrategy --dataset-width 7 --dataset-height 5
```

### Sequence 

Despite the efforts in *Basic Strategy*, the trained models were not able to compete
even with the Medium AI. The main innovation compared to the Easy AI is the ability
to count chains - i.e. only handing over the chain with the least amount of squares.
To do this, an AI needs to count the number of squares in each chain and decide
accordingly.

Previous datasets were used as training data for convolutional neural networks, which
lack the ability to count things. As a consequence, a new model is based on 
convolutional LSTM layers. That model needs another type of training data - only
the last state of the game is not enough, it also needs all states leading up to it.

This dataset is made of full Dots and Boxes games, played by two Hard AIs.

```
ksquares --generate 1000 --dataset-type LSTM
```
