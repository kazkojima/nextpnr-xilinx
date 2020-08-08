/*
 *  nextpnr -- Next Generation Place and Route
 *
 *  Copyright (C) 2020  David Shah <dave@ds0.me>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "nextpnr.h"

NEXTPNR_NAMESPACE_BEGIN

namespace ChannelRouter {
enum ChannelDirection
{
    DIR_HORIZ,
    DIR_VERT,
    DIR_ANY
};
struct ChannelPIP
{
    ChannelPIP(int src_along, int dst_along, int dst_type)
            : src_along(src_along), dst_along(dst_along), dst_type(dst_type)
    {
    }
    // For DIR_HORIZ/DIR_VERT channels
    int src_along, dst_along;

    // For DIR_ANY channels
    int src_dx, src_dy, dst_dx, dst_dy;

    int dst_type;
    ChannelPIP(int src_dx, int src_dy, int dst_dx, int dst_dy, int dst_type)
            : src_dx(src_dx), src_dy(src_dy), dst_dx(dst_dx), dst_dy(dst_dy), dst_type(dst_type)
    {
    }
};
struct Channel
{
    std::string type_name;
    ChannelDirection dir;
    int width;
    float cost;
    std::vector<ChannelPIP> downhill;
};
struct ChannelNode
{
    ChannelNode() : x(-1), y(-1), type(-1){};
    ChannelNode(int x, int y, int type) : x(x), y(y), type(type){};
    int x, y, type;
};
struct ChannelGraph
{
    virtual int get_width() const = 0;
    virtual int get_height() const = 0;
    virtual std::vector<Channel> get_channels() const = 0;
    virtual bool is_global_net(const NetInfo *net) const = 0;
    virtual ChannelNode get_source_node(const NetInfo *net) const = 0;
    virtual ChannelNode get_sink_node(const NetInfo *net, size_t usr_i) const = 0;
    virtual ~ChannelGraph(){};
};

struct ChannelRouterCfg
{
    int bb_margin_x = 3, bb_margin_y = 3;
    // Togo cost estimates
    int togo_cost_dx = 7, togo_cost_dy = 7, togo_cost_adder = 12;
    float bias_cost_factor = 0.5;
    float estimate_weight = 1.5;
    // Starting current and historical congestion cost factor
    float init_curr_cong_weight = 0.5, hist_cong_weight = 1.0;
    // Current congestion cost multiplier
    float curr_cong_mult = 2.0;

    bool perf_profile = true;
};

void run_channelrouter(Context *ctx, ChannelGraph *g, const ChannelRouterCfg &cfg);

}; // namespace ChannelRouter

NEXTPNR_NAMESPACE_END