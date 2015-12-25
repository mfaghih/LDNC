#include "ns3/random-variable-stream.h"
#include "bloom_filter.hpp"

using namespace std;
using namespace ns3;


class MyBloom_filter : public ns3::Object, public bloom_filter {
public:
  MyBloom_filter(const std::size_t& predicted_element_count,
                const double& false_positive_probability,
                const std::size_t& random_seed) : Object(), bloom_filter(predicted_element_count,false_positive_probability, random_seed){};
  ~MyBloom_filter(){};
};


class Neighbor
{
public:
  Neighbor();
  ~Neighbor ();
  bool fwd;
  double firstReceptionTime;
  double lastReceptionTime;
  uint32_t neighborReceivedPackets;
  uint32_t neighborReceivedBeacons;
  uint32_t neighborForwardedPackets;
  uint8_t neighborId;
  uint8_t neighborhoodSize;
  uint8_t neighborDecodingBufSize;
  uint16_t neighborRemainingCapacity;
  Ipv4Address neighborIp;
  std::vector<std::string> sourceForwarded;
  Ptr<MyBloom_filter> neighborDecodingFilter;
  Ptr<MyBloom_filter> neighborDecodedFilter;
  std::vector<PacketId> neighborNotSolvedVars;
  std::vector<PacketId> neighborSolvedVars;
  std::vector<double> neighborDecodedListUtilities;
  std::vector<double> neighborVarListUtilities;
};

struct WaitingListMember
{
  std::string pktId;
  uint64_t entranceTime;
  std::vector <std::string> nodeForwardedTo;

};

typedef std::list<WaitingListMember> WaitingList ;

class MyHeader : public Header
{
public:
  MyHeader ();
  virtual ~MyHeader ();
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;
  void SetDestination (uint8_t Id);
  uint8_t GetDestination (void) const;
 // void SetTime (uint32_t time);
 // uint32_t GetTime (void) const;
  void SetPacketType (uint8_t type);
  uint8_t GetPacketType (void) const;
  void SetNodeId (uint8_t id);
  uint8_t GetNodeId (void) const;
  void SetNeighborhoodSize (uint8_t size);
  uint8_t GetNeighborhoodSize(void) const;
  void SetNeighborDecodingBufSize (uint8_t size);
  uint8_t GetNeighborDecodingBufSize(void) const;
  void SetLinearCombinationSize (uint8_t size);
  uint8_t GetLinearCombinationSize (void) const;
  std::vector<LinearCombination> m_linComb;
  std::vector<uint8_t> m_pktIdLength;
  void PutDecodedBloomFilter (Ptr<MyBloom_filter> node);
  void PutDecodingBloomFilter (Ptr<MyBloom_filter> node);
  Ptr<MyBloom_filter> GetDecodedBloomFilter (const std::size_t predictedElementCount , const double falsePositiveProbability) const;
  Ptr<MyBloom_filter> GetDecodingBloomFilter (const std::size_t predictedElementCount , const double falsePositiveProbability) const;
  void SetRemainingCapacity (uint8_t remainingCapacity);
  uint8_t GetRemainingCapacity (void) const;
  unsigned char* m_decodedBitTable;
  unsigned char* m_decodingBitTable;
private:
  uint8_t m_destId;
  uint8_t m_packetType;
  uint8_t m_nodeId;
  uint8_t m_neighborhoodSize;
  uint8_t m_linCombSize;
 // uint32_t m_time;
  std::size_t m_decodedTableSize;
  std::size_t m_decodingTableSize;
  std::size_t m_decodedInsertedElementCount;
  std::size_t m_decodingInsertedElementCount;
  uint8_t m_remainingCapacity;
  uint8_t m_decodingBufSize;  //conveys neighbor's decodingBufSize
};

class MyNCApp : public Application
{
public:
   MyNCApp();
  ~MyNCApp();
  void SetupSockets();
  void SetupBeacon ();
  void Start();
  void Stop();
  void GenerateBeacon ();
  void Receive (Ptr<Socket> socket);
  void Forward ();
  void UpdateNeighborList(MyHeader, Ipv4Address);
  void UpdateNeighorhoodEst(std::string);
  NetworkCodedDatagram* Encode ();
  void Reduce (NetworkCodedDatagram& g);
  void UpdateVarList (NetworkCodedDatagram& g);
  int CheckCapacity(NetworkCodedDatagram& g);
  void GenerateMatrix ();
  int GausElim(int M, int N, Ptr<Packet> packetIn);
  void PermuteCol(int col1, int col2, int L);
  void PermuteLine(int lin1, int lin2, int L);
  void ExtractSolved(uint32_t M, uint32_t N, Ptr<Packet> packetIn);
  void Decode(NetworkCodedDatagram* g, Ptr<Packet> packetIn);
  //Specific to SourceNode
  void CheckWaitingList (std::list<Neighbor>::iterator);
  void MakeSource();
  void UpdateWaitingList(std::string);
  void GeneratePacket();
  void PacketInjector();
protected:
  std::list<Neighbor> m_neighborhood;
  bool m_amSource; // indicate that the node is a source
  bool m_idle; //indicate is the app is sending packet or just beacon
  uint32_t m_port;
  int MCLU;
  double beaconInterval;
  Ptr<Socket> sinkSock;
  Ptr<Socket> sourceSock;
  Ptr<Socket> beaconSock;
  galois::GaloisField *m_nodeGaloisField;
  std::size_t predictedElementCount;
  double falsePositiveProbability;
public:
  bool m_running;
  uint32_t m_rcvCounter;
  uint16_t counter;
  uint16_t m_myNodeId;
  uint16_t m_myNeighborhoodSize;
  Ipv4Address m_myNCAppIp;
  std::deque<NetworkCodedDatagram*> m_buffer;
    // List containing packets to decode
  std::vector<NetworkCodedDatagram*> m_decodingBuf;
    // List containing decoded packets
  std::vector<NetworkCodedDatagram*> m_decodedBuf;
  std::vector<PacketId> m_decodedList;
  std::vector<PacketId> m_varList;
  int m_rank;
  Matrix m_matrix;
  LPMatrix m_lpMatrix;//this matrix records unreceived vars. it will be used for constraints in LP
  uint32_t nNode; //Global Number of potential destination
  uint32_t nGeneratedBeacons;
  uint32_t nReceivedBeacons;
  uint32_t nDuplicateRec;
  uint32_t nReceivedBytes;
  uint32_t nReceivedPackets;
  uint32_t nSrcForwardedPackets;
  uint32_t nForwardedPackets;
  uint32_t nDroppedPackets;
  uint32_t m_pktSize;
  double m_packetInterval;
  double m_beaconInterval;
  double packetDelay;
  Ptr<ExponentialRandomVariable> expVar;
  Ptr<UniformRandomVariable> uniVar;

  // For case Node is a source
  uint32_t m_nGeneratedPackets;
  uint32_t m_nInjectedPackets;
  uint32_t m_nErasedElements;
  WaitingList m_waitingList;
};



class Experiment
{
private:
  void CheckThroughput ();
  Gnuplot2dDataset m_output;
  NodeContainer myNodes;
  ApplicationContainer sourceApps, rlyApps, allApps;
public:
  std::string s_output;
  double s_simulationTime;
  bool s_verbose;
  bool s_logComponent;
  uint32_t s_nSource;
  uint32_t s_nRelay;
  uint32_t s_packetSize;
  double s_beaconInterval;
  double s_packetInterval;
  uint32_t s_nGeneratedBeacons;
  uint32_t s_nReceivedBeacons;
  uint32_t s_totalInjectedPackets;
  uint32_t s_totalGeneratedPackets;
  uint32_t s_nSrcForwardedPackets;
  uint32_t s_nReceivedPackets;
  uint32_t s_nForwardedPackets;
  uint32_t s_nDuplicateRec;
  uint32_t s_nDroppedPackets;
  uint32_t s_bytesTotal;
  double s_packetDelay;

  Experiment();
  Experiment(std::string name);
  bool CommandSetup (int argc, char **argv);
  Gnuplot2dDataset ApplicationSetup (const WifiHelper &wifi, const YansWifiPhyHelper &wifiPhy, const NqosWifiMacHelper &wifiMac, const YansWifiChannelHelper &wifiChannel);
};


