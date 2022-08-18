import argparse
import pickle

def main():
    # leitura de parametros com biblioteca argparse
    parser = argparse.ArgumentParser(description='Varejo-iot: count.')
    parser.add_argument('--pickle', action='store', dest='pickle',
                        default='Hello, world!', required=False,
                        help='Arquivo pickle contendo os parâmetros. Formato: --yaml <nome arquivo>.pickle')


    arguments = parser.parse_args()

  
    objects = []
    with (open(str(arguments.pickle), "rb")) as openfile:
        while True:
            try:
                objects.append(pickle.load(openfile))
            except EOFError:
                break
    for obj in objects:
        # print(str(obj[2]) + ", " + str(obj[3]) + ", " + str(obj[4]))
        print(obj)
        # print(str(obj[3]) + ", " + str(obj[4]))

    # print(len(objects))
    # print(objects[0][0])
if __name__ == '__main__':
    main()