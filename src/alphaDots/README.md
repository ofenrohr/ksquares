
# AlphaDots in KSquares

AlphaDots uses KSquares for

* generating training data for [AlphaDots](AlphaDots.html)
* viewing training data
* evaluating trained models
* playing Dots and Boxes

## Setup

Please make sure to correctly configure AlphaDots in KSquares by entering the
Alpha Dots directory in KSquares -> Settings -> Configure KSquares -> Computer Player.

Clone this git repository to get Alpha Dots:

```
git clone https://gitlab.informatik.uni-bremen.de/ofenrohr/alphaDots.git
```

You might also want to install `tensorflow-gpu` in python for GPU acceleration.

## Data generators

You can generate training data by running ksquares with certain command line 
arguments. To explore some of the datasets with a graphical user interface, run this:

```
ksquares --show-generate
```

You can only have one board size per dataset. Models can be trained on many datasets 
of different size.

All dataset generators accept the following optional command line arguments:

* `--dataset-dest` Destination directory for the data (.npz file)
* `--dataset-width` Board width in boxes
* `--dataset-height` Board height in boxes
* `--threads` Number of threads

![FirstTry data generator](ksquares-show-generate-1.png)

[StageOne / StageTwo data generator](ksquares-show-generate-2.png)

[BasicStrategy](ksquares-show-generate-3.png)

[Sequence](ksquares-show-generate-4.png)

[StageThree](ksquares-show-generate-5.png)


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

![input image](input.png)

![output image](output.png)

### Stage One

The input and target images are no longer generated as individual image files but
are sent directly to the python script instead. Communication is facilitated with
zeroMQ which is used to send images that were serialized with the protobuf library.

This dataset also introduces weak (random) moves so that there are examples of 
early captures. 10% of all samples end with a random move. The target image is always
made with the reaction of the Hard AI. Due to an oversight, the first version had a 
bug so that 90% of all samples were made with a random move. Models trained with the
buggy version include StageOne and AlphaZeroV1,V2,V3.

```
ksquares --generate 1000 --dataset-generator stageOne --dataset-dest /mnt/DATA/stageOneDataset --dataset-width 7 --dataset-height 5
```

### Basic Strategy

During the first phase of a Dots and Boxes game, there is a very large amount of 
acceptable lines. Usually the Hard AI just selects one of those many lines at
random. The trained neural network does not know that there are many alternatives
to the shown target image. This dataset aims to remedy this by putting all
viable lines in one target image.

```
ksquares --generate 1000 --dataset-generator basicStrategy --dataset-width 7 --dataset-height 5
```

![Basic Strategy input image](basicStrategy_input.png)

![Basic Strategy output image](basicStrategy_output.png)


### [Sequence](SequenceData.html)

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
ksquares --generate 1000 --dataset-generator LSTM
```

![sequence data](test2.gif)

### Training Sequence

This dataset is like the Sequence dataset but with "Basic Strategy" target images.

```
ksquares --generate 1000 --dataset-generator LSTM2
```

### Stage Two

The data is generated just like Stage One, but this dataset generator uses 
[cnpy](https://github.com/rogersce/cnpy) to directly write the .npz file. This removes
the overhead of sending the data to a separate, single-threaded python process. As a
result, this dataset generator can fully utilize the CPU and is much faster. 

```
ksquares --generate 1000 --dataset-generator StageTwo --threads 8
```

### Stage Three

Data is generated like in Stage Two and additionally offers a value output for
models like AlphaZeroV6 and up. The value is designed to approximate the chance 
of winning the game where 1 means winning and -1 means losing the game. It is
calculated by playing the game to its end and then evaluating the number of 
captured boxes for each side as follows:

```
value = (OwnBoxes - EnemyBoxes) / (0.8 * TotalBoxes)
```

Create a Stage Three dataset by running:

```
ksquares --generate 1000 --dataset-generator StageThree --threads 8
```

### Stage Four / Stage Four (no MCTS)

This dataset generator comes in two flavors: with and without AlphaZero MCTS.
The version with MCTS uses KSquare's Hard AI and AlphaZero MCTS to generate data. 
As a result, it is possible to use this dataset generator to implement a simplified
self-play loop. 

Data is generated as follows: 

* First, a game is played by KSquare's Hard AI until a certain number of moves 
  are left. This state is used as input data. 
* Afterwards, the corresponding output data (i.e. the next move) is calculated 
  by AlphaZero MCTS, which uses a configurable neural network. If the dataset
  generator is configured without MCTS, KSquare's Hard AI will be used instead.
* Finally, the game is played to its end by KSquare's Hard AI, so that the
  game's value can be calculated.

In summary the Stage Four dataset generator uses AlphaZero MCTS for exactly one
move to minimize computational cost while still providing means to improve upon
the neural network.

The number of moves that are left before the AlphaZero MCTS calculates its move
is determined randomly according to a gaussian normal distribution. The 
distribution is scaled according to the number of lines so that most samples are
in the middle of the game. The following figure shows a histogram for 1.000.000
samples on a 3 x 3 board, which has 24 lines:

![Histogram of moves left on a 3x3 board for 1.000.000 samples](movesLeftDist.png)

The value is calculated as follows:

```
value = (OwnBoxes - EnemyBoxes) / TotalBoxes
```

Create a Stage Four dataset with one of the following commands:

```
ksquares --generate 10 --dataset-generator StageFour --threads 8 --gpu
ksquares --generate 1000 --dataset-generator StageFourNoMCTS --threads 8
```

## Model evaluation

The are two options to evaluate the trained models. The first option uses the native 
KSquares Dots and Boxes engine and displays all games while the second option is
optimized for fast evaluation. In both cases, a selection of models will be evaluated
by playing against the KSquares AIs `Easy`, `Medium` and `Hard`. 


### Slow GUI evaluation

Start the slow, GUI based model evaluation with:

```
ksquares --model-evaluation
```

### Fast evaluation

Start the multi-threaded fast model evaluation with:

```
ksquares --fast-model-evaluation --threads 8
```

### Common arguments

Both evaluation modes support the following optional arguments:

* `--models MODELS` 
  By default, all models will be evaluated. If you are only interested in a specific
  subset, use this optional argument. You can get a list of all available models with `ksquares --model-list`
* `--dataset-width WIDTH` Board width measured in boxes.
* `--dataset-height HEIGHT` Board height measured in boxes.
* `--gpu` Allow the model server to use the GPU.

### Example results

All games are played on a 5x5 board. The 5x5 board size is reserved for evaluation. No model was ever trained on any dataset with 5x5 boards.

Model|Games|Wins vs. Easyin 100 games|Wins vs. Mediumin 100 games|Wins vs. Hardin 100 games|Errors|Ends with Double Dealing|Preemtive Sacrifices
---|---|---|---|---|---|---|---
FirstTry|300|1|0|0|0|0|0
StageOne 3x3|300|58|27|6|0|0|0
StageOne 5x5|300|0|1|0|0|0|0
BasicStrategy|300|55|27|7|0|0|0
AlphaZeroV1|300|82|80|35|0|0|0
AlphaZeroV3|300|79|94|50|0|0|0
AlphaZeroV5|300|90|75|45|0|0|0
AlphaZeroV7|300|96|80|46|0|0|0

### Screenshot
![slow model evaluation with KSquares](ksquares_model_evaluation.png)

## Self-Play

KSquares can operate a self-play loop which works as follows:

* KSquares generates a chunk of training data with the StageFour dataset generator
* KSquares then trains a neural network on that data

Each iteration uses the network that was trained in the previous iteration. By
default, self-play starts with `AlphaZeroV7` and then trains `AlphaZeroV10`. 
The self-play mode will start with a pre-trained network and then improve the weights
by training on data generated with the latest version of the network. During self-play
each version of the model will be saved separately with a `.#iteration` suffix, while
the normal model name references the latest version.
Every training process is logged separately. To train the next iteration of a network,
KSquares calls [AlphaZeroV10.py](AlphaZeroV10.html) with appropriate arguments. Training
is done on the CPU, because the GPU is already used to generate more data.

Self-Play in KSquares can be started as follows:

```
ksquares --self-play
```

### Self-Play arguments

The following optional arguments will be considered by self-play:

* `--gpu` enables GPU acceleration of the data generator. Training the network will
  not use GPU acceleration, because it only takes a fraction of the overall time.
* `--threads N` number of thread to use when generating data with the MCTS AI 
  (StageFour dataset). Default: 4
* `--batch-prediction` if this flag is set, the MCTS AI threads will send their requests in batches.
  Batch size corresponds to the number of threads.
* `--iteration-size N` number of samples to generate per iteration
* `--initial-model` the name of the model to start generating training data with. Default: `alphaZeroV7`
* `--target-model` the name of the model to improve in self-play. Model name must be present in the 
  `models.yaml` list in `alphaDots/modelServer/models`. 
* `--debug` print debug information for individual prediction requests. Settings this flag
  will slow things down considerably.
* `--gpu-training` enable GPU acceleration for the training python script.
* `--dataset-generator` select the dataset generator. Supported generators are:
    * `StageFour` the StageFour dataset generator
    * `StageFourNoMCTS`
* `--epochs` number of epochs in training.

### Fast Self-Play

To execute fast self-play with gpu acceleration, execute the following command:

```
ksquares --self-play --gpu --batch-prediction --threads 16
```

### Using the self-play mode for training

It is possible to generate data very fast with the StageFourNoMCTS dataset 
generator and train a network on that data with KSquare's self-play mode.
StageFourNoMCTS data is generated purely with the Hard AI. So there is no
improvement loop, just data generation and training with this setup.

```
ksquares --self-play --iteration-size 10000 --threads 8 --gpu-training --dataset-generator StageFourNoMCTS --initial-model AlphaZeroV11 --target-model AlphaZeroV11 --epochs 3 
```

AlphaZeroV11 was trained for 38 iterations with a iteration size of 10,000
after it was created with the [IPython Notebook](AlphaZeroV11.html).

### Screenshots

[ksquares --self-play and system performance monitoring](ksquares-self-play.png)

![ksquares --self-play without MCTS](ksquares-self-play-nomcts.png)
