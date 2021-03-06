/* **********************************************************
 * Copyright (c) 2018 Google, Inc.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Google, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL VMWARE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef _OPCODE_MIX_H_
#define _OPCODE_MIX_H_ 1

#include <string>
#include <unordered_map>

#include "analysis_tool.h"
#include "raw2trace.h"
#include "raw2trace_directory.h"

class opcode_mix_t : public analysis_tool_t {
public:
    opcode_mix_t(const std::string &module_file_path, unsigned int verbose);
    virtual ~opcode_mix_t()
    {
    }
    std::string
    initialize() override;
    bool
    process_memref(const memref_t &memref) override;
    bool
    print_results() override;

protected:
    void *dcontext;
    std::string module_file_path;
    std::unique_ptr<module_mapper_t> module_mapper;
    // We reference directory.modfile_bytes throughout operation, so its lifetime
    // must match ours.
    raw2trace_directory_t directory;
    unsigned int knob_verbose;
    int_least64_t instr_count;
    std::unordered_map<int, int_least64_t> opcode_counts;
    std::unordered_map<app_pc, int> opcode_cache;
    static const std::string TOOL_NAME;
};

#endif /* _OPCODE_MIX_H_ */
