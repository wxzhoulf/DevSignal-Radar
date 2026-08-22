#include "techpulse/publish/report.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
namespace techpulse::publish { bool write_daily_report(const std::filesystem::path& root,const std::string& date,const std::vector<model::ScoredItem>& items,const std::vector<std::string>& errors){std::filesystem::create_directories(root/"reports/daily");std::ofstream out(root/"reports/daily"/(date+".md"));if(!out)return false;out<<"# DevSignal Radar · "<<date<<"\n\n"<<"候选信号："<<items.size()<<"；失败来源："<<errors.size()<<"\n\n## Top Signals\n\n";for(const auto&i:items){if(i.excluded)continue;out<<"### ["<<std::fixed<<std::setprecision(1)<<i.score.total<<"] "<<i.item.title<<"\n\n"<<"来源："<<i.item.source_type<<"  \\n"<<"[原文链接]("<<i.item.url<<")  \\n";for(const auto&r:i.reasons)out<<"- "<<r<<"\n";out<<"\n";}if(!errors.empty()){out<<"## 来源状态\n\n";for(const auto&e:errors)out<<"- "<<e<<"\n";}return true;} }
