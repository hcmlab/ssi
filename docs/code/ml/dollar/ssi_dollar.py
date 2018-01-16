import dollar
import pickle

def getOptions (opts, vars): 

    vars['recognizer'] = None


def getModelType(types, opts, vars):
    return types.CLASSIFICATION

def x2p (x):

    points = []
    for i in range(0,x.num):
        points.append((x[i*2],x[i*2+1]))

    return points


def train(xs, ys, opts, vars):
    
    recognizer = dollar.Recognizer ()

    for i in range(len(xs)):
        points = x2p(xs[i])
        label = str(ys[i])
        recognizer.AddTemplate(label, points)

    vars['recognizer'] = recognizer


def forward(x, probs, opts, vars):
	
    recognizer = vars['recognizer']

    if not recognizer is None:        
        points = x2p(x)
        recognizer.RecognizeProbs(points, probs)       

    return max(probs)


def load(path, opts, vars):

    recognizer = dollar.Recognizer ()
    recognizer.Templates = pickle.load (open(path, 'rb'))

    vars['recognizer'] = recognizer


def save(path, opts, vars):

    recognizer = vars['recognizer']

    if not recognizer is None:
        pickle.dump (recognizer.Templates, open(path, 'wb'))             

