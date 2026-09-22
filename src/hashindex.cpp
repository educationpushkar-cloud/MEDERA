#include "hashindex.h"

static std::string makeKey(const std::string &city, const std::string &area)
{
    return city + "|" + area;
}

HospitalIndex buildHospitalIndex(const std::vector<Hospital> &hospitals)
{
    HospitalIndex index;
    for (const auto &h : hospitals)
    {
        index[makeKey(h.city, h.area)].push_back(h);
    }
    return index;
}

std::vector<Hospital> lookupHospitals(const HospitalIndex &index,
                                       const std::string &city,
                                       const std::string &area)
{
    auto it = index.find(makeKey(city, area));
    if (it == index.end())
    {
        return {};
    }
    return it->second;
}
