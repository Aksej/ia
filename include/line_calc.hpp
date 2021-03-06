#ifndef LINE_CALC_HPP
#define LINE_CALC_HPP

#include <vector>

#include "rl_utils.hpp"

namespace line_calc
{

void init();

void calc_new_line(const P& origin, const P& tgt,
                   const bool SHOULD_STOP_AT_TARGET,
                   const int CHEB_TRAVEL_LIMIT,
                   const bool ALLOW_OUTSIDE_MAP,
                   std::vector<P>& line_ref);

const std::vector<P>* fov_delta_line(const P& delta,
                                     const double& MAX_DIST_ABS);

} //line_calc

#endif
