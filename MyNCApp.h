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
  uint32_t neighborReceivedStatusFeedbacks;
  uint32_t neighborForwardedPackets;
  uint8_t neighborId;
  uint8_t neighborhoodSize;
  uint8_t neighborDecodingBufSize;
  uint16_t neighborRemainingCapacity;
  Ipv4Address neighborIp;
  std::vector<std::string> sourceForwarded;
  Ptr<MyBloom_filter> neighborDecodingFilter;
  Ptr<MyBloom_filter> neighborDecodedFilter;
};

struct WaitingListMember
{
  std::string pktId;
  uint64_t entranceTime;
  std::vector <std::string> nodeForwardedTo;

};

typedef std::list<WaitingListMember> WaitingList ;

class PacketHeader : public Header
{
public:
  PacketHeader ();
  virtual ~PacketHeader ();
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;
  void SetDestination (uint8_t Id);
  uint8_t GetDestination (void) const;
  void SetPacketType (uint8_t type);
  uint8_t GetPacketType (void) const;
  void SetNodeId (uint8_t id);
  uint8_t GetNodeId (void) const;
public:
  uint8_t m_destId;
  uint8_t m_packetType;
  uint8_t m_nodeId;
};

class StatusFeedbackHeader: public PacketHeader
{
public:
  StatusFeedbackHeader();
  ~StatusFeedbackHeader();
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;
  void SetNeighborhoodSize (uint8_t size);
  uint8_t GetNeighborhoodSize(void) const;
  void SetNeighborDecodingBufSize (uint8_t size);
  uint8_t GetNeighborDecodingBufSize(void) const;
  void SetLinearCombinationSize ();
  uint8_t GetLinearCombinationSize (void) const;
  std::vector<LinearCombination> m_linComb;
  void PutDecodedBloomFilter (Ptr<MyBloom_filter> node);
  void PutDecodingBloomFilter (Ptr<MyBloom_filter> node);
//  void PuteBF(Ptr<MyBloom_filter> node);
  Ptr<MyBloom_filter> GetDecodedBloomFilter (const std::size_t predictedElementCount , const double falsePositiveProbability) const;
  Ptr<MyBloom_filter> GetDecodingBloomFilter (const std::size_t predictedElementCount , const double falsePositiveProbability) const;
 // Ptr<MyBloom_filter> GeteBF (const std::size_t predictedElementCount , const double falsePositiveProbability) const;
  void SetRemainingCapacity (uint8_t remainingCapacity);
  uint8_t GetRemainingCapacity (void) const;
  uint8_t m_neighborhoodSize;
  uint8_t m_linCombSize;
  std::size_t m_decodedInsertedElementCount;
  std::size_t m_decodingInsertedElementCount;
  uint8_t m_remainingCapacity;
  uint8_t m_decodingBufSize;  //conveys neighbor's decodingBufSize
  std::size_t m_decodedTableSize;
  std::size_t m_decodingTableSize;
  unsigned char* m_decodedBitTable;
  unsigned char* m_decodingBitTable;

};

class BeaconHeader: public StatusFeedbackHeader
{
 public:
  BeaconHeader();
  ~BeaconHeader();
  //static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;
  //std::vector<uint8_t> m_pktIdLength;
  void PuteBF(Ptr<MyBloom_filter> node);
  Ptr<MyBloom_filter> GeteBF (const std::size_t predictedElementCount , const double falsePositiveProbability) const;
  std::size_t m_eBfInsertedElementCount;
  std::size_t m_eBfTableSize;
  unsigned char* m_eBfBitTable;
};


class DecodedPacketStorage : public ns3::Object {
public :
  DecodedPacketStorage(){};
  DecodedPacketStorage(Ptr<NetworkCodedDatagram> nc);
  ~DecodedPacketStorage();
  NCAttribute attribute;
  Ptr<NetworkCodedDatagram> ncDatagram;
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
  void UpdateNeighborList(StatusFeedbackHeader, Ipv4Address);
  void UpdateNeighorhoodEst(std::string,uint8_t type);
  Ptr<NetworkCodedDatagram> Encode ();
  void Reduce (NetworkCodedDatagram& g);
  void UpdateVarList (NetworkCodedDatagram& g);
  int CheckCapacity(NetworkCodedDatagram& g);
  void GenerateMatrix ();
  int GausElim(int M, int N, Ptr<Packet> packetIn);
  void PermuteCol(int col1, int col2, int L);
  void PermuteLine(int lin1, int lin2, int L);
  void ExtractSolved(uint32_t M, uint32_t N, Ptr<Packet> packetIn);
  void Decode(Ptr<NetworkCodedDatagram> g, Ptr<Packet> packetIn);
  //Specific to SourceNode
  void CheckWaitingList (std::list<Neighbor>::iterator);
  void MakeSource();
  void UpdateWaitingList(std::string);
  void UpdateDeliveredList (std::string);
  void RemoveDeliveredPackets (Ptr<MyBloom_filter>);
  void GeneratePacket();
  void PacketInjector();
  void RemoveOldest ();
public:
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

  bool m_running;
  bool m_changed;
  bool m_degraded;//indicate if we are a degraded node (only using decoded packet in forwarding)
  uint32_t m_rcvCounter;
  uint16_t counter;
  uint16_t m_myNodeId;
  uint16_t m_myNeighborhoodSize;
  Ipv4Address m_myNCAppIp;
  std::deque<Ptr<NetworkCodedDatagram> > m_buffer;
  // List containing decoded packets
  std::vector<Ptr<DecodedPacketStorage> > m_decodedBuf;
  std::map<std::string, Ptr<DecodedPacketStorage> > m_decodedList;
  // List containing packets to decode
  std::vector<Ptr<NetworkCodedDatagram> > m_decodingBuf;
  std::map<std::string, NCAttribute> m_varList;
  std::vector<Ptr<NCAttribute> > m_variableList; //List only used during the decoding for swapping columns
  //std::map<std::string, NCAttribute> m_decodedList;
  std::vector<string> m_deliveredList;


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
  uint32_t oldestDiscardedNum;
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
  uint32_t s_oldestDiscardedNum;
  uint32_t s_bytesTotal;
  double s_packetDelay;

  Experiment();
  Experiment(std::string name);
  bool CommandSetup (int argc, char **argv);
  Gnuplot2dDataset ApplicationSetup (const WifiHelper &wifi, const YansWifiPhyHelper &wifiPhy, const NqosWifiMacHelper &wifiMac, const YansWifiChannelHelper &wifiChannel);
};
