
/* This fille is intend to count directed triangles in graph.*/
/* This is version 2, which utilizes map<> and set<> instead of structure and array,greatly simplify the code, */
/* It needs 4 superstep */


#include <stdio.h>
#include <string.h>
#include <math.h>

#include "GraphLite.h"
#include <set>
#include <map>
#include <vector>
#include <string>  
#include <iostream>
#include <algorithm>
using namespace std;


#define VERTEX_CLASS_NAME(name) DirectedTriangleCount##name

int in=0;
int out=0;
int through=0;
int cycle=0;

class VERTEX_CLASS_NAME(InputFormatter): public InputFormatter {
public:
    int64_t getVertexNum() {
        unsigned long long n;
        sscanf(m_ptotal_vertex_line, "%lld", &n);
        m_total_vertex= n;
        return m_total_vertex;
    }
    int64_t getEdgeNum() {
        unsigned long long n;
        sscanf(m_ptotal_edge_line, "%lld", &n);
        m_total_edge= n;
        return m_total_edge;
    }
    int getVertexValueSize() {
        m_n_value_size = sizeof(int64_t);
        return m_n_value_size;
    }
    int getEdgeValueSize() {
        m_e_value_size = sizeof(double);
        return m_e_value_size;
    }
    int getMessageValueSize() {
        m_m_value_size = sizeof(int64_t);
        return m_m_value_size;
    }
    void loadGraph() {
        unsigned long long last_vertex;
        unsigned long long from;
        unsigned long long to;
        double weight = 0;
        
        double value = 1;
        int outdegree = 0;
        
        const char *line= getEdgeLine();

        // Note: modify this if an edge weight is to be read
        //       modify the 'weight' variable

        sscanf(line, "%lld %lld", &from, &to);
        addEdge(from, to, &weight);

        last_vertex = from;
        ++outdegree;
        for (int64_t i = 1; i < m_total_edge; ++i) {
            line= getEdgeLine();

            // Note: modify this if an edge weight is to be read
            //       modify the 'weight' variable

            sscanf(line, "%lld %lld", &from, &to);
            if (last_vertex != from) {
                addVertex(last_vertex, &value, outdegree);
                last_vertex = from;
                outdegree = 1;
            } else {
                ++outdegree;
            }
            addEdge(from, to, &weight);
        }
        addVertex(last_vertex, &value, outdegree);
    }
};


class VERTEX_CLASS_NAME(OutputFormatter): public OutputFormatter {
public:
    void writeResult() {
    	char s[1024];
    	const char* count[4]={"in","out","through","cycle"};//////
    	int value[4]={ in,out, through, cycle};

    	int n=0;
    	for(int i=0; i<4; i++){
    		n = sprintf(s, "%s: %d\n", count[i], value[i]);
    		writeNextResLine(s, n); 
    	}
    }
};

// An aggregator that records a double value tom compute sum
class VERTEX_CLASS_NAME(Aggregator): public Aggregator<int> {
public:
    void init() {
        m_global = 0;
        m_local = 0;
    }
    void* getGlobal() {
        return &m_global;
    }
    void setGlobal(const void* p) {
        m_global = * (int *)p;
    }
    void* getLocal() {
        return &m_local;
    }
    void merge(const void* p) {
        m_global += * (int *)p;
    }
    void accumulate(const void* p) {
        m_local += * (int *)p;
    }
};



int addition=1;

class VERTEX_CLASS_NAME(): public Vertex <int64_t, double, int64_t> {
public:
    map<int64_t, vector<int64_t>> inNeiMap; // inNei map
	set<int64_t> neinei;//the neighbor of neighbor

    void compute(MessageIterator* pmsgs) {
    	   
        int64_t val = getVertexId(); //ownid
        vector<int64_t> outNei; // out neighbor
        OutEdgeIterator it = getOutEdgeIterator();
        for ( ; ! it.done(); it.next() ) {
            outNei.push_back(it.target()); 
	}

       
        
        if (getSuperstep() == 0) {   
            sendMessageToAllNeighbors(getVertexId());
        } 
        
        
        else if(getSuperstep() == 1) {

            vector<int64_t>& Inneigh = inNeiMap[val];// 

            for ( ; ! pmsgs->done(); pmsgs->next() ) {
                int64_t msg = pmsgs->getValue();
                Inneigh.push_back(msg);
                sendMessageToAllNeighbors(msg);
            	}
            
            }
       
        else if (getSuperstep() == 2) {         

		vector<int64_t>& Inneigh = inNeiMap[val];
           	for ( ; ! pmsgs->done(); pmsgs->next() ) {// //all in-in 
                	int64_t msg = pmsgs->getValue();
			neinei.insert(msg);   
   
                	for (vector<int64_t>::iterator iter = outNei.begin(); iter != outNei.end(); ++iter){
                		if(*iter == msg){   
                			accumulateAggr(1, &addition);    }
                	}

                }
            
            OutEdgeIterator it = getOutEdgeIterator();
            for ( ; ! it.done(); it.next() ) {
                int64_t Outneigh = it.target();
                sendMessageToAllNeighbors(Outneigh);
            }
        }
          
            else if (getSuperstep() == 3){

            	vector<int64_t>& Inneigh = inNeiMap[val];
            	for ( ; ! pmsgs->done(); pmsgs->next() ) {// //all in-out 
                    int64_t msg = pmsgs->getValue(); 
      		    neinei.insert(msg);
		}

                for (set<int64_t>::iterator iter = neinei.begin(); iter != neinei.end(); ++iter){
                    	
                   	for (vector<int64_t>::iterator iterr = Inneigh.begin(); iterr != Inneigh.end(); ++iterr){
                    		if(*iter == *iterr)
                    		{	accumulateAggr(0, &addition);  }                                       	
                    }                                                  	
            	}          	
            	 
            }
             
           
                           
        else if (getSuperstep() >= 4){
            in = * (int *)getAggrGlobal(0)/2;
            out = * (int *)getAggrGlobal(0)/2;
            through = * (int *)getAggrGlobal(0)/2;
            cycle = * (int *)getAggrGlobal(1);
            
            voteToHalt(); 
            return;
        }
        * mutableValue() = val;
       
    }
};

class VERTEX_CLASS_NAME(Graph): public Graph {
public:
    VERTEX_CLASS_NAME(Aggregator)* aggregator;

public:
    // argv[0]: PageRankVertex.so
    // argv[1]: <input path>
    // argv[2]: <output path>
    void init(int argc, char* argv[]) {

        setNumHosts(5);
        setHost(0, "localhost", 1411);
        setHost(1, "localhost", 1421);
        setHost(2, "localhost", 1431);
        setHost(3, "localhost", 1441);
        setHost(4, "localhost", 1451);

        if (argc < 3) {
           printf ("Usage: %s <input path> <output path>\n", argv[0]);
           exit(1);
        }

        m_pin_path = argv[1];
        m_pout_path = argv[2];

        aggregator = new VERTEX_CLASS_NAME(Aggregator)[2];
        regNumAggr(2);
        regAggr(0, &aggregator[0]);
        regAggr(1, &aggregator[1]);

    }

    void term() {
        delete[] aggregator;
    }
};

/* STOP: do not change the code below. */
extern "C" Graph* create_graph() {
    Graph* pgraph = new VERTEX_CLASS_NAME(Graph);

    pgraph->m_pin_formatter = new VERTEX_CLASS_NAME(InputFormatter);
    pgraph->m_pout_formatter = new VERTEX_CLASS_NAME(OutputFormatter);
    pgraph->m_pver_base = new VERTEX_CLASS_NAME();

    return pgraph;
}

extern "C" void destroy_graph(Graph* pobject) {
    delete ( VERTEX_CLASS_NAME()* )(pobject->m_pver_base);
    delete ( VERTEX_CLASS_NAME(OutputFormatter)* )(pobject->m_pout_formatter);
    delete ( VERTEX_CLASS_NAME(InputFormatter)* )(pobject->m_pin_formatter);
    delete ( VERTEX_CLASS_NAME(Graph)* )pobject;
}
