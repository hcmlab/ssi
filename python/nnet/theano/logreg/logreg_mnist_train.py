from logreg_mnist import sgd_optimization_mnist

if __name__ == '__main__':
    sgd_optimization_mnist(learning_rate=0.13, 
                           n_epochs=1000,
                           dataset='../../mnist/mnist.pkl.gz',
                           batch_size=600)