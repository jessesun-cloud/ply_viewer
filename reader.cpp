#include "scene.h"
#include <cmath>
#ifdef SUPPORT_PTX
#include <ptxreader.hpp>
#endif
#ifdef SUPPORT_E57
#include <e57reader.hpp>
#endif
#include <limits>
#ifdef _WIN32
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

namespace fs = std::experimental::filesystem;

void AddNode(vector< shared_ptr<ScanNode>> &nodes,
  QVector<float>& _pointsData,
  size_t& _pointsCount,
  QVector3D& _pointsBoundMin,
  QVector3D& _pointsBoundMax)
{
  size_t size = 0;
  for (int i = 0; i < nodes.size(); i++)
  {
    vector<float>& pts = nodes[i]->Position();
    size_t np = pts.size() / 3;
    size = _pointsCount;
    _pointsCount += np;
    _pointsData.resize((int)_pointsCount * 4);
    for (int j = 0; j < np; j++)
    {
      memcpy(_pointsData.data() + (size + j) * 4, pts.data() + j * 3, 12);
      _pointsData[((int)size + j) * 4 + 3] = i;
    }
    double mi[3], mx[3];
    nodes[i]->GetBox(mi, mx);
    if (i == 0)
    {
      for (int i = 0; i < 3; i++)
      {
        _pointsBoundMin[i] = mi[i];
        _pointsBoundMax[i] = mx[i];
      }
    }
    else
    {
      for (int i = 0; i < 3; i++)
      {
        _pointsBoundMax[i] = std::max((float)mx[i], _pointsBoundMax[i]);
        _pointsBoundMin[i] = std::min((float)mi[i], _pointsBoundMin[i]);
      }
    }
  }
}

void Scene:: _loadPTX(const std::string& FilePath)
{
#ifdef SUPPORT_PTX
  PtxReader reader(FilePath.c_str());
  vector< shared_ptr<ScanNode>> nodes;
  reader.LoadScan(1, nodes);
  _pointsData.clear();
  _pointsCount = 0;
  AddNode(nodes, _pointsData, _pointsCount,
     _pointsBoundMin, _pointsBoundMax);
#endif
}


bool LoadScan(E57Reader& reader, vector< shared_ptr<ScanNode>>& rNodes)
{
  while (true)
  {
    int columns, rows;
    if (reader.MoveNextScan() == false) { break; }
    if (false == reader.GetSize(columns, rows))
    {
      return false;
    }
    ScanNode* pNode = new ScanNode;
    double scannerMatrix3x4[12];
    double ucs[16];
    reader.GetHeader(scannerMatrix3x4, ucs);
    pNode->SetName(reader.GetScanName().c_str());
    pNode->SetMatrix(scannerMatrix3x4, ucs);
    auto ExportLambda = [&](int np, float * x,
                            float * pIntensity,
                            int* rgbColor)->void
    {
      pNode->Add(np, x, pIntensity, rgbColor);
    };
    reader.ReadPoints(ExportLambda);
    shared_ptr<ScanNode> scan(pNode);
    rNodes.push_back(scan);
  }
  return true;
}

void Scene::_loadE57(const std::string& FilePath)
{
#ifdef SUPPORT_E57
  E57Reader reader;
  if (reader.Open(FilePath.c_str()) == false)
  {
    return;
  }
  vector< shared_ptr<ScanNode>> nodes;
  LoadScan(reader,  nodes);
  _pointsData.clear();
  _pointsCount = 0;
  AddNode(nodes, _pointsData, _pointsCount,
    _pointsBoundMin, _pointsBoundMax);
#endif
}

void Scene::_load(const QString& FilePath)
{
  fs::path filename(FilePath.toStdString().c_str());
  std::string ext = filename.extension().u8string();
  const bool isply = ext == ".ply";
  if (isply)
  {
    _loadPLY(FilePath);
  }
#ifdef SUPPORT_PTX
  else if (ext == ".ptx")
  {
    _loadPTX(FilePath.toStdString());
  }
#endif
#ifdef SUPPORT_E57
  else if (ext == ".e57")
  {
    _loadE57(FilePath.toStdString());
  }
#endif
  else
  { throw std::runtime_error("not support"); }
}