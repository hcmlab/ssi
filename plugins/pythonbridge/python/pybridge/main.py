import ssi_bridge

def func(x):
    y = x * 0.1
    return y

client = ssi_bridge.client(transform=func, input_types="f", output_types="f")
client.start()