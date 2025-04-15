import papermill as pm
import sys

def run_notebook(file_path):
    pm.execute_notebook(
        'RunModelNB.ipynb',
        'output_notebook.ipynb',
        parameters=dict(path=file_path)
    )

if __name__ == '__main__':
    path = sys.argv[1]
    run_notebook(path)