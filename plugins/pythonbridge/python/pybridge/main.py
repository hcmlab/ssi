import ssi_bridge

def func(x):
    x_1 = x
    return x_1

client = ssi_bridge.client(transform=func, input_types="f", output_types="f")
client.start()