
#include<vector>
#include<algorithm>

// TODO: Implement kogmolgrov's kickass max-flow version
// http://pub.ist.ac.at/~vnk/papers/POTTS.html
// Currently using Dinic

template< class D, int N >
class GraphFlow{
 
public:
 
    int source, sink;
    std::vector< int > G[N];
 
    int T[N];
    bool viz[N];
	D Cap[N][N]; 
 
    GraphFlow( int source, int sink ) : source(source), sink(sink)
    {

    }
 
    GraphFlow(){
        source = sink = 0;
    }
 
    /*
     * Custom
     */
 
    void build_graph( int _source, int _sink, int Edges )
    {
        source = _source;
        sink   = _sink;
 
        for( int i = 0, a1, a2, cap; i < Edges; ++i )
        {
            std::cin >> a1 >> a2 >> cap;
            add_link( a1, a2, cap );
        }
    }
 
    void add_link( int node1, int node2, D cap )
    {
        G[node1].push_back(node2);
        G[node2].push_back(node1);
 
        Cap[node1][node2] = cap;
		Cap[node2][node1] = cap;
    }
 
    void link_to_source( int node, D cap )
    {
        G[source].push_back(node);
        Cap[source][node] = cap;
    }
 
    void link_to_sink( int node, D cap )
    {
        G[sink].push_back(node);
        Cap[node][sink] = cap;
    }
 
    bool BF()
    {
        memset( viz, 0, sizeof(viz) );
        std::queue< int > Q;
 
        Q.push( source );
        viz[source] = true;
 
        while( !Q.empty() )
        {
            int node = Q.front();
            Q.pop();
 
            if( node == sink )
                continue;
 
            for( uint i = 0, sz = G[node].size(); i < sz; ++i )
            {
                int v = G[node][i];
 
                if( viz[v] || !Cap[node][v] )
                    continue;
 
                Q.push(v);
                viz[v] = true;
                T[v] = node;
            }
        }
 
        return viz[sink];
    }
 
    /*
     * Dinic O( V^2 * E )
     */
 
    D MaxFlow()
    {
        D flow = 0, fmin;
        int node;
 
        while( BF() )
        {
            for( uint i = 0, sz = G[sink].size(); i < sz; ++i )
            {
                node = G[ sink ][ i ];
 
                if( !viz[node] || !Cap[node][sink] )
                    continue;
 
                T[sink] = node;
 
                fmin = Cap[ T[sink] ][ sink ];
 
                for( node = sink; node != source; node = T[node] )
                    fmin = fmin < Cap[ T[node] ][ node ] ? fmin : Cap[ T[node] ][ node ];
 
                for( node = sink; node != source; node = T[node] )
                {
                    Cap[ T[node] ][ node ] -= fmin;
                    Cap[ node ][ T[node] ] += fmin;
                }
 
                flow += fmin;
            }
        }
 
        return flow;
    }

	std::vector<int> MinCut()
	{
		std::vector< int > C;

        int nod = source, inc = 0, sf = 0, v;

		memset(viz, 0, sizeof(viz));
		viz[1] = 1;
		C.push_back( 1 ); ++sf;
		
		while( inc != sf )
        {
                nod = C[ inc++ ];
                viz[nod] = 1;

                for( unsigned int i = 0; i < G[nod].size(); ++i )
                {
                    v = G[nod][i];
//					std::cout << "nod " << nod << " " << Cap[nod][v] << " " << v << "\n";
                    if( !Cap[nod][v] || viz[ v ] )
						continue;

    				C.push_back( v ); ++sf;
                    viz[ v ]   = 1;
                }
        }
//		std::cout << "\n";

		return C;
	}
 
    void reset()
    {
        memset( Cap, 0, sizeof(Cap) );
        for( int i = 0; i < N; ++i ){
            G[i].clear();
        }
    }
 
};