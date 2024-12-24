import cudf
import cugraph
import time
import os
import sys

graphdf = cudf.read_csv(sys.argv[1], names=['src', 'dst'], delimiter=' ')
graph = cugraph.Graph()
graph.from_cudf_edgelist(graphdf, source='src', destination='dst')
print('Graph Loaded')
t1 = time.time()
cugraph.bfs(graph, graphdf['src'][0])
t2 = time.time()

print('cuGraph BFS Time taken: ', t2-t1)
os.system('nvidia-smi')