from __future__ import print_function
import argparse
import torch
from torchvision import datasets, transforms
from net import Net


def main():
    
    parser = argparse.ArgumentParser(description='PyTorch MNIST Example')
    parser.add_argument('--no-cuda', action='store_true', default=False,
                        help='disables CUDA training')
    parser.add_argument('--seed', type=int, default=1, metavar='S',
                        help='random seed (default: 1)')
    args = parser.parse_args()
    use_cuda = not args.no_cuda and torch.cuda.is_available()

    torch.manual_seed(args.seed)

    device = torch.device('cuda' if use_cuda else 'cpu')

    kwargs = {'num_workers': 1, 'pin_memory': True} if use_cuda else {}
    test_loader = torch.utils.data.DataLoader(
        datasets.MNIST('data', train=False, transform=transforms.Compose([
                           transforms.ToTensor()#,
                           #transforms.Normalize((0.1307,), (0.3081,))
                       ])),
        batch_size=1, shuffle=True, **kwargs)

    model = Net().to(device)
    model.load_state_dict(torch.load('./net.pth'))

    for data, target in test_loader:
        data, target = data.to(device), target.to(device)
        output = model(data).max(1)[1]
        print('{} <> {}'.format(target.item(),output.item()))

if __name__ == '__main__':
    main()