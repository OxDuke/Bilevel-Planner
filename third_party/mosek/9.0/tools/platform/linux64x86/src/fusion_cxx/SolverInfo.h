#include <mosek.h>
#include <string>
#include <vector>
#include <algorithm>
namespace mosek {
namespace fusion {
class SolverInfo
{
  private:  static std::vector<std::string> dinfnames;
  static std::vector<std::string> iinfnames;
  static std::vector<std::string> liinfnames;
  static MSKdinfiteme   dinfsyms[];
  static MSKiinfiteme   iinfsyms[];
  static MSKliinfiteme liinfsyms[];
  public:  static bool getdouinf( const std::string & infname, MSKdinfiteme  & key );
  static bool getintinf( const std::string & infname, MSKiinfiteme  & key );
  static bool getlintinf(const std::string & infname, MSKliinfiteme & key );
}; /* class SolverInfo */
} /* namespace fusion */
} /* namespace mosek */
