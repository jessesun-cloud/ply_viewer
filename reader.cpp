#include "scene.h"
#include <cmath>
#ifdef SUPPORT_PTX
#include <ptxreader.hpp>
#endif
#include <limits>
#ifdef _WIN32
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

namespace fs = std::experimental::filesystem;

void Scene:: _loadPTX(const QString& FilePath)
{
#ifdef SUPPORT_PTX
  PtxReader reader(FilePath.toStdString().c_str());
  vector< shared_ptr<ScanNode>> nodes;
  reader.LoadScan(1, nodes);
  _pointsData.clear();
  _pointsCount = 0;
  size_t size = 0;
  for (int i = 0; i < nodes.size(); i++)
  {
    vector<float>& pts = nodes[i]->Position();
    size_t np = pts.size() / 3;
    size = _pointsCount;
    _pointsCount += np;
    _pointsData.resize(_pointsCount * 4);
    for (int j = 0; j < np; j++)
    {
      memcpy(_pointsData.data() + (size + j) * 4, pts.data() + j * 3, 12);
    }
    nodes[i]->GetBox((double*)&_pointsBoundMin, (double*)&_pointsBoundMax);
  }
  // update bounds
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
    _loadPTX(FilePath);
  }
#endif
  else
  { throw std::runtime_error("not support"); }
}