import papermill as pm
import sys
import os
import pathlib

def run_notebook(file_path, model):
    currentDir = sys.path[0]
    notebook = os.path.join(currentDir, 'RunModelNB.ipynb')
    pm.execute_notebook(
        notebook,
        'output_notebook.ipynb',
        parameters=dict(path=file_path, modelPath=model)
    )

if __name__ == '__main__':
    myPath = sys.argv[1]
    currentDir = sys.path[0]
    myModel = os.path.join(currentDir, 'bad.model.pth')
    run_notebook(myPath, myModel)