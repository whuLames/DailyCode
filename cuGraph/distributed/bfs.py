import dask_cudf
from dask.distributed import Client
from dask_cuda import LocalCUDACluster

import cugraph
import cugraph.dask as dask_cugraph
import cugraph.dask.comms.comms as Comms
from cugraph.generators.rmat import rmat
import time
import sys
import os
import numpy as np

if __name__ == '__main__':
    input_data_path = "input_data.csv"

    # cluster initialization
    cluster = LocalCUDACluster()
    client = Client(cluster)
    Comms.initialize(p2p=True)

    print('cluster initialization done!')
    # helper function to generate random input data
    # input_data = rmat(
    #     scale=5,
    #     num_edges=400,
    #     a=0.30,
    #     b=0.65,
    #     c=0.05,
    #     seed=456,
    #     clip_and_flip=False,
    #     scramble_vertex_ids=False,
    #     create_using=None,
    # )
    # input_data.to_csv(input_data_path, index=False)

    print('graph generation done!')
    
    input_data_path = sys.argv[1]
    # helper function to set the reader chunk size to automatically get one partition per GPU
    chunksize = dask_cugraph.get_chunksize(input_data_path)

    # multi-GPU CSV reader
    e_list = dask_cudf.read_csv(
        input_data_path,
        blocksize=chunksize,
        names=['src', 'dst'],
        dtype=['int32', 'int32'],
        delimiter=' ',
    )

    # create graph from input data
    G = cugraph.Graph(directed=False)

    G.from_dask_cudf_edgelist(e_list, source='src', destination='dst', )
    print('vertex count:', G.number_of_vertices())
    print('edge count:', G.number_of_edges())

    t1 = time.time()
    # run PageRank
    
    bfs_df = dask_cugraph.bfs(G, 1)
    print('bfs done!')
    t2 = time.time()
    res = bfs_df.compute()
    print('bfs compute done!')
    print('BFS took: ', t2-t1)
    print('columns, ', res.columns)

    vertex = res['vertex'].to_numpy()
    distance = res['distance'].to_numpy()
    indices = np.argsort(vertex)
    vertex = vertex[indices]
    distance = distance[indices]
    print('vertex', vertex)
    print('distance', distance)  
    os.system('nvidia-smi')
    # cluster clean up
    Comms.destroy()
    client.close()
    cluster.close()