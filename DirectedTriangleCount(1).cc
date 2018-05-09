
/* This file utilizes struct and array, which idea is simple, but code is longer.*/
/* There is a problem of repeated counting.  */


#include <stdio.h>
#include <string.h>
#include <math.h>

#include "GraphLite.h"

#define VERTEX_CLASS_NAME(name)  DirectedTriangleCount##name

#define MAX 10000000


/*struct */

struct StructInfo{

int64_t vid;
int64_t OutNeighbor[MAX];
int64_t InNeighbor[MAX];	
};	

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
        m_n_value_size = sizeof(StructInfo);//VertexValue type
         return m_n_value_size;
    }
    int getEdgeValueSize() {
        m_e_value_size = sizeof(double);
        return m_e_value_size;
    }
    int getMessageValueSize() {
        m_m_value_size = sizeof(StructInfo);//MessageValue type
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
        const char* count[10]={"in","out","through","cycle"};//////writable
        int value[4]={ in,out, through, cycle};

        int n=0;
        for(int i=0; i<4; i++){
            n = sprintf(s, "%s: %lld\n", count[i], value[i]);
            writeNextResLine(s, n); 
	}
    }
};

// An aggregator that records a double value tom compute sum
class VERTEX_CLASS_NAME(Aggregator): public Aggregator<double> {
public:
    void init() {
        m_global = 0;
        m_local = 0;
    }
    void* getGlobal() {
        return &m_global;
    }
    void setGlobal(const void* p) {
        m_global = * (double *)p;
    }
    void* getLocal() {
        return &m_local;
    }
    void merge(const void* p) {
        m_global += * (double *)p;
    }
    void accumulate(const void* p) {
        m_local += * (double *)p;
    }
};

class VERTEX_CLASS_NAME(): public Vertex <StructInfo, double, StructInfo> {
public:
    void compute(MessageIterator* pmsgs) {
		
		StructInfo stInfo;
					
		if (getSuperstep() == 0) {
			
			stInfo.vid= getVertexId();//own vid			
			OutEdgeIterator itr=getOutEdgeIterator();
			const int64_t n = itr.size(); //n is the number of OutNeighbors
			stInfo.OutNeighbor[0]= n; 
			stInfo.InNeighbor[0]= 0;  //initial
		
			int64_t index=1;
			for (; ! itr.done(); itr.next() ) {
				stInfo.OutNeighbor[index]= itr.target();
	           		index++;
	        } //OutNeighbors ID array
			
			* mutableValue() = stInfo;
			sendMessageToAllNeighbors(stInfo);
		}
	
		else {
			
			if (getSuperstep() == 1) {
				
				stInfo= getValue();								
				int64_t index=1;
				for ( ; ! pmsgs->done(); pmsgs->next() ) {
					stInfo.InNeighbor[index]=pmsgs->getValue().vid;
					index++;
				}
				
				stInfo.InNeighbor[0]= index-1;								
				* mutableValue() = stInfo;	// change vertex value 
				sendMessageToAllNeighbors(stInfo);
				
				for(int64_t i=1; i<index; i++){
					sendMessageTo(stInfo.InNeighbor[i], stInfo);			
				}				
				
			}
				
			if (getSuperstep() == 2){
				
				stInfo= getValue();
				const int64_t messarray_num= pmsgs->m_pvector->size();//
				struct StructInfo messarray[messarray_num];
				int64_t index=0;
				
				for ( ; ! pmsgs->done(); pmsgs->next() ) {
					messarray[index]=pmsgs->getValue();
					index++;
				}
				
				
				int64_t inNei_num= stInfo.InNeighbor[0];
				int64_t outNei_num= stInfo.OutNeighbor[0];
				
				int  in_triangle= 0;
				int out_triangle= 0;
				int through_triangle= 0;
				int cycle_triangle= 0;
				

				StructInfo st_vsert;
				int64_t verID;
				int64_t allnei_num;
				
				for (int64_t i=1; i<inNei_num;i++){
					for (int64_t j=i+1; j<=inNei_num; j++){						
						for (int64_t k=1; k<=messarray_num; k++){
							if (messarray[k].vid==stInfo.InNeighbor[i]){
								st_vsert= messarray[k];
								verID= stInfo.InNeighbor[j];
								break;
								}
							else if(messarray[k].vid==stInfo.InNeighbor[j]){
								st_vsert= messarray[k];	
								verID= stInfo.InNeighbor[i];
								break;
								}
						}
						allnei_num=st_vsert.InNeighbor[0]+st_vsert.OutNeighbor[0];
						int64_t allnei[allnei_num];
						int64_t aindex=0;
						for (int64_t a=1;a<=st_vsert.InNeighbor[0];a++){
							allnei[aindex]=st_vsert.InNeighbor[a];
							aindex++;
						}
						for (int64_t b=1;b<=st_vsert.OutNeighbor[0];b++){
							allnei[aindex]=st_vsert.OutNeighbor[b];
							aindex++;
						}
						for (int64_t c=0; c<allnei_num;c++){
						    if(allnei[c]==verID){
						    	in_triangle++;
								break;   	}
																
						}

					 }
				}
				//outTriangle count
				for (int64_t ii=1; ii<outNei_num;ii++){
					for (int64_t jj=ii+1; jj<=outNei_num; jj++){						
						for (int64_t kk=1; kk<=messarray_num; kk++){
							if (messarray[kk].vid==stInfo.OutNeighbor[ii]){
								st_vsert= messarray[kk];
								verID= stInfo.OutNeighbor[jj];
								break;
								}
							else if(messarray[kk].vid==stInfo.OutNeighbor[jj]){
								st_vsert= messarray[kk];	
								verID= stInfo.OutNeighbor[ii];
								break;
								}
						}
						allnei_num=st_vsert.InNeighbor[0]+st_vsert.OutNeighbor[0];
						int64_t allnei1[allnei_num];
						int64_t aindex1=0;
						for (int64_t aa=1;aa<=st_vsert.InNeighbor[0];aa++){
							allnei1[aindex1]=st_vsert.InNeighbor[aa];
							aindex1++;
						}
						for (int64_t bb=1;bb<=st_vsert.OutNeighbor[0];bb++){
							allnei1[aindex1]=st_vsert.OutNeighbor[bb];
							aindex1++;
						}
						for (int64_t cc=0; cc<allnei_num;cc++){
						    if(allnei1[cc]==verID){
						    	out_triangle++;
							break;   	}
																
						}

					 }
				}
				
				//through /cycle count
				
				for (int64_t iz=1; iz<inNei_num;iz++){
					for (int64_t jz=iz+1; jz<=outNei_num; jz++){						
						for (int64_t kz=1; kz<=messarray_num; kz++){
							if (messarray[kz].vid==stInfo.InNeighbor[iz]){
								st_vsert= messarray[kz];
								verID= stInfo.OutNeighbor[jz];
								break;              }
						}
						
						for (int64_t az=1;az<=st_vsert.InNeighbor[0];az++){
							if(verID==st_vsert.InNeighbor[az]){
								cycle_triangle++;
								break;		}
							
						}
						for (int64_t bz=1;bz<=st_vsert.OutNeighbor[0];bz++){
							if(verID==st_vsert.OutNeighbor[bz]){
								through_triangle++;
								 break;		}				
						}
																
					}

				 }
				
				accumulateAggr(0, &in_triangle);
				accumulateAggr(1, &out_triangle);
				accumulateAggr(2, &through_triangle);
				accumulateAggr(3, &cycle_triangle);								
				
			}
		     else if (getSuperstep() >= 3){
		    	 in= * (int *)getAggrGlobal(0);
		    	 out = * (int *)getAggrGlobal(1);
		    	 through= * (int *)getAggrGlobal(2);
		    	 cycle= * (int *)getAggrGlobal(3);

		         voteToHalt(); 
		         return;
		        }
        }
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

        aggregator = new VERTEX_CLASS_NAME(Aggregator)[4];////writable
        regNumAggr(4);
        regAggr(0, &aggregator[0]);
	regAggr(1, &aggregator[1]);
	regAggr(2, &aggregator[2]);
	regAggr(3, &aggregator[3]);
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
