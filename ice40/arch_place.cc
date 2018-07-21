/*
 *  nextpnr -- Next Generation Place and Route
 *
 *  Copyright (C) 2018  Clifford Wolf <clifford@symbioticeda.com>
 *  Copyright (C) 2018  David Shah <david@symbioticeda.com>
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

#include "cells.h"
#include "nextpnr.h"
#include "util.h"

NEXTPNR_NAMESPACE_BEGIN

bool Arch::logicCellsCompatible(const std::vector<const CellInfo *> &cells) const
{
    bool dffs_exist = false, dffs_neg = false;
    const NetInfo *cen = nullptr, *clk = nullptr, *sr = nullptr;
    int locals_count = 0;

    for (auto cell : cells) {
        NPNR_ASSERT(cell->belType == TYPE_ICESTORM_LC);
        if (cell->lcInfo.dffEnable) {
            if (!dffs_exist) {
                dffs_exist = true;
                cen = cell->lcInfo.cen;
                clk = cell->lcInfo.clk;
                sr = cell->lcInfo.sr;

                if (cen != nullptr && !cen->is_global)
                    locals_count++;
                if (clk != nullptr && !clk->is_global)
                    locals_count++;
                if (sr != nullptr && !sr->is_global)
                    locals_count++;

                if (cell->lcInfo.negClk) {
                    dffs_neg = true;
                }
            } else {
                if (cen != cell->lcInfo.cen)
                    return false;
                if (clk != cell->lcInfo.clk)
                    return false;
                if (sr != cell->lcInfo.sr)
                    return false;
                if (dffs_neg != cell->lcInfo.negClk)
                    return false;
            }
        }

        locals_count += cell->lcInfo.inputCount;
    }

    return locals_count <= 32;
}

bool Arch::isBelLocationValid(BelId bel) const
{
    if (getBelType(bel) == TYPE_ICESTORM_LC) {
        std::vector<const CellInfo *> bel_cells;
        for (auto bel_other : getBelsAtSameTile(bel)) {
            IdString cell_other = getBoundBelCell(bel_other);
            if (cell_other != IdString()) {
                const CellInfo *ci_other = cells.at(cell_other).get();
                bel_cells.push_back(ci_other);
            }
        }
        return logicCellsCompatible(bel_cells);
    } else {
        IdString cellId = getBoundBelCell(bel);
        if (cellId == IdString())
            return true;
        else
            return isValidBelForCell(cells.at(cellId).get(), bel);
    }
}

bool Arch::isValidBelForCell(CellInfo *cell, BelId bel) const
{
    if (cell->type == id_icestorm_lc) {
        NPNR_ASSERT(getBelType(bel) == TYPE_ICESTORM_LC);

        std::vector<const CellInfo *> bel_cells;

        for (auto bel_other : getBelsAtSameTile(bel)) {
            IdString cell_other = getBoundBelCell(bel_other);
            if (cell_other != IdString() && bel_other != bel) {
                const CellInfo *ci_other = cells.at(cell_other).get();
                bel_cells.push_back(ci_other);
            }
        }

        bel_cells.push_back(cell);
        return logicCellsCompatible(bel_cells);
    } else if (cell->type == id_sb_io) {
        return getBelPackagePin(bel) != "";
    } else if (cell->type == id_sb_gb) {
        NPNR_ASSERT(cell->ports.at(id_glb_buf_out).net != nullptr);
        const NetInfo *net = cell->ports.at(id_glb_buf_out).net;
        IdString glb_net = getWireName(getWireBelPin(bel, PIN_GLOBAL_BUFFER_OUTPUT));
        int glb_id = std::stoi(std::string("") + glb_net.str(this).back());
        if (net->is_reset && net->is_enable)
            return false;
        else if (net->is_reset)
            return (glb_id % 2) == 0;
        else if (net->is_enable)
            return (glb_id % 2) == 1;
        else
            return true;
    } else {
        // TODO: IO cell clock checks
        return true;
    }
}

NEXTPNR_NAMESPACE_END
